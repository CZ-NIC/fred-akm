#ifndef I_MAILER_HH_6D59404C3B72544DBC5990EF3624B613//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define I_MAILER_HH_6D59404C3B72544DBC5990EF3624B613

#include <string>
#include <map>
#include <boost/optional.hpp>

#include "src/corba/generated/Mailer.hh"

namespace Fred {
namespace Akm {


class IMailer
{
public:
    typedef std::string TemplateName;
    typedef std::map<std::string, std::string> TemplateParameters;

    struct Header
    {
        Header(const std::string& _to,
               const boost::optional<std::string>& _from = boost::optional<std::string>(),
               const boost::optional<std::string>& _reply_to = boost::optional<std::string>())
            : to(_to), from(_from), reply_to(_reply_to)
        {
        }

        std::string to;
        boost::optional<std::string> from;
        boost::optional<std::string> reply_to;
    };

    virtual void enqueue(
        const Header& _header,
        const TemplateName& _template_name,
        const TemplateParameters& _template_params
    ) const = 0;
};


} // namespace Fred
} // namespace Akm

#endif//I_MAILER_HH_6D59404C3B72544DBC5990EF3624B613
