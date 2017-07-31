#include "src/cdnskey_scanner_impl/output_parser.hh"
#include "src/utils.hh"

#include <vector>
#include <string>
#include <boost/lexical_cast.hpp>

namespace Fred {
namespace Akm {

namespace {

    static constexpr const char* RESULT_TYPE_UNRESOLVED_IP = "unresolved-ip";
    static constexpr const char* RESULT_TYPE_INSECURE = "insecure";
    static constexpr const char* RESULT_TYPE_INSECURE_EMPTY = "insecure-empty";
    static constexpr const char* RESULT_TYPE_UNRESOLVED = "unresolved";
    static constexpr const char* RESULT_TYPE_SECURE = "secure";
    static constexpr const char* RESULT_TYPE_SECURE_EMPTY = "secure-empty";
    static constexpr const char* RESULT_TYPE_UNTRUSTWORTHY = "untrustworthy";
    static constexpr const char* RESULT_TYPE_UNKNOWN = "unknown";

    typedef std::vector<std::string> Tokens;

    ScanResult parse_type_insecure(const Tokens& _tokens)
    {
        ScanResult result;
        result.cdnskey_status = RESULT_TYPE_INSECURE;

        result.nameserver = _tokens.at(1);
        result.nameserver_ip = _tokens.at(2);
        result.domain_name = _tokens.at(3);
        result.cdnskey_flags = boost::lexical_cast<int>(_tokens.at(4));
        result.cdnskey_proto = boost::lexical_cast<int>(_tokens.at(5));
        result.cdnskey_alg = boost::lexical_cast<int>(_tokens.at(6));
        result.cdnskey_public_key = _tokens.at(7);

        return result;
    }

    ScanResult parse_type_insecure_empty(const Tokens& _tokens)
    {
        ScanResult result;
        result.cdnskey_status = RESULT_TYPE_INSECURE_EMPTY;

        result.nameserver = _tokens.at(1);
        result.nameserver_ip = _tokens.at(2);
        result.domain_name = _tokens.at(3);

        return result;
    }

    ScanResult parse_type_unresolved(const Tokens& _tokens)
    {
        ScanResult result;
        result.cdnskey_status = RESULT_TYPE_UNRESOLVED;

        result.nameserver = _tokens.at(1);
        result.nameserver_ip = _tokens.at(2);
        result.domain_name = _tokens.at(3);

        return result;
    }

    ScanResult parse_type_secure(const Tokens& _tokens)
    {
        ScanResult result;
        result.cdnskey_status = RESULT_TYPE_SECURE;

        result.domain_name = _tokens.at(1);
        result.cdnskey_flags = boost::lexical_cast<int>(_tokens.at(2));
        result.cdnskey_proto = boost::lexical_cast<int>(_tokens.at(3));
        result.cdnskey_alg = boost::lexical_cast<int>(_tokens.at(4));
        result.cdnskey_public_key = _tokens.at(5);

        return result;
    }

    ScanResult parse_type_untrustworthy(const Tokens& _tokens)
    {
        ScanResult result;
        result.cdnskey_status = RESULT_TYPE_UNTRUSTWORTHY;

        result.domain_name = _tokens.at(1);

        return result;
    }

    ScanResult parse_type_unknown(const Tokens& _tokens)
    {
        ScanResult result;
        result.cdnskey_status = RESULT_TYPE_UNKNOWN;

        result.domain_name = _tokens.at(1);

        return result;
    }

    ScanResult parse_type_secure_empty(const Tokens& _tokens)
    {
        ScanResult result;
        result.cdnskey_status = RESULT_TYPE_SECURE_EMPTY;

        result.domain_name = _tokens.at(1);

        return result;
    }

    ScanResult parse_type_unresolved_ip(const Tokens& _tokens)
    {
        ScanResult result;
        result.cdnskey_status = RESULT_TYPE_UNRESOLVED_IP;

        result.nameserver = _tokens.at(1);

        return result;
    }

}

ScanResult ScanResultParser::parse(const std::string& _line) const
{
    std::vector<std::string> tokens;
    tokens.reserve(8);
    split_on(_line, ' ', tokens);

    typedef std::function<ScanResult(const Tokens&)> SubParser;
    const std::map<std::string, SubParser> subparsers_mapping = {
        {RESULT_TYPE_UNRESOLVED_IP, parse_type_unresolved_ip},
        {RESULT_TYPE_INSECURE, parse_type_insecure},
        {RESULT_TYPE_INSECURE_EMPTY, parse_type_insecure_empty},
        {RESULT_TYPE_UNRESOLVED, parse_type_unresolved},
        {RESULT_TYPE_SECURE, parse_type_secure},
        {RESULT_TYPE_SECURE_EMPTY, parse_type_secure_empty},
        {RESULT_TYPE_UNTRUSTWORTHY, parse_type_untrustworthy},
        {RESULT_TYPE_UNKNOWN, parse_type_unknown},
    };

    if (tokens.size() == 0)
    {
        throw std::runtime_error("zero tokens parsed from scan result");
    }
    try
    {
        auto &subparser = subparsers_mapping.at(tokens.at(0));
        try
        {
            return subparser(tokens);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("scan result type '" + tokens.at(0) + "' parser error");
        }
    }
    catch (const std::out_of_range&)
    {
        throw std::runtime_error("unrecognized/unimplemented scan result type");
    }
}


std::vector<ScanResult> ScanResultParser::parse_multi(std::string& _lines) const
{
    const auto NL = '\n';
    std::vector<ScanResult> results;
    auto newline_ptr = std::find(_lines.begin(), _lines.end(), NL);
    while (newline_ptr != _lines.end())
    {
        const auto line = std::string(_lines.begin(), newline_ptr);
        _lines.erase(_lines.begin(), newline_ptr + 1);
        results.emplace_back(this->parse(line));
        newline_ptr = std::find(_lines.begin(), _lines.end(), NL);
    }
    return results;
}


} // namespace Akm
} // namespace Fred
