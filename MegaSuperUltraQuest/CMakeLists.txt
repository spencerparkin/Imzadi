# CMakeLists.txt for MegaSuperUltraQuest...quite possibly the greatest game ever conceived.

set(GAME_SOURCES
    Source/Main.cpp
    Source/Main.h
    Source/Game.cpp
    Source/Game.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GAME_SOURCES})

add_executable(MegaSuperUltraQuest WIN32 ${GAME_SOURCES})

target_link_libraries(MegaSuperUltraQuest PRIVATE
    CollisionLib
    ControllerLib
)

target_compile_definitions(MegaSuperUltraQuest PRIVATE
    WIN32_LEAN_AND_MEAN
)