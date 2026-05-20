#pragma once

#include "Valley/Core/TimeStep.h"

#include <string_view>

namespace Valley::Core {

class Module {
public:
    virtual ~Module() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;

    virtual void on_attach() {}
    virtual void on_fixed_update(const TimeStep&) {}
    virtual void on_update(const TimeStep&) {}
    virtual void on_detach() {}
};

} // namespace Valley::Core
