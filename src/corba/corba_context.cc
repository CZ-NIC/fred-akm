#include "src/corba/corba_context.hh"

namespace Fred {
namespace Akm {
namespace Corba {


CorbaContext::CorbaContext(int _argc, char* _argv[], const std::string& _ns_host, const unsigned int _ns_port, const char* options[][2])
    : orb_(CORBA::ORB_init(_argc, _argv, "", options)),
      ns_(orb_, _ns_host, _ns_port)
{
}


const Nameservice& CorbaContext::get_nameservice() const
{
    return ns_;
}


} // namespace Fred
} // namespace Akm
} // namespace Corba



