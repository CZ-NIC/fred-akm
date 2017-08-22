# Always set this so that we can just '#include "foo.hpp"' from anywhere,
# including our tests directory.
AM_CPPFLAGS = -I$(top_srcdir)

# This is the top-level Makefile.am for a non-recursive Autotools setup. In
# fact, in non-recursive structures, this is the only Makefile.am that is
# needed. The programs (just foo.exe in this case) will be left in the top-level
# directory.
bin_PROGRAMS = fred-akm

fred_akm_SOURCES = \
	3rd_party/sqlite3pp/headeronly_src/sqlite3pp.h \
	3rd_party/sqlite3pp/headeronly_src/sqlite3pp.ipp \
	src/i_mailer.hh \
	src/i_storage.hh \
	src/i_scanner.hh \
	src/i_loader.hh \
	src/i_loader_filter.hh \
	src/i_akm.hh \
	src/enum_conversions.hh \
	src/tech_contacts.hh \
	src/type_instance_map.hh \
	src/from_string.hh \
	src/scan_type.hh \
	src/scan_type.cc \
	src/utils.hh \
	src/utils.cc \
	src/args.hh \
	src/args.cc \
	src/conf.hh \
	src/conf.cc \
	src/log.hh \
	src/log.cc \
	src/subprocess.hh \
	src/corba/corba_context.hh \
	src/corba/corba_context.cc \
	src/corba/nameservice.hh \
	src/corba/nameservice.cc \
	src/corba/akm.cc \
	src/corba/akm.hh \
	src/corba/mailer.cc \
	src/corba/mailer.hh \
	src/sqlite/storage.hh \
	src/sqlite/storage.cc \
	src/sqlite/enum_conversion.hh \
	src/sqlite/scan_type_conversion.hh \
	src/cdnskey_scanner_impl/input_serializer.hh \
	src/cdnskey_scanner_impl/output_parser.hh \
	src/cdnskey_scanner_impl/output_parser.cc \
	src/cdnskey_scanner_impl/cdnskey_scanner_impl.hh \
	src/cdnskey_scanner_impl/cdnskey_scanner_impl.cc \
	src/loader_impl/file.hh \
	src/loader_impl/file.cc \
	src/loader_impl/backend.hh \
	src/loader_impl/backend.cc \
	src/loader_impl/domain_whitelist_filter.hh \
	src/loader_impl/domain_whitelist_filter.cc \
	src/scan_task.hh \
	src/scan_task.cc \
	src/command_load.hh \
	src/command_load.cc \
	src/command_scan.hh \
	src/command_scan.cc \
	src/command_notify.hh \
	src/command_notify.cc \
	src/command_update.hh \
	src/command_update.cc \
	src/notified_domain_status.hh \
	src/notified_domain_status.cc \
	src/notification.hh \
	src/notification.cc \
	src/notification_type.hh \
	src/notification_type.cc \
	src/keyset.hh \
	src/keyset.cc \
	src/nsset.hh \
	src/nsset.cc \
	src/cdnskey.hh \
	src/cdnskey.cc \
	src/dnskey.hh \
	src/dnskey.cc \
	src/domain.cc \
	src/domain.hh \
	src/domain_state.hh \
	src/domain_state.cc \
	src/domain_status.hh \
	src/domain_status.cc \
	src/scan_iteration.hh \
	src/scan_iteration.cc \
	src/scan_result_row.hh \
	src/scan_result_row.cc \
	src/domain_state_stack.hh \
	src/domain_state_stack.cc \
	src/domain_status_stack.hh \
	src/domain_status_stack.cc \
	src/command_clean.hh \
	src/command_clean.cc \
	src/main.cc

fred_akm_LDADD = \
	src/corba/generated/libakm.a \
	src/corba/generated/libmailer.a \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(OMNIORB4_LIBS) \
	$(SQLITE3_LDFLAGS)


CORBA_GENERATED_MAILER_SK = \
	src/corba/generated/DateTimeSK.cc \
	src/corba/generated/_dataTypesSK.cc \
	src/corba/generated/MailerSK.cc

CORBA_GENERATED_MAILER = \
	$(CORBA_GENERATED_MAILER_SK) \
	src/corba/generated/DateTimeDynSK.cc \
	src/corba/generated/DateTime.hh \
	src/corba/generated/_dataTypesDynSK.cc \
	src/corba/generated/_dataTypes.hh \
	src/corba/generated/MailerDynSK.cc \
	src/corba/generated/Mailer.hh

CORBA_GENERATED_AKM_SK = \
	src/corba/generated/AutomaticKeysetManagementSK.cc

CORBA_GENERATED_AKM = \
	$(CORBA_GENERATED_AKM_SK) \
	src/corba/generated/AutomaticKeysetManagementDynSK.cc \
	src/corba/generated/AutomaticKeysetManagement.hh

CLEANFILES = $(CORBA_GENERATED_MAILER) $(CORBA_GENERATED_AKM)

# To translate IDL file before any other dependency checking
BUILT_SOURCES = $(CORBA_GENERATED_MAILER_SK) $(CORBA_GENERATED_AKM_SK)


noinst_LIBRARIES = \
	src/corba/generated/libmailer.a \
	src/corba/generated/libakm.a

nodist_src_corba_generated_libmailer_a_SOURCES = \
	$(CORBA_GENERATED_MAILER) \
	src/corba/mailer.cc \
	src/corba/mailer.hh

nodist_src_corba_generated_libakm_a_SOURCES = \
	$(CORBA_GENERATED_AKM) \
	src/corba/akm.cc \
	src/corba/akm.hh



src/corba/generated/_dataTypesSK.cc: $(IDLDIR)/_dataTypes.idl
	omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba/generated/ $<
src/corba/generated/DateTimeSK.cc: $(IDLDIR)/DateTime.idl
	omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba/generated/ $<
src/corba/generated/MailerSK.cc: $(IDLDIR)/Mailer.idl $(IDLDIR)/_dataTypes.idl $(IDLDIR)/DateTime.idl
	omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba/generated/ $<
src/corba/generated/AutomaticKeysetManagementSK.cc: $(IDLDIR)/AutomaticKeysetManagement.idl
	omniidl -bcxx -Wbuse_quotes -Wba -C$(top_builddir)/src/corba/generated/ $<

# To specify extra flags for each target you can do this.
# But it is better to pass them on the command line (using a custom script to
# make the process easier; see the example in the debug folder). Doing it that
# way ensures the core build system stays free of cruft that may not port to
# other machines or compilers.
#foo_CXXFLAGS = -pedantic

# Testing setup. Type 'make check' or 'make recheck' to run the tests.
# TESTS defines a list of tests to run. Define their sources in the usual way.
#
# TESTS = passing failing

# passing_SOURCES = tests/passing_test.cpp
# failing_SOURCES = tests/failing_test.cpp

# This is needed to get ./configure to run.
# noinst_PROGRAMS = passing failing
