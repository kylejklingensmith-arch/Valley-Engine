#pragma once

#include "Valley/World/FloatingOrigin.h"
#include "Valley/World/WorldDebug.h"
#include "Valley/World/WorldRegistry.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Valley::World {

struct WorldStreamerDesc {
    WorldGridDesc grid;
    int load_radius_chunks = 1;
};

class WorldStreamer {
public:
    explicit WorldStreamer(WorldStreamerDesc desc = {});
    ~WorldStreamer();

    WorldStreamer(const WorldStreamer&) = delete;
    WorldStreamer& operator=(const WorldStreamer&) = delete;

    void start();
    void shutdown();
    void request_load(const ChunkCoord& coord);
    void request_unload(const ChunkCoord& coord);
    void update_observer(const WorldVec3& observer_world_position);
    void service_completed_requests();
    void wait_until_idle();

    [[nodiscard]] const WorldRegistry& registry() const;
    [[nodiscard]] WorldRegistry& registry();
    [[nodiscard]] const WorldStreamerDesc& desc() const;

private:
    enum class RequestKind {
        Load,
        Unload,
    };

    struct Request {
        RequestKind kind = RequestKind::Load;
        ChunkCoord coord;
    };

    struct Completion {
        RequestKind kind = RequestKind::Load;
        ChunkRecord record;
    };

    void worker_loop();
    void enqueue(Request request);
    [[nodiscard]] std::vector<ChunkCoord> desired_chunks_around(const ChunkCoord& center) const;

    WorldStreamerDesc m_desc;
    WorldRegistry m_registry;

    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::condition_variable m_idle_condition;
    std::queue<Request> m_requests;
    std::queue<Completion> m_completions;
    std::thread m_worker;
    bool m_running = false;
    bool m_stopping = false;
    unsigned int m_in_flight = 0;
};

} // namespace Valley::World
