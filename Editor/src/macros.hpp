#ifdef BUILDING_PLUGIN_LOADER
#define EDITOR_API __declspec(dllexport)
#else
#define EDITOR_API
#endif

//#define NO_COPY(ClassName)            \
//	ClassName(const ClassName&) = delete; \
//	ClassName& operator=(const ClassName&) = delete;
//
//#define NO_MOVE(ClassName)            \
//	ClassName(ClassName&&) = delete;      \
//	ClassName& operator=(ClassName&&) = delete;