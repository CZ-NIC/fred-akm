#ifndef LOG_HH_ACCBB04A19D59AEBCAF02C3F24435571//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define LOG_HH_ACCBB04A19D59AEBCAF02C3F24435571

#include <spdlog/spdlog.h>

namespace Fred {
namespace Akm {


void setup_logging(const std::vector<std::string>& _sinks, const std::string& _level);

void setup_logging();

std::shared_ptr<spdlog::logger> log();


} // namespace Akm
} // namespace Fred

#endif//LOG_HH_ACCBB04A19D59AEBCAF02C3F24435571
