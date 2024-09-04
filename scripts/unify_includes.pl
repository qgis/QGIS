#!/usr/bin/env perl
###########################################################################
#    unify_includes.pl
#    ---------------------
#    begin                : June 2015
#    copyright            : (C) 2015 by Juergen E. Fischer
#    email                : jef at norbit dot de
#
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

# adapted from scripts/sort_includes.sh

use strict;
use warnings;

our %inc;
our @inc;

END { die "header files not empty" if @inc; }

# Ensure that pointer members are always initialized to nullptr
# We don't run this check by default, there's a high likelihood of false positives...
# s#^(\s*(?!typedef|return|delete)(?:\s*(?:const)\s*)?[a-zA-Z0-9_:]+\s*\*+\s*[a-zA-Z0-9_]+)\s*;\s*$#$1 = nullptr;\n#;

if( /^\s*#include/ ) {
	push @inc, $_ unless exists $inc{$_};
	$inc{$_}=1;
	next unless eof;
}

if( %inc ) {
	print foreach @inc;
	undef %inc;
	undef @inc;
	last if eof;
}

print;
