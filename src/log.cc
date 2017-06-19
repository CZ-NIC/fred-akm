#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

namespace Fred {
namespace Akm {


void setup_logging()
{
    spdlog::set_async_mode(8192, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::seconds(2));
    std::vector<spdlog::sink_ptr> sinks = {
        std::make_shared<spdlog::sinks::stdout_sink_st>(),
        std::make_shared<spdlog::sinks::simple_file_sink_st>("fred-akm.log"),
    };
    auto logger = std::make_shared<spdlog::logger>("fred-akm", std::begin(sinks), std::end(sinks));
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);
}

std::shared_ptr<spdlog::logger> log()
{
    auto logger = spdlog::get("fred-akm");
    if (!logger)
    {
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_st>();
        logger = std::make_shared<spdlog::logger>("fred-akm", null_sink);
    }
    return logger;
}


} // namespace Akm
} // namespace Fred


