# CMakeLists.txt file for Sandbox application.

set(APP_SOURCES
    Source/App.cpp
    Source/App.h
    Source/Canvas.cpp
    Source/Canvas.h
    Source/Frame.cpp
    Source/Frame.h
    Source/Camera.cpp
    Source/Camera.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${APP_SOURCES})

set(WX_WIDGETS_ROOT "E:/wxWidgets")

if(WIN32)
    add_executable(SandboxApp WIN32 ${APP_SOURCES})
    target_compile_definitions(SandboxApp PUBLIC __WXMSW__ _UNICODE)
else()
    add_executable(SandboxApp ${APP_SOURCES})
endif()

target_link_libraries(SandboxApp PRIVATE
    CollisionLib
    ControllerLib
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(SandboxApp PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_x64_lib"
    )
else()
    target_link_directories(SandboxApp PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_lib"
    )
endif()

target_include_directories(SandboxApp PRIVATE
    "${WX_WIDGETS_ROOT}/include"
    "${WX_WIDGETS_ROOT}/include/msvc"
)
