#pragma once

#include "Valley/Core/Module.h"
#include "Valley/Simulation/SimulationScheduler.h"

namespace Valley::Simulation {

class SimulationModule final : public Core::Module {
public:
    [[nodiscard]] std::string_view name() const override;
    void on_attach() override;
    void on_fixed_update(const Core::TimeStep& step) override;
    void on_update(const Core::TimeStep& step) override;
    void on_detach() override;

    [[nodiscard]] const SimulationScheduler& scheduler() const;
    [[nodiscard]] SimulationScheduler& scheduler();

private:
    SimulationScheduler m_scheduler;
    unsigned long long m_last_reported_tick_count = 0;
};

} // namespace Valley::Simulation
