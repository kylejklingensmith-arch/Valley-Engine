#include "Valley/World/WorldStreamer.h"

#include <algorithm>
#include <chrono>
#include <sstream>

namespace Valley::World {
namespace {
std::string make_debug_name(const ChunkCoord& coord)
{
    return std::string("Chunk ").append(to_string(coord));
}
} // namespace

WorldStreamer::WorldStreamer(WorldStreamerDesc desc)
    : m_desc(desc)
{
    if (m_desc.load_radius_chunks < 0) {
        m_desc.load_radius_chunks = 0;
    }
}

WorldStreamer::~WorldStreamer()
{
    shutdown();
}

void WorldStreamer::start()
{
    std::lock_guard lock(m_mutex);
    if (m_running) {
        return;
    }

    m_stopping = false;
    m_running = true;
    m_worker = std::thread(&WorldStreamer::worker_loop, this);
}

void WorldStreamer::shutdown()
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
    while (!m_completions.empty()) {
        m_completions.pop();
    }
    m_in_flight = 0;
}

void WorldStreamer::request_load(const ChunkCoord& coord)
{
    const auto existing = m_registry.find(coord);
    if (existing && (existing->state == ChunkState::Loading || existing->state == ChunkState::Loaded || existing->state == ChunkState::Requested)) {
        return;
    }

    m_registry.upsert({
        .coord = coord,
        .state = ChunkState::Loading,
        .revision = existing ? existing->revision + 1U : 1U,
        .debug_name = make_debug_name(coord),
    });
    enqueue({ .kind = RequestKind::Load, .coord = coord });
}

void WorldStreamer::request_unload(const ChunkCoord& coord)
{
    const auto existing = m_registry.find(coord);
    if (!existing || existing->state == ChunkState::Unloading || existing->state == ChunkState::UnloadRequested) {
        return;
    }

    m_registry.upsert({
        .coord = coord,
        .state = ChunkState::Unloading,
        .revision = existing->revision + 1U,
        .debug_name = existing->debug_name,
    });
    enqueue({ .kind = RequestKind::Unload, .coord = coord });
}

void WorldStreamer::update_observer(const WorldVec3& observer_world_position)
{
    const ChunkCoord center = chunk_from_world_position(observer_world_position, m_desc.grid);
    const auto desired = desired_chunks_around(center);

    for (const auto& coord : desired) {
        request_load(coord);
    }

    const auto records = m_registry.records();
    for (const auto& record : records) {
        const bool still_desired = std::find(desired.begin(), desired.end(), record.coord) != desired.end();
        if (!still_desired) {
            request_unload(record.coord);
        }
    }
}

void WorldStreamer::service_completed_requests()
{
    std::queue<Completion> completions;
    {
        std::lock_guard lock(m_mutex);
        completions.swap(m_completions);
    }

    while (!completions.empty()) {
        const Completion completion = completions.front();
        completions.pop();

        if (completion.kind == RequestKind::Load) {
            m_registry.upsert(completion.record);
        } else {
            m_registry.erase(completion.record.coord);
        }
    }
}

void WorldStreamer::wait_until_idle()
{
    std::unique_lock lock(m_mutex);
    m_idle_condition.wait(lock, [this] {
        return m_requests.empty() && m_in_flight == 0;
    });
}

const WorldRegistry& WorldStreamer::registry() const
{
    return m_registry;
}

WorldRegistry& WorldStreamer::registry()
{
    return m_registry;
}

const WorldStreamerDesc& WorldStreamer::desc() const
{
    return m_desc;
}

void WorldStreamer::worker_loop()
{
    while (true) {
        Request request;
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

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        Completion completion {
            .kind = request.kind,
            .record = {
                .coord = request.coord,
                .state = request.kind == RequestKind::Load ? ChunkState::Loaded : ChunkState::Unloading,
                .revision = 1U,
                .debug_name = make_debug_name(request.coord),
            },
        };

        {
            std::lock_guard lock(m_mutex);
            m_completions.push(completion);
            --m_in_flight;
        }
        m_idle_condition.notify_all();
    }

    m_idle_condition.notify_all();
}

void WorldStreamer::enqueue(Request request)
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

std::vector<ChunkCoord> WorldStreamer::desired_chunks_around(const ChunkCoord& center) const
{
    std::vector<ChunkCoord> result;
    const int radius = m_desc.load_radius_chunks;
    result.reserve(static_cast<std::size_t>((radius * 2 + 1) * (radius * 2 + 1)));

    for (int z = -radius; z <= radius; ++z) {
        for (int x = -radius; x <= radius; ++x) {
            result.push_back({
                .x = center.x + x,
                .y = center.y,
                .z = center.z + z,
            });
        }
    }

    return result;
}

} // namespace Valley::World
