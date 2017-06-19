#ifndef CDNSKEY_SCANNER_HH_26D42C56B1493D7951148C69A80A5BA9//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define CDNSKEY_SCANNER_HH_26D42C56B1493D7951148C69A80A5BA9

#include "src/i_scanner.hh"

namespace Fred {
namespace Akm {


class ExternalScannerTool : public IScanner
{
public:
    ExternalScannerTool(const std::string& _external_tool_path, unsigned long long _queries_per_second);

    void scan(const NameserverDomainsCollection& _tasks, OnResultsCallback _callback) const;

private:
    unsigned long long queries_per_second_;
    std::vector<std::string> external_tool_path_;
    NameserverDomainsCollection tasks_;
};


} //namespace Akm
} //namespace Fred


#endif//CDNSKEY_SCANNER_HH_26D42C56B1493D7951148C69A80A5BA9
