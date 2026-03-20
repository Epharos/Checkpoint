#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../Data/Enums.hpp"

#include "PipelineStates.hpp"

namespace cp
{
	class IShaderModule;
	struct ShaderModuleInfo;

	class IPipelineLayout;
	struct PipelineLayoutInfo;

	enum class PipelineType : uint8_t
	{
		Graphics,
		Compute
	};

	struct GraphicsPipelineInfo
	{
		std::shared_ptr<IPipelineLayout> layout;

		std::shared_ptr<IShaderModule> shaderModule;
		std::vector<std::pair<ShaderStage, std::string_view>> stageMains;

		VertexInputState vertexInput;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;

		RasterizationState rasterization;
		DepthStencilState depthStencil;

		std::vector<BlendAttachmentState> blendAttachments;

		std::vector<Format> colorAttachmentFormats;
		Format depthStencilFormat = Format::Unknown;
	};

	struct ComputePipelineInfo
	{
		std::shared_ptr<IPipelineLayout> layout;

		std::shared_ptr<IShaderModule> computeShader;
		std::string_view mainMethodName;
	};

	class IPipeline
	{
	public:
		explicit IPipeline(const GraphicsPipelineInfo& _info) : info(_info), type(PipelineType::Graphics) {}
		explicit IPipeline(const ComputePipelineInfo& _info) : info(_info), type(PipelineType::Compute) {}

		virtual ~IPipeline() = default;

		[[nodiscard]] PipelineType GetType() const { return type; }

		[[nodiscard]] const std::variant<const GraphicsPipelineInfo, const ComputePipelineInfo>& GetInfo() const { return info; }

	protected:
		const std::variant<const GraphicsPipelineInfo, const ComputePipelineInfo> info;

		const PipelineType type;
	};
}
