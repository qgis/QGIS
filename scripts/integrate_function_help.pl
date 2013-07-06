#!/usr/bin/perl

use strict;
use warnings;

use Data::Dumper;
use XML::Simple;

my %langmap = (
		"it_IT" => "it",
		"pl_PL" => "pl_PL",
		"pt_PT" => "pt_PT",
		"pt_BR" => "pt_BR",
		"sk_SK" => "sk",
		"sv_SE" => "sv",
		"fr_FR" => "fr",
		"de_DE" => "de",
		"ru_RU" => "ru",
		"ja_JP" => "ja",
		"es_ES" => "es",
	);

my @lang;
my @context;

foreach (@ARGV) {
	if( /-lang=(.*)/ ) {
		push @lang, $1;
	}

	if( /-context=(.*)/ ) {
		push @context, $1;
	}
}


my %langs;
my %src;
for my $f (<resources/{function_help,context_help}/*-*>) {
	my ($context,$id,$lang) = $f =~ m#^resources/(function_help|context_help)/(.+)-(.+)$#;

	next if @lang && !grep($lang, @lang);
	next if @context && !grep($context, @context);

	die "lang undefined in $f" unless defined $lang;
	$langs{$lang}{ts} = "";

	open F, "<:encoding(UTF-8)", $f;
	$src{$id}{$lang} = join("", <F>);
	$src{$id}{$lang} =~ s/\\/\&#92;/mg;
	close F;
}

delete $langs{en_US};

my $cmd = "scripts/update_ts_files.sh";
foreach my $l (keys %langs) {
	unless( exists $langmap{$l} ) {
		die "no ts lang for $l found\n";
	} else {
		my $ts = "i18n/qgis_" . $langmap{$l} . ".ts";
		die "LANG: no ts for $ts found\n" unless -f $ts;
		$langs{$l}{ts} = $ts;
		$cmd .= " $langmap{$l}";
	}
}

system $cmd;

foreach my $l (keys %langs) {
	print "LANG:$l\n";

	open my $in, $langs{$l}{ts};
	open my $out, ">$langs{$l}{ts}.new";
	binmode $out, ":utf8";

	my $line = <$in> . <$in>;
	print $out $line;

	my $xml = XMLin( $in, ForceArray=>1, keeproot=>1);

	my %ot;
	foreach my $c ( @{ $xml->{TS}->[0]->{context} } ) {
		my $name = $c->{name}->[0];
		next unless $name eq "QObject" || $name eq "QgsExpressionBuilderWidget";

		foreach my $m ( @{ $c->{message} } ) {
			my $s = $m->{source}->[0];
			my $t = $m->{translation}->[0];
			$t = $t->{content} if ref($t) eq "HASH" && defined $t->{content};
			$ot{$t} = $s;
		}

	}

	my %t;
	foreach my $oid ( keys %src ) {
		my $id = $oid;
		next unless exists $src{$id}{$l};

		unless(exists $src{$id}{en_US} ) {
			if( exists $ot{$id} ) {
				$id = $ot{$id};
			} else {
				die "No translation for $id found.";
			}
		}

		unless(exists $src{$id}{en_US} ) {
			die "source for $id not found" unless exists $src{$id}{en_US};
		}

		$t{ $src{$id}{en_US} } = $src{$oid}{$l};
	}
	
	die "T empty" unless %t;

	foreach my $c ( @{ $xml->{TS}->[0]->{context} } ) {
		my $name = $c->{name}->[0];

		if( $name eq "context_help" || $name eq "function_help" ) {
			foreach my $m ( @{ $c->{message} } ) {
				my $s = $m->{source}->[0];
				my $t = $m->{translation}->[0];
				my $translation = $t{ $s };
				next unless $translation;

				if( ref($t) eq "HASH" ) {
					warn "Previous content overwritten: " . $t->{content} if exists $t->{content} && $t->{content} ne $translation;
					$t->{content} = $translation;
				} else {
					$m->{translation}->[0] = $translation;
				}
			}
		}
	}

	close $in;

	my $xmlout = XMLout($xml, keeproot=>1);
	print $out $xmlout;
	close $out;

	rename "$langs{$l}{ts}", "$langs{$l}{ts}.orig";
	rename "$langs{$l}{ts}.new", "$langs{$l}{ts}";
}

system $cmd;
