# CMakeLists.txt file for CollisionSandbox application.

set(COLLISION_SANDBOX_SOURCES
    Source/App.cpp
    Source/App.h
    Source/Canvas.cpp
    Source/Canvas.h
    Source/Frame.cpp
    Source/Frame.h
    Source/Camera.cpp
    Source/Camera.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${COLLISION_SANDBOX_SOURCES})

set(WX_WIDGETS_ROOT "E:/wxWidgets")

if(WIN32)
    add_executable(CollisionSandboxApp WIN32 ${COLLISION_SANDBOX_SOURCES})
    target_compile_definitions(CollisionSandboxApp PUBLIC __WXMSW__ _UNICODE)
else()
    add_executable(CollisionSandboxApp ${APP_SOURCES})
endif()

target_link_libraries(CollisionSandboxApp PRIVATE
    ImzadiGameEngine
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(CollisionSandboxApp PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_x64_lib"
    )
else()
    target_link_directories(CollisionSandboxApp PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_lib"
    )
endif()

target_include_directories(CollisionSandboxApp PRIVATE
    "${WX_WIDGETS_ROOT}/include"
    "${WX_WIDGETS_ROOT}/include/msvc"
)
