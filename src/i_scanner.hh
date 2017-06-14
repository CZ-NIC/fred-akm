#ifndef I_SCANNER_H_BA9A1E563A15C26233F87B62E27EB0FD//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_SCANNER_H_BA9A1E563A15C26233F87B62E27EB0FD

#include <string>
#include <vector>
#include <boost/optional.hpp>

#include "src/nameserver_domains.hh"

namespace Fred {
namespace Akm {


enum class CdnskeyStatus
{
    UNKNOWN,
    INSECURE,
    SECURE,
    UNTRUSTWORTHY,
    UNRESOLVED
};

struct ScanResult
{
    //CdnskeyStatus cdnskey_status;
    std::string cdnskey_status;
    std::string domain_name;
    boost::optional<std::string> nameserver;
    boost::optional<std::string> nameserver_ip;
    boost::optional<int> cdnskey_flags;
    boost::optional<int> cdnskey_proto;
    boost::optional<int> cdnskey_alg;
    boost::optional<std::string> cdnskey_public_key;
};


typedef std::vector<ScanResult> ScanResults;


class IScanner
{
public:
    typedef std::function<void(const ScanResult& _result)> OnResultCallback;

    //virtual ~IScanner() = 0;

    virtual void add_tasks(std::vector<NameserverDomains>& _tasks) = 0;

    virtual void scan(OnResultCallback _callback) const = 0;
};


} //namespace Akm
} //namespace Fred


#endif//I_SCANNER_H_BA9A1E563A15C26233F87B62E27EB0FD
