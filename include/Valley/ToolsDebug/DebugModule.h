#pragma once

#include "Valley/Assets/AssetManager.h"
#include "Valley/Core/Module.h"

namespace Valley::ToolsDebug {

class DebugModule final : public Core::Module {
public:
    explicit DebugModule(const Assets::AssetManager* assets = nullptr);

    [[nodiscard]] std::string_view name() const override;
    void on_attach() override;
    void on_update(const Core::TimeStep& step) override;
    void on_detach() override;

private:
    const Assets::AssetManager* m_assets = nullptr;
};

} // namespace Valley::ToolsDebug
