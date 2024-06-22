# CMakeLists.txt for ImzadiAssetConverter tool.

set(ASSET_CONVERTER_SOURCES
    Source/App.cpp
    Source/App.h
    Source/Frame.cpp
    Source/Frame.h
    Source/Canvas.cpp
    Source/Canvas.h
    Source/GamePreview.cpp
    Source/GamePreview.h
    Source/Converter.cpp
    Source/Converter.h
    Source/LogWindow.cpp
    Source/LogWindow.h
    Source/FontMaker.cpp
    Source/FontMaker.h
    Source/JsonUtils.cpp
    Source/JsonUtils.h
    Source/RenderObjectList.cpp
    Source/RenderObjectList.h
    Source/RenderObjectProperties.cpp
    Source/RenderObjectProperties.h
)

set(FreeTypeFolder "freetype")

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ASSET_CONVERTER_SOURCES})

add_executable(ImzadiAssetConverter WIN32 ${ASSET_CONVERTER_SOURCES})
target_compile_definitions(ImzadiAssetConverter PUBLIC __WXMSW__ _UNICODE)

target_link_libraries(ImzadiAssetConverter PRIVATE
    ImzadiGameEngine
    assimp-vc143-mt.lib
    freetype.lib
)

target_compile_definitions(ImzadiAssetConverter PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _USE_MATH_DEFINES
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(ImzadiAssetConverter PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_x64_lib"
    )
else()
    target_link_directories(ImzadiAssetConverter PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_lib"
    )
endif()

target_link_directories(ImzadiAssetConverter PRIVATE
    "${PROJECT_SOURCE_DIR}/ThirdParty/Assimp/lib/x64"
    
)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    target_link_directories(ImzadiAssetConverter PRIVATE
        "${PROJECT_SOURCE_DIR}/ThirdParty/${FreeTypeFolder}/objs/x64/Debug Static"
    )
else()
    target_link_directories(ImzadiAssetConverter PRIVATE
        "${PROJECT_SOURCE_DIR}/ThirdParty/${FreeTypeFolder}/objs/x64/Release Static"
    )
endif()

target_include_directories(ImzadiAssetConverter PRIVATE
    "${WX_WIDGETS_ROOT}/include"
    "${WX_WIDGETS_ROOT}/include/msvc"
    "${PROJECT_SOURCE_DIR}/ThirdParty/Assimp/include"
    "${PROJECT_SOURCE_DIR}/ThirdParty/${FreeTypeFolder}/include"
)