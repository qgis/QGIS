#!/usr/bin/perl
###########################################################################
#    processing2cpp.pl
#    ---------------------
#    begin                : July 2015
#    copyright            : (C) 2015 by Juergen E. Fischer
#    email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

use XML::Simple;
use Data::Dumper;

die "usage: $0 dest.cpp\n" unless @ARGV==1;

open F, ">$ARGV[0]";

print F <<EOF;
/*
 This is NOT a proper c++ source code. This file is only designed to be caught
 by qmake and included in lupdate. It contains all translateable strings collected
 by processing2cpp.pl.
*/

EOF

my %strings;

for my $f (<python/plugins/processing/algs/otb/description/*.xml>) {
	my $xml = XMLin($f, ForceArray=>1);

	foreach my $k (qw/longname group description/) {
		$strings{"OTBAlgorithm"}{$xml->{$k}->[0]} = 1;
	}
}

for my $f (<python/plugins/processing/algs/grass*/description/*.txt>) {
	open I, $f;
	my $name = scalar(<I>);
	my $desc = scalar(<I>);
	my $group = scalar(<I>);
	close I;

	chop $desc;
	chop $group;

	$strings{"GrassAlgorithm"}{$desc} = 1;
	$strings{"GrassAlgorithm"}{$group} = 1;
}

for my $f (<python/plugins/processing/algs/taudem/description/*/*.txt>) {
	open I, $f;
	my $desc = scalar(<I>);
	my $name = scalar(<I>);
	my $group = scalar(<I>);
	close I;

	chop $desc;
	chop $group;

	$strings{"TAUDEMAlgorithm"}{$desc} = 1;
	$strings{"TAUDEMAlgorithm"}{$group} = 1;
}

for my $f (<python/plugins/processing/algs/saga/description/*/*.txt>) {
	open I, $f;
	my $desc = scalar(<I>);
	close I;

	chop $desc;

	$strings{"SAGAAlgorithm"}{$desc} = 1;
}

for my $f ( ("python/plugins/processing/gui/algclasssification.txt", "python/plugins/processing/gui/algnames.txt") ) {
	open I, $f;
	while(<I>) {
		chop;
		s/^.*,//;
		foreach my $v (split "/", $_) {
			$strings{"AlgorithmClassification"}{$v} = 1;
		}
	}
	close I;
}

foreach my $k (keys %strings) {
	foreach my $v (keys %{ $strings{$k} } ) {
		$v =~ s/\\/\\\\/g;
		$v =~ s/"/\\"/g;
		$v =~ s/\n/\\n/g;

		print F "translate(\"$k\", \"$v\");\n";
	}
}

close F;
