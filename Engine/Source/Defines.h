#pragma once

#include <assert.h>

#if defined IMZADI_EXPORT
#	define IMZADI_API		__declspec(dllexport)
#elif defined IMZADI_IMPORT
#	define IMZADI_API		__declspec(dllimport)
#else
#	define IMZADI_API
#endif

#define IMZADI_MIN(a, b)			((a) < (b) ? (a) : (b))
#define IMZADI_MAX(a, b)			((a) > (b) ? (a) : (b))
#define IMZADI_CLAMP(x, a, b)		IMZADI_MAX(IMZADI_MIN(x, b), a)
#define IMZADI_SQUARED(x)			((x) * (x))
#define IMZADI_SIGN(x)				((x) < 0.0 ? -1.0 : 1.0)
#define IMZADI_DEGS_TO_RADS(x)		((x) * (180.0 / M_PI))
#define IMZADI_RADS_TO_DEGS(x)		((x) * (M_PI / 180.0))
#define IMZADI_IS_POW_TWO(x)		((x & (x - 1)) == 0)

#define IMZADI_DRAW_FLAG_SHAPES				0x00000001
#define IMZADI_DRAW_FLAG_SHAPE_BOXES		0x00000002
#define IMZADI_DRAW_FLAG_AABB_TREE			0x00000004

#define IMZADI_ASSERT(condition)			assert(condition)

#define IMZADI_ADD_FLAG_ALLOW_SPLIT			0x00000001

#define IMZADI_MIN_NODE_VOLUME				(50.0 * 50.0 * 50.0)

#define IMZADI_AXIS_FLAG_X					0x00000001
#define IMZADI_AXIS_FLAG_Y					0x00000002
#define IMZADI_AXIS_FLAG_Z					0x00000004

template<typename T>
void IMZADI_API SafeRelease(T& thing)
{
	if (thing)
	{
		thing->Release();
		thing = nullptr;
	}
}