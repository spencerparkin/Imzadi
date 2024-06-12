# CMakeLists.txt for ImzadiGameEditor tool.

set(GAME_EDITOR_SOURCES
    Source/App.cpp
    Source/App.h
    Source/Frame.cpp
    Source/Frame.h
    Source/Canvas.cpp
    Source/Canvas.h
    Source/GameEditor.cpp
    Source/GameEditor.h
    Source/EditorAssetCache.cpp
    Source/EditorAssetCache.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GAME_EDITOR_SOURCES})

add_executable(ImzadiGameEditor WIN32 ${GAME_EDITOR_SOURCES})
target_compile_definitions(ImzadiGameEditor PUBLIC __WXMSW__ _UNICODE)

target_link_libraries(ImzadiGameEditor PRIVATE
    ImzadiGameEngine
    assimp-vc143-mt.lib
)

target_compile_definitions(ImzadiGameEditor PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _USE_MATH_DEFINES
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(ImzadiGameEditor PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_x64_lib"
    )
else()
    target_link_directories(ImzadiGameEditor PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_lib"
    )
endif()

target_link_directories(ImzadiGameEditor PRIVATE
    ${PROJECT_SOURCE_DIR}/ThirdParty/Assimp/lib/x64
)

target_include_directories(ImzadiGameEditor PRIVATE
    "${WX_WIDGETS_ROOT}/include"
    "${WX_WIDGETS_ROOT}/include/msvc"
    ${PROJECT_SOURCE_DIR}/ThirdParty/Assimp/include
)