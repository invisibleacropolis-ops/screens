cmake_minimum_required(VERSION 3.20)
project(ScreenSaver CXX)

set(CMAKE_CXX_STANDARD 17)

# Add source files
set(SOURCES
    src/main.cpp
    src/renderer.cpp
    src/renderer.h
    src/SystemMonitor.cpp
    src/SystemMonitor.h
)

add_executable(ScreenSaver WIN32 ${SOURCES})

# Link against Windows libraries
target_link_libraries(ScreenSaver PRIVATE
    opengl32
    glu32
    pdh
)

# Rename executable to .scr for screensaver functionality
set_target_properties(ScreenSaver PROPERTIES SUFFIX ".scr")
