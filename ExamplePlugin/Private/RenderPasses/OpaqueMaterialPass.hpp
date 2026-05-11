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
#include <Common/IO/FileHelper.hpp>

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

#include "../Components/Components.hpp"

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

            const auto shaderPath = FindFileInParentTree("Phong.spv");
            CP_ASSERT_MSG(!shaderPath.empty(), "Could not find Phong.spv from current working directory hierarchy");
            const std::vector<uint8_t> shaderBytecode = LoadBinaryFile(shaderPath);

            const ShaderModuleInfo shaderModuleInfo
            {
                .stages = ShaderStage::Vertex | ShaderStage::Fragment,
                .bytecode = ShaderBytecode{
                    .format = ShaderBinaryFormat::SpirV,
                    .data = shaderBytecode.data(),
                    .sizeBytes = shaderBytecode.size()
                }
            };
            data.shaderModule = _context.rhi.GetDevice().CreateShaderModule(shaderModuleInfo);

            const DescriptorSetLayoutInfo descriptorSetLayoutInfo
            {
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

            const PipelineLayoutInfo pipelineLayoutInfo{
                .setLayouts = { data.descriptorSetLayout }
            };
            data.pipelineLayout = _context.rhi.GetDevice().CreatePipelineLayout(pipelineLayoutInfo);

            const GraphicsPipelineInfo graphicsPipelineInfo
            {
                .layout = data.pipelineLayout,
                .shaderModule = data.shaderModule,
                .stageMains = {
                    { ShaderStage::Vertex, "VSMain" },
                    { ShaderStage::Fragment, "FSMain" },
                },
                .vertexInput = VertexInputState{
                    .bindings = {
                        VertexBindingInfo{
                            .binding = 0,
                            .strideBytes = static_cast<uint32_t>(sizeof(OpaqueMaterialPassData::PhongVertex)),
                            .inputRate = VertexInputRate::PerVertex
                        }
                    },
                    .attributes = {
                        VertexAttributeInfo{
                            .location = 0,
                            .binding = 0,
                            .format = VertexFormat::R32G32B32_FLOAT,
                            .offsetBytes = static_cast<uint32_t>(offsetof(OpaqueMaterialPassData::PhongVertex, position))
                        },
                        VertexAttributeInfo{
                            .location = 1,
                            .binding = 0,
                            .format = VertexFormat::R32G32B32_FLOAT,
                            .offsetBytes = static_cast<uint32_t>(offsetof(OpaqueMaterialPassData::PhongVertex, normal))
                        },
                        VertexAttributeInfo{
                            .location = 2,
                            .binding = 0,
                            .format = VertexFormat::R32G32B32_FLOAT,
                            .offsetBytes = static_cast<uint32_t>(offsetof(OpaqueMaterialPassData::PhongVertex, color))
                        }
                    }
                },
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
                .usage = TextureUsage::DepthStencilAttachment,
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

            const DepthStencilAttachmentInfo depthAttachment {
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
                0.f,
                0.f,
                static_cast<float>(data.renderExtent.x()),
                static_cast<float>(data.renderExtent.y())
            });
            cmd.SetScissor(Rectangle2D{
                Extent2D{ 0, 0 },
                data.renderExtent
            });
            cmd.BeginRendering(renderingInfo);

            if (!data.ecsWorld || !data.pipeline || !data.descriptorSetLayout || data.rhi == nullptr)
            {
                cmd.EndRendering();
                return;
            }

            data.transientFrameIndex = (data.transientFrameIndex + 1) % data.transientFrameBuffers.size();
            std::vector<std::shared_ptr<IBuffer>>& transientBuffers = data.transientFrameBuffers[data.transientFrameIndex];
            std::vector<std::shared_ptr<IDescriptorSet>>& transientDescriptorSets = data.transientFrameDescriptorSets[data.transientFrameIndex];
            transientBuffers.clear();
            transientDescriptorSets.clear();

            struct Batch
            {
                AssetHandle<Mesh> mesh;
                std::vector<OpaqueMaterialPassData::InstanceDataGpu> instances;
            };

            std::unordered_map<std::string, Batch> batches;
            data.ecsWorld->RunSystem(
                ecs::ReadAccess<MeshRenderer, Transform>{},
                ecs::WriteAccess<>{},
                [&batches](const MeshRenderer& _meshRenderer, const Transform& _transform)
                {
                    if (!_meshRenderer.visible || !_meshRenderer.meshId.IsValid())
                    {
                        return;
                    }

                    const std::string meshPath(_meshRenderer.meshId.GetAssetID());
                    if (meshPath.empty())
                    {
                        return;
                    }

                    auto& batch = batches[meshPath];
                    if (!batch.mesh.IsValid())
                    {
                        batch.mesh = _meshRenderer.meshId;
                    }

                    batch.instances.push_back(OpaqueMaterialPassData::InstanceDataGpu{
                        .model = BuildModelMatrix(_transform)
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

            const std::shared_ptr<IBuffer> cameraBuffer = CreateCpuVisibleBuffer(
                sizeof(OpaqueMaterialPassData::CameraDataGpu),
                BufferUsage::Uniform
            );
            const OpaqueMaterialPassData::CameraDataGpu cameraGpuData {
                .viewProjection = cameraData.viewProjection
            };
            if (!cameraBuffer || !UploadBytes(*cameraBuffer, &cameraGpuData, sizeof(OpaqueMaterialPassData::CameraDataGpu)))
            {
                cmd.EndRendering();
                return;
            }
            transientBuffers.push_back(cameraBuffer);

            cmd.BindPipeline(*data.pipeline);

            for (auto& [meshPath, batch] : batches)
            {
                if (batch.instances.empty() || !batch.mesh.IsValid())
                {
                    continue;
                }

                OpaqueMaterialPassData::MeshGpuResources* meshGpu = GetOrCreateMeshGpuResources(meshPath, batch.mesh);
                if (meshGpu == nullptr || !meshGpu->vertexBuffer || meshGpu->vertexCount == 0)
                {
                    continue;
                }

                const size_t instanceBytes = batch.instances.size() * sizeof(OpaqueMaterialPassData::InstanceDataGpu);
                const std::shared_ptr<IBuffer> instanceBuffer = CreateCpuVisibleBuffer(instanceBytes, BufferUsage::Storage);
                if (!instanceBuffer || !UploadBytes(*instanceBuffer, batch.instances.data(), instanceBytes))
                {
                    continue;
                }
                transientBuffers.push_back(instanceBuffer);

                const std::shared_ptr<IDescriptorSet> descriptorSet = data.rhi->GetDevice().CreateDescriptorSet(*data.descriptorSetLayout);
                if (!descriptorSet)
                {
                    continue;
                }

                descriptorSet->UpdateBuffers({
                    DescriptorBufferBinding{
                        .binding = 0,
                        .buffer = cameraBuffer.get(),
                        .offsetBytes = 0,
                        .rangeBytes = sizeof(OpaqueMaterialPassData::CameraDataGpu)
                    },
                    DescriptorBufferBinding{
                        .binding = 1,
                        .buffer = instanceBuffer.get(),
                        .offsetBytes = 0,
                        .rangeBytes = instanceBytes
                    }
                });

                transientDescriptorSets.push_back(descriptorSet);

                cmd.BindDescriptorSet(0, *descriptorSet);
                cmd.BindVertexBuffer(0, *meshGpu->vertexBuffer);
                cmd.Draw(
                    meshGpu->vertexCount,
                    static_cast<uint32_t>(batch.instances.size()),
                    0,
                    0
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
        /**
         * @brief Resolve camera data for this frame.
         *
         * In Editor mode the pre-computed data from the context is used directly.
         * In Runtime / DevelopmentBuild modes the first enabled primary Camera
         * entity is searched in the ECS world.
         */
        [[nodiscard]] cp::ResolvedCameraData ResolveCamera(
            const cp::RenderPassExecutionContext& _context
        ) const
        {
            if (_context.environment == cp::RendererEnvironment::Editor)
                return _context.camera;

            if (!data.ecsWorld)
                return {};

            Camera camComponent {};
            Transform camTransform {};
            bool found = false;

            data.ecsWorld->RunSystem(
                ecs::ReadAccess<Camera, Transform>{},
                ecs::WriteAccess<>{},
                [&](const Camera& _cam, const Transform& _t)
                {
                    if (found || !_cam.enabled || !_cam.isPrimary)
                        return;
                    camComponent = _cam;
                    camTransform = _t;
                    found = true;
                }
            );

            if (!found)
                return {};

            const float aspect =
                static_cast<float>(data.renderExtent.x()) /
                static_cast<float>(std::max(1u, data.renderExtent.y()));

            cp::ResolvedCameraData result;
            result.view = BuildViewMatrix(camTransform);
            result.projection = BuildPerspectiveMatrix(
                camComponent.fovYDegrees,
                aspect,
                camComponent.nearPlane,
                camComponent.farPlane
            );
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
            model = glm::translate(model, glm::vec3(_transform.x, _transform.y, _transform.z));
            model = glm::rotate(model, glm::radians(_transform.roll), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, glm::radians(_transform.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(_transform.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(_transform.scaleX, _transform.scaleY, _transform.scaleZ));
            return model;
        }

        static glm::mat4 BuildViewMatrix(const Transform& _transform)
        {
            glm::mat4 cameraWorld(1.0f);
            cameraWorld = glm::translate(cameraWorld, glm::vec3(_transform.x, _transform.y, _transform.z));
            cameraWorld = glm::rotate(cameraWorld, glm::radians(_transform.roll), glm::vec3(0.0f, 0.0f, 1.0f));
            cameraWorld = glm::rotate(cameraWorld, glm::radians(_transform.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            cameraWorld = glm::rotate(cameraWorld, glm::radians(_transform.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
            return glm::inverse(cameraWorld);
        }

        static glm::mat4 BuildPerspectiveMatrix(
            float _fovYDegrees,
            float _aspectRatio,
            float _nearPlane,
            float _farPlane)
        {
            const float clampedAspect = _aspectRatio > 1e-6f ? _aspectRatio : 1.0f;
            const float clampedNear = std::max(0.001f, _nearPlane);
            const float clampedFar = std::max(clampedNear + 0.001f, _farPlane);
            const float clampedFov = std::max(1.0f, _fovYDegrees);

            glm::mat4 projection = glm::perspective(
                glm::radians(clampedFov),
                clampedAspect,
                clampedNear,
                clampedFar
            );
            projection[1][1] *= -1.0f;
            return projection;
        }

        static float Clamp01(const float _value)
        {
            if (_value < 0.f)
            {
                return 0.f;
            }
            if (_value > 1.f)
            {
                return 1.f;
            }
            return _value;
        }

        static OpaqueMaterialPassData::PhongVertex ConvertVertex(const MeshVertex& _vertex)
        {
            OpaqueMaterialPassData::PhongVertex result{};
            result.position[0] = _vertex.position[0];
            result.position[1] = _vertex.position[1];
            result.position[2] = _vertex.position[2];

            result.normal[0] = _vertex.normal[0];
            result.normal[1] = _vertex.normal[1];
            result.normal[2] = _vertex.normal[2];

            result.color[0] = Clamp01(std::fabs(_vertex.normal[0]));
            result.color[1] = Clamp01(std::fabs(_vertex.normal[1]));
            result.color[2] = Clamp01(std::fabs(_vertex.normal[2]));
            return result;
        }

        static std::vector<OpaqueMaterialPassData::PhongVertex> BuildDrawVertices(const Mesh& _mesh)
        {
            std::vector<OpaqueMaterialPassData::PhongVertex> vertices;
            const std::vector<MeshVertex>& sourceVertices = _mesh.GetVertices();
            const std::vector<uint32_t>& sourceIndices = _mesh.GetIndices();

            vertices.reserve(sourceIndices.size());
            for (const uint32_t index : sourceIndices)
            {
                if (index >= sourceVertices.size())
                {
                    continue;
                }
                vertices.push_back(ConvertVertex(sourceVertices[index]));
            }

            return vertices;
        }

        std::shared_ptr<IBuffer> CreateCpuVisibleBuffer(const size_t _sizeBytes, const BufferUsage _usage) const
        {
            if (_sizeBytes == 0)
            {
                return nullptr;
            }

            const BufferInfo bufferInfo{
                .sizeBytes = _sizeBytes,
                .usage = _usage,
                .cpuVisible = true
            };
            return data.rhi->CreateBuffer(bufferInfo);
        }

        static bool UploadBytes(IBuffer& _buffer, const void* _srcData, const size_t _sizeBytes)
        {
            if (_srcData == nullptr || _sizeBytes == 0)
            {
                return false;
            }

            void* mapped = _buffer.Map();
            if (mapped == nullptr)
            {
                return false;
            }

            std::memcpy(mapped, _srcData, _sizeBytes);
            _buffer.Unmap();
            return true;
        }

        OpaqueMaterialPassData::MeshGpuResources* GetOrCreateMeshGpuResources(
            const std::string& _meshPath,
            const AssetHandle<Mesh>& _meshHandle)
        {
            auto it = data.meshCache.find(_meshPath);
            if (it != data.meshCache.end())
            {
                return &it->second;
            }

            const Mesh* mesh = _meshHandle.Get();
            if (mesh == nullptr)
            {
                return nullptr;
            }

            std::vector<OpaqueMaterialPassData::PhongVertex> drawVertices = BuildDrawVertices(*mesh);
            if (drawVertices.empty())
            {
                return nullptr;
            }

            const size_t vertexBytes = drawVertices.size() * sizeof(OpaqueMaterialPassData::PhongVertex);
            std::shared_ptr<IBuffer> vertexBuffer = CreateCpuVisibleBuffer(vertexBytes, BufferUsage::Vertex);
            if (!vertexBuffer || !UploadBytes(*vertexBuffer, drawVertices.data(), vertexBytes))
            {
                return nullptr;
            }

            auto [insertedIt, _] = data.meshCache.insert_or_assign(
                _meshPath,
                OpaqueMaterialPassData::MeshGpuResources{
                    .vertexBuffer = std::move(vertexBuffer),
                    .vertexCount = static_cast<uint32_t>(drawVertices.size())
                }
            );
            return &insertedIt->second;
        }
    };
}
