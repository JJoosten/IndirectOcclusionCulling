// OpenGL related configuration (Direct State Access usage toggling)
//#define CFC_CONF_RENDERER_GL_BUFFER_DSA
//#define CFC_CONF_RENDERER_GL_SHADER_DSA

// Add safety checks for array lookups and buffer overflows in various places in the engine.
#define CFC_CONF_SAFETY

// Enable/disable logging
#define CFC_CONF_LOGGING

// Enable/disable profiling
#define CFC_CONF_PROFILING_ENABLED

// Run unit tests
#define CFC_CONF_RUNTIME_TESTS				

// Enable memory leak detection (VLD: Visual Leak Detector, only for visual studio)
//#define CFC_CONF_ENABLE_VLD

// Debug memory tracking - shows which objects through the memory allocator are still alive, and where they have been allocated (through signature).
// Warning: Slows things down. Every allocation accesses a std::map with allocation information. Use only for debug builds!
//#define CFC_CONF_ALLOCATOR_TRACKER