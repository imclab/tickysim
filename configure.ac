# Initialise autoconf with the specified project name, version and maintainer
# address
AC_INIT([tickysim], [0.1], [jdh@cs.man.ac.uk])

# Initialise automake "foreign" removes requirement for the standard GNU-style
# files such as NEWS etc.
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Search for compilers installed on the system
AC_PROG_CC

# Try and make the compiler work in c99 mode
AC_PROG_CC_C99

# Compiler must support compile "-c" flag and output file "-o" flag together
AM_PROG_CC_C_O

# Produce a header with useful #defines in it called config.h.
AC_CONFIG_HEADERS([config.h])

# Create the specified files by creating "[file]" based on "[file].in" with
# appropriate substitutions.
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile
               ])

# Test for the "Check" unit testing library (defined using deprecated syntax due
# to its also pulling in some other macros).
AM_PATH_CHECK

# Test that libconfig is available
PKG_CHECK_MODULES([LIBCONFIG], [libconfig >= 1.4],,
	AC_MSG_ERROR([libconfig 1.4 or newer not found.])
)

# Do all the configuration actions now! We're done.
AC_OUTPUT
