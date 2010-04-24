#!/usr/bin/perl

use XML::Simple;
use Data::Dumper;

print <<EOF;
/*
 This is NOT a proper c++ source code. This file is only designed to be caught
 by qmake and included in lupdate. It contains all translateable strings collected
 by pylupdate4.
*/

EOF

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
					$label =~ ucfirst $label;
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

foreach (sort keys %labels) {
	print "translate( \"grasslabel\", \"$_\" );\n";
}
