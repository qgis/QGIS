#!/usr/bin/perl -i.sortinc -n
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

# Also fix doxygen comments
s#^(\s*)/\*[*!]\s*([^\s*].*)\s*$#$1/** \u$2\n#;

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
