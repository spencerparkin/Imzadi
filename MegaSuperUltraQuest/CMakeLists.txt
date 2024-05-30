# CMakeLists.txt for MegaSuperUltraQuest...quite possibly the greatest game ever conceived.

set(GAME_SOURCES
    Source/Main.cpp
    Source/Main.h
    Source/Game.cpp
    Source/Game.h
    Source/Scene.cpp
    Source/Scene.h
    Source/Camera.cpp
    Source/Camera.h
    Source/AssetCache.cpp
    Source/AssetCache.h
    Source/Reference.cpp
    Source/Reference.h
    Source/Entity.cpp
    Source/Entity.h
    Source/RenderObjects/RenderMeshInstance.cpp
    Source/RenderObjects/RenderMeshInstance.h
    Source/Assets/RenderMesh.cpp
    Source/Assets/RenderMesh.h
    Source/Assets/Shader.cpp
    Source/Assets/Shader.h
    Source/Assets/Texture.cpp
    Source/Assets/Texture.h
    Source/Assets/Buffer.cpp
    Source/Assets/Buffer.h
    Source/Assets/CollisionShapeSet.cpp
    Source/Assets/CollisionShapeSet.h
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