#ifndef FILE_HH_D9B81F2EB1F976701281C2A111CA7704//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define FILE_HH_D9B81F2EB1F976701281C2A111CA7704

#include "src/i_loader.hh"

#include <string>

namespace Fred {
namespace Akm {


class FileLoader : public ILoader
{
public:
    FileLoader(std::string _filename);

    void load_insecure_tasks(DomainScanTaskCollection& _scan_tasks) const;

    void load_secure_auto_tasks(DomainScanTaskCollection& _scan_tasks) const;

    void load_secure_noauto_tasks(DomainScanTaskCollection& _scan_tasks) const;

private:
    DomainScanTaskCollection scan_tasks_;
};


} //namespace Akm
} //namespace Fred


#endif//FILE_HH_D9B81F2EB1F976701281C2A111CA7704
