#include "../Public/ShaderCompiler/ShaderCompiler.hpp"

#include <slang.h>
#include <slang-com-ptr.h>

#include <fstream>
#include <sstream>

namespace cp
{
    static std::string BlobToString(slang::IBlob* blob)
    {
        if (!blob || blob->getBufferSize() == 0)
        {
            return {};
        }

        return std::string { static_cast<const char*>(blob->getBufferPointer()), blob->getBufferSize() };
    }

    static void AppendDiagnostics(std::string& out, slang::IBlob* blob)
    {
        std::string msg = BlobToString(blob);
        if (!msg.empty())
        {
            if (!out.empty()) out += '\n';
            out += msg;
        }
    }

    static SlangCompileTarget TargetFormat(ShaderTarget t)
    {
        switch (t)
        {
            case ShaderTarget::Vulkan_SPIRV: return SLANG_SPIRV;
            case ShaderTarget::DX12_DXIL: return SLANG_DXIL;
        }

        return SLANG_SPIRV;
    }

    static const char* TargetProfile(ShaderTarget t)
    {
        switch (t)
        {
            case ShaderTarget::Vulkan_SPIRV: return "spirv_1_6";
            case ShaderTarget::DX12_DXIL: return "sm_6_5";
        }

        return "spirv_1_5";
    }

    static void FillBinaryResult(CompileResult& result, slang::IBlob* codeBlob)
    {
        const auto* data = static_cast<const uint8_t*>(codeBlob->getBufferPointer());
        result.binary.assign(data, data + codeBlob->getBufferSize());
        result.success = true;
    }

    static std::string ReadFile(const std::filesystem::path& filePath, std::string& outError)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            outError = "Cannot open file: " + filePath.string();
            return {};
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    using SlangScalarType = slang::TypeReflection::ScalarType;

    static ScalarKind ConvertScalar(slang::TypeReflection::ScalarType st)
    {
        switch (st)
        {
            case SlangScalarType::Bool: return ScalarKind::Bool;
            case SlangScalarType::Int8: return ScalarKind::Int8;
            case SlangScalarType::Int16: return ScalarKind::Int16;
            case SlangScalarType::Int32: return ScalarKind::Int32;
            case SlangScalarType::Int64: return ScalarKind::Int64;
            case SlangScalarType::UInt8: return ScalarKind::UInt8;
            case SlangScalarType::UInt16: return ScalarKind::UInt16;
            case SlangScalarType::UInt32: return ScalarKind::UInt32;
            case SlangScalarType::UInt64: return ScalarKind::UInt64;
            case SlangScalarType::Float16: return ScalarKind::Float16;
            case SlangScalarType::Float32: return ScalarKind::Float32;
            case SlangScalarType::Float64: return ScalarKind::Float64;
            default: return ScalarKind::Unknown;
        }
    }

    static std::string ScalarKindName(ScalarKind k)
    {
        switch (k)
        {
            case ScalarKind::Bool: return "bool";
            case ScalarKind::Int8: return "int8_t";
            case ScalarKind::Int16: return "int16_t";
            case ScalarKind::Int32: return "int";
            case ScalarKind::Int64: return "int64_t";
            case ScalarKind::UInt8: return "uint8_t";
            case ScalarKind::UInt16: return "uint16_t";
            case ScalarKind::UInt32: return "uint";
            case ScalarKind::UInt64: return "uint64_t";
            case ScalarKind::Float16: return "half";
            case ScalarKind::Float32: return "float";
            case ScalarKind::Float64: return "double";
            default: return "unknown";
        }
    }

    static void FillTypeInfo(FieldReflection& field, slang::TypeLayoutReflection* typeLayout);

    static void ExtractUserAttributes(std::vector<std::string>& outAnnotations, slang::VariableReflection* var)
    {
        if (!var)
        {
            return;
        }

        const unsigned int attrCount = var->getUserAttributeCount();
        outAnnotations.reserve(attrCount);

        for (unsigned int j = 0; j < attrCount; j++)
        {
            slang::UserAttribute* attr = var->getUserAttributeByIndex(j);
            if (attr)
            {
                if (const char* attrName = attr->getName())
                {
                    outAnnotations.push_back(attrName);
                }
            }
        }
    }

    static FieldReflection ExtractField(slang::VariableLayoutReflection* varLayout)
    {
        FieldReflection field;

        if (const char* n = varLayout->getName())
        {
            field.name = n;
        }

        slang::TypeLayoutReflection* typeLayout = varLayout->getTypeLayout();
        field.offset = static_cast<uint32_t>(varLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM));
        field.size = static_cast<uint32_t>(typeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM));

        ExtractUserAttributes(field.annotations, varLayout->getVariable());
        FillTypeInfo(field, typeLayout);
        return field;
    }

    static void FillTypeInfo(FieldReflection& field, slang::TypeLayoutReflection* typeLayout)
    {
        if (const char* n = typeLayout->getName())
        {
            field.typeName = n;
        }

        using Kind = slang::TypeReflection::Kind;
        const Kind kind = typeLayout->getKind();

        switch (kind)
        {
        case Kind::Scalar:
        {
            field.kind = FieldKind::Scalar;
            field.scalar = ConvertScalar(typeLayout->getScalarType());

            if (field.typeName.empty())
            {
                field.typeName = ScalarKindName(field.scalar);
            }

            break;
        }

        case Kind::Vector:
        {
            field.kind = FieldKind::Vector;
            field.rows = 1;
            field.columns = typeLayout->getColumnCount();
            field.scalar = ConvertScalar(typeLayout->getScalarType());

            if (field.typeName.empty())
            {
                field.typeName = ScalarKindName(field.scalar) + std::to_string(field.columns);
            }

            break;
        }

        case Kind::Matrix:
        {
            field.kind = FieldKind::Matrix;
            field.rows = typeLayout->getRowCount();
            field.columns = typeLayout->getColumnCount();
            field.scalar = ConvertScalar(typeLayout->getScalarType());

            if (field.typeName.empty())
            {
                field.typeName = ScalarKindName(field.scalar) + std::to_string(field.rows) + "x" + std::to_string(field.columns);
            }

            break;
        }

        case Kind::Struct:
        {
            field.kind = FieldKind::Struct;
            const uint32_t fieldCount = typeLayout->getFieldCount();
            field.fields.reserve(fieldCount);

            for (uint32_t i = 0; i < fieldCount; i++)
            {
                field.fields.push_back(ExtractField(typeLayout->getFieldByIndex(i)));
            }

            break;
        }

        case Kind::Array:
        {
            field.kind = FieldKind::Array;
            field.arrayCount = static_cast<uint32_t>(typeLayout->getElementCount());
            field.stride = static_cast<uint32_t>(typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM));

            slang::TypeLayoutReflection* elementTypeLayout = typeLayout->getElementTypeLayout();
            if (elementTypeLayout)
            {
                if (const char* elemName = elementTypeLayout->getName())
                {
                    field.typeName = std::string(elemName) + "[" + std::to_string(field.arrayCount) + "]";
                }

                if (elementTypeLayout->getKind() == Kind::Struct)
                {
                    const uint32_t fieldCount = elementTypeLayout->getFieldCount();
                    field.fields.reserve(fieldCount);

                    for (uint32_t i = 0; i < fieldCount; i++)
                    {
                        field.fields.push_back(ExtractField(elementTypeLayout->getFieldByIndex(i)));
                    }
                }
            }

            break;
        }

        default:
            break;
        }
    }

    static std::vector<FieldReflection> ExtractStructFields(slang::TypeLayoutReflection* structureTypeLayout)
    {
        std::vector<FieldReflection> fields;
        if (!structureTypeLayout || structureTypeLayout->getKind() != slang::TypeReflection::Kind::Struct)
        {
            return fields;
        }

        const uint32_t fieldCount = structureTypeLayout->getFieldCount();
        fields.reserve(fieldCount);

        for (uint32_t i = 0; i < fieldCount; i++)
        {
            fields.push_back(ExtractField(structureTypeLayout->getFieldByIndex(i)));
        }

        return fields;
    }

    static BindingKind DetermineBindingKind(slang::TypeLayoutReflection* typeLayout, slang::ParameterCategory category)
    {
        using Kind = slang::TypeReflection::Kind;

        if (category == slang::ParameterCategory::PushConstantBuffer)
        {
            return BindingKind::PushConstant;
        }

        switch (typeLayout->getKind())
        {
        case Kind::ConstantBuffer: return BindingKind::ConstantBuffer;
        case Kind::ParameterBlock: return BindingKind::ParameterBlock;
        case Kind::SamplerState:
        {
            const char* name = typeLayout->getName();

            if (name && std::string_view(name).find("Comparison") != std::string_view::npos)
            {
                return BindingKind::SamplerComparison;
            }

            return BindingKind::Sampler;
        }

        case Kind::Resource:
        {
            const SlangResourceShape shapeFlags = typeLayout->getResourceShape();
            const auto baseShape  = static_cast<SlangResourceShape>(shapeFlags & SLANG_RESOURCE_BASE_SHAPE_MASK);
            const SlangResourceAccess access = typeLayout->getResourceAccess();

            const bool isRW =
                (access == SLANG_RESOURCE_ACCESS_READ_WRITE
                || access == SLANG_RESOURCE_ACCESS_WRITE
                || access == SLANG_RESOURCE_ACCESS_APPEND
                || access == SLANG_RESOURCE_ACCESS_CONSUME);

            const bool isArray = (shapeFlags & SLANG_TEXTURE_ARRAY_FLAG) != 0;
            const bool isMS = (shapeFlags & SLANG_TEXTURE_MULTISAMPLE_FLAG) != 0;

            switch (baseShape)
            {
            case SLANG_TEXTURE_1D:
                if (isRW) return isArray ? BindingKind::RWTexture1DArray : BindingKind::RWTexture1D;
                return isArray ? BindingKind::Texture1DArray : BindingKind::Texture1D;
            case SLANG_TEXTURE_2D:
                if (isRW) return isArray ? BindingKind::RWTexture2DArray : BindingKind::RWTexture2D;
                if (isMS) return isArray ? BindingKind::Texture2DMSArray : BindingKind::Texture2DMS;
                return isArray ? BindingKind::Texture2DArray : BindingKind::Texture2D;
            case SLANG_TEXTURE_3D:
                return isRW ? BindingKind::RWTexture3D : BindingKind::Texture3D;
            case SLANG_TEXTURE_CUBE:
                return isArray ? BindingKind::TextureCubeArray : BindingKind::TextureCube;
            case SLANG_STRUCTURED_BUFFER:
                return isRW ? BindingKind::RWStructuredBuffer : BindingKind::StructuredBuffer;
            case SLANG_BYTE_ADDRESS_BUFFER:
                return isRW ? BindingKind::RWByteAddressBuffer : BindingKind::ByteAddressBuffer;
            case SLANG_ACCELERATION_STRUCTURE:
                return BindingKind::AccelerationStructure;
            default:
                return BindingKind::Unknown;
            }
        }

        default:
            return BindingKind::Unknown;
        }
    }

    static ShaderReflection ExtractReflection(slang::ProgramLayout* layout)
    {
        ShaderReflection reflection;
        if (!layout) return reflection;

        const uint32_t paramCount = layout->getParameterCount();
        reflection.bindings.reserve(paramCount);

        for (uint32_t i = 0; i < paramCount; i++)
        {
            slang::VariableLayoutReflection* param = layout->getParameterByIndex(i);
            slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();

            BindingReflection binding;

            if (const char* n = param->getName())
            {
                binding.name = n;
            }

            if (const char* n = typeLayout->getName())
            {
                binding.typeName = n;
            }

            binding.binding = param->getBindingIndex();
            binding.set = param->getBindingSpace();
            binding.kind = DetermineBindingKind(typeLayout, param->getCategory());

            ExtractUserAttributes(binding.annotations, param->getVariable());

            slang::TypeLayoutReflection* elementTypeLayout = typeLayout->getElementTypeLayout();

            switch (binding.kind)
            {
            case BindingKind::ConstantBuffer:
            case BindingKind::PushConstant:
            case BindingKind::ParameterBlock:
                if (elementTypeLayout)
                {
                    binding.size = static_cast<uint32_t>(elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM));
                    binding.fields = ExtractStructFields(elementTypeLayout);

                    if (binding.typeName.empty())
                    {
                        if (const char* n = elementTypeLayout->getName())
                        {
                            binding.typeName = n;
                        }
                    }
                }

                break;

            case BindingKind::StructuredBuffer:
            case BindingKind::RWStructuredBuffer:
                if (elementTypeLayout)
                {
                    binding.stride = static_cast<uint32_t>(elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM));
                    binding.fields = ExtractStructFields(elementTypeLayout);

                    if (binding.typeName.empty())
                    {
                        if (const char* n = elementTypeLayout->getName())
                        {
                            binding.typeName = n;
                        }
                    }
                }

                break;

            default:
                break;
            }

            reflection.bindings.push_back(std::move(binding));
        }

        return reflection;
    }

    struct ShaderCompiler::Impl
    {
        Slang::ComPtr<slang::IGlobalSession> globalSession;

        Impl()
        {
            slang::createGlobalSession(globalSession.writeRef());
        }

        Slang::ComPtr<slang::ISession> CreateSession(
            ShaderTarget target,
            const char* searchPath,
            bool wholeProgram,
            std::string& outDiag
        ) const
        {
            slang::TargetDesc targetDesc {};
            targetDesc.format = TargetFormat(target);
            targetDesc.profile = globalSession->findProfile(TargetProfile(target));
            if (wholeProgram)
            {
                targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;
            }

            slang::SessionDesc sessionDesc {};
            sessionDesc.targets = &targetDesc;
            sessionDesc.targetCount = 1;

            std::vector<slang::CompilerOptionEntry> compilerOptions;

            if (target == ShaderTarget::Vulkan_SPIRV)
            {
                compilerOptions.push_back({ slang::CompilerOptionName::VulkanUseEntryPointName, { .intValue0 = 1 }});
                compilerOptions.push_back({ slang::CompilerOptionName::EmitSpirvDirectly, { .intValue0 = 1 }});
                compilerOptions.push_back({ slang::CompilerOptionName::MatrixLayoutRow, { .intValue0 = 0 }});
                compilerOptions.push_back({ slang::CompilerOptionName::MatrixLayoutColumn, { .intValue0 = 1 }});
            }

            sessionDesc.compilerOptionEntryCount = compilerOptions.size();
            sessionDesc.compilerOptionEntries = compilerOptions.data();

            const char* searchPaths[] = { searchPath };
            if (searchPath)
            {
                sessionDesc.searchPaths = searchPaths;
                sessionDesc.searchPathCount = 1;
            }

            Slang::ComPtr<slang::ISession> session;
            if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
            {
                outDiag = "Failed to create Slang session.";
            }

            return session;
        }

        slang::IModule* LoadModule(
            slang::ISession* session,
            std::string_view source,
            std::string_view moduleName,
            std::string_view virtualPath,
            std::string& outDiag
        )
        {
            Slang::ComPtr<slang::IBlob> diagBlob;
            slang::IModule* slangModule = session->loadModuleFromSourceString(
                std::string(moduleName).c_str(),
                std::string(virtualPath).c_str(),
                std::string(source).c_str(),
                diagBlob.writeRef()
            );

            AppendDiagnostics(outDiag, diagBlob);

            return slangModule;
        }

        CompileResult Compile(
            std::string_view source,
            std::string_view moduleName,
            std::string_view virtualPath,
            std::string_view entryPointName,
            ShaderTarget target,
            const char* searchPath
        )
        {
            CompileResult result;

            const auto session = CreateSession(target, searchPath, false, result.diagnostics);
            if (!session) return result;

            slang::IModule* slangModule = LoadModule(session, source, moduleName, virtualPath, result.diagnostics);
            if (!slangModule) return result;

            Slang::ComPtr<slang::IBlob> diagBlob;

            Slang::ComPtr<slang::IEntryPoint> entryPoint;
            if (SLANG_FAILED(slangModule->findEntryPointByName(std::string(entryPointName).c_str(), entryPoint.writeRef())))
            {
                if (!result.diagnostics.empty()) result.diagnostics += '\n';
                result.diagnostics += "Entry point not found: " + std::string(entryPointName);
                return result;
            }

            slang::IComponentType* components[] = { slangModule, entryPoint.get() };
            Slang::ComPtr<slang::IComponentType> program;
            if (SLANG_FAILED(session->createCompositeComponentType(components, 2, program.writeRef(), diagBlob.writeRef())))
            {
                AppendDiagnostics(result.diagnostics, diagBlob);
                return result;
            }

            Slang::ComPtr<slang::IComponentType> linkedProgram;
            if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagBlob.writeRef())))
            {
                AppendDiagnostics(result.diagnostics, diagBlob);
                return result;
            }

            Slang::ComPtr<slang::IBlob> codeBlob;
            if (SLANG_FAILED(linkedProgram->getEntryPointCode(0, 0, codeBlob.writeRef(), diagBlob.writeRef())))
            {
                AppendDiagnostics(result.diagnostics, diagBlob);
                return result;
            }

            AppendDiagnostics(result.diagnostics, diagBlob);

            slang::ProgramLayout* layout = linkedProgram->getLayout(0, diagBlob.writeRef());
            AppendDiagnostics(result.diagnostics, diagBlob);

            result.entryPoints.emplace_back(entryPointName);
            result.reflection = ExtractReflection(layout);
            FillBinaryResult(result, codeBlob);
            return result;
        }

        CompileResult ReflectOnly(
            std::string_view source,
            std::string_view moduleName,
            std::string_view virtualPath,
            const char* searchPath
        )
        {
            CompileResult result;

            const auto session = CreateSession(ShaderTarget::Vulkan_SPIRV, searchPath, false, result.diagnostics);
            if (!session) return result;

            slang::IModule* slangModule = LoadModule(session, source, moduleName, virtualPath, result.diagnostics);
            if (!slangModule) return result;

            Slang::ComPtr<slang::IBlob> diagBlob;

            slang::IComponentType* components[] = { slangModule };
            Slang::ComPtr<slang::IComponentType> program;
            if (SLANG_FAILED(session->createCompositeComponentType(components, 1, program.writeRef(), diagBlob.writeRef())))
            {
                AppendDiagnostics(result.diagnostics, diagBlob);
                return result;
            }

            Slang::ComPtr<slang::IComponentType> linkedProgram;
            if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagBlob.writeRef())))
            {
                AppendDiagnostics(result.diagnostics, diagBlob);
                return result;
            }

            slang::ProgramLayout* layout = linkedProgram->getLayout(0, diagBlob.writeRef());
            AppendDiagnostics(result.diagnostics, diagBlob);

            if (layout)
            {
                result.reflection = ExtractReflection(layout);
                result.success = true;
            }

            return result;
        }

        CompileResult CompileAll(
            std::string_view source,
            std::string_view moduleName,
            std::string_view virtualPath,
            ShaderTarget target,
            const char* searchPath
        )
        {
            CompileResult result;

            const auto session = CreateSession(target, searchPath, true, result.diagnostics);
            if (!session) return result;

            slang::IModule* slangModule = LoadModule(session, source, moduleName, virtualPath, result.diagnostics);
            if (!slangModule) return result;

            const int entrypointCount = slangModule->getDefinedEntryPointCount();
            if (entrypointCount == 0)
            {
                result.diagnostics += "No entry points found in module.";
                return result;
            }

            std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints(entrypointCount);
            std::vector<slang::IComponentType*> components;
            components.reserve(entrypointCount + 1);
            components.push_back(slangModule);

            for (int i = 0; i < entrypointCount; i++)
            {
                slangModule->getDefinedEntryPoint(i, entryPoints[i].writeRef());
                components.push_back(entryPoints[i].get());
            }

            Slang::ComPtr<slang::IBlob> diagBlob;

            Slang::ComPtr<slang::IComponentType> program;
            if (SLANG_FAILED(session->createCompositeComponentType(
                components.data(), static_cast<SlangInt>(components.size()),
                program.writeRef(), diagBlob.writeRef())))
            {
                AppendDiagnostics(result.diagnostics, diagBlob);
                return result;
            }

            Slang::ComPtr<slang::IComponentType> linkedProgram;
            if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagBlob.writeRef())))
            {
                AppendDiagnostics(result.diagnostics, diagBlob);
                return result;
            }

            slang::ProgramLayout* layout = linkedProgram->getLayout(0, diagBlob.writeRef());
            AppendDiagnostics(result.diagnostics, diagBlob);

            if (layout)
            {
                const SlangUInt reflectedCount = layout->getEntryPointCount();
                result.entryPoints.reserve(reflectedCount);

                for (SlangUInt i = 0; i < reflectedCount; i++)
                {
                    result.entryPoints.emplace_back(layout->getEntryPointByIndex(i)->getName());
                }

                result.reflection = ExtractReflection(layout);
            }

            Slang::ComPtr<slang::IBlob> codeBlob;
            if (SLANG_FAILED(linkedProgram->getTargetCode(0, codeBlob.writeRef(), diagBlob.writeRef())))
            {
                AppendDiagnostics(result.diagnostics, diagBlob);
                return result;
            }

            AppendDiagnostics(result.diagnostics, diagBlob);

            FillBinaryResult(result, codeBlob);
            return result;
        }
    };

    ShaderCompiler::ShaderCompiler() : impl(std::make_unique<Impl>()) {}
    ShaderCompiler::~ShaderCompiler() = default;

    CompileResult ShaderCompiler::CompileFile(
        const std::filesystem::path& _filePath,
        std::string_view _entryPoint,
        ShaderTarget _target
    ) const
    {
        CompileResult err;
        std::string source = ReadFile(_filePath, err.diagnostics);
        if (!err.diagnostics.empty()) return err;

        const std::string searchPath = _filePath.parent_path().string();
        return impl->Compile(
            source,
            _filePath.stem().string(),
            _filePath.filename().string(),
            _entryPoint,
            _target,
            searchPath.c_str()
        );
    }

    CompileResult ShaderCompiler::CompileString(
        std::string_view _source,
        std::string_view _moduleName,
        std::string_view _entryPoint,
        ShaderTarget _target
    ) const
    {
        const std::string virtualPath = std::string(_moduleName) + ".slang";
        return impl->Compile(_source, _moduleName, virtualPath, _entryPoint, _target, nullptr);
    }

    CompileResult ShaderCompiler::CompileAllFile(
        const std::filesystem::path& _filePath,
        ShaderTarget _target
    ) const
    {
        CompileResult err;
        std::string source = ReadFile(_filePath, err.diagnostics);
        if (!err.diagnostics.empty()) return err;

        const std::string searchPath = _filePath.parent_path().string();
        return impl->CompileAll(
            source,
            _filePath.stem().string(),
            _filePath.filename().string(),
            _target,
            searchPath.c_str()
        );
    }

    CompileResult ShaderCompiler::ReflectFile(
        const std::filesystem::path& _filePath
    ) const
    {
        CompileResult err;
        std::string source = ReadFile(_filePath, err.diagnostics);
        if (!err.diagnostics.empty()) return err;

        const std::string searchPath = _filePath.parent_path().string();
        return impl->ReflectOnly(
            source,
            _filePath.stem().string(),
            _filePath.filename().string(),
            searchPath.c_str()
        );
    }

    CompileResult ShaderCompiler::CompileAllString(
        std::string_view _source,
        std::string_view _moduleName,
        ShaderTarget _target
    ) const
    {
        const std::string virtualPath = std::string(_moduleName) + ".slang";
        return impl->CompileAll(_source, _moduleName, virtualPath, _target, nullptr);
    }
}
