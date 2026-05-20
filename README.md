# Valley Engine

Valley Engine is the standalone engine foundation for **Veil of the Valley**. The engine is intentionally separate from RPG content: quests, characters, items, medieval rules, assets, and gameplay code should live in a later game layer that depends on this reusable engine.

## Current scope

This repository currently provides the smallest stable foundation that can compile and run:

- A CMake-based C++20 project.
- A minimal `ValleyEngine` executable.
- A core application loop driven by one authoritative engine clock.
- A platform window abstraction with an X11 backend when available and a headless fallback for automated environments.
- Fixed timestep update support, independent render/update frequencies, pause/resume, debug time scaling, and queued debug stepping.
- A renderer framework with a swappable backend interface, render graph, camera, scene, transforms, mesh placeholders, directional light, and debug overlay data.
- A renderer test scene containing a ground plane and cube, rendered by the engine-level software raster backend for verification.
- A world streaming framework with deterministic chunk coordinates, a lightweight registry, asynchronous load/unload requests, floating-origin preparation, and debug chunk visualization.
- A procedural terrain framework with layered noise, biome classification, LOD selection, async mesh generation, terrain streaming integration, and debug biome visualization.
- A deterministic simulation scheduler with one authoritative clock source, fixed ticks, stable per-tick seeds, and lightweight profiling/debug snapshots.
- Empty engine modules for tools/debug systems.
- Smoke tests for the runnable app, engine time progression, renderer architecture, world streaming infrastructure, terrain framework, and simulation scheduler.

No gameplay systems are included yet.

## Architecture

The engine is split into reusable modules under `include/Valley` and `src`:

| Module | Responsibility | Not included yet |
| --- | --- | --- |
| `Core` | Application lifetime, authoritative engine clock, fixed timestep scheduling, module interface, frame timing, and logging. | RPG state, save games, quest flow. |
| `Platform` | Window creation, event polling, and native platform boundaries. | Input mapping, platform services beyond the initial window. |
| `Renderer` | Swappable render-path boundary, render graph, camera, scene data, transform and mesh placeholders, debug overlays, and a software raster verification backend. | Game assets, materials, terrain streaming data, character rendering. |
| `World` | Deterministic chunk coordinates, chunk registry, asynchronous streaming requests, floating-origin preparation, and debug chunk maps. | Medieval locations, NPCs, dungeons, game maps. |
| `Terrain` | Scalable procedural terrain infrastructure: deterministic layered noise, biome IDs, LOD metadata, async mesh placeholders, and debug biome maps. | Final terrain visuals, rivers, erosion, roads, villages, forests, gameplay placement. |
| `Simulation` | Deterministic fixed-timestep scheduler, stable simulation tick data, per-system update ordering, pause/debug-step support via the Core clock, and lightweight profiling. | Combat, AI behavior, RPG rules, NPC/economy/ecology/weather content. |
| `ToolsDebug` | Diagnostics and future editor/debug surfaces. | Game-content authoring tools. |

The intended dependency direction is:

```text
Game layer (future Veil of the Valley content)
    -> Valley Engine modules
        -> Core / Platform abstractions
```

Engine modules must stay content-agnostic. If a system needs knowledge of Veil of the Valley lore, gameplay, or assets, it belongs outside this repository or behind a future game-layer boundary.

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
    -> RenderGraph: Clear -> SceneGeometry -> DebugOverlay
    -> RenderScene: render entities, transforms, mesh placeholders, directional light
    -> Camera: movable engine camera used by render paths
    -> IRendererBackend
        -> SoftwareRasterBackend today
        -> Native raster backend later
        -> Experimental equation/splat renderer later
```

The current backend is a small software raster verification path. It renders the built-in engine test scene offscreen and can dump a PPM image in headless mode. This proves the renderer framework, scene submission, camera movement, light data, and debug overlay statistics without committing the engine to one graphics API yet.

The scene types are placeholders by design. They are not gameplay entities and do not contain RPG rules, lore, quests, or content-specific behavior. Future open-world support should build on the same scene boundary with streaming partitions, culling, resource handles, and renderer-owned visibility data.


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
include/Valley/Platform/    Window abstraction
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

Configure and build:

```sh
cmake -S . -B build
cmake --build build
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

After building, run the smoke test:

```sh
ctest --test-dir build --output-on-failure
```

The smoke test starts `ValleyEngine` in headless mode for three frames and verifies that the engine loop exits cleanly. The clock test verifies real-time progression, simulation-time progression, fixed timestep accumulation, pause/resume, debug time scaling, queued debug stepping, and invalid clock configuration handling. The renderer architecture test verifies the engine-level test scene, render graph pass order, raster backend path, and debug overlay statistics. The world streaming test verifies deterministic negative chunk coordinates, async chunk loading/unloading, registry state, debug chunk maps, and floating-origin rebasing. The terrain framework test verifies deterministic layered noise, biome classification, LOD resolution changes, async mesh generation, terrain streaming integration with loaded world chunks, and debug biome maps. The simulation framework test verifies fixed tick ordering, stable deterministic seeds, paused debug stepping, and profiler records.

## Next engine-only steps

Keep future changes small and engine-focused. Good next steps include:

1. Add a real input abstraction under `Platform`, then wire interactive simulation pause/step controls through it.
2. Replace placeholder meshes with an engine asset/mesh resource interface.
3. Connect a native graphics API backend to the same renderer abstraction.
4. Add terrain streaming priorities, cancellation, mesh stitching, and renderer-facing terrain resource handles.
5. Connect renderer visibility and simulation partitioning to chunk and terrain records.
6. Add simulation persistence snapshots and deterministic replay checks before adding game-layer systems.
7. Add a separate game-layer sample only after the engine boundary is explicit.
