#include "Valley/Renderer/RenderDiagnostics.h"

#include <algorithm>

namespace Valley::Renderer {

void TimingProfiler::record(std::string_view name, double ms)
{
    auto& r = m_records[std::string(name)];
    r.last = ms;
    r.total += ms;
    r.max = std::max(r.max, ms);
    r.n += 1;
}

std::vector<TimingSample> TimingProfiler::snapshot() const
{
    std::vector<TimingSample> out;
    out.reserve(m_records.size());
    for (const auto& [name, r] : m_records) {
        out.push_back({ .name = name, .last_ms = r.last, .avg_ms = r.n ? r.total / static_cast<double>(r.n) : 0.0, .max_ms = r.max, .samples = r.n });
    }
    return out;
}

} // namespace Valley::Renderer
