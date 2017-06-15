#ifndef CORBA_CONTEXT_HH_F0575D63F39590879FA80C010411F66A//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_CONTEXT_HH_F0575D63F39590879FA80C010411F66A

#include <memory>
#include <string>

#include "src/corba/nameservice.hh"

namespace Fred {
namespace Akm {
namespace Corba {


class CorbaContext
{
public:
    CorbaContext(int argc, char* argv[], const std::string& _ns_host, const unsigned int _ns_port);

    const Nameservice& get_nameservice() const;

private:
    CORBA::ORB_var orb_;
    Nameservice ns_;
};


} // namespace Fred
} // namespace Akm
} // namespace Corba

#endif//CORBA_CONTEXT_HH_F0575D63F39590879FA80C010411F66A
