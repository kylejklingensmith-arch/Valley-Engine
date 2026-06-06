#include "Valley/Core/Application.h"
#include "Valley/Core/Logger.h"
#include "Valley/Renderer/RendererModule.h"
#include "Valley/Simulation/SimulationModule.h"
#include "Valley/ToolsDebug/DebugModule.h"
#include "Valley/World/WorldModule.h"

#include <charconv>
#include <exception>
#include <string>
#include <memory>
#include <string_view>

namespace {

void print_usage()
{
    Valley::Core::Log::info("App", "Usage: ValleyEngine [--headless] [--frames N] [--paused] [--time-scale S] [--fixed-step S] [--step N] [--renderer-path raster|splat] [--dump-render PATH]");
}

bool parse_unsigned(std::string_view text, unsigned long long& value)
{
    const auto* begin = text.data();
    const auto* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc {} && result.ptr == end;
}

bool parse_double(std::string_view text, double& value)
{
    try {
        std::size_t parsed_characters = 0;
        const std::string owned_text(text);
        value = std::stod(owned_text, &parsed_characters);
        return parsed_characters == owned_text.size();
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace

int main(int argc, char** argv)
{
    Valley::Core::ApplicationDesc desc;
    Valley::Renderer::RendererModuleDesc renderer_desc;
    desc.window.title = "Valley Engine";
    desc.window.width = 1280;
    desc.window.height = 720;

    for (int index = 1; index < argc; ++index) {
        const std::string_view argument = argv[index];

        if (argument == "--headless") {
            desc.window.headless = true;
        } else if (argument == "--paused") {
            desc.clock.start_paused = true;
        } else if (argument == "--time-scale") {
            if (index + 1 >= argc || !parse_double(argv[index + 1], desc.clock.time_scale)) {
                Valley::Core::Log::error("App", "--time-scale requires a non-negative number.");
                print_usage();
                return 2;
            }
            ++index;
        } else if (argument == "--fixed-step") {
            if (index + 1 >= argc || !parse_double(argv[index + 1], desc.clock.fixed_timestep_seconds)) {
                Valley::Core::Log::error("App", "--fixed-step requires a positive number of seconds.");
                print_usage();
                return 2;
            }
            ++index;
        } else if (argument == "--step") {
            unsigned long long step_count = 0;
            if (index + 1 >= argc || !parse_unsigned(argv[index + 1], step_count)) {
                Valley::Core::Log::error("App", "--step requires an unsigned integer value.");
                print_usage();
                return 2;
            }
            desc.clock.queued_debug_steps = static_cast<unsigned int>(step_count);
            ++index;
        } else if (argument == "--renderer-path") {
            if (index + 1 >= argc) {
                Valley::Core::Log::error("App", "--renderer-path requires raster or splat.");
                print_usage();
                return 2;
            }

            const std::string_view renderer_path = argv[index + 1];
            if (renderer_path == "raster") {
                renderer_desc.renderer_path = Valley::Renderer::RendererPath::TraditionalRaster;
            } else if (renderer_path == "splat") {
                renderer_desc.renderer_path = Valley::Renderer::RendererPath::ExperimentalEquationSplat;
            } else {
                Valley::Core::Log::error("App", "--renderer-path requires raster or splat.");
                print_usage();
                return 2;
            }
            ++index;
        } else if (argument == "--dump-render") {
            if (index + 1 >= argc) {
                Valley::Core::Log::error("App", "--dump-render requires an output PPM path.");
                print_usage();
                return 2;
            }
            renderer_desc.debug_frame_path = argv[index + 1];
            ++index;
        } else if (argument == "--frames") {
            if (index + 1 >= argc || !parse_unsigned(argv[index + 1], desc.max_frames)) {
                Valley::Core::Log::error("App", "--frames requires an unsigned integer value.");
                print_usage();
                return 2;
            }
            ++index;
        } else if (argument == "--help" || argument == "-h") {
            print_usage();
            return 0;
        } else {
            Valley::Core::Log::error("App", "Unknown command-line argument.");
            print_usage();
            return 2;
        }
    }

    try {
        Valley::Core::Application app(desc);
        app.add_module(std::make_unique<Valley::World::WorldModule>());
        app.add_module(std::make_unique<Valley::Simulation::SimulationModule>());
        app.add_module(std::make_unique<Valley::Renderer::RendererModule>(renderer_desc, &app.input(), &app.debug_controls()));
        app.add_module(std::make_unique<Valley::ToolsDebug::DebugModule>());
        return app.run();
    } catch (const std::exception& exception) {
        Valley::Core::Log::error("App", exception.what());
        return 1;
    }
}
