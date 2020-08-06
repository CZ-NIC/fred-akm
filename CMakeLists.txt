cmake_minimum_required(VERSION 3.5)

set(project_name fred-akm)
project(${project_name} CXX)

macro(set_default variable_name variable_value)
    if(NOT ${variable_name})
        set(${variable_name} ${variable_value})
    endif()
endmacro()

#Use in config.h
set_default(PACKAGE ${project_name})

#Make sure that custom modules like FindOMNIORB are found
set_default(IDL_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/idl)
set(IDL_SOURCE_DIR ${IDL_PROJECT_DIR}/idl)
set(IDL_DESTINATION_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/corba/generated)
list(INSERT CMAKE_MODULE_PATH 0 ${IDL_PROJECT_DIR}/cmake)

##############################################
# Declare dependencies
find_package(OMNIORB4 REQUIRED)
find_program(GIT_PROGRAM git)
find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

macro(normalize_path variable_name path)
    string(REGEX REPLACE "//+" "/" ${variable_name} "${path}")
    if(NOT "${${variable_name}}" STREQUAL "/")
        string(REGEX REPLACE "/$" "" ${variable_name} "${${variable_name}}")
    endif()
endmacro()

macro(set_default_path variable_name path)
    if(NOT ${variable_name})
        normalize_path(${variable_name} "${path}")
    endif()
endmacro()

set_default(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set_default(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT VERSION)
    if(NOT GIT_PROGRAM)
        message(FATAL_ERROR "git not found; you need to pass -DVERSION=")
    endif()
    execute_process(COMMAND ${GIT_PROGRAM} "describe" "--tags"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE RAWVERSION)
    string(REGEX REPLACE "(-.*)?\n" "" VERSION "${RAWVERSION}")
endif()
set(TARBALL_FILE_NAME "${project_name}-${VERSION}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)

set(USR_PREFIX "${CMAKE_INSTALL_PREFIX}")
normalize_path(NORMALIZED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
if(NOT "${NORMALIZED_INSTALL_PREFIX}" STREQUAL "/usr/local" AND USE_USR_PREFIX)
    set(USR_PREFIX "${CMAKE_INSTALL_PREFIX}/usr")
endif()
set_default_path(BINDIR ${USR_PREFIX}/bin)

find_package(Boost 1.53.0
    COMPONENTS
        program_options
        regex
    REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_search_module(SQLITE3 REQUIRED sqlite3)

include(CheckIncludeFileCXX)
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/spdlog/include)
set(CMAKE_REQUIRED_FLAGS "-std=c++14")
check_include_file_cxx(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/sqlite3pp/headeronly_src/sqlite3pp.h sqlite3pp_h_FOUND)
if(NOT sqlite3pp_h_FOUND)
    message(FATAL_ERROR "Header sqlite3pp.h not found!")
endif()
check_include_file_cxx(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/spdlog/include/spdlog/spdlog.h spdlog_h_FOUND)
if(NOT spdlog_h_FOUND)
    message(FATAL_ERROR "Header spdlog.h not found!")
endif()

set(CORBA_LIBRARIES ${OMNIDYNAMIC4_LIBRARIES})

set_default(TARBALL_FILE_NAME "${project_name}-${VERSION}")
set_default(SPDLOG_ENABLE_SYSLOG "")

##############################################
# How to generate C++ files from IDL
OMNIORB4_IDL_TO_CPP(${IDL_SOURCE_DIR} ${IDL_DESTINATION_DIR} IDL_CC_FILES IDL_HH_FILES
    _dataTypes.idl
    DateTime.idl
    Mailer.idl
    AutomaticKeysetManagement.idl)

function(set_common_target_compile_options target_name)
    target_compile_options(${target_name}
        PRIVATE
            "-ggdb"
            "-grecord-gcc-switches"
            "-Wall"
            "-Wextra"
            "-fdiagnostics-color=always")
endfunction()

function(set_common_library_target_properties target_name)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
    set_common_target_compile_options(${target_name})
    target_include_directories(${target_name} PUBLIC
        $<INSTALL_INTERFACE:>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/spdlog/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
endfunction()

function(add_common_library_target target_name)
    message(STATUS "add library: " ${target_name})
    add_library(${target_name} STATIC EXCLUDE_FROM_ALL ${ARGN})
    set_common_library_target_properties(${target_name})
endfunction()

function(add_corba_library_target target_name)
    message(STATUS "add corba library: " ${target_name})
    add_library(${target_name} STATIC EXCLUDE_FROM_ALL ${ARGN})
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
    target_compile_options(${target_name}
        PRIVATE
            "-ggdb"
            "-grecord-gcc-switches"
            "-Wall"
            "-Wextra"
            "-Wno-unused-variable"
            "-Wno-unused-parameter"
            "-fdiagnostics-color=always")
    target_include_directories(${target_name} PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>)
    target_link_libraries(${target_name} PRIVATE ${CORBA_LIBRARIES})
endfunction()

function(set_common_executable_target_properties target_name)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
    set_common_target_compile_options(${target_name})
endfunction()

function(add_common_executable_target target_name)
    message(STATUS "add executable: " ${target_name})
    add_executable(${target_name} ${ARGN})
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR})
    set_common_executable_target_properties(${target_name})
endfunction()

add_corba_library_target(corba_datetime
    ${IDL_DESTINATION_DIR}/DateTimeSK.cc
    ${IDL_DESTINATION_DIR}/DateTimeDynSK.cc)

add_corba_library_target(corba_datatypes
    ${IDL_DESTINATION_DIR}/_dataTypesSK.cc
    ${IDL_DESTINATION_DIR}/_dataTypesDynSK.cc)
target_link_libraries(corba_datatypes PRIVATE corba_datetime)

add_corba_library_target(corba_mailer
    ${IDL_DESTINATION_DIR}/MailerSK.cc
    ${IDL_DESTINATION_DIR}/MailerDynSK.cc)
target_link_libraries(corba_mailer PRIVATE corba_datatypes)

add_corba_library_target(corba_akm
    ${IDL_DESTINATION_DIR}/AutomaticKeysetManagementSK.cc
    ${IDL_DESTINATION_DIR}/AutomaticKeysetManagementDynSK.cc)

add_common_library_target(akm
    src/corba/akm.cc
    src/corba/akm.hh)
target_link_libraries(akm PRIVATE corba_akm)

add_common_library_target(storage
    src/sqlite/storage.cc)
target_link_libraries(storage PRIVATE ${SQLITE3_LIBRARIES})
target_compile_options(storage PUBLIC ${SQLITE3_CFLAGS})

add_common_library_target(fred_akm_impl
    3rd_party/spdlog/include/spdlog/fmt/bundled/format.cc
    src/scan_type.cc
    src/utils.cc
    src/args.cc
    src/conf.cc
    src/log.cc
    src/corba/corba_context.cc
    src/corba/nameservice.cc
    src/corba/akm.cc
    src/corba/mailer.cc
    src/cdnskey_scanner_impl/output_parser.cc
    src/cdnskey_scanner_impl/cdnskey_scanner_impl.cc
    src/loader_impl/file.cc
    src/loader_impl/backend.cc
    src/loader_impl/domain_whitelist_filter.cc
    src/scan_task.cc
    src/command_load.cc
    src/command_scan.cc
    src/command_notify.cc
    src/command_update.cc
    src/domain_notified_status.cc
    src/notification.cc
    src/notification_type.cc
    src/keyset.cc
    src/nsset.cc
    src/cdnskey.cc
    src/dnskey.cc
    src/domain.cc
    src/domain_state.cc
    src/domain_status.cc
    src/scan_iteration.cc
    src/scan_result_row.cc
    src/domain_state_stack.cc
    src/domain_united_state.cc
    src/domain_united_state_stack.cc
    src/command_clean.cc
    src/scan_date_time.cc)
target_link_libraries(fred_akm_impl corba_mailer)

add_common_executable_target(fred-akm src/main.cc)
target_link_libraries(fred-akm
    akm
    fred_akm_impl
    storage
    Boost::regex
    Boost::program_options)

install(TARGETS fred-akm DESTINATION ${BINDIR})
add_custom_target(uninstall COMMAND rm ${BINDIR}/fred-akm)

if(EXISTS ${CMAKE_SOURCE_DIR}/.git AND GIT_PROGRAM)
    if(NOT TARGET dist)
        add_custom_target(dist_idl
            COMMAND ${GIT_PROGRAM} archive --format=tar --prefix=${TARBALL_FILE_NAME}/external/idl/ HEAD > ${CMAKE_CURRENT_SOURCE_DIR}/.idl.tar
            WORKING_DIRECTORY ${IDL_PROJECT_DIR})
        add_custom_target(dist_sqlite
            COMMAND ${GIT_PROGRAM} archive --format=tar --prefix=${TARBALL_FILE_NAME}/3rd_party/sqlite3pp/ HEAD > ${CMAKE_CURRENT_SOURCE_DIR}/.sqlite3pp.tar
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rd_party/sqlite3pp)
        add_custom_target(dist_spdlog
            COMMAND ${GIT_PROGRAM} archive --format=tar --prefix=${TARBALL_FILE_NAME}/3rd_party/spdlog/ HEAD > ${CMAKE_CURRENT_SOURCE_DIR}/.spdlog.tar
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rd_party/spdlog)
        add_custom_target(dist
            COMMAND ${GIT_PROGRAM} archive --format=tar --prefix=${TARBALL_FILE_NAME}/ HEAD > .${TARBALL_FILE_NAME}.tar
            COMMAND tar --concatenate --file=.${TARBALL_FILE_NAME}.tar .idl.tar
            COMMAND tar --concatenate --file=.${TARBALL_FILE_NAME}.tar .sqlite3pp.tar
            COMMAND tar --concatenate --file=.${TARBALL_FILE_NAME}.tar .spdlog.tar
            COMMAND cat .${TARBALL_FILE_NAME}.tar | gzip > ${CMAKE_BINARY_DIR}/${TARBALL_FILE_NAME}.tar.gz
            COMMAND rm .${TARBALL_FILE_NAME}.tar .idl.tar .sqlite3pp.tar .spdlog.tar
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        add_dependencies(dist dist_idl dist_sqlite dist_spdlog)
    endif()

    if(DEFINED ENV{DISTCHECK_CMAKE_FLAGS})
        set(distcheck_flags $ENV{DISTCHECK_CMAKE_FLAGS})
    endif()

    if(NOT TARGET distcheck)
        set(DISTCHECK_BUILD_DIR "_build")
        set(DISTCHECK_INSTALL_DIR "_inst")
        if(${CMAKE_GENERATOR} STREQUAL "Ninja")
            set(DISTCHECK_BUILD_CMD "ninja")
            set(DISTCHECK_MAKEFLAGS "")
        else()
            set(DISTCHECK_BUILD_CMD "make")
            execute_process(COMMAND "grep" "-c" "model name" "/proc/cpuinfo"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE NOCPUS)
            string(REGEX REPLACE "\n" "" NOCPUS ${NOCPUS})
            set(DISTCHECK_MAKEFLAGS "MAKEFLAGS=-j${NOCPUS}")
        endif()
        add_custom_target(distcheck
            COMMAND
            rm -rf ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR} ${TARBALL_FILE_NAME}
            && mkdir ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR}
            && (tar xzf ${TARBALL_FILE_NAME}.tar.gz
                || (echo 'Error: failed to unpack the archive' 2>&1\; false))
            && (cd ${DISTCHECK_BUILD_DIR}
                && (${CMAKE_COMMAND} -G${CMAKE_GENERATOR} ${distcheck_flags} -DCMAKE_INSTALL_PREFIX=../${DISTCHECK_INSTALL_DIR} ../${TARBALL_FILE_NAME}
                    || (echo 'Error: failed to configure the archive' 2>&1\; false))
                && (${DISTCHECK_MAKEFLAGS} ${DISTCHECK_BUILD_CMD}
                    || (echo 'Error: the build target failed ' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} install
                    || (echo 'Error: the install target failed' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} uninstall
                    || (echo 'Error: the uninstall target failed' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} clean
                    || (echo 'Error: the clean target failed' 2>&1\; false)))
            && rm -rf ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR} ${TARBALL_FILE_NAME}
            && echo 'PASS: ' ${TARBALL_FILE_NAME}.tar.gz 'is ready for distribution.'
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Checking the distribution archive...")
        add_dependencies(distcheck dist)
    endif()
else()
    message(STATUS "no git -- not including dist and distcheck targets")
endif()
