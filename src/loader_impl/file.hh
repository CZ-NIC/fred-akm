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

    void load_domains(NameserverDomainsCollection&) const;

private:
    std::string filename_;
};


} //namespace Akm
} //namespace Fred


#endif//FILE_HH_D9B81F2EB1F976701281C2A111CA7704
