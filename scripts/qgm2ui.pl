#!/usr/bin/env perl
###########################################################################
#    qgm2cpp.pl
#    ---------------------
#    begin                : December 2009
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
use Data::Dumper;

sub xmlescape {
  my $data = shift;
  $data =~ s/&/&amp;/sg;
  $data =~ s/</&lt;/sg;
  $data =~ s/>/&gt;/sg;
  $data =~ s/"/&quot;/sg;
  return $data;
}

my %labels;
my $file;

sub parse {
	foreach my $a (@_) {
		if( ref($a) eq "ARRAY" ) {
			foreach my $b ( @$a ) {
				parse($b);
			}
		} elsif( ref($a) eq "HASH" ) {
			foreach my $b ( keys %$a ) {
				if( $b eq "label" ) {
					my $label = $a->{$b};
					die "expected string" unless ref($label) eq "";
					print STDERR "warning[$file]: '$label' should start with a uppercase character or digit and not start or end with whitespaces"
						if $label =~ /^\s+/ || $label =~ /\s+$/ || $label !~ /^[A-Z0-9(]/;	
					$label =~ s/^\s+//;
					$label =~ s/\s+$//;
					$label = ucfirst $label;
					$labels{$label} = 1;
				} else {
					parse($a->{$b});
				}
			}
#		} elsif(ref($a) eq "") {
#			warn "found: " . $a;
#		} else {
#			warn "found: " . ref($a) . " " . Dumper($a);
		}
	}
}

open I, "find src/plugins/grass -name '*.qgm' -o -name '*.qgc'|";
while($file = <I>) {
	#print STDERR "$file\n";
	chop $file;
	parse XMLin($file, ForceArray=>1);
	#print STDERR "$file DONE\n";
}
close I;

print <<EOF;
<?xml version="1.0" encoding="UTF-8"?>
 <!--
 This is NOT a proper UI code. This file is only designed to be caught
 by qmake and included in lupdate. It contains all translateable strings collected
 by scripts/qgm2ui.pl.
 -->
<ui version="4.0">
  <class>grasslabels</class>;
EOF

foreach (sort keys %labels) {
	print "  <property><string>" . xmlescape($_) . "</string></property>\n";
}

print "</ui>\n";
