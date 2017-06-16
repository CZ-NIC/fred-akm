#include <regex>

#include "src/utils.hh"

namespace Fred {
namespace Akm {


void split_on(const std::string& _in_string, const char _delimiter, std::vector<std::string>& _out_tokens)
{
    auto beg_ptr = _in_string.begin();
    auto end_ptr = _in_string.end();
    auto nxt_ptr = std::find(beg_ptr, end_ptr, _delimiter);
    while (nxt_ptr != end_ptr)
    {
        _out_tokens.emplace_back(std::string(beg_ptr, nxt_ptr));
        beg_ptr = nxt_ptr + 1;
        nxt_ptr = std::find(beg_ptr, end_ptr, _delimiter);
    }
    _out_tokens.emplace_back(beg_ptr, nxt_ptr);
}


} // namespace Akm
} // namespace Fred
