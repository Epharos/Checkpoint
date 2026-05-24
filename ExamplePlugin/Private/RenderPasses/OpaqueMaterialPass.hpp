#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <Common/Data/Rectangle.hpp>
#include <Common/Data/Viewport.hpp>
#include <Common/ShaderReflection.hpp>
#include <ECS/ECS.hpp>

#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

#include <Rendering/FrameGraph/FrameGraphBuilder.hpp>
#include <Rendering/FrameGraph/Renderpass.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Rendering/Camera.hpp>
#include <Rendering/IShaderProvider.hpp>

#include "../Assets/Material.hpp"
#include "../Assets/MaterialInstance.hpp"
#include "../Components/Components.hpp"
#include "../ExamplePluginDir.hpp"

namespace cp
{
    struct OpaqueMaterialPassData
    {
        struct CameraDataGpu
        {
            glm::mat4 viewProjection{1.0f};
        };

        struct InstanceDataGpu
        {
            glm::mat4 model{1.0f};
        };

        struct PhongVertex
        {
            float position[3]{};
            float normal[3]{};
            float color[3]{};
            float uv[2]{};
        };

        struct MeshGpuResources
        {
            std::shared_ptr<IBuffer> vertexBuffer;
            uint32_t vertexCount = 0;
        };

        FramegraphResourceHandle* finalRendering = nullptr;
        FramegraphResourceHandle* depthBuffer = nullptr;
        Extent2D<uint32_t> renderExtent{};
        std::shared_ptr<ecs::World> ecsWorld;
        RenderingHardwareInterface* rhi = nullptr;
        IShaderProvider* shaderProvider = nullptr;

        // Default pipeline for meshes without a material instance (2 bindings: camera + instances).
        std::shared_ptr<IShaderModule> shaderModule;
        std::shared_ptr<IDescriptorSetLayout> descriptorSetLayout;
        std::shared_ptr<IPipelineLayout> pipelineLayout;
        std::shared_ptr<IPipeline> pipeline;

        std::unordered_map<std::string, MeshGpuResources> meshCache;

        std::array<std::vector<std::shared_ptr<IBuffer>>, 3> transientFrameBuffers{};
        std::array<std::vector<std::shared_ptr<IDescriptorSet>>, 3> transientFrameDescriptorSets{};
        size_t transientFrameIndex = 0;
    };

    class OpaqueMaterialPass : public RenderPass<OpaqueMaterialPassData>, public IConfigurableRenderPass
    {
    public:
        OpaqueMaterialPass()
        {
            name = "OpaqueMaterialPass";
        }

        void Configure(const RenderPassInitContext& _context) override
        {
            data.renderExtent = _context.renderExtent;
            data.rhi = &_context.rhi;
            data.shaderProvider = _context.shaderProvider;

            if (!_context.shaderProvider)
            {
                return;
            }

            const ShaderProviderResult shader = _context.shaderProvider->GetShader(
                "Phong", GetExamplePluginDir() / "Shaders");

            if (!shader.success || shader.binary.empty())
            {
                return;
            }

            const ShaderModuleInfo shaderModuleInfo {
                .stages = ShaderStage::Vertex | ShaderStage::Fragment,
                .bytecode = ShaderBytecode{
                    .format = shader.format,
                    .data = shader.binary.data(),
                    .sizeBytes = shader.binary.size()
                }
            };
            data.shaderModule = _context.rhi.GetDevice().CreateShaderModule(shaderModuleInfo);

            const DescriptorSetLayoutInfo descriptorSetLayoutInfo {
                .bindings = {
                    DescriptorBinding{
                        .binding = 0,
                        .type = DescriptorType::UniformBuffer,
                        .count = 1,
                        .visibility = ShaderStage::Vertex
                    },
                    DescriptorBinding{
                        .binding = 1,
                        .type = DescriptorType::StorageBuffer,
                        .count = 1,
                        .visibility = ShaderStage::Vertex
                    }
                }
            };
            data.descriptorSetLayout = _context.rhi.GetDevice().CreateDescriptorSetLayout(descriptorSetLayoutInfo);

            const PipelineLayoutInfo pipelineLayoutInfo {
                .setLayouts = { data.descriptorSetLayout }
            };
            data.pipelineLayout = _context.rhi.GetDevice().CreatePipelineLayout(pipelineLayoutInfo);

            const GraphicsPipelineInfo graphicsPipelineInfo {
                .layout = data.pipelineLayout,
                .shaderModule = data.shaderModule,
                .stageMains = {
                    { ShaderStage::Vertex, "VSMain" },
                    { ShaderStage::Fragment, "FSMain" },
                },
                .vertexInput = GetPhongVertexInput(),
                .topology = PrimitiveTopology::TriangleList,
                .rasterization = RasterizationState{},
                .depthStencil = DepthStencilState{},
                .blendAttachments = {},
                .colorAttachmentFormats = { Format::R8G8B8A8_UNORM },
                .depthStencilFormat = Format::D32_FLOAT
            };
            data.pipeline = _context.rhi.GetDevice().CreateGraphicsPipeline(graphicsPipelineInfo);
        }

        void DeclareResources(FrameGraphBuilder& _builder) override
        {
            const TextureInfo finalInfo{
                .type = TextureType::Texture2D,
                .extent = Extent3D<uint32_t>{ data.renderExtent.x(), data.renderExtent.y(), 1 },
                .mipLevels = 1,
                .arrayLayers = 1,
                .format = Format::R8G8B8A8_UNORM,
                .usage = TextureUsage::ColorAttachment | TextureUsage::Storage | TextureUsage::TransferSrc,
                .aspect = TextureAspect::Color,
            };
            data.finalRendering = _builder.CreateTexture("FinalRendering", finalInfo);

            const TextureInfo depthBufferInfo{
                .type = TextureType::Texture2D,
                .extent = Extent3D<uint32_t>{ data.renderExtent.x(), data.renderExtent.y(), 1 },
                .mipLevels = 1,
                .arrayLayers = 1,
                .format = Format::D32_FLOAT,
                .usage = TextureUsage::DepthStencilAttachment | TextureUsage::Sampled,
                .aspect = TextureAspect::Depth,
            };
            data.depthBuffer = _builder.CreateTexture("DepthBuffer", depthBufferInfo);
        }

        void Setup(FrameGraphBuilder& _builder) override
        {
            _builder.UseTexture(data.finalRendering, {
                .layout = TextureLayout::ColorAttachment,
                .stage = PipelineStage::ColorAttachment,
                .access = Access::ColorAttachmentWrite
            }, ResourceUsage::WriteOnly);

            _builder.SetTextureFinalState(data.finalRendering, {
                .layout = TextureLayout::TransferSrc,
                .stage = PipelineStage::Transfer,
                .access = Access::TransferRead
            });

            _builder.UseTexture(data.depthBuffer, {
                .layout = TextureLayout::DepthStencilAttachment,
                .stage = PipelineStage::EarlyDepth | PipelineStage::LateDepth,
                .access = Access::DepthStencilWrite | Access::DepthStencilRead
            }, ResourceUsage::ReadWrite);

            data.ecsWorld = _builder.GetCpuResource<ecs::World>("ECS.World");
        }

        void Execute(RenderPassExecutionContext& _context) override
        {
            ICommandBuffer& cmd = _context;
            ITexture* finalTexture = data.finalRendering->GetTexture();
            ITexture* depthBuffer = data.depthBuffer->GetTexture();

            const ColorAttachmentInfo colorAttachment{
                .texture = finalTexture,
                .loadOp = LoadOp::Clear,
                .storeOp = StoreOp::Store,
                .clearValue = Color(ColorRGBA8(10, 10, 10, 255))
            };
            const DepthStencilAttachmentInfo depthAttachment{
                .texture = depthBuffer,
                .depthLoadOp = LoadOp::Clear,
                .depthStoreOp = StoreOp::Store,
                .stencilLoadOp = LoadOp::Clear,
                .stencilStoreOp = StoreOp::Store,
                .clearValue = ClearDepthStencil(1.0f, 0)
            };
            const RenderingInfo renderingInfo{
                .extent = data.renderExtent,
                .layers = 1,
                .colorAttachments = { colorAttachment },
                .depthStencilAttachment = depthAttachment
            };

            cmd.SetViewport(Viewport{
                0.f, 0.f,
                static_cast<float>(data.renderExtent.x()),
                static_cast<float>(data.renderExtent.y())
            });
            cmd.SetScissor(Rectangle2D{
                Extent2D{ 0, 0 },
                data.renderExtent
            });
            cmd.BeginRendering(renderingInfo);

            if (!data.ecsWorld || data.rhi == nullptr)
            {
                cmd.EndRendering();
                return;
            }

            data.transientFrameIndex = (data.transientFrameIndex + 1) % data.transientFrameBuffers.size();
            auto& transientBuffers = data.transientFrameBuffers[data.transientFrameIndex];
            auto& transientDescriptorSets = data.transientFrameDescriptorSets[data.transientFrameIndex];
            transientBuffers.clear();
            transientDescriptorSets.clear();

            struct Batch
            {
                AssetHandle<Mesh> mesh;
                AssetHandle<MaterialInstance> materialInstance;
                std::vector<OpaqueMaterialPassData::InstanceDataGpu> instances;
            };

            std::unordered_map<std::string, Batch> batches;
            data.ecsWorld->RunSystem(
                ecs::ReadAccess<MeshRenderer, Transform>{},
                ecs::WriteAccess<>{},
                [&batches](const MeshRenderer& _mr, const Transform& _t)
                {
                    if (!_mr.visible || !_mr.meshId.IsValid())
                    {
                        return;
                    }

                    const std::string meshPath(_mr.meshId.GetAssetID());
                    if (meshPath.empty())
                    {
                        return;
                    }

                    std::string matInstPath;
                    if (_mr.materialInstance.IsValid())
                    {
                        matInstPath = _mr.materialInstance.GetAssetID();
                    }

                    const std::string batchKey = meshPath + "|" + matInstPath;
                    auto& batch = batches[batchKey];
                    if (!batch.mesh.IsValid())
                    {
                        batch.mesh = _mr.meshId;
                        batch.materialInstance = _mr.materialInstance;
                    }
                    batch.instances.push_back(OpaqueMaterialPassData::InstanceDataGpu {
                        .model = BuildModelMatrix(_t)
                    });
                }
            );

            if (batches.empty())
            {
                cmd.EndRendering();
                return;
            }

            const cp::ResolvedCameraData cameraData = ResolveCamera(_context);
            if (!cameraData.isValid)
            {
                cmd.EndRendering();
                return;
            }

            const std::shared_ptr<IBuffer> cameraBuffer =
                CreateCpuVisibleBuffer(sizeof(OpaqueMaterialPassData::CameraDataGpu), BufferUsage::Uniform);
            const OpaqueMaterialPassData::CameraDataGpu cameraGpuData {
                .viewProjection = cameraData.viewProjection
            };

            if (!cameraBuffer || !UploadBytes(*cameraBuffer, &cameraGpuData, sizeof(cameraGpuData)))
            {
                cmd.EndRendering();
                return;
            }

            transientBuffers.push_back(cameraBuffer);

            for (auto& [batchKey, batch] : batches)
            {
                if (batch.instances.empty() || !batch.mesh.IsValid())
                {
                    continue;
                }

                IPipeline* activePipeline = data.pipeline.get();
                IDescriptorSetLayout* activeLayout = data.descriptorSetLayout.get();
                const MaterialInstance* activeMat = nullptr;
                const ShaderReflection* activeReflect = nullptr;

                if (batch.materialInstance.IsValid() && data.shaderProvider)
                {
                    MaterialInstance* inst = batch.materialInstance.Get();
                    if (inst && inst->material.IsValid())
                    {
                        Material* mat = inst->material.Get();
                        if (mat)
                        {
                            if (!mat->IsCompiled())
                            {
                                const MaterialCompileContext compileCtx {
                                    .rhi = *data.rhi,
                                    .shaderProvider = *data.shaderProvider,
                                    .searchRoot = GetExamplePluginDir(),
                                    .vertexInput = GetPhongVertexInput(),
                                    .colorAttachmentFormats = { Format::R8G8B8A8_UNORM },
                                    .depthStencilFormat = Format::D32_FLOAT
                                };

                                mat->Compile(compileCtx);
                            }

                            if (mat->IsCompiled())
                            {
                                if (inst->gpuDirty)
                                {
                                    inst->RebuildGpuBuffers(*data.rhi, mat->reflection);
                                }

                                activePipeline = mat->pipeline.get();
                                activeLayout = mat->descriptorSetLayout.get();
                                activeMat = inst;
                                activeReflect = &mat->reflection;
                            }
                        }
                    }
                }

                if (!activePipeline || !activeLayout)
                {
                    continue;
                }

                const std::string meshPath = std::string(batch.mesh.GetAssetID());
                auto* meshGpu = GetOrCreateMeshGpuResources(meshPath, batch.mesh);
                if (!meshGpu || !meshGpu->vertexBuffer || meshGpu->vertexCount == 0)
                {
                    continue;
                }

                const size_t instanceBytes = batch.instances.size() * sizeof(OpaqueMaterialPassData::InstanceDataGpu);
                const auto instanceBuffer = CreateCpuVisibleBuffer(instanceBytes, BufferUsage::Storage);

                if (!instanceBuffer || !UploadBytes(*instanceBuffer, batch.instances.data(), instanceBytes))
                {
                    continue;
                }

                transientBuffers.push_back(instanceBuffer);

                const auto descriptorSet = data.rhi->GetDevice().CreateDescriptorSet(*activeLayout);
                if (!descriptorSet)
                {
                    continue;
                }

                std::vector<DescriptorBufferBinding> bufferBindings;

                bufferBindings.push_back(DescriptorBufferBinding {
                    .binding = 0,
                    .buffer = cameraBuffer.get(),
                    .offsetBytes = 0,
                    .rangeBytes = sizeof(OpaqueMaterialPassData::CameraDataGpu)
                });

                bufferBindings.push_back(DescriptorBufferBinding {
                    .binding = 1,
                    .buffer = instanceBuffer.get(),
                    .offsetBytes = 0,
                    .rangeBytes = instanceBytes
                });

                if (activeMat)
                {
                    for (const GpuParamBuffer& gpuBuf : activeMat->gpuParamBuffers)
                    {
                        if (!gpuBuf.buffer)
                        {
                            continue;
                        }

                        bufferBindings.push_back(DescriptorBufferBinding {
                            .binding = gpuBuf.binding,
                            .buffer = gpuBuf.buffer.get(),
                            .offsetBytes = 0,
                            .rangeBytes = gpuBuf.sizeBytes
                        });
                    }
                }

                std::vector<DescriptorTextureBinding> textureBindings;
                if (activeMat && activeReflect)
                {
                    for (const BindingReflection& b : activeReflect->bindings)
                    {
                        if (b.binding <= 1)
                        {
                            continue;
                        }

                        const bool isTexture =
                            b.kind == BindingKind::Texture1D ||
                            b.kind == BindingKind::Texture1DArray ||
                            b.kind == BindingKind::Texture2D ||
                            b.kind == BindingKind::Texture2DArray ||
                            b.kind == BindingKind::Texture2DMS ||
                            b.kind == BindingKind::Texture2DMSArray ||
                            b.kind == BindingKind::Texture3D ||
                            b.kind == BindingKind::TextureCube ||
                            b.kind == BindingKind::TextureCubeArray;

                        if (!isTexture)
                        {
                            continue;
                        }

                        const auto it = activeMat->parameters.find(b.name);
                        if (it == activeMat->parameters.end())
                        {
                            continue;
                        }

                        const auto* handle = std::get_if<AssetHandle<ITexture>>(&it->second);
                        if (!handle || !handle->IsValid())
                        {
                            continue;
                        }

                        textureBindings.push_back(DescriptorTextureBinding {
                            .binding = b.binding,
                            .texture = handle->Get(),
                            .layout = TextureLayout::ShaderReadOnly,
                            .sampler = nullptr
                        });
                    }
                }

                descriptorSet->UpdateBuffers(bufferBindings);

                if (!textureBindings.empty())
                {
                    descriptorSet->UpdateTextures(textureBindings);
                }
                transientDescriptorSets.push_back(descriptorSet);

                cmd.BindPipeline(*activePipeline);
                cmd.BindDescriptorSet(0, *descriptorSet);
                cmd.BindVertexBuffer(0, *meshGpu->vertexBuffer);
                cmd.Draw(
                    meshGpu->vertexCount,
                    static_cast<uint32_t>(batch.instances.size()),
                    0, 0
                );
            }

            cmd.EndRendering();
        }

        void OnReset() override
        {
            data.finalRendering = nullptr;
            data.ecsWorld.reset();
            data.meshCache.clear();

            for (auto& buffers : data.transientFrameBuffers)
            {
                buffers.clear();
            }

            for (auto& descriptorSets : data.transientFrameDescriptorSets)
            {
                descriptorSets.clear();
            }
        }

    private:
        static VertexInputState GetPhongVertexInput()
        {
            using V = OpaqueMaterialPassData::PhongVertex;
            return VertexInputState {
                .bindings = {
                    VertexBindingInfo {
                        .binding = 0,
                        .strideBytes = static_cast<uint32_t>(sizeof(V)),
                        .inputRate = VertexInputRate::PerVertex
                    }
                },
                .attributes = {
                    VertexAttributeInfo {
                        .location = 0,
                        .binding = 0,
                        .format = VertexFormat::R32G32B32_FLOAT,
                        .offsetBytes = static_cast<uint32_t>(offsetof(V, position))
                    },
                    VertexAttributeInfo {
                        .location = 1,
                        .binding = 0,
                        .format = VertexFormat::R32G32B32_FLOAT,
                        .offsetBytes = static_cast<uint32_t>(offsetof(V, normal))
                    },
                    VertexAttributeInfo {
                        .location = 2,
                        .binding = 0,
                        .format = VertexFormat::R32G32B32_FLOAT,
                        .offsetBytes = static_cast<uint32_t>(offsetof(V, color))
                    },
                    VertexAttributeInfo {
                        .location = 3,
                        .binding = 0,
                        .format = VertexFormat::R32G32_FLOAT,
                        .offsetBytes = static_cast<uint32_t>(offsetof(V, uv))
                    }
                }
            };
        }

        [[nodiscard]] cp::ResolvedCameraData ResolveCamera(
            const cp::RenderPassExecutionContext& _context
        ) const
        {
            if (_context.environment == cp::RendererEnvironment::Editor)
            {
                return _context.camera;
            }

            if (!data.ecsWorld)
            {
                return {};
            }

            Camera camComponent{};
            Transform camTransform{};
            bool found = false;

            data.ecsWorld->RunSystem(
                ecs::ReadAccess<Camera, Transform>{},
                ecs::WriteAccess<>{},
                [&](const Camera& _cam, const Transform& _t)
                {
                    if (found || !_cam.enabled || !_cam.isPrimary)
                    {
                        return;
                    }

                    camComponent = _cam;
                    camTransform = _t;
                    found = true;
                }
            );

            if (!found)
            {
                return {};
            }

            const float aspect =
                static_cast<float>(data.renderExtent.x()) /
                static_cast<float>(std::max(1u, data.renderExtent.y()));

            cp::ResolvedCameraData result;
            result.view = BuildViewMatrix(camTransform);
            result.projection = BuildPerspectiveMatrix(
                camComponent.fovYDegrees, aspect,
                camComponent.nearPlane,   camComponent.farPlane);
            result.viewProjection = result.projection * result.view;
            result.worldPosition = { camTransform.x, camTransform.y, camTransform.z };
            result.nearPlane = camComponent.nearPlane;
            result.farPlane = camComponent.farPlane;
            result.fovYDegrees = camComponent.fovYDegrees;
            result.isValid = true;
            return result;
        }

        static glm::mat4 BuildModelMatrix(const Transform& _transform)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, { _transform.x, _transform.y, _transform.z });
            model = glm::rotate(model, glm::radians(_transform.roll),  { 0.f, 0.f, 1.f });
            model = glm::rotate(model, glm::radians(_transform.yaw),   { 0.f, 1.f, 0.f });
            model = glm::rotate(model, glm::radians(_transform.pitch), { 1.f, 0.f, 0.f });
            model = glm::scale(model, { _transform.scaleX, _transform.scaleY, _transform.scaleZ });
            return model;
        }

        static glm::mat4 BuildViewMatrix(const Transform& _transform)
        {
            glm::mat4 world(1.0f);
            world = glm::translate(world, { _transform.x, _transform.y, _transform.z });
            world = glm::rotate(world, glm::radians(_transform.roll),  { 0.f, 0.f, 1.f });
            world = glm::rotate(world, glm::radians(_transform.yaw),   { 0.f, 1.f, 0.f });
            world = glm::rotate(world, glm::radians(_transform.pitch), { 1.f, 0.f, 0.f });
            return glm::inverse(world);
        }

        static glm::mat4 BuildPerspectiveMatrix(float _fovY, float _aspect, float _nearP, float _farP)
        {
            const float aspect = _aspect > 1e-6f ? _aspect : 1.0f;
            const float near = std::max(0.001f,  _nearP);
            const float far = std::max(near + 0.001f, _farP);
            const float fov = std::max(1.0f,    _fovY);

            glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, near, far);
            projection[1][1] *= -1.0f;
            return projection;
        }

        static float Clamp01(float v) { return v < 0.f ? 0.f : v > 1.f ? 1.f : v; }

        static OpaqueMaterialPassData::PhongVertex ConvertVertex(const MeshVertex& _vertex)
        {
            OpaqueMaterialPassData::PhongVertex out{};
            out.position[0] = _vertex.position[0];
            out.position[1] = _vertex.position[1];
            out.position[2] = _vertex.position[2];

            out.normal[0] = _vertex.normal[0];
            out.normal[1] = _vertex.normal[1];
            out.normal[2] = _vertex.normal[2];

            out.color[0]    = Clamp01(std::fabs(_vertex.normal[0]));
            out.color[1]    = Clamp01(std::fabs(_vertex.normal[1]));
            out.color[2]    = Clamp01(std::fabs(_vertex.normal[2]));

            out.uv[0] = _vertex.texCoord[0];
            out.uv[1] = _vertex.texCoord[1];

            return out;
        }

        static std::vector<OpaqueMaterialPassData::PhongVertex> BuildDrawVertices(const Mesh& mesh)
        {
            const auto& src = mesh.GetVertices();
            const auto& indices = mesh.GetIndices();

            std::vector<OpaqueMaterialPassData::PhongVertex> vertex;
            vertex.reserve(indices.size());

            for (uint32_t idx : indices)
            {
                if (idx < src.size())
                {
                    vertex.push_back(ConvertVertex(src[idx]));
                }
            }

            return vertex;
        }

        std::shared_ptr<IBuffer> CreateCpuVisibleBuffer(size_t sizeBytes, BufferUsage usage) const
        {
            if (sizeBytes == 0)
            {
                return nullptr;
            }

            return data.rhi->CreateBuffer(BufferInfo {
                .sizeBytes = sizeBytes,
                .usage = usage,
                .cpuVisible = true
            });
        }

        static bool UploadBytes(IBuffer& buf, const void* src, size_t size)
        {
            if (!src || size == 0)
            {
                return false;
            }

            if (void* mapped = buf.Map())
            {
                std::memcpy(mapped, src, size);
                buf.Unmap();
                return true;
            }

            return false;
        }


        OpaqueMaterialPassData::MeshGpuResources* GetOrCreateMeshGpuResources(
            const std::string& meshPath,
            const AssetHandle<Mesh>& meshHandle
        )
        {
            const auto it = data.meshCache.find(meshPath);
            if (it != data.meshCache.end())
            {
                return &it->second;
            }

            const Mesh* mesh = meshHandle.Get();
            if (!mesh)
            {
                return nullptr;
            }

            auto drawVertex = BuildDrawVertices(*mesh);
            if (drawVertex.empty())
            {
                return nullptr;
            }

            const size_t vertexBytes = drawVertex.size() * sizeof(OpaqueMaterialPassData::PhongVertex);
            auto vb = CreateCpuVisibleBuffer(vertexBytes, BufferUsage::Vertex);
            if (!vb || !UploadBytes(*vb, drawVertex.data(), vertexBytes))
            {
                return nullptr;
            }

            auto [ins, _] = data.meshCache.insert_or_assign(
                meshPath,
                OpaqueMaterialPassData::MeshGpuResources {
                    .vertexBuffer = std::move(vb),
                    .vertexCount = static_cast<uint32_t>(drawVertex.size())
                });

            return &ins->second;
        }
    };
}
