#ifndef INPUT_SERIALIZER_HH_F4E5BB7519898756BDB5624932F3E8C6//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define INPUT_SERIALIZER_HH_F4E5BB7519898756BDB5624932F3E8C6

#include "src/scan_task.hh"

#include <string>
#include <unordered_set>

namespace Fred {
namespace Akm {


class ScanTaskSerializer
{
private:
    unsigned long long insecure_domains_counter;
    unsigned long long secure_domains_counter;

public:
    ScanTaskSerializer() : insecure_domains_counter(0), secure_domains_counter(0) { }

    unsigned long long get_insecure_domains_counter() const
    {
        return insecure_domains_counter;
    }

    unsigned long long get_secure_domains_counter() const
    {
        return secure_domains_counter;
    }

    template<class Writter>
    void serialize_insecure(const NameserverToDomainScanTaskAdapter& _tasks, Writter& _writter)
    {
        bool insecure_marker_written = false;
        for (const auto& kv : _tasks)
        {
            const auto& nameserver = kv.first;
            std::string line;
            for (const auto task : _tasks.find_all(nameserver))
            {
                const auto& domain = task->domain;
                if (domain.has_keyset == false)
                {
                    if (!insecure_marker_written)
                    {
                        _writter("[insecure]\n");
                        insecure_marker_written = true;
                    }
                    line.append(" " + domain.fqdn);
                    insecure_domains_counter += 1;
                }
            }
            if (insecure_marker_written && !line.empty())
            {
                _writter(nameserver + line + "\n");
            }
        }
    }

    template<class Writter>
    void serialize_secure(const NameserverToDomainScanTaskAdapter& _tasks, Writter& _writter)
    {
        std::unordered_set<std::string> written_domains;
        bool secure_marker_written = false;
        const auto DELIMITER = " ";
        for (const auto& kv : _tasks)
        {
            const auto& nameserver = kv.first;
            for (const auto task : _tasks.find_all(nameserver))
            {
                const auto& domain = task->domain;
                if (domain.has_keyset == true && written_domains.count(domain.fqdn) == 0)
                {
                    if (!secure_marker_written)
                    {
                        _writter("[secure]\n");
                        _writter(domain.fqdn);
                        secure_marker_written = true;
                    }
                    else
                    {
                        _writter(DELIMITER + domain.fqdn);
                    }
                    written_domains.insert(domain.fqdn);
                    secure_domains_counter += 1;
                }
            }
        }
        if (secure_marker_written)
        {
            _writter("\n");
        }
    }

    template<class Writter>
    void serialize(const NameserverToDomainScanTaskAdapter& _tasks, Writter& _writter)
    {
        serialize_insecure(_tasks, _writter);
        serialize_secure(_tasks, _writter);
    }

    template<class Writter>
    void serialize(const DomainScanTaskCollection& _tasks, Writter& _writter)
    {
        NameserverToDomainScanTaskAdapter tasks_by_nameserver(_tasks);
        serialize(tasks_by_nameserver, _writter);
    }
};


} // namespace Akm
} // namespace Fred


#endif//INPUT_SERIALIZER_HH_F4E5BB7519898756BDB5624932F3E8C6
