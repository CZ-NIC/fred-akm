#ifndef EXTERNAL_CDNSKEY_SCANNER_IMPL_HH_26D42C56B1493D7951148C69A80A5BA9//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define EXTERNAL_CDNSKEY_SCANNER_IMPL_HH_26D42C56B1493D7951148C69A80A5BA9

#include "src/i_scanner.hh"

namespace Fred {
namespace Akm {


class ExternalCdnskeyScannerImpl : public IScanner
{
public:
    ExternalCdnskeyScannerImpl(const std::string& _scanner_path);

    void scan(const NameserverDomainsCollection& _tasks, OnResultsCallback _callback) const;

private:
    std::vector<std::string> scanner_path_;
    NameserverDomainsCollection tasks_;
};


} //namespace Akm
} //namespace Fred


#endif//EXTERNAL_CDNSKEY_SCANNER_IMPL_HH_26D42C56B1493D7951148C69A80A5BA9
