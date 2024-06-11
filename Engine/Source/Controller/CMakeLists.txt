# CMakeLists.txt for ControllerLib library.

set(LIBRARY_SOURCES
    Source/Controller.cpp
    Source/Controller.h
)
    
source_group("Source" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LIBRARY_SOURCES})

add_library(ControllerLib SHARED
    ${LIBRARY_SOURCES}
)

target_compile_definitions(ControllerLib PRIVATE
    CONTROLLER_LIB_EXPORT
)

target_include_directories(ControllerLib PUBLIC
    "Source"
)

target_link_libraries(ControllerLib PRIVATE
    Xinput9_1_0.lib
)