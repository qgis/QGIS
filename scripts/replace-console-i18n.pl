#!/usr/bin/perl

use strict;
use warnings;

my %src;

for my $f (<python/console/help/i18n/*.properties>) {
	my($lang) = $f =~ m#python/console/help/i18n/(.+)\.properties$#;
	print "$f [$lang]\n";

	my $title;
	my $str;

	open F, $f;
	while( my $l = <F>) {
		$l =~ s/\s+$//;
		if( $l =~ /\\$/ ) {
			$l =~ s/\\$/\n/;
			while(<F>) {
				s/\\\s*$/\n/;
				$l .= $_;
				last if /",$/;
			}
		}

		next if $l =~ /^i18n_dict/;
		next if $l =~ /^};$/;

		if( ($title, $str) = $l =~ /"([^"]+)"\s*:\s*"([^"]*)",?$/m ) {
			#print "TITLE:$title STR:$str\n";
			$src{$lang}{$title} = $str;
		} else {
			die "$lang:|$l| not parsed";
		}
	}
	close F;
}

open F, "resources/context_help/PythonConsole";
my $src = join("", <F>);
close F;

foreach my $lang (keys %src) {
	my $dst = $src;

	my %t;
	foreach my $title ( keys %{ $src{$lang} } ) {
		my $d = $src{$lang}{$title};

		die "dst for $title undefined" unless defined $d;

		my $dst0 = $dst;
		$dst =~ s/#$title#/$d/gm;

		warn "$lang.$title [$d] not replaced\n$dst" if $dst0 eq $dst;
	}

	open F, ">resources/context_help/PythonConsole-$lang";
	print F $dst;
	close F;
}
