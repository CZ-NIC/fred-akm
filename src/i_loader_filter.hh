#ifndef I_LOADER_OUTPUT_FILTER_HH_91740628434C3CA25C9FD101A19C777D//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_LOADER_OUTPUT_FILTER_HH_91740628434C3CA25C9FD101A19C777D

#include "src/i_loader.hh"
#include <vector>


namespace Fred {
namespace Akm {


class ILoaderOutputFilter
{
public:
    virtual ~ILoaderOutputFilter() = default;
    virtual void apply(DomainScanTaskCollection& _collection) const = 0;
};


} //namespace Akm
} //namespace Fred


#endif//I_LOADER_OUTPUT_FILTER_HH_91740628434C3CA25C9FD101A19C777D
