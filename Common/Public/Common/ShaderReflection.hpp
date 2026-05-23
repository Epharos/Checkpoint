#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cp
{
    enum class ScalarKind : uint8_t
    {
        Unknown,
        Bool,
        Int8, Int16, Int32, Int64,
        UInt8, UInt16, UInt32, UInt64,
        Float16, Float32, Float64,
    };

    enum class FieldKind : uint8_t
    {
        Unknown,
        Scalar, // float, int, bool, …
        Vector, // float3, int4, …
        Matrix, // float4x4, …
        Struct, // user-defined struct
        Array, // T[N]
    };

    struct FieldReflection
    {
        std::string name;
        std::string typeName; // "float3", "float4x4", "CameraData", …

        FieldKind kind = FieldKind::Unknown;
        ScalarKind scalar = ScalarKind::Unknown;

        uint32_t rows = 0; // matrix rows => 1 for vectors
        uint32_t columns = 0; // matrix columns => component count for vectors

        uint32_t offset = 0; // byte offset within the parent struct
        uint32_t size = 0; // byte size
        uint32_t arrayCount = 0; // element count  (0 when not an array)
        uint32_t stride = 0; // element stride (0 when not an array)

        std::vector<FieldReflection> fields; // nested fields for Struct and AOS
        std::vector<std::string> annotations; // user-defined Slang attributes, e.g. "MaterialParam", "Color"
    };

    enum class BindingKind : uint8_t
    {
        Unknown,

        ConstantBuffer,
        PushConstant,

        StructuredBuffer,
        RWStructuredBuffer,
        ByteAddressBuffer,
        RWByteAddressBuffer,

        Texture1D, Texture1DArray,
        Texture2D, Texture2DArray,
        Texture2DMS, Texture2DMSArray,
        Texture3D,
        TextureCube, TextureCubeArray,

        RWTexture1D, RWTexture1DArray,
        RWTexture2D, RWTexture2DArray,
        RWTexture3D,

        Sampler,
        SamplerComparison,

        AccelerationStructure,
        ParameterBlock,
    };

    struct BindingReflection
    {
        std::string name;
        std::string typeName; // element type name ("CameraData", "float4", …)

        BindingKind kind = BindingKind::Unknown;
        uint32_t binding = 0; // binding index / register slot
        uint32_t set = 0; // descriptor set (Vulkan) / register space (DX12)

        // Meaningful for ConstantBuffer, PushConstant and ParameterBlock
        uint32_t size = 0; // total byte size of the block

        // Meaningful for StructuredBuffer and RWStructuredBuffer
        uint32_t stride = 0; // element byte size

        // Inner fields
        std::vector<FieldReflection> fields;

        std::vector<std::string> annotations; // user-defined Slang attributes, e.g. "MaterialParam"
    };

    struct ShaderReflection
    {
        std::vector<BindingReflection> bindings;
    };
}
