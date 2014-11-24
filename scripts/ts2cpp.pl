#!/usr/bin/perl
###########################################################################
#    ts2cpp.pl
#    ---------------------
#    begin                : April 2009
#    copyright            : (C) 2009 by Juergen E. Fischer
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

die "usage: $0 source.ts dest.cpp\n" unless @ARGV==2 && -f $ARGV[0];

my $xml = XMLin($ARGV[0], ForceArray=>1);

open F, ">$ARGV[1]";

print F <<EOF;
/*
 This is NOT a proper c++ source code. This file is only designed to be caught
 by qmake and included in lupdate. It contains all translateable strings collected
 by pylupdate4.
*/

EOF

die "context expected" unless exists $xml->{context};

foreach my $context ( @{ $xml->{context} } ) {
	print F "\n// context: $context->{name}->[0]\n\n";

	foreach my $message ( @{ $context->{message} } ) {
		$message->{source}->[0] =~ s/"/\\"/g;
		$message->{source}->[0] =~ s/\n/\\n/g;

		print F "translate( \"$context->{name}->[0]\", \"$message->{source}->[0]\"";

		if( exists $message->{comment} && $message->{comment}->[0] ne "") {
			$message->{comment}->[0] =~ s/"/\\"/g;
			$message->{comment}->[0] =~ s/\n/\\n/g;

			print F ",\"$context->{comment}->[0]\"";
		}

		if( exists $message->{numerus} && $message->{numerus} eq "yes" ) {
			print '"",' unless exists $message->{comment} && $message->{comment}->[0] ne "";
			print F ",1"
		}

		print F ");\n";
	}
}

close F;
