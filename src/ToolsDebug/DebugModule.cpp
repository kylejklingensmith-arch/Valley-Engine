#include "Valley/ToolsDebug/DebugModule.h"

#include "Valley/Core/Logger.h"

namespace Valley::ToolsDebug {

std::string_view DebugModule::name() const
{
    return "ToolsDebug";
}

void DebugModule::on_attach()
{
    Core::Log::info("ToolsDebug", "Debug module attached. Diagnostics only; no gameplay tooling yet.");
}

void DebugModule::on_update(const Core::TimeStep& step)
{
    if (step.frame_index == 0) {
        Core::Log::info("ToolsDebug", "Engine loop reached its first frame.");
    }
}

void DebugModule::on_detach()
{
    Core::Log::info("ToolsDebug", "Debug module detached.");
}

} // namespace Valley::ToolsDebug
