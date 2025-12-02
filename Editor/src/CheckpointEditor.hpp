#pragma once

#include "pch.hpp"
#include <nlohmann/json.hpp>

namespace cp {
	enum class EditorVersionType : uint8_t {
		Alpha = 0,
		Beta = 1,
		ReleaseCandidate = 2,
		Release = 3
	};

	struct EditorVersion {
		EditorVersionType type : 2;
		uint8_t major : 8;
		uint8_t minor : 8;
		uint16_t patch : 14;

		constexpr EditorVersion() : type(EditorVersionType::Alpha), major(0), minor(0), patch(0) {
		}

		constexpr EditorVersion(EditorVersionType type, uint8_t major, uint8_t minor, uint16_t patch)
			: type(type), major(major), minor(minor), patch(patch) {
		}

		constexpr uint32_t ToUint32() const {
			return (static_cast<uint32_t>(type) << 30) | (static_cast<uint32_t>(major) << 22) | (static_cast<uint32_t>(minor) << 14) | static_cast<uint32_t>(patch);
		}

		constexpr bool operator>=(const EditorVersion& other) const {
			return ToUint32() >= other.ToUint32();
		}
	};

	struct Project {
		std::string name;
		std::string path;
		uint64_t creationDate;
		uint64_t lastOpened;
		EditorVersion engineVersion;

		std::string GetProjectPath() {
			return path;
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
				{"type", static_cast<uint32_t>(engineVersion.type)},
				{"major", static_cast<uint32_t>(engineVersion.major)},
				{"minor", static_cast<uint32_t>(engineVersion.minor)},
				{"patch", static_cast<uint32_t>(engineVersion.patch)}
			};
			return obj;
		}
	};

	struct CheckpointEditor {
		static cp::VulkanContext VulkanCtx;
		static constexpr EditorVersion CurrentVersion{ EditorVersionType::Alpha, 0, 1, 0 };
		static Project CurrentProject;

		static bool IsProjectUpToDate(const Project& project) {
			return project.engineVersion >= CurrentVersion;
		}

		static void SetupVulkanContext() {
			cp::VulkanContextInfo contextInfo;
			contextInfo.appName = "Checkpoint Editor"; // Application name
			contextInfo.appVersion = VK_MAKE_API_VERSION(0, 1, 0, 0); // Application version
			contextInfo.extensions.instanceExtensions = {
				VK_KHR_SURFACE_EXTENSION_NAME,
				#ifdef _WIN32
				"VK_KHR_win32_surface",
				#endif
				#ifdef __linux__
				"VK_KHR_xcb_surface",
				#endif
			};

			cp::CheckpointEditor::VulkanCtx.Initialize(contextInfo);
		}

		static void LoadProject(const std::string& projectPath) {
			std::ifstream file(projectPath + "/project.data");

			if (!file.is_open()) {
				throw std::runtime_error("Failed to open project file: " + projectPath + "/project.data");
			}

			nlohmann::json projectJson;
			file >> projectJson;

			CurrentProject.FromJson(projectJson);

			file.close();

			CurrentProject.lastOpened = static_cast<uint64_t>(std::time(nullptr));

			std::ofstream outFile(projectPath + "/project.data");

			if (!outFile.is_open()) {
				throw std::runtime_error("Failed to open project file for writing: " + projectPath + "/project.data");
			}

			outFile << CurrentProject.ToJson().dump(4);
			outFile.close();
		}
	};
}