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

#define COLOR_TO_UINT32(r, g, b, a) (  \
	(static_cast<uint32_t>(r) << 24) | \
	(static_cast<uint32_t>(g) << 16) | \
	(static_cast<uint32_t>(b) << 8)  | \
	(static_cast<uint32_t>(a))         \
)

#define UINT32_TO_R(color) (static_cast<uint8_t>((color >> 24) & 0xFF))
#define UINT32_TO_G(color) (static_cast<uint8_t>((color >> 16) & 0xFF))
#define UINT32_TO_B(color) (static_cast<uint8_t>((color >> 8) & 0xFF))
#define UINT32_TO_A(color) (static_cast<uint8_t>(color & 0xFF))

#define COLOR_AS_HEXA(color) (  \
	"#" +                                 \
	std::to_string(UINT32_TO_R(color)) + \
	std::to_string(UINT32_TO_G(color)) + \
	std::to_string(UINT32_TO_B(color)) + \
	std::to_string(UINT32_TO_A(color))   \
)

#define COLOR_RGB_AS_HEXA(r, g, b) (  \
	"#" +                             \
	std::to_string(r) +               \
	std::to_string(g) +               \
	std::to_string(b)                 \
)