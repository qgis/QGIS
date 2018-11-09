#!/usr/bin/env perl
###########################################################################
#    ts-clear.pl
#    ---------------------
#    begin                : October 2018
#    copyright            : (C) 2018 by Juergen E. Fischer
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
use Carp qw/croak/;
use XML::Simple;
use Data::Dumper;

$SIG{__WARN__} = sub { croak @_; };

my $xml = XMLin("i18n/qgis_en.ts", ForceArray=>1);

die "context expected" unless exists $xml->{context};

foreach my $context ( @{ $xml->{context} } ) {
	foreach my $message ( @{ $context->{message} } ) {
		if(exists $message->{numerus} && $message->{numerus} eq "yes") {
			for my $nf ( @{ $message->{translation}->[0]->{numerusform} } ) {
				$nf = $message->{source}->[0];
			}
		} elsif(ref($message->{translation}->[0]) eq "") {
			$message->{translation}->[0] = $message->{source}->[0];
		}
	}
}

my $xmlout = XMLout($xml, KeepRoot=>1, RootName => "TS");

my $out;
open $out, ">i18n/qgis_en.ts";
print $out "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE TS>";
binmode($out, ":utf8");
print $out $xmlout;
close $out;
