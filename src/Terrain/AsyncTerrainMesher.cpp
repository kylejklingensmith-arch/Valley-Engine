#include "Valley/Terrain/AsyncTerrainMesher.h"

#include <utility>

namespace Valley::Terrain {

AsyncTerrainMesher::AsyncTerrainMesher(TerrainGenerator generator)
    : m_generator(std::move(generator))
{
}

AsyncTerrainMesher::~AsyncTerrainMesher()
{
    shutdown();
}

void AsyncTerrainMesher::start()
{
    std::lock_guard lock(m_mutex);
    if (m_running) {
        return;
    }
    m_stopping = false;
    m_running = true;
    m_worker = std::thread(&AsyncTerrainMesher::worker_loop, this);
}

void AsyncTerrainMesher::shutdown()
{
    {
        std::lock_guard lock(m_mutex);
        if (!m_running) {
            return;
        }
        m_stopping = true;
    }
    m_condition.notify_all();
    if (m_worker.joinable()) {
        m_worker.join();
    }

    std::lock_guard lock(m_mutex);
    m_running = false;
    while (!m_requests.empty()) {
        m_requests.pop();
    }
    while (!m_completed.empty()) {
        m_completed.pop();
    }
    m_in_flight = 0;
}

void AsyncTerrainMesher::request_mesh(const TerrainMeshRequest& request)
{
    {
        std::unique_lock lock(m_mutex);
        if (!m_running) {
            lock.unlock();
            start();
            lock.lock();
        }
        m_requests.push(request);
    }
    m_condition.notify_one();
}

std::vector<TerrainMesh> AsyncTerrainMesher::collect_completed_meshes()
{
    std::vector<TerrainMesh> meshes;
    std::lock_guard lock(m_mutex);
    while (!m_completed.empty()) {
        meshes.push_back(std::move(m_completed.front()));
        m_completed.pop();
    }
    return meshes;
}

void AsyncTerrainMesher::wait_until_idle()
{
    std::unique_lock lock(m_mutex);
    m_idle_condition.wait(lock, [this] { return m_requests.empty() && m_in_flight == 0; });
}

void AsyncTerrainMesher::worker_loop()
{
    while (true) {
        TerrainMeshRequest request;
        {
            std::unique_lock lock(m_mutex);
            m_condition.wait(lock, [this] { return m_stopping || !m_requests.empty(); });
            if (m_stopping && m_requests.empty()) {
                break;
            }
            request = m_requests.front();
            m_requests.pop();
            ++m_in_flight;
        }

        TerrainMesh mesh = m_generator.build_mesh(m_generator.generate_chunk(request.coord, request.lod));

        {
            std::lock_guard lock(m_mutex);
            m_completed.push(std::move(mesh));
            --m_in_flight;
        }
        m_idle_condition.notify_all();
    }
    m_idle_condition.notify_all();
}

} // namespace Valley::Terrain
