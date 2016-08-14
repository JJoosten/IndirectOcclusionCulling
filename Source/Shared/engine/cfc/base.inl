
#include <cfc/stl/stl_common.hpp>

CFC_NAMESPACE1(cfc)

#ifdef CFC_ENVIRONMENT_64BIT
static const usize invalid_index = ~(0ULL);
#else
static const usize invalid_index = ~(0UL);
#endif

class CFC_API object
{
public:
	object();
	virtual ~object();

	virtual const char* GetTypeString() { return "object"; }
	static int GetNumberOfObjectsAlive();
protected:
	object& operator =(const object& o) = delete;
	object(const object& o) = delete;
};

CFC_END_NAMESPACE1(cfc)