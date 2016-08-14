#pragma warning(push, 0)   

#pragma comment(lib,"ws2_32.lib")

#define MICROPROFILE_GPU_TIMERS_D3D12
#define MICROPROFILE_API __declspec(dllexport)
#define MICROPROFILE_IMPL
#include "microprofile.h"

#pragma warning(pop)