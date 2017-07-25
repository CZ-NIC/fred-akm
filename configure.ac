#AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS], [TAR-NAME], [URL])
AC_INIT([fred-akm], [0.1.0], [jiri.sadek@nic.cz], [fred-akm], [http://fred.nic.cz])

# Force autoconf to be at least this version number.
AC_PREREQ([2.68])

# Put autotools auxiliary files in a subdir, so they don't clutter top dir.
# I am not actually doing this at the moment because there should be relatively
# little in the top directory anyway.
#AC_CONFIG_AUX_DIR([build-aux])
#AC_CONFIG_MACRO_DIR([m4])

# Enable "automake" to simplify creating makefiles. foreign relaxes some GNU
# checks. -Wall and -Werror are instructions to Automake, not gcc.
# Because this is a non-recursive automake, we use subdir-objects to tell
# automake to create objects such as .o in sub-directories rather than placing
# all artifacts in the top level directory.
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall])
AM_PROG_AR

# These are the files to be generated.
AC_CONFIG_FILES([Makefile])

# Safety check - list a source file that wouldn't be in other directories.
AC_CONFIG_SRCDIR([src/main.cc])

# Put configuration results in this file for us to #include in our program.
AC_CONFIG_HEADERS([config.h])

# Check for a C++ compiler.
AC_PROG_CXX
AC_PROG_RANLIB

AX_CXX_COMPILE_STDCXX_11

# Check for libraries, headers, data etc here.
AX_BOOST_BASE
AX_BOOST_PROGRAM_OPTIONS
AX_LIB_SQLITE3

PKG_CHECK_MODULES(OMNIORB4, [omniORB4 > 4.1.1, omniDynamic4, omnithread3])

AC_LANG_PUSH([C++])
CPPFLAGS_SAVED=$CPPFLAGS
CPPFLAGS="-std=c++11 $CPPFLAGS"
AC_CHECK_HEADER([spdlog/spdlog.h], [], [AC_MSG_ERROR([Missing spdlog header])])
AC_DEFINE([SPDLOG_ENABLE_SYSLOG], [], [enable logging to syslog])
CPPFLAGS=$CPPFLAGS_SAVED

AC_ARG_WITH(
	[idldir],
	[AS_HELP_STRING(
		[--with-idldir],
		[location of idl directory containing IDL interfaces [DATAROOTDIR/idl/fred/]]
	)],
	[AC_SUBST([IDLDIR], [$withval])],
	[AC_SUBST([IDLDIR], [$DATAROOTDIR/idl/fred/])]
)

AC_CHECK_FILE(
	[$IDLDIR/Mailer.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Mailer.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/AutomaticKeysetManagement.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain AutomaticKeysetManagement.idl
		(use --with-idldir)]
	)]
)

# Last line: actually generate all required output files.
AC_OUTPUT
