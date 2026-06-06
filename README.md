# Valley Engine

Valley Engine is the standalone engine foundation for **Veil of the Valley**. The engine is intentionally separate from RPG content: quests, characters, items, medieval rules, assets, and gameplay code should live in a later game layer that depends on this reusable engine.

## Current scope

This repository currently provides the smallest stable foundation that can compile and run:

- A CMake-based C++20 project.
- A minimal `ValleyEngine` executable.
- A core application loop driven by one authoritative engine clock.
- A platform window abstraction with an X11 backend when available and a headless fallback for automated environments.
- An engine-level input abstraction for keyboard/mouse state, named actions, and pressed/held/released transitions.
- Fixed timestep update support, independent render/update frequencies, pause/resume, debug time scaling, queued debug stepping, debug overlay toggles, and free-camera controls.
- A renderer framework with a swappable backend interface, render graph, camera, scene transforms, mesh/material/texture resource handles, directional light, and debug overlay data.
- A renderer test scene submitted through engine mesh/material handles and rendered by the engine-level software raster backend for verification.
- A world streaming framework with deterministic chunk coordinates, a lightweight registry, asynchronous load/unload requests, floating-origin preparation, and debug chunk visualization.
- A procedural terrain framework with layered noise, biome classification, LOD selection, async mesh generation, terrain streaming integration, and debug biome visualization.
- A deterministic simulation scheduler with one authoritative clock source, fixed ticks, stable per-tick seeds, and lightweight profiling/debug snapshots.
- Empty engine modules for tools/debug systems.
- Smoke tests for the runnable app, engine time progression, renderer architecture, world streaming infrastructure, terrain framework, simulation scheduler, and input/action mapping.

No gameplay systems are included yet.

## Architecture

The engine is split into reusable modules under `include/Valley` and `src`:

| Module | Responsibility | Not included yet |
| --- | --- | --- |
| `Core` | Application lifetime, authoritative engine clock, fixed timestep scheduling, module interface, frame timing, and logging. | RPG state, save games, quest flow. |
| `Platform` | Window creation, event polling, keyboard/mouse state, named action maps, and native platform boundaries. | Platform services beyond the initial window/input seams. |
| `Renderer` | Swappable render-path boundary, render graph, camera, scene data, transforms, mesh/material handles, debug overlays, and a software raster verification backend. | Game assets, final materials, terrain streaming data, character rendering. |
| `World` | Deterministic chunk coordinates, chunk registry, asynchronous streaming requests, floating-origin preparation, and debug chunk maps. | Medieval locations, NPCs, dungeons, game maps. |
| `Terrain` | Scalable procedural terrain infrastructure: deterministic layered noise, biome IDs, LOD metadata, async mesh placeholders, and debug biome maps. | Final terrain visuals, rivers, erosion, roads, villages, forests, gameplay placement. |
| `Simulation` | Deterministic fixed-timestep scheduler, stable simulation tick data, per-system update ordering, pause/debug-step support via the Core clock, and lightweight profiling. | Combat, AI behavior, RPG rules, NPC/economy/ecology/weather content. |
| `ToolsDebug` | Diagnostics, reusable debug controls, simulation pause/step/time-scale commands, overlay toggles, and free-camera movement. | Game-content authoring tools. |

The intended dependency direction is:

```text
Game layer (future Veil of the Valley content)
    -> Valley Engine modules
        -> Core / Platform abstractions
```

Engine modules must stay content-agnostic. If a system needs knowledge of Veil of the Valley lore, gameplay, or assets, it belongs outside this repository or behind a future game-layer boundary.

## Input and debug controls architecture

Input lives at the Core/Platform boundary and remains engine-level. Platform windows own an `InputSystem`; native backends translate OS events into key, mouse button, and mouse-position state, while the headless backend simply advances empty input frames. Actions are named strings bound to keys or mouse buttons, and each query reports pressed, held, and released state for the current frame.

```text
Window backend
    -> InputSystem: keyboard state, mouse button state, mouse position
        -> ActionMap: named actions bound to keys/buttons
Application
    -> DebugControls: consumes engine debug actions
        -> EngineClock: pause/resume, one fixed-tick step, time scale
        -> RendererModule/Camera: overlay toggle and free-camera movement
```

Default engine debug bindings are deliberately content-agnostic: `P` toggles pause, `.` queues one fixed debug tick, `=`/`-` adjust time scale, backquote toggles debug overlays, and `W/A/S/D/Q/E` move the free camera with Shift for faster movement. These controls do not create gameplay concepts and can be rebound by replacing the action map before running the application.

## Simulation architecture

The simulation framework uses the Core `EngineClock` as the single authoritative source for simulation time. Render frames and variable per-frame module updates can run at whatever frequency the platform allows, while deterministic simulation work runs only through fixed timestep ticks.

```text
Application render/update frame
    -> EngineClock::advance(real_delta)
        -> scaled simulation time, pause state, debug-step queue
        -> fixed update count
    -> Module::on_fixed_update(...) zero or more times
        -> SimulationModule
            -> SimulationScheduler
                -> SimulationSystem fixed_update(SimulationTick) in stable order
                -> SimulationProfiler records tick/system costs
    -> Module::on_update(...) once for render/debug coordination
```

`SimulationTick` carries deterministic tick index, fixed delta, fixed elapsed time, a stable per-tick seed, and whether the tick came from debug stepping. This is the intended engine seam for future NPC life simulation, economy, ecology, weather, AI scheduling, and persistence systems.

Pause and time scaling are owned by the Core clock so there are not multiple competing clocks. `--paused --step N` can advance exactly `N` fixed simulation ticks while render/update frames still run normally for diagnostics.


## Renderer architecture

The first renderer framework is intentionally engine-level and backend-agnostic:

```text
RendererModule
    -> AssetManager: stable IDs plus CPU mesh/material/texture resources
    -> RenderGraph: Clear -> SceneGeometry -> DebugOverlay
    -> RenderScene: render entities, transforms, mesh/material handles, directional light
    -> Camera: movable engine camera used by render paths
    -> IRendererBackend
        -> SoftwareRasterBackend today
        -> Native raster backend later
        -> Experimental equation/splat renderer later
```

The current backend is a small software raster verification path. It resolves engine mesh handles from the scene resource provider, renders the built-in engine test scene offscreen, and can dump a PPM image in headless mode. This proves the renderer framework, scene submission through handles, camera movement, light data, and debug overlay statistics without committing the engine to one graphics API yet.

The scene and resource types are placeholders by design. Meshes are CPU-side engine resources with stable IDs and simple primitive hints; materials support base color and optional albedo texture handles. They are not gameplay entities and do not contain RPG rules, lore, quests, or content-specific behavior. Future open-world support should build on the same scene boundary with streaming partitions, culling, renderer resource uploads, and renderer-owned visibility data.


## World streaming architecture

The world framework is infrastructure-only. It does not generate terrain, place gameplay entities, or know about RPG content.

```text
WorldModule
    -> WorldStreamer: asynchronous chunk load/unload request queue
    -> WorldRegistry: lightweight authoritative chunk state table
    -> WorldCoordinates: deterministic world -> chunk/local coordinate split
    -> FloatingOrigin: large-world precision rebase preparation
    -> WorldDebug: ASCII loaded-chunk visualization for diagnostics
```

World coordinates are split into integer chunk coordinates plus local offsets so negative positions and very large positions remain deterministic. The streamer tracks a bounded set of desired chunks around an observer and processes load/unload requests on a worker thread. Loaded chunks are registry records only; no terrain, assets, or game-specific content are created at this layer.

This is intended as the future seam for extremely large worlds: terrain, world entities, renderer visibility, and simulation partitions can later attach to chunk records without changing the core coordinate model.

## Terrain generation architecture

The terrain framework is infrastructure-only. It generates deterministic terrain data and placeholder meshes so streaming, LOD, and biome systems can be validated before final rendering or world content exists.

```text
TerrainStreamer
    -> syncs with WorldRegistry loaded chunks
    -> chooses TerrainLod from observer distance
    -> AsyncTerrainMesher worker queue
        -> TerrainGenerator
            -> LayeredNoiseGenerator: height, moisture, temperature channels
            -> BiomeClassifier: engine-level biome IDs and thresholds
            -> TerrainMesh: placeholder vertices/indices tagged with biome IDs
    -> TerrainDebugSnapshot: ASCII biome visualization
```

The framework is prepared for future rivers, erosion, roads, villages, and forest simulation by keeping biome data, terrain samples, mesh generation, and chunk streaming separate. Those systems should become additional engine/data layers that consume terrain chunks rather than hardcoded gameplay or final visuals.

Current limitations:

- Terrain meshes are CPU-side placeholders and are not submitted to the renderer yet.
- Noise and biome rules are deterministic defaults, not art-directed world generation.
- LOD chooses mesh resolution by distance, but there is no stitching, skirts, geomorphing, or GPU resource management yet.
- Async meshing uses one simple worker queue without priorities, cancellation, or per-frame budgets.
- Debug biome visualization is text-based for headless verification.

## Project layout

```text
apps/ValleyEngine/          Minimal executable entry point
include/Valley/Core/        Engine loop, authoritative clock, modules, timing, logging
include/Valley/Assets/      Stable IDs, typed resource handles, CPU mesh/material/texture resources
include/Valley/Platform/    Window and input abstractions
include/Valley/Renderer/    Renderer abstraction, render graph, scene, camera, debug overlay data
include/Valley/Terrain/     Layered noise, biomes, LOD, async terrain meshing, debug biome maps
include/Valley/World/       Chunk coordinates, registry, async streaming, floating origin, debug maps
include/Valley/Simulation/  Deterministic scheduler, tick data, profiler, simulation module boundary
include/Valley/ToolsDebug/  Debug/tools module boundary
src/                        Implementations for the modules above
```

## Build

Requirements:

- CMake 3.20 or newer.
- A C++20 compiler.
- Optional on Linux: X11 development files for the native window backend.

Configure and build from a clean checkout:

```sh
rm -rf build
cmake -S . -B build
cmake --build build --parallel
```

If X11 is unavailable, the project still builds with the headless backend.

## Run

Run with a native window when the platform backend and display are available:

```sh
./build/ValleyEngine
```

Run in headless mode, useful for CI and servers:

```sh
./build/ValleyEngine --headless --frames 3
```

Command-line options:

- `--headless`: force the headless platform backend.
- `--frames N`: stop after `N` frames. If omitted, the app runs until the window is closed.
- `--paused`: start with simulation time paused while real time continues.
- `--time-scale S`: set debug simulation time scale. `1.0` is real time, `0.5` is half speed.
- `--fixed-step S`: set fixed update step length in seconds. The default is `1 / 60`.
- `--step N`: queue `N` fixed simulation debug steps, useful with `--paused` for deterministic inspection.
- `--renderer-path raster|splat`: choose the renderer path. `splat` is reserved and currently falls back to the raster backend.
- `--dump-render PATH`: write the first rendered frame to a binary PPM file for headless verification.
- `--help`: print usage.

## Test

After building, run the full test suite:

```sh
ctest --test-dir build --output-on-failure
```

The suite includes engine smoke and module framework tests. The smoke test starts `ValleyEngine` in headless mode for three frames and verifies that the engine loop exits cleanly. The clock test verifies real-time progression, simulation-time progression, fixed timestep accumulation, pause/resume, debug time scaling, queued debug stepping, and invalid clock configuration handling. The renderer architecture test verifies the engine-level test scene, render graph pass order, raster backend path, renderer scene submission through handles, and debug overlay statistics. The asset pipeline test verifies stable asset IDs, mesh/material/texture registration, CPU mesh storage, resource lookup, simple import stubs, and hot reload. The world streaming test verifies deterministic negative chunk coordinates, async chunk loading/unloading, registry state, debug chunk maps, and floating-origin rebasing. The terrain framework test verifies deterministic layered noise, biome classification, LOD resolution changes, async mesh generation, terrain streaming integration with loaded world chunks, and debug biome maps. The simulation framework test verifies fixed tick ordering, stable deterministic seeds, paused debug stepping, and profiler records. The input test verifies keyboard/mouse transitions, named action mapping, combined bindings, and engine debug-control actions.

## Next engine-only steps

Keep future changes small and engine-focused. Good next steps include:

1. Expand native input coverage beyond the initial keyboard/mouse debug bindings.
2. Add renderer-facing GPU upload/cache objects behind the mesh/material/texture resource handles.
3. Connect a native graphics API backend to the same renderer abstraction.
4. Add terrain streaming priorities, cancellation, mesh stitching, and renderer-facing terrain resource handles.
5. Connect renderer visibility and simulation partitioning to chunk and terrain records.
6. Add simulation persistence snapshots and deterministic replay checks before adding game-layer systems.
7. Add a separate game-layer sample only after the engine boundary is explicit.
