# CMakeLists.txt for the BenzoBonanza game application.

set(BENZO_BONANZA_SOURCES
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
    Source/Characters/Alice.cpp
    Source/Characters/Alice.h
    Source/Characters/Spencer.cpp
    Source/Characters/Spencer.h
    Source/Characters/Borggy.cpp
    Source/Characters/Borggy.h
    Source/Characters/Bob.cpp
    Source/Characters/Bob.h
    Source/Characters/Cue.cpp
    Source/Characters/Cue.h
    Source/Entities/ZipLineEntity.cpp
    Source/Entities/ZipLineEntity.h
    Source/Entities/ZipLineRider.cpp
    Source/Entities/ZipLineRider.h
    Source/Entities/RubiksCubie.cpp
    Source/Entities/RubiksCubie.h
    Source/Entities/RubiksCubeMaster.cpp
    Source/Entities/RubiksCubeMaster.h
    Source/Entities/SlidingDoor.cpp
    Source/Entities/SlidingDoor.h
    Source/Entities/Pickup.cpp
    Source/Entities/Pickup.h
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
    Source/Assets/SlidingDoorData.cpp
    Source/Assets/SlidingDoorData.h
    Source/RenderObjects/HUDRenderObject.cpp
    Source/RenderObjects/HUDRenderObject.h
    Assets/Icons/Icon.rc
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${BENZO_BONANZA_SOURCES})

add_executable(BenzoBonanza WIN32 ${BENZO_BONANZA_SOURCES})

target_link_libraries(BenzoBonanza PRIVATE
    ImzadiGameEngine
)

target_compile_definitions(BenzoBonanza PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _USE_MATH_DEFINES
)

target_include_directories(BenzoBonanza PRIVATE
    "Source"
    ${PROJECT_SOURCE_DIR}/Engine/Source
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(BenzoBonanza PUBLIC
        IMZADI_LOGGING
    )
endif()