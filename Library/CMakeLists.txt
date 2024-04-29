# CMakeLists.txt for CollisionSystem library.

set(LIBRARY_SOURCES
    Source/System.cpp
    Source/System.h
    Source/Thread.cpp
    Source/Thread.h
    Source/Task.cpp
    Source/Task.h
    Source/Query.cpp
    Source/Query.h
    Source/Command.cpp
    Source/Command.h
    Source/Shape.cpp
    Source/Shape.h
    Source/Error.cpp
    Source/Error.h
    Source/Result.cpp
    Source/Result.h
    Source/BoundingBoxTree.cpp
    Source/BoundingBoxTree.h
    Source/CollisionPairCalculator.cpp
    Source/CollisionPairCalculator.h
    Source/CollisionPair.cpp
    Source/CollisionPair.h
    Source/Defines.h
    Source/Math/AxisAlignedBoundingBox.cpp
    Source/Math/AxisAlignedBoundingBox.h
    Source/Math/Matrix3x3.cpp
    Source/Math/Matrix3x3.h
    Source/Math/Transform.cpp
    Source/Math/Transform.h
    Source/Math/Vector3.cpp
    Source/Math/Vector3.h
    Source/Math/Ray.cpp
    Source/Math/Ray.h
    Source/Math/Plane.cpp
    Source/Math/Plane.h
    Source/Math/LineSegment.cpp
    Source/Math/LineSegment.h
    Source/Math/Quaternion.cpp
    Source/Math/Quaternion.h
    Source/Math/Quadratic.cpp
    Source/Math/Quadratic.h
    Source/Shapes/Box.cpp
    Source/Shapes/Box.h
    Source/Shapes/Capsule.cpp
    Source/Shapes/Capsule.h
    Source/Shapes/Polygon.cpp
    Source/Shapes/Polygon.h
    Source/Shapes/Sphere.cpp
    Source/Shapes/Sphere.h
)
    
source_group("Source" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LIBRARY_SOURCES})

add_library(CollisionLib SHARED
    ${LIBRARY_SOURCES}
)

target_compile_definitions(CollisionLib PRIVATE
    COLLISION_LIB_EXPORT
    _USE_MATH_DEFINES
)

target_include_directories(CollisionLib PUBLIC
    "Source"
)