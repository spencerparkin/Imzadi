# CMakeLists.txt for SearchForTheSacredChaliceOfRixx game application.

set(RIXX_SOURCES
    Source/Main.cpp
    Source/Main.h
    Source/GameApp.cpp
    Source/GameApp.h
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