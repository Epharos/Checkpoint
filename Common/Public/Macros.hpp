#pragma once

// 0 Alpha, 1 Beta, 2 Release Candidate, 3 Release
#define CP_MAKE_VERSION(type, major, minor, patch) ((type << 30) | (major << 22) | (minor << 14) | (patch))

#define CP_VERSION_TYPE(version) ((version >> 30) & 0x3)
#define CP_VERSION_MAJOR(version) ((version >> 22) & 0xFF)
#define CP_VERSION_MINOR(version) ((version >> 14) & 0xFF)
#define CP_VERSION_PATCH(version) (version & 0x3FFF)

#define CP_VERSION CP_MAKE_VERSION(0, 1, 0, 0) // Alpha 1.0.0