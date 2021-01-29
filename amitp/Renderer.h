#pragma once

#include <d3d9.h>
#include <Dxva2api.h>
#include <evr.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfobjects.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include <iostream>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "evr.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Strmiids")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Dxva2.lib")

//for debugging


//define structure to describe the source format
struct sourceFormatDescriptor
{
	unsigned __int32 width;
	unsigned __int32 height;
	unsigned __int32 bitDepth;
	
};


class Renderer
{
	public:
		//define constructor
		Renderer(unsigned __int32 m_width, unsigned __int32 m_height, unsigned __int32 m_bit_depth, HWND m_hwnd);
};