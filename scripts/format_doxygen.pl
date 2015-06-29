#!/usr/bin/perl -i.formatdoc -n
###########################################################################
#    format_doxygen.pl
#    ---------------------
#    begin                : June 2015
#    copyright            : (C) 2015 by Nyall Dawson
#    email                : nyall dot dawson at gmail dot com
#
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

# adapted from scripts/unify_includes.pl

use strict;
use warnings;

if( $_ =~ m/^(\s*)\/\*[\*!]\s*([^\*])(.*)$/s )
{
	print $1, "/** ", uc($2), $3;
	next unless eof;
}

print;
