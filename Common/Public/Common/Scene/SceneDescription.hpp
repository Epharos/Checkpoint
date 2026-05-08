#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cp::scene
{
    struct SceneGuid
    {
        uint64_t high = 0;
        uint64_t low = 0;

        bool operator==(const SceneGuid&) const = default;
    };

    struct SystemsConfig
    {
        std::vector<std::string> enabledSystemGuids;
    };

    struct SceneDescription
    {
        static constexpr uint32_t MAGIC = 0x53434E45; // "SCNE"
        static constexpr uint32_t VERSION = 1;

        std::string name;
        SceneGuid sceneGuid;
        uint32_t version = VERSION;
        uint64_t lastModified = 0;

        SystemsConfig systemsConfig;
        std::vector<std::string> activePassNames;
    };
}
