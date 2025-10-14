#include "pch.hpp"
#include "SlangCompiler.hpp"

#include <spirv-tools/linker.hpp>

#include "Resources/Material.hpp"

namespace cp
{
	ShaderResourceKind SlangCompiler::DetermineResourceKind(TypeLayoutReflection* layout)
	{
		switch (layout->getKind())
		{
		case TypeReflection::Kind::ConstantBuffer:
			return ShaderResourceKind::ConstantBuffer;
		case TypeReflection::Kind::Resource:
		case TypeReflection::Kind::ShaderStorageBuffer:
		case TypeReflection::Kind::ParameterBlock:
		{
			switch (layout->getResourceShape())
			{
			case SlangResourceShape::SLANG_STRUCTURED_BUFFER:
				return ShaderResourceKind::StructuredBuffer;
			case SlangResourceShape::SLANG_TEXTURE_1D:
			case SlangResourceShape::SLANG_TEXTURE_2D:
			case SlangResourceShape::SLANG_TEXTURE_3D:
			case SlangResourceShape::SLANG_TEXTURE_CUBE:
			case SlangResourceShape::SLANG_TEXTURE_1D_ARRAY:
			case SlangResourceShape::SLANG_TEXTURE_2D_ARRAY:
			case SlangResourceShape::SLANG_TEXTURE_CUBE_ARRAY:
			case SlangResourceShape::SLANG_TEXTURE_BUFFER:
				return ShaderResourceKind::TextureResource;
			}
			break;
		}
		case TypeReflection::Kind::SamplerState:
			return ShaderResourceKind::Sampler;
		}
	}

	ShaderField SlangCompiler::ExtractFieldInfo(TypeLayoutReflection* typeLayout)
	{
		ShaderField field;

		field.typeName = typeLayout->getName();
		field.size = typeLayout->getSize();
		field.alignment = typeLayout->getAlignment();
		field.stride = typeLayout->getStride();

		if (typeLayout->getKind() == TypeReflection::Kind::Struct)
		{
			for (uint32_t i = 0; i < typeLayout->getFieldCount(); i++)
			{
				auto fieldLayout = typeLayout->getFieldByIndex(i);

				ShaderField subField = ExtractFieldInfo(fieldLayout->getTypeLayout());

				subField.name = fieldLayout->getName();
				subField.offset = fieldLayout->getOffset();

				field.fields.push_back(subField);
			}
		}
		else if (typeLayout->getKind() == TypeReflection::Kind::Vector)
		{
			field.vectorType = typeLayout->getElementTypeLayout()->getName();
			field.vectorSize = typeLayout->getElementCount();
		}
		else if (typeLayout->getKind() == TypeReflection::Kind::Matrix)
		{
			field.matrixType = typeLayout->getElementTypeLayout()->getElementTypeLayout()->getName();
			field.matrixRows = typeLayout->getRowCount();
			field.matrixColumns = typeLayout->getColumnCount();
		}

		return field;
	}

	SlangCompiler::SlangCompiler()
	{
		if (createGlobalSession(globalSession.writeRef()) != SLANG_OK)
			//Note : The global session is not thread-safe as of now, for each thread used to compile, we need to create a new one
		{
			LOG_ERROR("Failed to create global session");
			throw std::runtime_error("Failed to create global session");
		}
	}

	bool SlangCompiler::CompileMaterialSlangToSpirV(cp::Material& _material)
	{
		SessionDesc sessionDesc;

		TargetDesc targetDesc;
		targetDesc.format = SLANG_SPIRV;
		targetDesc.profile = globalSession->findProfile("spirv_1_6");

		CompilerOptionEntry compilerOptions[] = {
			{ CompilerOptionName::VulkanUseEntryPointName, {.intValue0 = 1 } },
			{ CompilerOptionName::EmitSpirvDirectly, {.intValue0 = 1 } }
		};

		sessionDesc.compilerOptionEntries = compilerOptions;
		sessionDesc.compilerOptionEntryCount = 2;
		sessionDesc.targets = &targetDesc;
		sessionDesc.targetCount = 1;

		//std::string shaderModulePath = Project::GetProjectPath() + "/ShaderModules";
		//const char* searchPaths[] = { shaderModulePath.c_str() }; //TODO : Add user defined search paths
		//sessionDesc.searchPaths = searchPaths;
		//sessionDesc.searchPathCount = 1;

		PreprocessorMacroDesc macros[] = { { "CP_DEBUG", "1" } }; //TODO : Add user defined macros
		sessionDesc.preprocessorMacros = macros;
		sessionDesc.preprocessorMacroCount = 1;

		ComPtr<ISession> session;
		if (globalSession->createSession(sessionDesc, session.writeRef()) != SLANG_OK)
		{
			LOG_ERROR("Failed to create session");
			return false;
		}

		ComPtr<IModule> module;
		{
			ComPtr<IBlob> diagnostics;
			const char* moduleName = _material.GetName().c_str();
			const char* modulePath = _material.GetShaderPath().c_str();
			std::string source = Helper::File::FileContentToString(_material.GetShaderPath());
			module = session->loadModuleFromSourceString(moduleName, modulePath, source.c_str(), diagnostics.writeRef());

			if (diagnostics)
			{
				LOG_ERROR("Failed to load module: " + std::string(static_cast<const char*>(diagnostics->getBufferPointer())));
			}

			if (!module)
			{
				LOG_ERROR("Failed to load module");
				return false;
			}
		}

		const uint32_t entryPointCount = module->getDefinedEntryPointCount();
		std::vector<IEntryPoint*> entryPoints(entryPointCount);

		for (uint32_t i = 0; i < entryPointCount; i++)
		{
			ComPtr<IEntryPoint> entryPoint;
			module->getDefinedEntryPoint(i, entryPoint.writeRef());
			entryPoints[i] = entryPoint.get();
		}

		std::vector<IComponentType*> entryPointsArray(entryPointCount + 1);

		entryPointsArray[0] = module;

		for (uint32_t i = 1; i <= entryPointCount; i++)
		{
			entryPointsArray[i] = entryPoints[i - 1];
		}

		LOG_INFO(MF("Compiling with ", entryPointCount, " entry points"));

		ComPtr<IComponentType> composed;
		{
			ComPtr<IBlob> diagnostics;
			SlangResult result = session->createCompositeComponentType(entryPointsArray.data(), entryPointCount + 1, composed.writeRef(), diagnostics.writeRef());

			if (diagnostics)
			{
				LOG_ERROR("Failed to create component type: " + std::string(static_cast<const char*>(diagnostics->getBufferPointer())));
			}

			SLANG_RETURN_FALSE_ON_FAIL(result);
		}

		ComPtr<IComponentType> linkedProgram;
		{
			ComPtr<IBlob> diagnostics;
			SlangResult result = composed->link(linkedProgram.writeRef(), diagnostics.writeRef());

			if (diagnostics)
			{
				LOG_ERROR("Failed to link program: " + std::string(static_cast<const char*>(diagnostics->getBufferPointer())));
			}

			SLANG_RETURN_FALSE_ON_FAIL(result);
		}

		ComPtr<IBlob> compiledCode;
		{
			ComPtr<IBlob> diagnostics;
			SlangResult result = linkedProgram->getTargetCode(0, compiledCode.writeRef(), diagnostics.writeRef());

			if (diagnostics)
			{
				LOG_ERROR("Failed to get compiled code: " + std::string(static_cast<const char*>(diagnostics->getBufferPointer())));
			}

			SLANG_RETURN_FALSE_ON_FAIL(result);
		}

		LOG_INFO(MF("Defined entry points: ", module->getDefinedEntryPointCount()));

		for (uint32_t i = 0; i < module->getDefinedEntryPointCount(); i++)
		{
			ComPtr<IEntryPoint> entryPoint;
			module->getDefinedEntryPoint(i, entryPoint.writeRef());
			LOG_INFO(MF("Entry point name: ", entryPoint->getFunctionReflection()->getName()));
		}

		std::string outputPath = _material.GetShaderPath();
		outputPath = outputPath.substr(0, outputPath.find_last_of('.')) + ".spv";
		std::ofstream outputFile(outputPath, std::ios::binary);
		if (!outputFile.is_open())
		{
			LOG_ERROR("Failed to open output file");
			return false;
		}

		size_t codeSize = compiledCode->getBufferSize();
		const char* codeData = static_cast<const char*>(compiledCode->getBufferPointer());

		outputFile.write(codeData, codeSize);
		outputFile.close();

		LOG_INFO("Compiled code written to " + outputPath);

		//DEBUG LOG INFOS

		ShaderReflection* shaderReflection = new ShaderReflection();
		ProgramLayout* programLayout = linkedProgram->getLayout(0);

		LOG_INFO(MF("Compiled shader has ", programLayout->getParameterCount(), " parameters"));

		for (unsigned int i = 0; i < programLayout->getParameterCount(); i++)
		{
			VariableLayoutReflection* paramLayout = programLayout->getParameterByIndex(i);
			//LOG_INFO(MF("\n", Helper::Slang::VariableToString(paramLayout, 0), "\n"));

			ShaderResource resource;
			resource.name = paramLayout->getName();
			resource.binding = paramLayout->getBindingIndex();
			resource.set = paramLayout->getBindingSpace();
			resource.typeName = paramLayout->getTypeLayout()->getName();
			resource.kind = DetermineResourceKind(paramLayout->getTypeLayout());

			if(paramLayout->getTypeLayout()->getElementTypeLayout())
				resource.field = ExtractFieldInfo(paramLayout->getTypeLayout()->getElementTypeLayout());

			shaderReflection->resources.push_back(resource);
		}

		LOG_INFO(MF("Shader has ", programLayout->getEntryPointCount(), " entry points"));

		auto EntryPointStageName = [](SlangStage stage) -> std::string
			{
				switch (stage)
				{
				case SlangStage::SLANG_STAGE_COMPUTE:
					return "Compute";
				case SlangStage::SLANG_STAGE_VERTEX:
					return "Vertex";
				case SlangStage::SLANG_STAGE_FRAGMENT:
					return "Fragment";
				case SlangStage::SLANG_STAGE_GEOMETRY:
					return "Geometry";
				case SlangStage::SLANG_STAGE_HULL:
					return "Hull";
				case SlangStage::SLANG_STAGE_DOMAIN:
					return "Domain";
				default:
					return "Unknown";
				}
			};

		auto EntryPointShaderStage = [](SlangStage stage) -> cp::ShaderStages
			{
				switch (stage)
				{
				case SlangStage::SLANG_STAGE_COMPUTE:
					return cp::ShaderStages::Compute;
				case SlangStage::SLANG_STAGE_VERTEX:
					return cp::ShaderStages::Vertex;
				case SlangStage::SLANG_STAGE_FRAGMENT:
					return cp::ShaderStages::Fragment;
				case SlangStage::SLANG_STAGE_GEOMETRY:
					return cp::ShaderStages::Geometry;
				case SlangStage::SLANG_STAGE_HULL:
					return cp::ShaderStages::TessellationControl;
				case SlangStage::SLANG_STAGE_DOMAIN:
					return cp::ShaderStages::TessellationEvaluation;
				default:
					return cp::ShaderStages(0);
				}
			};

		for (unsigned int i = 0; i < programLayout->getEntryPointCount(); i++)
		{
			EntryPointReflection* entryPointLayout = programLayout->getEntryPointByIndex(i);

			EntryPoint entryPoint;
			entryPoint.name = entryPointLayout->getName();
			entryPoint.stage = EntryPointShaderStage(entryPointLayout->getStage());
			shaderReflection->entryPoints.push_back(entryPoint);
		}

		if (_material.GetShaderReflection() != nullptr)
		{
			delete _material.GetShaderReflection();
		}

		_material.SetShaderReflection(shaderReflection);

		return true;
	}

#ifdef IN_EDITOR
	QWidget* SlangCompiler::CreateFieldWidget(const ShaderField& field, QWidget* parent)
	{
		QWidget* widget = new QWidget(parent);
		QVBoxLayout* fieldLayout = new QVBoxLayout(widget);
		//widget->setLayout(fieldLayout);
		fieldLayout->setContentsMargins(8, 0, 0, 0);
		fieldLayout->setSpacing(0);

		if (!field.name.empty() && !field.typeName.empty())
		{
			QWidget* nameTypeWidget = new QWidget(widget);
			QHBoxLayout* nameTypeLayout = new QHBoxLayout(nameTypeWidget);
			nameTypeLayout->setContentsMargins(0, 0, 0, 0);
			nameTypeLayout->setSpacing(0);
			nameTypeWidget->setLayout(nameTypeLayout);
			QLabel* nameLabel = new QLabel(QString::fromStdString(field.name), widget);
			nameLabel->setStyleSheet("font-weight: bold;");
			nameTypeLayout->addWidget(nameLabel);
			QLabel* typeLabel = new QLabel(QString::fromStdString(" : " + field.typeName), widget);
			typeLabel->setStyleSheet("color : #aaa;");
			nameTypeLayout->addWidget(typeLabel);
			
			if (field.typeName.compare("vector") == 0)
			{
				typeLabel->setText(QString::fromStdString(" : " + field.vectorType + std::to_string(field.vectorSize)));
			}
			else if (field.typeName.compare("matrix") == 0)
			{
				typeLabel->setText(QString::fromStdString(" : " + field.matrixType + std::to_string(field.matrixRows) + "x" + std::to_string(field.matrixColumns)));
			}

			nameTypeLayout->addStretch();

			fieldLayout->addWidget(nameTypeWidget);
		}

		if (field.size > 0 && field.size != ~size_t(0))
		{
			QLabel* sizeLabel = new QLabel(QString::fromStdString("Size: " + std::to_string(field.size)), widget);
			sizeLabel->setStyleSheet("color : #aaa;");
			fieldLayout->addWidget(sizeLabel);
		}

		if(field.offset != -1)
		{
			QLabel* offsetLabel = new QLabel(QString::fromStdString("Offset: " + std::to_string(field.offset)), widget);
			offsetLabel->setStyleSheet("color : #aaa;");
			fieldLayout->addWidget(offsetLabel);
		}

		if (field.alignment > 0)
		{
			QLabel* alignmentLabel = new QLabel(QString::fromStdString("Alignment: " + std::to_string(field.alignment)), widget);
			alignmentLabel->setStyleSheet("color : #aaa;");
			fieldLayout->addWidget(alignmentLabel);
		}
			
		if (field.stride > 0)
		{
			QLabel* strideLabel = new QLabel(QString::fromStdString("Stride: " + std::to_string(field.stride)), widget);
			strideLabel->setStyleSheet("color : #aaa;");
			fieldLayout->addWidget(strideLabel);
		}

		if (field.fields.size() > 0)
		{
			for (const auto& subField : field.fields)
			{
				QWidget* subFieldWidget = CreateFieldWidget(subField, widget);
				fieldLayout->addWidget(subFieldWidget);
			}
		}

		return widget;
	}

	QWidget* SlangCompiler::CreateResourceWidget(const ShaderResource& resource, QWidget* parent, const bool& _showEngineSets)
	{
		if (!_showEngineSets && resource.set <= 1)
		{
			return nullptr; // Skip engine sets if not requested
		}

		QWidget* widget = new QWidget(parent);
		QVBoxLayout* layout = new QVBoxLayout(widget);

		QLabel* bindingLabel = new QLabel(QString::fromStdString("Binding: " + std::to_string(resource.binding)), widget);
		bindingLabel->setStyleSheet("font-weight: bold;");
		layout->addWidget(bindingLabel);

		if (resource.kind == ShaderResourceKind::ConstantBuffer)
		{
			QLabel* kindLabel = new QLabel("Constant Buffer", widget);
			kindLabel->setStyleSheet("color : #aaa;");
			layout->addWidget(kindLabel);
		}
		else if (resource.kind == ShaderResourceKind::StructuredBuffer)
		{
			QLabel* kindLabel = new QLabel("Structured Buffer", widget);
			kindLabel->setStyleSheet("color : #aaa;");
			layout->addWidget(kindLabel);
		}
		else if (resource.kind == ShaderResourceKind::Sampler)
		{
			QLabel* kindLabel = new QLabel("Sampler", widget);
			kindLabel->setStyleSheet("color : #aaa;");
			layout->addWidget(kindLabel);
		}
		else if (resource.kind == ShaderResourceKind::TextureResource)
		{
			QLabel* kindLabel = new QLabel("Texture", widget);
			kindLabel->setStyleSheet("color : #aaa;");
			layout->addWidget(kindLabel);
		}

		layout->addWidget(CreateFieldWidget(resource.field, widget));

		return widget;
	}
#endif
	void ShaderField::Serialize(ISerializer& _serializer) const
	{
		_serializer.WriteString("name", name);
		_serializer.WriteString("typeName", typeName);
		_serializer.WriteInt("size", size);
		_serializer.WriteInt("offset", offset);
		_serializer.WriteInt("alignment", alignment);
		_serializer.WriteInt("stride", stride);

		_serializer.BeginObjectArrayWriting("fields");
		for (const auto& field : fields)
		{
			_serializer.BeginObjectArrayElementWriting();
			field.Serialize(_serializer);
			_serializer.EndObjectArrayElement();
		}
		_serializer.EndObjectArray();

		if (!vectorType.empty())
		{
			_serializer.WriteString("vectorType", vectorType);
			_serializer.WriteInt("vectorSize", vectorSize);
		}

		if (!matrixType.empty())
		{
			_serializer.WriteString("matrixType", matrixType);
			_serializer.WriteInt("matrixRows", matrixRows);
			_serializer.WriteInt("matrixColumns", matrixColumns);
		}
	}

	void ShaderField::Deserialize(ISerializer& _serializer)
	{
		name = _serializer.ReadString("name", name);
		typeName = _serializer.ReadString("typeName", typeName);
		size = _serializer.ReadInt("size", size);
		offset = _serializer.ReadInt("offset", offset);
		alignment = _serializer.ReadInt("alignment", alignment);
		stride = _serializer.ReadInt("stride", stride);

		size_t fieldCount = _serializer.BeginObjectArrayReading("fields");
		for (size_t i = 0; i < fieldCount; i++)
		{
			if (_serializer.BeginObjectArrayElementReading(i))
			{
				ShaderField field;
				field.Deserialize(_serializer);
				fields.push_back(field);
				_serializer.EndObjectArrayElement();
			}
		}
		_serializer.EndObjectArray();

		vectorType = _serializer.ReadString("vectorType", "");
		vectorSize = _serializer.ReadInt("vectorSize", 0);
		matrixType = _serializer.ReadString("matrixType", "");
		matrixRows = _serializer.ReadInt("matrixRows", 0);
		matrixColumns = _serializer.ReadInt("matrixColumns", 0);
	}

	void ShaderResource::Serialize(ISerializer& _serializer) const
	{
		_serializer.WriteString("name", name);
		_serializer.WriteInt("binding", binding);
		_serializer.WriteInt("set", set);
		_serializer.WriteString("typeName", typeName);
		_serializer.WriteInt("kind", static_cast<int>(kind));

		_serializer.BeginObjectWriting("field");
		field.Serialize(_serializer);
		_serializer.EndObject();
	}

	void ShaderResource::Deserialize(ISerializer& _serializer)
	{
		name = _serializer.ReadString("name", name);
		binding = _serializer.ReadInt("binding", binding);
		set = _serializer.ReadInt("set", set);
		typeName = _serializer.ReadString("typeName", typeName);
		kind = static_cast<ShaderResourceKind>(_serializer.ReadInt("kind", static_cast<int>(kind)));

		if (_serializer.BeginObjectReading("field"))
		{
			field.Deserialize(_serializer);
			_serializer.EndObject();
		}
	}

	void ShaderReflection::Serialize(ISerializer& _serializer) const
	{
		_serializer.BeginObjectArrayWriting("resources");
		for (const auto& resource : resources)
		{
			_serializer.BeginObjectArrayElementWriting();
			resource.Serialize(_serializer);
			_serializer.EndObjectArrayElement();
		}
		_serializer.EndObjectArray();

		_serializer.BeginObjectArrayWriting("entryPoints");
		for (const auto& entryPoint : entryPoints)
		{
			_serializer.BeginObjectArrayElementWriting();
			entryPoint.Serialize(_serializer);
			_serializer.EndObjectArrayElement();
		}
		_serializer.EndObjectArray();
	}

	void ShaderReflection::Deserialize(ISerializer& _serializer)
	{
		size_t resourceCount = _serializer.BeginObjectArrayReading("resources");

		for (size_t i = 0; i < resourceCount; i++)
		{
			if (_serializer.BeginObjectArrayElementReading(i))
			{
				ShaderResource resource;
				resource.Deserialize(_serializer);
				resources.push_back(resource);
				_serializer.EndObjectArrayElement();
			}
		}

		_serializer.EndObjectArray();

		size_t entryPointCount = _serializer.BeginObjectArrayReading("entryPoints");

		for (size_t i = 0; i < entryPointCount; i++)
		{
			if (_serializer.BeginObjectArrayElementReading(i))
			{
				EntryPoint entryPoint;
				entryPoint.Deserialize(_serializer);
				entryPoints.push_back(entryPoint);
				_serializer.EndObjectArrayElement();
			}
		}

		_serializer.EndObjectArray();
	}

	void EntryPoint::Serialize(ISerializer& _serializer) const
	{
		_serializer.WriteString("name", name);
		_serializer.WriteInt("stage", static_cast<int>(stage));
	}

	void EntryPoint::Deserialize(ISerializer& _serializer)
	{
		name = _serializer.ReadString("name", name);
		stage = static_cast<cp::ShaderStages>(_serializer.ReadInt("stage", static_cast<int>(stage)));
	}
}