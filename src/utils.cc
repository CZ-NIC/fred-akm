#include <regex>

#include "src/utils.hh"

namespace Fred {
namespace Akm {


std::vector<std::string> split_on(const std::string& _string, const char delimiter)
{
    std::regex split_on("[" + std::string(1, delimiter) + "]");
    std::sregex_token_iterator beg(_string.begin(), _string.end(), split_on, -1);
    std::sregex_token_iterator end;
    return {beg, end};
}


} // namespace Akm
} // namespace Fred
