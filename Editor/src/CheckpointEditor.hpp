#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace cp {
	enum class EditorVersionType : uint8_t {
		Alpha = 0,
		Beta = 1,
		ReleaseCandidate = 2,
		Release = 3
	};

	struct EditorVersion : uint32_t {
		EditorVersionType type : 2;
		uint8_t major : 8;
		uint8_t minor : 8;
		uint16_t patch : 14;

		constexpr EditorVersion(EditorVersionType type, uint8_t major, uint8_t minor, uint16_t patch)
			: type(type), major(major), minor(minor), patch(patch) {
		}

		constexpr operator uint32_t() const {
			return MAKE_EDITOR_VERSION(static_cast<uint32_t>(type), major, minor, patch);
		}
	};

	struct Project {
		std::string name;
		std::string path;
		uint64_t creationDate;
		uint64_t lastOpened;
		EditorVersion engineVersion;

		std::string GetProjectPath() {
			return Project::data.path.toStdString();
		}

		std::string GetResourcePath() {
			return Project::GetProjectPath() + "/Resources";
		}

		std::string GetResourceRelativePath(const std::string& _resource) {
			return _resource.substr(Project::GetResourcePath().size());
		}

		void FromJson(const nlohmann::json& obj) {
			name = obj["name"].get<std::string>();
			path = obj["path"].get<std::string>();
			creationDate = obj["creationDate"].get<uint64_t>();
			lastOpened = obj["lastOpened"].get<uint64_t>();
			engineVersion = EditorVersion(
				static_cast<EditorVersionType>(obj["engineVersion"]["type"].get<uint8_t>()),
				obj["engineVersion"]["major"].get<uint8_t>(),
				obj["engineVersion"]["minor"].get<uint8_t>(),
				obj["engineVersion"]["patch"].get<uint16_t>()
			);
		}

		nlohmann::json ToJson() const {
			nlohmann::json obj;
			obj["name"] = name;
			obj["path"] = path;
			obj["creationDate"] = creationDate;
			obj["lastOpened"] = lastOpened;
			obj["engineVersion"] = {
				{"type", static_cast<uint8_t>(engineVersion.type)},
				{"major", engineVersion.major},
				{"minor", engineVersion.minor},
				{"patch", engineVersion.patch}
			};
			return obj;
		}
	};

	struct CheckpointEditor {
		static constexpr EditorVersion CurrentVersion = EditorVersion(EditorVersionType::Alpha, 0, 1, 0);
		static Project CurrentProject;

		static bool IsProjectUpToDate(const Project& project) {
			return project.engineVersion >= CurrentVersion;
		}


	};
}

cp::Project cp::CheckpointEditor::CurrentProject = {};