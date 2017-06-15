#ifndef COMMAND_SCAN_HH_71A7268F402AFF447235AE37B5B43621//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define COMMAND_SCAN_HH_71A7268F402AFF447235AE37B5B43621

#include "src/i_storage.hh"
#include "src/i_scanner.hh"

namespace Fred {
namespace Akm {


void command_scan(const IStorage& _storage, IScanner& _scanner);


} //namespace Akm
} //namespace Fred

#endif//COMMAND_SCAN_HH_71A7268F402AFF447235AE37B5B43621
