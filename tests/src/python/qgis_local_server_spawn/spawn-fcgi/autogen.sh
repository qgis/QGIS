#!/bin/sh
# Run this to generate all the initial makefiles, etc.

ACLOCAL=${ACLOCAL:-aclocal}
AUTOHEADER=${AUTOHEADER:-autoheader}
AUTOMAKE=${AUTOMAKE:-automake}
AUTOMAKE_FLAGS="--add-missing --copy"
AUTOCONF=${AUTOCONF:-autoconf}

ARGV0=$0

set -e


run() {
	echo "$ARGV0: running \`$@'"
	$@
}

run $ACLOCAL $ACLOCAL_FLAGS
run $AUTOHEADER
run $AUTOMAKE $AUTOMAKE_FLAGS
run $AUTOCONF
echo "Now type './configure ...' and 'make' to compile."
