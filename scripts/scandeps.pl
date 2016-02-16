#!/usr/bin/perl
###########################################################################
#    scandeps.pl
#    ---------------------
#    begin                : October 2010
#    copyright            : (C) 2010 by Juergen E. Fischer
#    email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

use strict;
use warnings;

my @dists;
open I, "debian/rules";
while(<I>) {
	if( /ifneq \(\$\(DISTRIBUTION\),\$\(findstring \$\(DISTRIBUTION\),"(.*)"\)\)/ ) {
		for my $d (split / /, $1) {
			next if $d =~ /oracle/;
			push @dists, $d;
		}
		push @dists, "sid";
		last;
	}
}
close I;

die "no dists" unless @dists;

open I, "doc/linux.t2t";
open O, ">doc/linux.t2t.new";
while(<I>) {
	last if /^\|\| Distribution \| install command for packages \|\n$/;
	print O;
}

print O "|| Distribution | install command for packages |\n";

for my $dist (@dists) {
	system("git checkout debian/control" )==0 or die "git checkout failed: $!";
	system("make -f debian/rules DISTRIBUTION=$dist cleantemplates templates" )==0 or die "make failed: $!";

	open F, "debian/control";
	while(<F>) {
		chop;
		last if /^Build-Depends:/i;
	}

	s/^Build-Depends:\s*//;
	my $deps = $_;

	while(<F>) {
		chop;
		last if /^\S/;
		$deps .= $_;
	}
	close F;
	system("git checkout debian/control" )==0 or die "git checkout failed: $!";

	my @deps;
	foreach my $p (split /,/, $deps) {
		$p =~ s/^\s+//;
		$p =~ s/\s+.*$//;
		next if $p =~ /^(debhelper|subversion|python-central)$/;
		push @deps, $p;
	}

	my $dep="";
	my @dep;
	foreach my $p (sort @deps) {
		if( length("$dep $p") > 60 ) {
			push @dep, $dep;
			$dep = $p;
		} else {
			$dep .= " $p";
		}
	}

	push @dep, $dep;
	push @dep, "cmake-curses-gui";

	print O "| $dist | ``apt-get install" . join( " ", @dep ) . "`` |\n";
}

while(<I>) {
	last if /^$/;
}

print O;

while(<I>) {
	print O;
}

close O;
close I;

rename "doc/linux.t2t", "doc/linux.t2t.orig";
rename "doc/linux.t2t.new", "doc/linux.t2t";
my $time = time;
utime $time, $time, "doc/INSTALL.t2t";
