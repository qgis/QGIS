#!/usr/bin/perl -i.sortinc -n
###########################################################################
#    sort_includes.pl
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

our %uis;
our %sys;
our %others;
our $sorting;

BEGIN { $sorting = 0; }
END { die "header files not empty" if keys %uis || keys %sys || keys %others; }

if(/^\s*#include/ ) {
	if(/"ui_/ ) {
		$uis{$_}=1;
	} elsif(/</) {
		$sys{$_}=1;
	} else {
		$others{$_}=1;
	}
	$sorting=1;
	next;
}

if( $sorting ) {
	print foreach sort keys %uis;
	print foreach sort keys %sys;
	print foreach sort keys %others;

	undef %uis;
	undef %sys;
	undef %others;
}


$sorting=0;

print;
