#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

#include "src/utils.hh"

namespace Fred {
namespace Akm {


void setup_logging()
{
    spdlog::set_async_mode(8192, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::seconds(2));
    auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_st>();

    std::vector<spdlog::sink_ptr> sinks = {
        std::make_shared<spdlog::sinks::stdout_sink_st>(),
        std::make_shared<spdlog::sinks::simple_file_sink_st>("fred-akm.log"),
    };
    auto logger = std::make_shared<spdlog::logger>("fred-akm", std::begin(sinks), std::end(sinks));
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);
}


void setup_logging(const std::vector<std::string>& _sinks, const std::string& _level)
{
    std::vector<spdlog::sink_ptr> sinks;

    for (const auto& sink : _sinks)
    {
        std::vector<std::string> sink_tokens;
        split_on(sink, ' ', sink_tokens);
        if (sink_tokens.size())
        {
            if (sink_tokens.at(0) == "console")
            {
                sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
            }
            else if (sink_tokens.at(0) == "file" && sink_tokens.size() == 2)
            {
                sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_st>(sink_tokens.at(1)));
            }
        }
    }
    if (sinks.size() == 0)
    {
        std::cerr << "no logging sinks specified..." << std::endl;
        return;
    }

    auto logger = std::make_shared<spdlog::logger>("fred-akm", std::begin(sinks), std::end(sinks));
    if (_level == "trace")
    {
        logger->set_level(spdlog::level::trace);
    }
    else if (_level == "debug")
    {
        logger->set_level(spdlog::level::debug);
    }
    else if (_level == "info")
    {
        logger->set_level(spdlog::level::info);
    }
    else if (_level == "warn")
    {
        logger->set_level(spdlog::level::warn);
    }
    else if (_level == "err")
    {
        logger->set_level(spdlog::level::err);
    }
    else if (_level == "critical")
    {
        logger->set_level(spdlog::level::critical);
    }
    else
    {
        logger->set_level(spdlog::level::info);
    }
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


