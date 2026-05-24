#pragma once

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Common/Data/Rectangle.hpp>
#include <Common/Data/Viewport.hpp>
#include <Common/IO/FileHelper.hpp>

#include "../ExamplePluginDir.hpp"

#include <ECS/ECS.hpp>

#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

#include <Rendering/FrameGraph/FrameGraph.hpp>
#include <Rendering/FrameGraph/FrameGraphBuilder.hpp>
#include <Rendering/FrameGraph/Renderpass.hpp>
#include <Rendering/IShaderProvider.hpp>

#include <Resources/CubemapLoader.hpp>

#include "../Components/Components.hpp"

namespace cp
{
    inline constexpr std::string_view SkyboxPassTypeName = "SkyboxPass";

    struct SkyboxPassData
    {
        struct SkyboxUboGpu
        {
            glm::mat4 invViewRotationProj { 1.0f };
        };

        FramegraphResourceHandle* finalRendering = nullptr;
        FramegraphResourceHandle* depthBuffer = nullptr;

        Extent2D<uint32_t> renderExtent {};
        std::shared_ptr<ecs::World> ecsWorld;
        RenderingHardwareInterface* rhi = nullptr;

        std::shared_ptr<IShaderModule> shaderModule;
        std::shared_ptr<ISampler> sampler;
        std::shared_ptr<IDescriptorSetLayout> descriptorSetLayout;
        std::shared_ptr<IDescriptorSet> descriptorSet;
        std::shared_ptr<IPipelineLayout> pipelineLayout;
        std::shared_ptr<IPipeline> pipeline;
        std::shared_ptr<IBuffer> uboBuffer;

        std::shared_ptr<ITexture> cubemapTexture;
    };

    class SkyboxPass : public RenderPass<SkyboxPassData>, public IConfigurableRenderPass
    {
    public:
        SkyboxPass()
        {
            name = "SkyboxPass";
        }

        [[nodiscard]] std::string_view GetCubemapBasePath() const { return cubemapBasePath; }

        void SetCubemapPath(std::string _path)
        {
            cubemapBasePath = std::move(_path);
            cubemapDirty = true;
        }

        void Serialize(ISerializer& _serializer) const override
        {
            _serializer.WriteString(cubemapBasePath);
        }

        void Deserialize(IDeserializer& _deserializer) override
        {
            std::string path;
            if (_deserializer.ReadString(path))
            {
                cubemapBasePath = std::move(path);
                cubemapDirty = true;
            }
        }

        void Configure(const RenderPassInitContext& _context) override
        {
            data.renderExtent = _context.renderExtent;
            data.rhi = &_context.rhi;

            if (!_context.shaderProvider)
            {
                return;
            }

            const ShaderProviderResult shader = _context.shaderProvider->GetShader(
                "Skybox", GetExamplePluginDir() / "Shaders"
            );

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

            constexpr SamplerInfo samplerInfo {
                .minFilter = Filter::Linear,
                .magFilter = Filter::Linear,
                .mipmapMode = MipmapMode::Linear,
                .addressU = AddressMode::ClampToEdge,
                .addressV = AddressMode::ClampToEdge,
                .addressW = AddressMode::ClampToEdge,
                .anisotropyEnable = false,
            };

            data.sampler = _context.rhi.CreateSampler(samplerInfo);

            // binding 0 : UBO (vertex)
            // binding 1 : standalone sampler (fragment)
            // binding 2 : sampled cubemap image (fragment)
            const DescriptorSetLayoutInfo descriptorSetLayoutInfo{
                .bindings = {
                    DescriptorBinding {
                        .binding = 0,
                        .type = DescriptorType::UniformBuffer,
                        .count = 1,
                        .visibility = ShaderStage::Vertex
                    },
                    DescriptorBinding {
                        .binding = 1,
                        .type = DescriptorType::Sampler,
                        .count = 1,
                        .visibility = ShaderStage::Fragment
                    },
                    DescriptorBinding {
                        .binding = 2,
                        .type = DescriptorType::SampledTexture,
                        .count = 1,
                        .visibility = ShaderStage::Fragment
                    }
                }
            };
            data.descriptorSetLayout = _context.rhi.GetDevice().CreateDescriptorSetLayout(descriptorSetLayoutInfo);
            data.descriptorSet = _context.rhi.GetDevice().CreateDescriptorSet(*data.descriptorSetLayout);

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
                .vertexInput = VertexInputState {},
                .topology = PrimitiveTopology::TriangleList,
                .rasterization = RasterizationState {
                    .cullMode = CullMode::None,
                },
                .depthStencil = DepthStencilState {
                    .depthTestEnable = true,
                    .depthWriteEnable = false,
                    .depthCompareOp = CompareOp::LessOrEqual,
                },
                .blendAttachments = {},
                .colorAttachmentFormats = { Format::R8G8B8A8_UNORM },
                .depthStencilFormat = Format::D32_FLOAT,
            };

            data.pipeline = _context.rhi.GetDevice().CreateGraphicsPipeline(graphicsPipelineInfo);

            const BufferInfo uboInfo {
                .sizeBytes = sizeof(SkyboxPassData::SkyboxUboGpu),
                .usage = BufferUsage::Uniform,
                .cpuVisible = true
            };

            data.uboBuffer = _context.rhi.CreateBuffer(uboInfo);

            ReloadCubemapIfNeeded();
        }

        void DeclareDependencies(FrameGraph& _framegraph) override
        {
            _framegraph.AddPassDependencyIfPresent(GetName(), "OpaqueMaterialPass");
        }

        void Setup(FrameGraphBuilder& _builder) override
        {
            data.finalRendering = _builder.UseTexture("ColorOutput", {
                .layout = TextureLayout::ColorAttachment,
                .stage = PipelineStage::ColorAttachment,
                .access = Access::ColorAttachmentWrite
            }, ResourceUsage::ReadWrite);

            data.depthBuffer = _builder.UseTexture("MainDepth", {
                .layout = TextureLayout::DepthStencilReadOnly,
                .stage = PipelineStage::EarlyDepth,
                .access = Access::DepthStencilRead
            }, ResourceUsage::ReadOnly);

            data.ecsWorld = _builder.GetCpuResource<ecs::World>("ECS.World");
        }

        void Execute(RenderPassExecutionContext& _context) override
        {
            if (!data.pipeline || !data.descriptorSet || !data.cubemapTexture)
                return;

            ReloadCubemapIfNeeded();

            ICommandBuffer& cmd = _context;
            ITexture* colorTarget = data.finalRendering ? data.finalRendering->GetTexture() : nullptr;
            ITexture* depthTarget = data.depthBuffer ? data.depthBuffer->GetTexture() : nullptr;

            if (!colorTarget || !depthTarget)
                return;

            if (!UpdateUbo(ResolveCamera(_context)))
                return;

            const ColorAttachmentInfo colorAttachment {
                .texture = colorTarget,
                .loadOp = LoadOp::Load,
                .storeOp = StoreOp::Store,
                .clearValue = {}
            };
            const DepthStencilAttachmentInfo depthAttachment {
                .texture = depthTarget,
                .depthLoadOp = LoadOp::Load,
                .depthStoreOp = StoreOp::Store,
                .stencilLoadOp = LoadOp::Load,
                .stencilStoreOp = StoreOp::Store,
                .clearValue = ClearDepthStencil(1.0f, 0)
            };

            const RenderingInfo renderingInfo {
                .extent = data.renderExtent,
                .layers = 1,
                .colorAttachments = { colorAttachment },
                .depthStencilAttachment = depthAttachment
            };

            cmd.SetViewport(Viewport(
                0.f,
                0.f,
                static_cast<float>(data.renderExtent.x()),
                static_cast<float>(data.renderExtent.y())
            ));

            cmd.SetScissor(Rectangle2D(
                Extent2D<int>(0, 0),
                data.renderExtent
            ));

            cmd.BeginRendering(renderingInfo);
            cmd.BindPipeline(*data.pipeline);
            cmd.BindDescriptorSet(0, *data.descriptorSet);
            cmd.Draw(3, 1, 0, 0);
            cmd.EndRendering();
        }

        void OnPostCompile(FrameGraph& _framegraph) override
        {
            BindDescriptors();
        }

        void OnReset() override
        {
            data.finalRendering = nullptr;
            data.depthBuffer = nullptr;
            data.ecsWorld.reset();
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

        bool UpdateUbo(const cp::ResolvedCameraData& _camera) const
        {
            if (!data.uboBuffer || !_camera.isValid)
                return false;

            glm::mat4 viewRotOnly = _camera.view;
            viewRotOnly[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

            const SkyboxPassData::SkyboxUboGpu uboData {
                .invViewRotationProj = glm::inverse(_camera.projection * viewRotOnly)
            };

            void* mapped = data.uboBuffer->Map();

            if (!mapped)
                return false;

            std::memcpy(mapped, &uboData, sizeof(uboData));
            data.uboBuffer->Unmap();
            return true;
        }

        void ReloadCubemapIfNeeded()
        {
            if (!cubemapDirty || data.rhi == nullptr)
            {
                return;
            }

            cubemapDirty = false;

            if (cubemapBasePath.empty())
            {
                data.cubemapTexture.reset();
                return;
            }

            const auto resolvedPath = FindFileFromDirectory(cubemapBasePath, GetExamplePluginDir());

            const std::filesystem::path basePath =
                resolvedPath.empty() ? std::filesystem::path(cubemapBasePath) : resolvedPath;

            data.cubemapTexture = LoadCubemapTexture(basePath, *data.rhi);

            if (data.cubemapTexture)
            {
                BindDescriptors();
            }
        }

        void BindDescriptors()
        {
            if (!data.descriptorSet || !data.uboBuffer)
            {
                return;
            }

            data.descriptorSet->UpdateBuffers({
                DescriptorBufferBinding {
                    .binding = 0,
                    .buffer = data.uboBuffer.get(),
                    .offsetBytes = 0,
                    .rangeBytes = sizeof(SkyboxPassData::SkyboxUboGpu)
                }
            });

            std::vector<DescriptorTextureBinding> textureBindings;

            // Standalone sampler at binding 1
            if (data.sampler)
            {
                textureBindings.push_back(DescriptorTextureBinding {
                    .binding = 1,
                    .texture = nullptr,
                    .layout = TextureLayout::Undefined,
                    .sampler = data.sampler.get()
                });
            }

            // Sampled cubemap image at binding 2
            if (data.cubemapTexture)
            {
                textureBindings.push_back(DescriptorTextureBinding {
                    .binding = 2,
                    .texture = data.cubemapTexture.get(),
                    .layout = TextureLayout::ShaderReadOnly,
                    .sampler = nullptr
                });
            }

            if (!textureBindings.empty())
            {
                data.descriptorSet->UpdateTextures(textureBindings);
            }
        }

        static glm::mat4 BuildViewMatrix(const Transform& _t)
        {
            glm::mat4 world(1.0f);
            world = glm::translate(world, glm::vec3(_t.x, _t.y, _t.z));
            world = glm::rotate(world, glm::radians(_t.roll), glm::vec3(0.f, 0.f, 1.f));
            world = glm::rotate(world, glm::radians(_t.yaw), glm::vec3(0.f, 1.f, 0.f));
            world = glm::rotate(world, glm::radians(_t.pitch),  glm::vec3(1.f, 0.f, 0.f));
            return glm::inverse(world);
        }

        static glm::mat4 BuildPerspectiveMatrix(float _fovY, float _aspect, float _near, float _far)
        {
            const float a = _aspect > 1e-6f ? _aspect : 1.0f;
            const float nearP = std::max(0.001f, _near);
            const float farP = std::max(nearP + 0.001f, _far);
            const float fov = std::max(1.0f, _fovY);
            glm::mat4 proj = glm::perspective(glm::radians(fov), a, nearP, farP);
            proj[1][1] *= -1.0f;
            return proj;
        }

        std::string cubemapBasePath;
        bool cubemapDirty = false;
    };
}
