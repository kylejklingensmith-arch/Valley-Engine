#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Valley::Renderer {

struct TimingSample {
    std::string name;
    double last_ms = 0.0;
    double avg_ms = 0.0;
    double max_ms = 0.0;
    unsigned long long samples = 0;
};

class TimingProfiler {
public:
    void record(std::string_view name, double ms);
    [[nodiscard]] std::vector<TimingSample> snapshot() const;
private:
    struct Acc { double last = 0.0; double total = 0.0; double max = 0.0; unsigned long long n = 0; };
    std::unordered_map<std::string, Acc> m_records;
};

struct MemoryStats {
    std::size_t mesh_cache_bytes = 0;
    std::size_t texture_cache_bytes = 0;
    std::size_t total_bytes = 0;
};

struct FrameDiagnostics {
    TimingProfiler cpu;
    TimingProfiler gpu;
    TimingProfiler pass;
    TimingProfiler chunk_streaming;
    TimingProfiler terrain;
    MemoryStats memory;
    std::size_t draw_calls = 0;
};

} // namespace Valley::Renderer
