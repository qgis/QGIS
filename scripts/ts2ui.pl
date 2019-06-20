#!/usr/bin/env perl
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

use strict;
use warnings;
use Carp qw/croak/;
use XML::Simple;
use Data::Dumper;

sub xmlescape {
  my $data = shift;
  $data =~ s/&/&amp;/sg;
  $data =~ s/</&lt;/sg;
  $data =~ s/>/&gt;/sg;
  $data =~ s/"/&quot;/sg;
  return $data;
}

$SIG{__WARN__} = sub { croak @_; };

die "usage: $0 source.ts destdir\n" unless @ARGV==2 && -f $ARGV[0] && -d $ARGV[1];

my $xml = XMLin($ARGV[0], ForceArray=>1);

open CPP, ">$ARGV[1]/numerus-i18n.cpp";
binmode(CPP, ":utf8");

print CPP <<EOF;
/*
 This is NOT a proper c++ source code. This file is only designed to be caught
 by qmake and included in lupdate. It contains all translateable strings collected
 by pylupdate5.
*/

EOF

die "context expected" unless exists $xml->{context};

my $i = 0;
foreach my $context ( @{ $xml->{context} } ) {
	$i++;
	my $name = $context->{name}->[0];

	open UI, ">$ARGV[1]/$name-$i-i18n.ui";
	binmode(UI, ":utf8");

	print UI <<EOF;
<?xml version="1.0" encoding="UTF-8"?>
 <!--
 This is NOT a proper UI code. This file is only designed to be caught
 by qmake and included in lupdate. It contains all translateable strings collected
 by scripts/ts2ui.pl.
 -->
<ui version="4.0">
  <class>$name</class>;
EOF

	foreach my $message ( @{ $context->{message} } ) {
		if( exists $message->{numerus} && $message->{numerus} eq "yes" ) {
			$message->{source}->[0] =~ s/"/\\"/g;
			$message->{source}->[0] =~ s/\n/\\n/g;

			print CPP "translate( \"$context->{name}->[0]\", \"$message->{source}->[0]\"";

			if( exists $message->{comment} && $message->{comment}->[0] ne "") {
				$message->{comment}->[0] =~ s/"/\\"/g;
				$message->{comment}->[0] =~ s/\n/\\n/g;
				print CPP ", \"$message->{comment}->[0]\"";
			}

			print CPP '"",' unless exists $message->{comment} && $message->{comment}->[0] ne "";
			print CPP ", 1);\n"

		} else {
			print UI "  <property><string";
			print UI " comment=\"" . xmlescape($message->{comment}->[0]) . "\"" if exists $message->{comment} && $message->{comment}->[0] ne "";
			print UI ">" . xmlescape($message->{source}->[0]) . "</string></property>\n";
			print UI "<!--\n" . $message->{source}->[0] . "\n-->\n";
		}
	}

	print UI "</ui>\n";

	close UI;
}

close CPP;
