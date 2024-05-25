# CMakeLists.txt for MegaSuperUltraQuest...quite possibly the greatest game ever conceived.

set(GAME_SOURCES
    Source/Main.cpp
    Source/Main.h
    Source/Game.cpp
    Source/Game.h
    Source/Scene.cpp
    Source/Scene.h
    Source/RenderMesh.cpp
    Source/RenderMesh.h
    Source/Camera.cpp
    Source/Camera.h
    Source/Shader.cpp
    Source/Shader.h
    Source/Texture.cpp
    Source/Texture.h
    Source/Buffer.cpp
    Source/Buffer.h
    Source/AssetCache.cpp
    Source/AssetCache.h
    Source/Reference.cpp
    Source/Reference.h
    Source/Entity.cpp
    Source/Entity.h
    Source/Entities/Hero.cpp
    Source/Entities/Hero.h
    Source/Entities/Enemy.cpp
    Source/Entities/Enemy.h
    Source/Entities/FollowCam.cpp
    Source/Entities/FollowCam.h
    Source/Entities/Level.cpp
    Source/Entities/Level.h
    Source/Entities/FreeCam.cpp
    Source/Entities/FreeCam.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GAME_SOURCES})

add_executable(MegaSuperUltraQuest WIN32 ${GAME_SOURCES})

target_link_libraries(MegaSuperUltraQuest PRIVATE
    CollisionLib
    ControllerLib
    d3d11.lib
    d3dcompiler.lib
)

target_compile_definitions(MegaSuperUltraQuest PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _USE_MATH_DEFINES
)

target_include_directories(MegaSuperUltraQuest PRIVATE
    "ThirdParty"
    "Source"
)