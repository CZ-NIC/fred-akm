cmake_minimum_required(VERSION 3.5)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(program_name fred-akm)
project(${program_name} CXX)

if(NOT VERSION)
    execute_process(COMMAND "git" "describe" "--tags"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    OUTPUT_VARIABLE RAWVERSION)
    string(REGEX REPLACE "-.*\n" "" VERSION ${RAWVERSION})
endif()

find_package(Boost 1.53.0
    COMPONENTS
    program_options
    regex
    REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(OMNIORB4 REQUIRED omniORB4>=4.1.2)
pkg_search_module(OMNIDYNAMIC4 REQUIRED omniDynamic4>=4.1.2)
pkg_search_module(SQLITE3 REQUIRED sqlite3)

macro(create_link_function some_package its_dependencies)
    function(link_targets_against_${some_package})
        set(func_ARGN ARGN)
        foreach(target_name ${${func_ARGN}})
            message(STATUS "link against " ${some_package} ": " ${target_name})
            target_link_libraries(${target_name} ${${its_dependencies}})
        endforeach()
    endfunction()
endmacro()

create_link_function(boost_regex Boost_REGEX_LIBRARY)
create_link_function(boost_program_options Boost_PROGRAM_OPTIONS_LIBRARY)
create_link_function(omniorb4 OMNIORB4_LIBRARIES)
create_link_function(omnidynamic4 OMNIDYNAMIC4_LIBRARIES)
create_link_function(sqlite3 SQLITE3_LIBRARIES)

if(NOT IDL_DIR)
    set(IDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../idl/idl)
endif()
if(NOT SPDLOG_ENABLE_SYSLOG)
    set(SPDLOG_ENABLE_SYSLOG "")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

function(compile_idl_files)
    message(STATUS "Compiling idl...")
    execute_process(
        COMMAND mkdir -p generated
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/corba/)
    foreach(idl_name ${ARGN})
        execute_process(
            COMMAND omniidl -bcxx -Wbuse_quotes -Wba -C . ${IDL_DIR}/${idl_name}.idl
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/corba/generated/
            OUTPUT_VARIABLE OMNIIDL_MESSAGE${idl_name})
        message(STATUS ${OMNIIDL_MESSAGE}${idl_name})
    endforeach()
    message(STATUS "...done")
endfunction()

compile_idl_files(
    _dataTypes
    DateTime
    Mailer
    AutomaticKeysetManagement)

function(set_common_properties_on_targets)
    foreach(target_name ${ARGN})
        message(STATUS "setting properties on: " ${target_name})
        set_target_properties(${target_name} PROPERTIES
            CXX_STANDARD 14
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
        target_compile_options(${target_name}
            PUBLIC "-O2"
            PUBLIC "-ggdb"
            PUBLIC "-grecord-gcc-switches"
            PUBLIC "-Wall"
            PUBLIC "-Wextra"
            PUBLIC "-Wno-unused-parameter"
            PUBLIC "-fdiagnostics-color=auto")
        target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
        target_include_directories(${target_name} PUBLIC ${CMAKE_BINARY_DIR})
    endforeach()
endfunction()

add_library(mailer STATIC
    src/corba/generated/DateTimeSK.cc
    src/corba/generated/_dataTypesSK.cc
    src/corba/generated/MailerSK.cc
    src/corba/generated/DateTimeDynSK.cc
    src/corba/generated/DateTime.hh
    src/corba/generated/_dataTypesDynSK.cc
    src/corba/generated/_dataTypes.hh
    src/corba/generated/MailerDynSK.cc
    src/corba/generated/Mailer.hh
    src/corba/mailer.cc
    src/corba/mailer.hh)

add_library(akm STATIC
    src/corba/generated/AutomaticKeysetManagementSK.cc
    src/corba/generated/AutomaticKeysetManagementDynSK.cc
    src/corba/generated/AutomaticKeysetManagement.hh
    src/corba/akm.cc
    src/corba/akm.hh)

add_executable(${program_name} src/main.cc)

set_common_properties_on_targets(${program_name} mailer akm)

target_sources(${program_name}
    PRIVATE 3rd_party/sqlite3pp/headeronly_src/sqlite3pp.h
    PRIVATE 3rd_party/sqlite3pp/headeronly_src/sqlite3pp.ipp
    PRIVATE src/i_mailer.hh
    PRIVATE src/i_storage.hh
    PRIVATE src/i_scanner.hh
    PRIVATE src/i_loader.hh
    PRIVATE src/i_loader_filter.hh
    PRIVATE src/i_akm.hh
    PRIVATE src/enum_conversions.hh
    PRIVATE src/type_instance_map.hh
    PRIVATE src/from_string.hh
    PRIVATE src/scan_type.hh
    PRIVATE src/scan_type.cc
    PRIVATE src/utils.hh
    PRIVATE src/utils.cc
    PRIVATE src/args.hh
    PRIVATE src/args.cc
    PRIVATE src/conf.hh
    PRIVATE src/conf.cc
    PRIVATE src/log.hh
    PRIVATE src/log.cc
    PRIVATE src/email_addresses.hh
    PRIVATE src/subprocess.hh
    PRIVATE src/corba/corba_context.hh
    PRIVATE src/corba/corba_context.cc
    PRIVATE src/corba/nameservice.hh
    PRIVATE src/corba/nameservice.cc
    PRIVATE src/corba/akm.cc
    PRIVATE src/corba/akm.hh
    PRIVATE src/corba/mailer.cc
    PRIVATE src/corba/mailer.hh
    PRIVATE src/sqlite/storage.hh
    PRIVATE src/sqlite/storage.cc
    PRIVATE src/sqlite/enum_conversion.hh
    PRIVATE src/sqlite/scan_type_conversion.hh
    PRIVATE src/cdnskey_scanner_impl/input_serializer.hh
    PRIVATE src/cdnskey_scanner_impl/output_parser.hh
    PRIVATE src/cdnskey_scanner_impl/output_parser.cc
    PRIVATE src/cdnskey_scanner_impl/cdnskey_scanner_impl.hh
    PRIVATE src/cdnskey_scanner_impl/cdnskey_scanner_impl.cc
    PRIVATE src/loader_impl/file.hh
    PRIVATE src/loader_impl/file.cc
    PRIVATE src/loader_impl/backend.hh
    PRIVATE src/loader_impl/backend.cc
    PRIVATE src/loader_impl/domain_whitelist_filter.hh
    PRIVATE src/loader_impl/domain_whitelist_filter.cc
    PRIVATE src/scan_task.hh
    PRIVATE src/scan_task.cc
    PRIVATE src/command_load.hh
    PRIVATE src/command_load.cc
    PRIVATE src/command_scan.hh
    PRIVATE src/command_scan.cc
    PRIVATE src/command_notify.hh
    PRIVATE src/command_notify.cc
    PRIVATE src/command_update.hh
    PRIVATE src/command_update.cc
    PRIVATE src/domain_notified_status.hh
    PRIVATE src/domain_notified_status.cc
    PRIVATE src/notification.hh
    PRIVATE src/notification.cc
    PRIVATE src/notification_type.hh
    PRIVATE src/notification_type.cc
    PRIVATE src/keyset.hh
    PRIVATE src/keyset.cc
    PRIVATE src/nsset.hh
    PRIVATE src/nsset.cc
    PRIVATE src/cdnskey.hh
    PRIVATE src/cdnskey.cc
    PRIVATE src/dnskey.hh
    PRIVATE src/dnskey.cc
    PRIVATE src/domain.cc
    PRIVATE src/domain.hh
    PRIVATE src/domain_state.hh
    PRIVATE src/domain_state.cc
    PRIVATE src/domain_status.hh
    PRIVATE src/domain_status.cc
    PRIVATE src/scan_iteration.hh
    PRIVATE src/scan_iteration.cc
    PRIVATE src/scan_result_row.hh
    PRIVATE src/scan_result_row.cc
    PRIVATE src/domain_state_stack.hh
    PRIVATE src/domain_state_stack.cc
    PRIVATE src/domain_united_state.hh
    PRIVATE src/domain_united_state.cc
    PRIVATE src/domain_united_state_stack.hh
    PRIVATE src/domain_united_state_stack.cc
    PRIVATE src/command_clean.hh
    PRIVATE src/command_clean.cc
    PRIVATE src/scan_date_time.hh
    PRIVATE src/scan_date_time.cc)
target_link_libraries(${program_name}
    akm
    mailer)
link_targets_against_boost_regex(${program_name})
link_targets_against_boost_program_options(${program_name})
link_targets_against_omniorb4(${program_name})
link_targets_against_omnidynamic4(${program_name})
link_targets_against_sqlite3(${program_name})

include_directories("${CMAKE_SOURCE_DIR}" PUBLIC)

include(CheckIncludeFileCXX)
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_REQUIRED_FLAGS "-std=c++14")
check_include_file_cxx(3rd_party/sqlite3pp/headeronly_src/sqlite3pp.h FOUND_sqlite3pp)
if(NOT FOUND_sqlite3pp)
    message(FATAL_ERROR "Header sqlite3pp.h not found!")
endif()
check_include_file_cxx(spdlog/spdlog.h FOUND_spdlog)
if(NOT FOUND_spdlog)
    message(FATAL_ERROR "Header spdlog.h not found!")
endif()

install(TARGETS ${program_name} DESTINATION bin)

SET(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_SOURCE_IGNORE_FILES "_CPack_Packages;CMakeCache;CPack*;cmake_install.cmake;CMakeFiles;CTestTestfile*;.git*")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${program_name}-${VERSION}")
include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

