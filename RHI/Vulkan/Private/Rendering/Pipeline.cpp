#include "../pch.hpp"

#include "Pipeline.hpp"

#include "PipelineLayout.hpp"
#include "ShaderModule.hpp"
#include "../Core/Device.hpp"
#include "../Utilities/VulkanConverter.hpp"

#include <array>
#include <string>

namespace cp
{
	namespace
	{
		vk::PrimitiveTopology ToVulkanTopology(const PrimitiveTopology _topology)
		{
			switch (_topology)
			{
				case PrimitiveTopology::TriangleList: return vk::PrimitiveTopology::eTriangleList;
				case PrimitiveTopology::TriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
				case PrimitiveTopology::LineList: return vk::PrimitiveTopology::eLineList;
				case PrimitiveTopology::LineStrip: return vk::PrimitiveTopology::eLineStrip;
				case PrimitiveTopology::PointList: return vk::PrimitiveTopology::ePointList;
				default: throw std::logic_error("Unrecognized primitive topology");
			}
		}

		vk::PolygonMode ToVulkanPolygonMode(const PolygonMode _polygonMode)
		{
			switch (_polygonMode)
			{
				case PolygonMode::Fill: return vk::PolygonMode::eFill;
				case PolygonMode::Line: return vk::PolygonMode::eLine;
				case PolygonMode::Point: return vk::PolygonMode::ePoint;
				default: throw std::logic_error("Unrecognized polygon mode");
			}
		}

		vk::CullModeFlags ToVulkanCullMode(const CullMode _cullMode)
		{
			switch (_cullMode)
			{
				case CullMode::None: return vk::CullModeFlagBits::eNone;
				case CullMode::Front: return vk::CullModeFlagBits::eFront;
				case CullMode::Back: return vk::CullModeFlagBits::eBack;
				default: throw std::logic_error("Unrecognized cull mode");
			}
		}

		vk::FrontFace ToVulkanFrontFace(const FrontFace _frontFace)
		{
			switch (_frontFace)
			{
				case FrontFace::CounterClockwise: return vk::FrontFace::eCounterClockwise;
				case FrontFace::Clockwise: return vk::FrontFace::eClockwise;
				default: throw std::logic_error("Unrecognized front face");
			}
		}

		vk::CompareOp ToVulkanCompareOp(const CompareOp _compareOp)
		{
			switch (_compareOp)
			{
				case CompareOp::Never: return vk::CompareOp::eNever;
				case CompareOp::Less: return vk::CompareOp::eLess;
				case CompareOp::Equal: return vk::CompareOp::eEqual;
				case CompareOp::LessOrEqual: return vk::CompareOp::eLessOrEqual;
				case CompareOp::Greater: return vk::CompareOp::eGreater;
				case CompareOp::NotEqual: return vk::CompareOp::eNotEqual;
				case CompareOp::GreaterOrEqual: return vk::CompareOp::eGreaterOrEqual;
				case CompareOp::Always: return vk::CompareOp::eAlways;
				default: throw std::logic_error("Unrecognized compare op");
			}
		}

		vk::BlendFactor ToVulkanBlendFactor(const BlendFactor _blendFactor)
		{
			switch (_blendFactor)
			{
				case BlendFactor::Zero: return vk::BlendFactor::eZero;
				case BlendFactor::One: return vk::BlendFactor::eOne;
				case BlendFactor::SrcColor: return vk::BlendFactor::eSrcColor;
				case BlendFactor::OneMinusSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
				case BlendFactor::DstColor: return vk::BlendFactor::eDstColor;
				case BlendFactor::OneMinusDstColor: return vk::BlendFactor::eOneMinusDstColor;
				case BlendFactor::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
				case BlendFactor::OneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
				case BlendFactor::DstAlpha: return vk::BlendFactor::eDstAlpha;
				case BlendFactor::OneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
				default: throw std::logic_error("Unrecognized blend factor");
			}
		}

		vk::BlendOp ToVulkanBlendOp(const BlendOp _blendOp)
		{
			switch (_blendOp)
			{
				case BlendOp::Add: return vk::BlendOp::eAdd;
				case BlendOp::Subtract: return vk::BlendOp::eSubtract;
				case BlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
				case BlendOp::Min: return vk::BlendOp::eMin;
				case BlendOp::Max: return vk::BlendOp::eMax;
				default: throw std::logic_error("Unrecognized blend op");
			}
		}

		vk::ColorComponentFlags ToVulkanColorMask(const ColorWriteMask _mask)
		{
			vk::ColorComponentFlags flags;
			if (static_cast<uint8_t>(_mask) & static_cast<uint8_t>(ColorWriteMask::R)) flags |= vk::ColorComponentFlagBits::eR;
			if (static_cast<uint8_t>(_mask) & static_cast<uint8_t>(ColorWriteMask::G)) flags |= vk::ColorComponentFlagBits::eG;
			if (static_cast<uint8_t>(_mask) & static_cast<uint8_t>(ColorWriteMask::B)) flags |= vk::ColorComponentFlagBits::eB;
			if (static_cast<uint8_t>(_mask) & static_cast<uint8_t>(ColorWriteMask::A)) flags |= vk::ColorComponentFlagBits::eA;
			return flags;
		}

		vk::VertexInputRate ToVulkanVertexInputRate(const VertexInputRate _inputRate)
		{
			switch (_inputRate)
			{
				case VertexInputRate::PerVertex: return vk::VertexInputRate::eVertex;
				case VertexInputRate::PerInstance: return vk::VertexInputRate::eInstance;
				default: throw std::logic_error("Unrecognized vertex input rate");
			}
		}

		vk::Format ToVulkanVertexFormat(const VertexFormat _format)
		{
			switch (_format)
			{
				case VertexFormat::R32_FLOAT: return vk::Format::eR32Sfloat;
				case VertexFormat::R32G32_FLOAT: return vk::Format::eR32G32Sfloat;
				case VertexFormat::R32G32B32_FLOAT: return vk::Format::eR32G32B32Sfloat;
				case VertexFormat::R32G32B32A32_FLOAT: return vk::Format::eR32G32B32A32Sfloat;
				case VertexFormat::R8G8B8A8_UNORM: return vk::Format::eR8G8B8A8Unorm;
				default: throw std::logic_error("Unrecognized vertex format");
			}
		}
	}

	Pipeline::Pipeline(const GraphicsPipelineInfo& _info, Device& _device)
		: IPipeline(_info), device(_device)
	{
		InitializeGraphics(_info);
	}

	Pipeline::Pipeline(const ComputePipelineInfo& _info, Device& _device)
		: IPipeline(_info), device(_device)
	{
		InitializeCompute(_info);
	}

	Pipeline::~Pipeline()
	{
		Cleanup();
	}

	void Pipeline::InitializeGraphics(const GraphicsPipelineInfo& _info)
	{
		CP_EXPECT_MSG(_info.layout, "Graphics pipeline layout is null");
		CP_EXPECT_MSG(_info.shaderModule, "Graphics pipeline shader module is null");
		CP_EXPECT_MSG(!_info.stageMains.empty(), "Graphics pipeline requires at least one shader stage");

		const auto& shaderModule = static_cast<const ShaderModule&>(*_info.shaderModule);
		const auto& layout = static_cast<const PipelineLayout&>(*_info.layout);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(_info.stageMains.size());
		std::vector<std::string> entryPoints;
		entryPoints.reserve(_info.stageMains.size());

		for (const auto& [stage, mainName] : _info.stageMains)
		{
			entryPoints.emplace_back(mainName);

			vk::PipelineShaderStageCreateInfo stageInfo;
			stageInfo.setStage(EnumCast<vk::ShaderStageFlagBits>(stage));
			stageInfo.setModule(shaderModule.GetHandle());
			stageInfo.setPName(entryPoints.back().c_str());

			shaderStages.push_back(stageInfo);
		}

		std::vector<vk::VertexInputBindingDescription> vertexBindings;
		vertexBindings.reserve(_info.vertexInput.bindings.size());

		for (const VertexBindingInfo& bindingInfo : _info.vertexInput.bindings)
		{
			vk::VertexInputBindingDescription description;
			description.setBinding(bindingInfo.binding);
			description.setStride(bindingInfo.strideBytes);
			description.setInputRate(ToVulkanVertexInputRate(bindingInfo.inputRate));

			vertexBindings.push_back(description);
		}

		std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
		vertexAttributes.reserve(_info.vertexInput.attributes.size());

		for (const VertexAttributeInfo& attributeInfo : _info.vertexInput.attributes)
		{
			vk::VertexInputAttributeDescription description;
			description.setLocation(attributeInfo.location);
			description.setBinding(attributeInfo.binding);
			description.setFormat(ToVulkanVertexFormat(attributeInfo.format));
			description.setOffset(attributeInfo.offsetBytes);

			vertexAttributes.push_back(description);
		}

		vk::PipelineVertexInputStateCreateInfo vertexInputState;
		vertexInputState.setVertexBindingDescriptionCount(static_cast<uint32_t>(vertexBindings.size()));
		vertexInputState.setPVertexBindingDescriptions(vertexBindings.data());
		vertexInputState.setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexAttributes.size()));
		vertexInputState.setPVertexAttributeDescriptions(vertexAttributes.data());

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		inputAssemblyState.setTopology(ToVulkanTopology(_info.topology));
		inputAssemblyState.setPrimitiveRestartEnable(false);

		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.setViewportCount(1);
		viewportState.setScissorCount(1);

		vk::PipelineRasterizationStateCreateInfo rasterizationState;
		rasterizationState.setDepthClampEnable(_info.rasterization.depthClampEnable);
		rasterizationState.setRasterizerDiscardEnable(false);
		rasterizationState.setPolygonMode(ToVulkanPolygonMode(_info.rasterization.polygonMode));
		rasterizationState.setCullMode(ToVulkanCullMode(_info.rasterization.cullMode));
		rasterizationState.setFrontFace(ToVulkanFrontFace(_info.rasterization.frontFace));
		rasterizationState.setDepthBiasEnable(_info.rasterization.depthBiasEnable);
		rasterizationState.setDepthBiasConstantFactor(_info.rasterization.depthBiasConstantFactor);
		rasterizationState.setDepthBiasSlopeFactor(_info.rasterization.depthBiasSlopeFactor);
		rasterizationState.setLineWidth(1.0f);

		vk::PipelineMultisampleStateCreateInfo multisampleState;
		multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);

		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		depthStencilState.setDepthTestEnable(_info.depthStencil.depthTestEnable);
		depthStencilState.setDepthWriteEnable(_info.depthStencil.depthWriteEnable);
		depthStencilState.setDepthCompareOp(ToVulkanCompareOp(_info.depthStencil.depthCompareOp));
		depthStencilState.setStencilTestEnable(_info.depthStencil.stencilTestEnable);

		std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
		colorBlendAttachments.reserve(_info.colorAttachmentFormats.size());

		if (_info.blendAttachments.empty())
		{
			for (size_t i = 0; i < _info.colorAttachmentFormats.size(); ++i)
			{
				vk::PipelineColorBlendAttachmentState blendAttachment;
				blendAttachment.setBlendEnable(false);
				blendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
				colorBlendAttachments.push_back(blendAttachment);
			}
		}
		else
		{
			CP_EXPECT_MSG(
				_info.blendAttachments.size() == _info.colorAttachmentFormats.size(),
				"Graphics pipeline blend attachment count must match color attachment format count"
			);

			for (const BlendAttachmentState& blendState : _info.blendAttachments)
			{
				vk::PipelineColorBlendAttachmentState blendAttachment;
				blendAttachment.setBlendEnable(blendState.blendEnable);
				blendAttachment.setSrcColorBlendFactor(ToVulkanBlendFactor(blendState.srcColorFactor));
				blendAttachment.setDstColorBlendFactor(ToVulkanBlendFactor(blendState.dstColorFactor));
				blendAttachment.setColorBlendOp(ToVulkanBlendOp(blendState.colorOp));
				blendAttachment.setSrcAlphaBlendFactor(ToVulkanBlendFactor(blendState.srcAlphaFactor));
				blendAttachment.setDstAlphaBlendFactor(ToVulkanBlendFactor(blendState.dstAlphaFactor));
				blendAttachment.setAlphaBlendOp(ToVulkanBlendOp(blendState.alphaOp));
				blendAttachment.setColorWriteMask(ToVulkanColorMask(blendState.writeMask));

				colorBlendAttachments.push_back(blendAttachment);
			}
		}

		vk::PipelineColorBlendStateCreateInfo colorBlendState;
		colorBlendState.setAttachmentCount(static_cast<uint32_t>(colorBlendAttachments.size()));
		colorBlendState.setPAttachments(colorBlendAttachments.data());

		constexpr std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()));
		dynamicState.setPDynamicStates(dynamicStates.data());

		std::vector<vk::Format> colorFormats;
		colorFormats.reserve(_info.colorAttachmentFormats.size());
		for (const Format colorFormat : _info.colorAttachmentFormats)
		{
			colorFormats.push_back(EnumCast<vk::Format>(colorFormat));
		}

		vk::PipelineRenderingCreateInfo pipelineRenderingInfo;
		pipelineRenderingInfo.setColorAttachmentCount(static_cast<uint32_t>(colorFormats.size()));
		pipelineRenderingInfo.setPColorAttachmentFormats(colorFormats.data());
		pipelineRenderingInfo.setDepthAttachmentFormat(
			_info.depthStencilFormat == Format::Unknown ? vk::Format::eUndefined : EnumCast<vk::Format>(_info.depthStencilFormat)
		);
		pipelineRenderingInfo.setStencilAttachmentFormat(vk::Format::eUndefined);

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.setPNext(&pipelineRenderingInfo);
		pipelineCreateInfo.setStageCount(static_cast<uint32_t>(shaderStages.size()));
		pipelineCreateInfo.setPStages(shaderStages.data());
		pipelineCreateInfo.setPVertexInputState(&vertexInputState);
		pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyState);
		pipelineCreateInfo.setPViewportState(&viewportState);
		pipelineCreateInfo.setPRasterizationState(&rasterizationState);
		pipelineCreateInfo.setPMultisampleState(&multisampleState);
		pipelineCreateInfo.setPDepthStencilState(&depthStencilState);
		pipelineCreateInfo.setPColorBlendState(&colorBlendState);
		pipelineCreateInfo.setPDynamicState(&dynamicState);
		pipelineCreateInfo.setLayout(layout.GetHandle());
		pipelineCreateInfo.setRenderPass(VK_NULL_HANDLE);
		pipelineCreateInfo.setSubpass(0);

		pipeline = device.GetHandle().createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreateInfo).value;
		CP_ENSURE_MSG(pipeline != VK_NULL_HANDLE, "Graphics pipeline was not initialized");
	}

	void Pipeline::InitializeCompute(const ComputePipelineInfo& _info)
	{
		CP_EXPECT_MSG(_info.layout, "Compute pipeline layout is null");
		CP_EXPECT_MSG(_info.computeShader, "Compute pipeline shader module is null");

		const auto& shaderModule = static_cast<const ShaderModule&>(*_info.computeShader);
		const auto& layout = static_cast<const PipelineLayout&>(*_info.layout);

		vk::PipelineShaderStageCreateInfo shaderStage;
		shaderStage.setStage(vk::ShaderStageFlagBits::eCompute);
		shaderStage.setModule(shaderModule.GetHandle());
		shaderStage.setPName(std::string(_info.mainMethodName).c_str());

		vk::ComputePipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.setStage(shaderStage);
		pipelineCreateInfo.setLayout(layout.GetHandle());

		pipeline = device.GetHandle().createComputePipeline(VK_NULL_HANDLE, pipelineCreateInfo).value;
		CP_ENSURE_MSG(pipeline != VK_NULL_HANDLE, "Compute pipeline was not initialized");
	}

	void Pipeline::Cleanup() const
	{
		if (pipeline != VK_NULL_HANDLE)
		{
			device.GetHandle().destroyPipeline(pipeline);
		}
	}
}
