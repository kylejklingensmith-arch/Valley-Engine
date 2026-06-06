cmake_minimum_required(VERSION 3.20)

project(ValleyEngine
    VERSION 0.1.0
    DESCRIPTION "Reusable engine foundation for Veil of the Valley"
    LANGUAGES CXX)

option(VALLEY_BUILD_TESTS "Build and register engine smoke tests" ON)
option(VALLEY_USE_X11 "Build the Linux X11 window backend when available" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ValleyEngineCore
    src/Assets/AssetManager.cpp
    src/Assets/AssetRegistry.cpp
    src/Assets/AssetTypes.cpp
    src/Core/Application.cpp
    src/Core/EngineClock.cpp
    src/Core/Logger.cpp
    src/Platform/Input.cpp
    src/Platform/Window.cpp
    src/Terrain/AsyncTerrainMesher.cpp
    src/Terrain/Biome.cpp
    src/Terrain/Noise.cpp
    src/Terrain/TerrainGenerator.cpp
    src/Terrain/TerrainStreamer.cpp
    src/Terrain/TerrainTypes.cpp
    src/Renderer/Camera.cpp
    src/Renderer/RenderDiagnostics.cpp
    src/Renderer/RenderGraph.cpp
    src/Renderer/RendererModule.cpp
    src/Renderer/Scene.cpp
    src/Renderer/SoftwareRasterBackend.cpp
    src/World/FloatingOrigin.cpp
    src/World/WorldChunk.cpp
    src/World/WorldCoordinates.cpp
    src/World/WorldDebug.cpp
    src/World/WorldModule.cpp
    src/World/WorldRegistry.cpp
    src/World/WorldStreamer.cpp
    src/Simulation/SimulationModule.cpp
    src/Simulation/SimulationProfiler.cpp
    src/Simulation/SimulationScheduler.cpp
    src/ToolsDebug/DebugControls.cpp
    src/ToolsDebug/DebugModule.cpp)

add_library(Valley::Engine ALIAS ValleyEngineCore)

target_include_directories(ValleyEngineCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_compile_features(ValleyEngineCore PUBLIC cxx_std_20)

target_compile_definitions(ValleyEngineCore PUBLIC VALLEY_SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(ValleyEngineCore PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>)

if(VALLEY_USE_X11)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(X11 QUIET x11)
    endif()

    if(X11_FOUND)
        target_sources(ValleyEngineCore PRIVATE src/Platform/X11Window.cpp)
        target_compile_definitions(ValleyEngineCore PRIVATE VALLEY_PLATFORM_X11=1)
        target_include_directories(ValleyEngineCore PRIVATE ${X11_INCLUDE_DIRS})
        target_link_libraries(ValleyEngineCore PRIVATE ${X11_LIBRARIES})
    else()
        message(STATUS "X11 was not found; building with the headless window backend only.")
    endif()
endif()

add_executable(ValleyEngine apps/ValleyEngine/main.cpp)
target_link_libraries(ValleyEngine PRIVATE Valley::Engine)

if(VALLEY_BUILD_TESTS)
    enable_testing()

    add_executable(ValleyEngineClockTests tests/EngineClockTests.cpp)
    target_link_libraries(ValleyEngineClockTests PRIVATE Valley::Engine)

    add_executable(ValleyRendererArchitectureTests tests/RendererArchitectureTests.cpp)
    target_link_libraries(ValleyRendererArchitectureTests PRIVATE Valley::Engine)

    add_executable(ValleyAssetPipelineTests tests/AssetPipelineTests.cpp)
    target_link_libraries(ValleyAssetPipelineTests PRIVATE Valley::Engine)

    add_executable(ValleyWorldStreamingTests tests/WorldStreamingTests.cpp)
    target_link_libraries(ValleyWorldStreamingTests PRIVATE Valley::Engine)

    add_executable(ValleyTerrainFrameworkTests tests/TerrainFrameworkTests.cpp)
    target_link_libraries(ValleyTerrainFrameworkTests PRIVATE Valley::Engine)

    add_executable(ValleySimulationFrameworkTests tests/SimulationFrameworkTests.cpp)
    target_link_libraries(ValleySimulationFrameworkTests PRIVATE Valley::Engine)

    add_executable(ValleyInputTests tests/InputTests.cpp)
    target_link_libraries(ValleyInputTests PRIVATE Valley::Engine)

    add_test(NAME valley_engine_smoke COMMAND ValleyEngine --headless --frames 3)
    add_test(NAME valley_engine_clock_tests COMMAND ValleyEngineClockTests)
    add_test(NAME valley_renderer_architecture_tests COMMAND ValleyRendererArchitectureTests)
    add_test(NAME valley_asset_pipeline_tests COMMAND ValleyAssetPipelineTests)
    add_test(NAME valley_world_streaming_tests COMMAND ValleyWorldStreamingTests)
    add_test(NAME valley_terrain_framework_tests COMMAND ValleyTerrainFrameworkTests)
    add_test(NAME valley_simulation_framework_tests COMMAND ValleySimulationFrameworkTests)
    add_test(NAME valley_input_tests COMMAND ValleyInputTests)
endif()
