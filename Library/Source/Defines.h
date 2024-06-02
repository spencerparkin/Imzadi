#pragma once

#include <assert.h>

#if defined COLLISION_LIB_EXPORT
#	define COLLISION_LIB_API		__declspec(dllexport)
#elif defined COLLISION_LIB_IMPORT
#	define COLLISION_LIB_API		__declspec(dllimport)
#else
#	define COLLISION_LIB_API
#endif

#define COLL_SYS_MIN(a, b)			((a) < (b) ? (a) : (b))
#define COLL_SYS_MAX(a, b)			((a) > (b) ? (a) : (b))
#define COLL_SYS_SQUARED(x)			((x) * (x))
#define COLL_SYS_SIGN(x)			((x) < 0.0 ? -1.0 : 1.0)
#define COLL_SYS_DEGS_TO_RADS(x)	((x) * (180.0 / M_PI))
#define COLL_SYS_RADS_TO_DEGS(x)	((x) * (M_PI / 180.0))

#define COLL_SYS_DRAW_FLAG_SHAPES				0x00000001
#define COLL_SYS_DRAW_FLAG_SHAPE_BOXES			0x00000002
#define COLL_SYS_DRAW_FLAG_AABB_TREE			0x00000004

#define COLL_SYS_ASSERT(condition)		assert(condition)

#define COLL_SYS_ADD_FLAG_ALLOW_SPLIT			0x00000001

#define COLL_SYS_MIN_NODE_VOLUME			(50.0 * 50.0 * 50.0)

#define COLL_SYS_AXIS_FLAG_X			0x00000001
#define COLL_SYS_AXIS_FLAG_Y			0x00000002
#define COLL_SYS_AXIS_FLAG_Z			0x00000004