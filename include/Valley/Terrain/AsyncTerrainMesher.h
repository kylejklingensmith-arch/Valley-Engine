#pragma once

#include "Valley/Terrain/TerrainGenerator.h"

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

namespace Valley::Terrain {

struct TerrainMeshRequest {
    World::ChunkCoord coord;
    TerrainLod lod;
};

class AsyncTerrainMesher {
public:
    explicit AsyncTerrainMesher(TerrainGenerator generator = TerrainGenerator {});
    ~AsyncTerrainMesher();

    AsyncTerrainMesher(const AsyncTerrainMesher&) = delete;
    AsyncTerrainMesher& operator=(const AsyncTerrainMesher&) = delete;

    void start();
    void shutdown();
    void request_mesh(const TerrainMeshRequest& request);
    [[nodiscard]] std::vector<TerrainMesh> collect_completed_meshes();
    void wait_until_idle();

private:
    void worker_loop();

    TerrainGenerator m_generator;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::condition_variable m_idle_condition;
    std::queue<TerrainMeshRequest> m_requests;
    std::queue<TerrainMesh> m_completed;
    std::thread m_worker;
    bool m_running = false;
    bool m_stopping = false;
    unsigned int m_in_flight = 0;
};

} // namespace Valley::Terrain
