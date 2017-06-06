#ifndef CORBA_MAILER_HH_207838907A1B10CB5C28A8E6721AFAD7//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_MAILER_HH_207838907A1B10CB5C28A8E6721AFAD7

#include "src/corba/nameservice.hh"
#include "src/i_mailer.hh"

namespace Fred {
namespace Akm {
namespace Corba {


class Mailer : public IMailer
{
public:
    Mailer(const Nameservice& _ns, const std::string& _ns_path_mailer);

    void enqueue(
        const Header& _header,
        const TemplateName& _template_name,
        const TemplateParameters& _template_params
    ) const;

private:
    const Nameservice& ns_;
    std::string _ns_path_mailer_;
};


} // namespace Fred
} // namespace Akm
} // namespace Corba

#endif//CORBA_MAILER_HH_207838907A1B10CB5C28A8E6721AFAD7
