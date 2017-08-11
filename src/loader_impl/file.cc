#include "src/loader_impl/file.hh"
#include "src/log.hh"
#include "src/utils.hh"

#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace Fred {
namespace Akm {


FileLoader::FileLoader(std::string _filename) : filename_(std::move(_filename))
{
}


void FileLoader::load_domains(DomainScanTaskCollection& _collection) const
{
    std::ifstream file(filename_, std::ifstream::ate | std::ifstream::binary);
    const auto size = file.tellg();
    file.clear();
    file.seekg(0, std::ios::beg);

    log()->info("input file size: {} [b]", size);

    std::string line;
    const auto KiB = 1024;
    line.reserve(KiB);

    DomainScanTaskCollection collection;
    while (std::getline(file, line))
    {
        std::vector<std::string> tokens;
        tokens.reserve(4);
        split_on(line, ' ', tokens);

        if (tokens.size() == 4)
        {
            const auto& current_ns = tokens[0];
            const auto domain = Domain(
                boost::lexical_cast<unsigned long long>(tokens[1]),
                tokens[2],
                boost::lexical_cast<bool>(tokens[3])
            );

            collection.insert_or_update(domain, current_ns);
        }
        else
        {
            log()->error("not enough tokens skipping (line={})", line);
        }
    }

    _collection.merge(collection);
    log()->info("loaded tasks from input file ({} domain(s))", _collection.size());
}


} //namespace Akm
} //namespace Fred
