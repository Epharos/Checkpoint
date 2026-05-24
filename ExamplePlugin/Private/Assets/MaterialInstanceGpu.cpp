#include "MaterialInstance.hpp"

#include <RHI/RenderingHardwareInterface.hpp>

#include <cstring>

namespace cp
{
    static void WriteFieldToBuffer(
        uint8_t* _bufferBase,
        const uint32_t _bufferSize,
        const FieldReflection& _field,
        const ParameterValue& _value
    )
    {
        if (_field.size == 0 || _field.offset + _field.size > _bufferSize)
        {
            return;
        }

        uint8_t* dst = _bufferBase + _field.offset;
        const uint32_t capacity = _field.size;

        std::visit([&](const auto& v)
        {
            using T = std::decay_t<decltype(v)>;

            if constexpr (std::is_same_v<T, bool>)
            {
                const uint32_t boolVal = v ? 1u : 0u;
                std::memcpy(dst, &boolVal, std::min(capacity, 4u));
            }
            else if constexpr (std::is_same_v<T, int32_t>)
            {
                std::memcpy(dst, &v, std::min(capacity, 4u));
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                std::memcpy(dst, &v, std::min(capacity, 4u));
            }
            else if constexpr (std::is_same_v<T, std::array<float, 2>>)
            {
                std::memcpy(dst, v.data(), std::min(capacity, 8u));
            }
            else if constexpr (std::is_same_v<T, std::array<float, 3>>)
            {
                std::memcpy(dst, v.data(), std::min(capacity, 12u));
            }
            else if constexpr (std::is_same_v<T, std::array<float, 4>>)
            {
                std::memcpy(dst, v.data(), std::min(capacity, 16u));
            }
        }, _value);
    }

    void MaterialInstance::RebuildGpuBuffers(
        RenderingHardwareInterface& _rhi,
        const ShaderReflection& _reflection
    )
    {
        gpuParamBuffers.clear();

        for (const BindingReflection& binding : _reflection.bindings)
        {
            // Bindings 0 (camera) and 1 (instances) are provided by the render pass.
            // TODO : Should be using set 0 for pass data and above for user data
            if (binding.binding <= 1)
            {
                continue;
            }

            const bool isConstantBuffer = binding.kind == BindingKind::ConstantBuffer;
            const bool isStructureBuffer =
                binding.kind == BindingKind::StructuredBuffer ||
                binding.kind == BindingKind::RWStructuredBuffer ||
                binding.kind == BindingKind::ByteAddressBuffer ||
                binding.kind == BindingKind::RWByteAddressBuffer;

            if (!isConstantBuffer && !isStructureBuffer)
            {
                continue;
            }

            if (binding.size == 0)
            {
                continue;
            }

            std::vector<uint8_t> staging(binding.size, 0);

            for (const FieldReflection& field : binding.fields)
            {
                const auto it = parameters.find(field.name);

                if (it != parameters.end())
                {
                    WriteFieldToBuffer(
                        staging.data(),
                        static_cast<uint32_t>(staging.size()),
                        field,
                        it->second
                    );
                }
            }

            const BufferInfo bufInfo {
                .sizeBytes = binding.size,
                .usage = isConstantBuffer ? BufferUsage::Uniform : BufferUsage::Storage,
                .cpuVisible = true
            };

            auto buf = _rhi.CreateBuffer(bufInfo);

            if (!buf)
            {
                continue;
            }

            if (void* mapped = buf->Map())
            {
                std::memcpy(mapped, staging.data(), staging.size());
                buf->Unmap();
            }

            gpuParamBuffers.push_back(GpuParamBuffer{
                .binding = binding.binding,
                .sizeBytes = binding.size,
                .buffer = std::move(buf)
            });
        }

        gpuDirty = false;
    }
}
