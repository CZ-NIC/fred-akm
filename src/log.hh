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
#ifndef LOG_HH_ACCBB04A19D59AEBCAF02C3F24435571//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define LOG_HH_ACCBB04A19D59AEBCAF02C3F24435571

#include "3rd_party/spdlog/include/spdlog/spdlog.h"

namespace Fred {
namespace Akm {


void setup_logging(const std::vector<std::string>& _sinks);

void setup_logging();

std::shared_ptr<spdlog::logger> log();


} // namespace Akm
} // namespace Fred

#endif//LOG_HH_ACCBB04A19D59AEBCAF02C3F24435571
