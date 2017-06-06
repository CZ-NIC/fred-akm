#include "src/corba/mailer.hh"

namespace Fred {
namespace Akm {
namespace Corba {


Mailer::Mailer(const Nameservice& _ns, const std::string& _ns_path_mailer)
    : ns_(_ns), _ns_path_mailer_(_ns_path_mailer)
{
}


void Mailer::enqueue(
    const Header& _header,
    const TemplateName& _template_name,
    const TemplateParameters& _template_params
) const
{
    try
    {

        ccReg::Mailer_var mailer = ccReg::Mailer::_narrow(ns_.resolve(_ns_path_mailer_));

        ccReg::MailHeader header;
        header.h_to = CORBA::string_dup(_header.to.c_str());
        if (_header.from)
        {
            header.h_from = CORBA::string_dup((*_header.from).c_str());
        }
        if (_header.reply_to)
        {
           header.h_reply_to = CORBA::string_dup((*_header.reply_to).c_str());
        }

        ccReg::KeyValues params;
        params.length(_template_params.size());
        auto params_idx = 0;
        for (const auto& kv : _template_params)
        {
            params[params_idx].key = CORBA::string_dup(kv.first.c_str());
            params[params_idx].value = CORBA::string_dup(kv.second.c_str());

            params_idx += 1;
        }

        CORBA::String_var dummy;
        CORBA::Long mail_id = mailer->mailNotify(
            CORBA::string_dup(_template_name.c_str()),
            header,
            params,
            ccReg::Lists(),
            ccReg::Attachment_seq(),
            false,
            dummy
        );
    }
    catch (const CORBA::SystemException &e)
    {
        throw std::runtime_error(e._name());
    }
    catch (const CORBA::Exception &e)
    {
        throw std::runtime_error(e._name());
    }
}


} // namespace Fred
} // namespace Akm
} // namespace Corba
