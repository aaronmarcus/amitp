#pragma once

//MF headers
#include <mfapi.h>
#include <mfidl.h>
#include <Mferror.h>

//MF libraries
#pragma comment(lib, "mfplat.lib")

//structure to describe the source format
struct sourceFormatDescriptor
{
	unsigned __int32 width;
	unsigned __int32 height;
	unsigned __int32 bitDepth;
	
};


class Renderer
{
	Renderer(unsigned __int32 width, unsigned __int32 height, unsigned __int32 bitDepth)
	{
		sourceFormatDescriptor sourceFormat = {width, height, bitDepth};
	}
	HRESULT hr = S_OK;

public:
	
	
	
};