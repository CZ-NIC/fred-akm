# Always set this so that we can just '#include "foo.hpp"' from anywhere,
# including our tests directory.
AM_CPPFLAGS = -I$(top_srcdir)/src

# This is the top-level Makefile.am for a non-recursive Autotools setup. In
# fact, in non-recursive structures, this is the only Makefile.am that is
# needed. The programs (just foo.exe in this case) will be left in the top-level
# directory.
bin_PROGRAMS = fred-akm

fred_akm_SOURCES = \
	src/type_instance_map.hh \
	src/args.hh \
	src/args.cc \
	src/conf.hh \
	src/conf.cc \
	src/corba/corba_context.hh \
	src/corba/corba_context.cc \
	src/corba/nameservice.hh \
	src/corba/nameservice.cc \
	src/main.cc

fred_akm_LDADD = \
	src/corba/generated/libakm.a \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(OMNIORB4_LIBS)


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

nodist_src_corba_generated_libmailer_a_SOURCES = $(CORBA_GENERATED_MAILER)

nodist_src_corba_generated_libakm_a_SOURCES = $(CORBA_GENERATED_AKM)


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
