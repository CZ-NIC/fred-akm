#ifndef CORBA_NAMESERVICE_D305B935F627D70603120E224F16D731//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_NAMESERVICE_D305B935F627D70603120E224F16D731

#include <string>
#include <omniORB4/CORBA.h>

namespace Fred {
namespace Akm {
namespace Corba {


class Nameservice
{
public:
    Nameservice(CORBA::ORB_ptr _orb, const std::string &_host, unsigned int _port);

    CORBA::Object_ptr resolve(const std::string& _object_path) const;

private:
    CORBA::ORB_ptr orb_;
    std::string host_;
    unsigned int port_;
};


} // namespace Fred
} // namespace Akm
} // namespace Corba

#endif//CORBA_NAMESERVICE_D305B935F627D70603120E224F16D731
