#pragma once

#include <cfc/base.h>

CFC_NAMESPACE1(cfc)

class CFC_API iobuffer : public object
{
public:
	iobuffer() {}
	iobuffer(const iobuffer& o);
	iobuffer& operator= (const iobuffer& o);
	~iobuffer();
	u8* data=nullptr;
	usize size=cfc::invalid_index;

	operator bool() { return size != cfc::invalid_index; }
protected:
	
};

class CFC_API io : public object
{
public:
	virtual u32_64 GetFileSize(const char* path) { return 0; }
	virtual bool Exists(const char* path) { return false; }
	virtual iobuffer ReadFileToMemory(const char* path) { return iobuffer(); }
	virtual bool ShowOpenFileDialog(const char* filter, char* destination, i32 destinationBufferSize) { return false; }
};

CFC_END_NAMESPACE1(cfc)