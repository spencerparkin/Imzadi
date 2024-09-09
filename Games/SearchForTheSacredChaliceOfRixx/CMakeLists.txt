# CMakeLists.txt for SearchForTheSacredChaliceOfRixx game application.

set(RIXX_SOURCES
    Source/Main.cpp
    Source/Main.h
    Source/GameApp.cpp
    Source/GameApp.h
    Source/GameLevel.cpp
    Source/GameLevel.h
    Source/DialogSystem.cpp
    Source/DialogSystem.h
    Source/Character.cpp
    Source/Character.h
    Source/CustomAssetCache.cpp
    Source/CustomAssetCache.h
    Source/Pickup.cpp
    Source/Pickup.h
    Source/Characters/DeannaTroi.cpp
    Source/Characters/DeannaTroi.h
    Source/Characters/LwaxanaTroi.cpp
    Source/Characters/LwaxanaTroi.h
    Source/Characters/Borg.cpp
    Source/Characters/Borg.h
    Source/Entities/ZipLineEntity.cpp
    Source/Entities/ZipLineEntity.h
    Source/Entities/ZipLineRider.cpp
    Source/Entities/ZipLineRider.h
    Source/Entities/RubiksCubie.cpp
    Source/Entities/RubiksCubie.h
    Source/Entities/RubiksCubeMaster.cpp
    Source/Entities/RubiksCubeMaster.h
    Source/Assets/DialogData.cpp
    Source/Assets/DialogData.h
    Source/Assets/GameProgress.cpp
    Source/Assets/GameProgress.h
    Source/Assets/ZipLine.cpp
    Source/Assets/ZipLine.h
    Source/Assets/GameLevelData.cpp
    Source/Assets/GameLevelData.h
    Source/Assets/RubiksCubieData.cpp
    Source/Assets/RubiksCubieData.h
    Source/RenderObjects/HUDRenderObject.cpp
    Source/RenderObjects/HUDRenderObject.h
    Assets/Icons/Icon.rc
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RIXX_SOURCES})

add_executable(SearchForTheSacredChaliceOfRixx WIN32 ${RIXX_SOURCES})

target_link_libraries(SearchForTheSacredChaliceOfRixx PRIVATE
    ImzadiGameEngine
)

target_compile_definitions(SearchForTheSacredChaliceOfRixx PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _USE_MATH_DEFINES
)

target_include_directories(SearchForTheSacredChaliceOfRixx PRIVATE
    "Source"
    ${PROJECT_SOURCE_DIR}/Engine/Source
)