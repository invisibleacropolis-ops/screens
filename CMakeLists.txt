cmake_minimum_required(VERSION 3.20)
project(ScreenSaver C CXX)

set(CMAKE_CXX_STANDARD 17)

# Add source files
set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/Config.h
    src/Logger.h


    src/Particles.cpp
    src/Particles.h
    src/glad/glad.c
    src/glad/glad.h
    src/SystemMonitor.cpp
    src/SystemMonitor.h
    src/DiagTest.h
    src/stb_image.h
    src/resource.h
    src/SettingsDialog.h
    src/SettingsDialog.cpp
    # Engine modules
    src/engine/Math.h
    src/engine/Math.cpp
    src/engine/Engine.h
    src/engine/Engine.cpp
    # Graphics modules
    src/graphics/Mesh.h
    src/graphics/Mesh.cpp
    src/graphics/Shader.h
    src/graphics/Shader.cpp
    src/graphics/Texture.h
    src/graphics/Texture.cpp
    src/graphics/PostProcessConfig.h
    src/graphics/VisualizerLayer.h
    src/graphics/LayerCompositor.h
    # Visualizers
    src/visualizers/IVisualizer.h
    src/visualizers/CPUVisualizer.h
    src/visualizers/RAMVisualizer.h
    src/visualizers/DiskVisualizer.h
    # Settings modules
    src/settings/ISettingsPanel.h
    src/settings/SettingsHost.h
    src/settings/VisualEffectsPanel.h
    src/settings/CameraPanel.h
    src/settings/MetricsPanel.h
    src/settings/LayerLayoutPanel.h
    src/settings/LayerFXPanel.h
)

add_executable(ScreenSaver WIN32 ${SOURCES})

target_include_directories(ScreenSaver PRIVATE
    src
)

# Link against Windows libraries
target_link_libraries(ScreenSaver PRIVATE
    opengl32
    glu32
    pdh
)

# Rename executable to .scr for screensaver functionality
set_target_properties(ScreenSaver PROPERTIES SUFFIX ".scr")

# Copy assets to the output directory
add_custom_command(TARGET ScreenSaver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:ScreenSaver>/assets
    COMMENT "Copying assets to output directory..."
)
