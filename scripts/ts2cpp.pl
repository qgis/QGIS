#!/usr/bin/perl

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

		print F ");\n";
	}
}

close F;
