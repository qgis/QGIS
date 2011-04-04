#!/usr/bin/perl

use strict;
use warnings;

open I, "doc/linux.t2t";
open O, ">doc/linux.t2t.new";
while(<I>) {
	last if /^\|\| Distribution \| install command for packages \|\n$/;
	print O;
}

print O "|| Distribution | install command for packages |\n";

for my $c (<debian/control.*>) {
	my ($dist) = $c =~ /^.*\/control\.(.*)$/;

	open F, $c;
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
