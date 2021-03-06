/*
 * Copyright (C) 2017-2018  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "config.h"

#include "src/utils.hh"

#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/spdlog.h>

#include <iostream>

namespace Fred {
namespace Akm {

namespace {

spdlog::level::level_enum to_spdlog_level(const std::string& _level)
{
    if (_level == "trace")
    {
        return spdlog::level::trace;
    }
    else if (_level == "debug")
    {
        return spdlog::level::debug;
    }
    else if (_level == "info")
    {
        return spdlog::level::info;
    }
    else if (_level == "warn" || _level == "warning")
    {
        return spdlog::level::warn;
    }
    else if (_level == "err" || _level == "error")
    {
        return spdlog::level::err;
    }
    else if (_level == "critical")
    {
        return spdlog::level::critical;
    }
    else
    {
        return spdlog::level::info;
    }
}

} // namespace Fred::Akm::{anonymous}

void setup_logging()
{
    spdlog::set_async_mode(8192, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::seconds(2));
    auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_st>();

    std::vector<spdlog::sink_ptr> sinks = {
        std::make_shared<spdlog::sinks::stdout_sink_st>(),
        std::make_shared<spdlog::sinks::simple_file_sink_st>("fred-akm.log"),
#ifdef SPDLOG_ENABLE_SYSLOG
        std::make_shared<spdlog::sinks::syslog_sink>(),
#endif
    };
    auto logger = std::make_shared<spdlog::logger>("fred-akm", std::begin(sinks), std::end(sinks));
    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);
}


void setup_logging(const std::vector<std::string>& _sinks)
{
    std::vector<spdlog::sink_ptr> sinks;

    for (const auto& sink : _sinks)
    {
        std::vector<std::string> sink_tokens;
        split_on(sink, ' ', sink_tokens);
        if (sink_tokens.size())
        {
            const std::string& sink_name = sink_tokens.front();
            const std::string& sink_level = sink_tokens.back();
            const bool sink_extra_arguments = sink_tokens.size() > 2;
            if (sink_name == "console")
            {
                auto sink_console = std::make_shared<spdlog::sinks::stdout_sink_st>();
                sink_console->set_level(to_spdlog_level(sink_level));
                sinks.push_back(sink_console);
            }
            else if (sink_name == "file" && sink_extra_arguments)
            {
                auto sink_file = std::make_shared<spdlog::sinks::simple_file_sink_st>(sink_tokens.at(1));
                sink_file->set_level(to_spdlog_level(sink_level));
                sinks.push_back(sink_file);
            }
#ifdef SPDLOG_ENABLE_SYSLOG
            else if (sink_name == "syslog")
            {
                auto sink_syslog = sink_extra_arguments
                                           ? std::make_shared<spdlog::sinks::syslog_sink>(sink_tokens.at(1))
                                           : std::make_shared<spdlog::sinks::syslog_sink>();
                sink_syslog->set_level(to_spdlog_level(sink_level));
                sinks.push_back(sink_syslog);
            }
#endif
        }
    }
    if (sinks.size() == 0)
    {
        std::cerr << "no logging sinks specified..." << std::endl;
        return;
    }

    auto logger = std::make_shared<spdlog::logger>("fred-akm", std::begin(sinks), std::end(sinks));
    logger->set_level(spdlog::level::trace); // default level is spdlog::level::info, setting this to maximum enables sink-specific levels to apply as expected
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


