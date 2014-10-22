#!/usr/bin/perl
###########################################################################
#    tsstat.pl
#    ---------------------
#    begin                : March 2009
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
use Locale::Language;
use Locale::Country;

my @lang;

# script to generate a html list of the qgis application translations
# showing the percentage finished and the names of the translators

# without argument it generates html which is used in the about-dialog of the application
# output to std-out, to be piped to doc/TRANSLATORS so it can be used in dialog
#   scripts/tsstat.pl > doc/TRANSLATORS
# this version needs flag images from the resources

# with argument 'site' a more complete html page is create to be used on a website
#   scripts/tsstat.pl site > page.html
# this version needs flag images in a directory 'flags'

# translator names here as a hash where the key is the lang_country code used for the ts file name
my $translators= {
	af => '',
	ar => 'Ichaouia Amine, Hosham Munier',
	bg => 'Захари Савов, Jordan Tzvetkov',
	bs => 'Almir Karabegovic',
	ca => 'Albert F, Pau Reguant Ridó, Xavier Roijals',
	cs => 'Jan Helebrant, Martin Landa, Peter Antolik, Martin Dzurov, Stanislav Horáček',
	da => 'Jacob Overgaard Madsen, Bo Victor Thomsen',
	de => 'Jürgen E. Fischer, Stephan Holl, Otto Dassau, Werner Macho',
	es => 'Carlos Dávila, Javier César Aldariz, Gabriela Awad, Edwin Amado, Mayeul Kauffmann, Diana Galindo',
	el => 'Theodoros Vakkas, Ioannis Tsimpiris, Evripidis Argyropoulos, Mike Pegnigiannis, Nikos Ves',
	et => 'Veiko Viil',
	eu => 'Asier Sarasua Garmendia, Irantzu Alvarez',
	fa => 'Mola Pahnadayan, Masoud Pashotan , Masoud Erfanyan',
	fi => 'Kari Salovaara, Marko Järvenpää',
	fr => 'Stéphane Brunner, Olivier Dalang, Nicolas Damien, Michael Douchin, Régis Haubourg, Matthias Khun, Sylvain Maillard, Jean-Roc Morreale, Mehdi Semchaoui, Harrissou Sant-anna',
	gl => 'Xan Vieiro',
	hi => 'Harish Kumar Solanki',
	hu => 'Zoltan Siki',
	hr => 'Zoran Jankovic',
	is => 'Ásta Kristín Óladóttir, Thordur Ivarsson',
	id => 'Emir Hartato, Muhammad Iqnaul Haq Siregar, Trias Aditya, Januar V. Simarmata, I Made Anombawa',
	it => 'Roberto Angeletti, Michele Beneventi, Marco Braida, Stefano Campus, Luca Casagrande, Paolo Cavallini, Giuliano Curti, Luca Delucchi, Alessandro Fanna, Michele Ferretti, Matteo Ghetta, Anne Gishla, Maurizio Napolitano, Flavio Rigolon',
	ja => 'BABA Yoshihiko, Yoichi Kayama, Minoru Akagi, Takayuki Nuimura, Takayuki Mizutani, Norihiro Yamate',
	ka => 'Shota Murtskhvaladze, George Machitidze',
	km => 'Khoem Sokhem',
	ko => 'OSGeo Korean Chapter',
	lo => 'Anousak Souphavanh, Soukanh Lathsavong',
	lv => 'Maris Nartiss, Pēteris Brūns',
	lt => 'Paulius Litvinas, Tomas Straupis, Kestas M',
	ml => 'Vinayan Parameswaran',
	mn => 'Bayarmaa Enkhtur',
	mr => '',
	nb => 'James Stott, Maléne Peterson',
	nl => 'Richard Duivenvoorde, Raymond Nijssen, Carlo van Rijswijk, Diethard Jansen, Willem Hoffmans, Dick Groskamp',
	pl => 'Robert Szczepanek, Milena Nowotarska, Borys Jurgiel, Mateusz Łoskot, Tomasz Paul, Andrzej Świąder ',
	pt_BR => 'Sidney Schaberle Goveia, Arthur Nanni, Marcelo Soares Souza, Narcélio de Sá Pereira Filho, Leônidas Descovi Filho, Felipe Sodré Barros ',
	pt_PT => 'Giovanni Manghi, Joana Simões, Duarte Carreira, Alexandre Neto, Pedro Pereira, Pedro Palheiro, Nelson Silva, Ricardo Sena, Leandro Infantini, João Gaspar',
	ro => 'Sorin Călinică, Lonut Losifescu-Enescu, Bogdan Pacurar',
	ru => 'Alexander Bruy, Artem Popov',
	sk => 'Lubos Balazovic, Jana Kormanikova, Ivan Mincik',
	sl => 'Jože Detečnik, Dejan Gregor, Jaka Kranjc',
	sq => '',
	sr_Latn => 'Goran Ivanković',
	sr_Cyrl => 'Goran Ivanković',
	sv => 'Lars Luthman, Magnus Homann, Victor Axbom',
	sw => '',
	ta => '',
	te => '',
	th => 'Man Chao',
        tl => 'Kathrina Gregana',
	tr => 'Osman Yalçın YILMAZ, Omur Saygin',
	uk => 'Alexander Bruy',
	vi => 'Phan Anh, Bùi Hữu Mạnh',
	zh_CN => 'Calvin Ngei, Zhang Jun, Richard Xie',
	zh_TW => 'Nung-yao Lin',
};

my $maxn;

for my $i (<i18n/qgis_*.ts>) {
	my ($langcode) = $i =~ /i18n\/qgis_(.*).ts/;
	next if $langcode eq "en";

	my $translator = $translators->{$langcode} || "(orphaned)";

	my $charset = "";
	my $lc = $langcode;
	if( $langcode =~ /(.*)_Latn/ ) {
		$charset = " (latin)";
		$langcode = $1;
	} elsif( $langcode =~ /(.*)_Cyrl/ ) {
		$charset = " (cyrillic)";
		$langcode = $1;
	}

	my $name;
	if($langcode =~ /(.*)_(.*)/) {
		my $lang = code2language(lc $1);
		my $country = code2country(lc $2);
		$name = "$lang ($country)";
	} else {
		$name = code2language(lc $langcode);
	}

	$name .= $charset;

	open F, "lrelease $i|";

	my($translations,$finished,$unfinished);
	my $untranslated=0;

	while(<F>) {
		if(/Generated (\d+) translation\(s\) \((\d+) finished and (\d+) unfinished\)/) {
			$translations=$1;
			$finished=$2;
			$unfinished=$3;
		} elsif(/Ignored (\d+) untranslated source text\(s\)/) {
			$untranslated=$1;
		}
	}

	close F;

	my $n = $translations+$untranslated;
	$maxn = $n unless defined $maxn;

	if( $n>$maxn ) {
		print STDERR "$i: more translations than others. ($n>$maxn)\n";
		$maxn = $n;
	}

	push @lang, {
		code=>$langcode,
		origcode=>$lc,
		name=>$name, n=>$n,
		translations=>$translations,
		finished=>$finished,
		unfinished=>$unfinished,
		untranslated=>$untranslated,
		translator=>$translator
	};
}

foreach my $l (@lang) {
	$l->{diff}       = $l->{n}-$maxn;
	$l->{percentage} = ($l->{finished}+$l->{unfinished}/2)/$maxn*100;
}

if ( @ARGV && $ARGV[0] eq "site") {
	print "<html><body>";
	print "<head>";
	print "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>";
	print "<style>";
	print "body{font-family:sans-serif;}";
	print "table {font-size:80%;border-collapse: collapse;}";
	print "td {border-left:solid 1px #aaaaaa;border-right:solid 1px #aaaaaa;padding:1px 10px;}";
	print ".bartodo{ background-color:red;width:100px;height:20px;}";
	print ".bardone{ background-color:green;width:80px;height:20px;font-size:80%;text-align:center;padding-top:4px;height:16px;color:white;}";
	print "</style></head>";
	print "<table>";
	print "<tr><td colspan=\"2\" style=\"width:250px;\">Language</td><td>Count</td><td>Finished</td><td>Unfinished</td><td>Untranslated</td><td>Percentage</td><td>Translators</td></tr>\n";
	for my $l (sort { $b->{percentage} <=> $a->{percentage} } @lang) {
		last if $l->{percentage} < 35;
		printf "\n<tr>"
			. '<td><img src="flags/%s.png"></td><td nowrap>%s</td>'
			. '<td nowrap>%s</td><td>%d</td><td>%d</td><td>%d</td>'
			. '<td><div class="bartodo"><div class="bardone" style="width:%dpx">%.1f</div></div></td>'
			. '<td>%s</td>'
			. '</tr>',
			$l->{code}, $l->{name},
			$l->{diff}==0 ? $l->{n} : "$l->{n} ($l->{diff})",
			$l->{finished}, $l->{unfinished}, $l->{untranslated},
			$l->{percentage}, $l->{percentage},
			$l->{translator};
	}
	print "</table></body></html>\n";
} else {
	print "<style>";
	print "table {font-size:80%;}";
	print "th {text-align:left; }";
	print ".bartodo{ background-color:red;width:100px;height:20px;}";
	print ".bardone{ background-color:green;width:80px;height:20px;font-size:80%;text-align:center;padding-top:4px;height:16px;color:white;}";
	print "</style>";
	print "<table>";
	print "<tr><th colspan=\"2\" style=\"width:250px;\">Language</th><th>Finished %</th><th>Translators</th></tr>\n";
	for my $l (sort { $b->{percentage} <=> $a->{percentage} } @lang) {
		last if $l->{percentage} < 35;
		printf "\n<tr>"
			. '<td><img src="qrc:/images/flags/%s.png"></td><td>%s</td>'
			. '<td><div title="finished:%d unfinished:%d untranslated:%d" class="bartodo"><div class="bardone" style="width:%dpx">%.1f</div></div></td>'
			. '<td>%s</td>'
			. '</tr>',
			$l->{code}, $l->{name},
			$l->{finished}, $l->{unfinished}, $l->{untranslated},
			$l->{percentage}, $l->{percentage},
			$l->{translator};
	}
	print "</table>\n";
}

my @ts;
for my $l (sort { $a->{code} cmp $b->{code} } @lang) {
	next if $l->{percentage} < 35;
	push @ts, $l->{origcode};
}

rename "i18n/CMakeLists.txt", "i18n/CMakeLists.txt.temp" || die "cannot rename i18n/CMakeLists.txt: $!";

open I, "i18n/CMakeLists.txt.temp";
open O, ">i18n/CMakeLists.txt";
while(<I>) {
	if( /^SET\(TS_FILES / || /^FILE \(GLOB TS_FILES \*\.ts\)/ ) {
		print O "SET(TS_FILES " . join( " ", map { "qgis_$_\.ts"; } @ts ) . ")\n";
	} else {
		print O;
	}
}
close O;
close I;

unlink "i18n/CMakeLists.txt.temp";
