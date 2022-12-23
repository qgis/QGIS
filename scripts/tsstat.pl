#!/usr/bin/env perl
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
use Locales;

my @lang;

# script to generate a html list of the qgis application translations
# showing the percentage finished and the names of the translators
#
# try to always use ISO 639-1 language codes
#
# without argument it generates html which is used in the about-dialog of the application
# output to std-out, to be piped to doc/TRANSLATORS so it can be used in dialog
#   scripts/tsstat.pl > doc/TRANSLATORS
# this version needs flag images from the resources

# with argument 'site' a more complete html page is create to be used on a website
#   scripts/tsstat.pl site > page.html
# this version needs flag images in a directory 'flags'

# translator names here as a hash where the key is the lang_country code used for the ts file name
my $translators= {
	'af' => '',
	'ar' => 'Ichaouia Amine, Hosham Munier, Ammar Shaarbaf',
	'bg' => 'Захари Савов, Jordan Tzvetkov',
	'bs' => 'Almir Karabegovic',
	'ca' => 'Albert F, Pau Reguant Ridó, Xavier Roijals',
	'cs' => 'Jan Helebrant, Martin Landa, Peter Antolik, Martin Dzurov, Stanislav Horáček',
	'da' => 'Jacob Overgaard Madsen, Bo Victor Thomsen',
	'de' => 'Jürgen E. Fischer, Stephan Holl, Otto Dassau, Werner Macho',
	'es' => 'Carlos Dávila, Javier César Aldariz, Gabriela Awad, Edwin Amado, Mayeul Kauffmann, Diana Galindo, Fran Raga',
	'el' => 'Theodoros Vakkas, Ioannis Tsimpiris, Evripidis Argyropoulos, Mike Pegnigiannis, Nikos Ves',
	'eo' => 'Augustin Roche, Nikolay Korotkiy',
	'et' => 'Veiko Viil',
	'eu' => 'Asier Sarasua Garmendia, Irantzu Alvarez',
	'fa' => 'Mola Pahnadayan, Masoud Pashotan , Masoud Erfanyan',
	'fi' => 'Kari Mikkonen, Matti Mäntynen',
	'fr' => 'Arnaud Morvan, Augustin Roche, Didier Vanden Berghe, Dofabien, Etienne Trimaille, Harrissou Sant-anna, Jean-Roc Morreale, Jérémy Garniaux, Loïc Buscoz, Lsam, Marc-André Saia, Marie Silvestre, Mathieu Bossaert, Mathieu Lattes, Mayeul Kauffmann, Médéric Ribreux, Mehdi Semchaoui, Michael Douchin, Nicolas Boisteault, Nicolas Rochard, Pascal Obstetar, Robin Prest, Rod Bera, Stéphane Henriod, Stéphane Possamai, sylther, Sylvain Badey, Sylvain Maillard, Vincent Picavet, Xavier Tardieu, Yann Leveille-Menez, yoda89, Vincent Bré',
	'gl' => 'Xan Vieiro',
	'hi' => 'Harish Kumar Solanki',
	'hu' => 'Zoltan Siki, Zoltan Toldi, Peter Bathory',
	'hr' => 'Zoran Jankovic',
	'is' => 'Ásta Kristín Óladóttir, Thordur Ivarsson, Sveinn í Felli',
	'id' => 'Emir Hartato, Muhammad Iqnaul Haq Siregar, Trias Aditya, Januar V. Simarmata, I Made Anombawa',  #spellok
	'it' => 'Marco Braida, Stefano Campus, Roberta Castelli, Francesco D\'Amore, Eleonora D\'Elia, Simone Falceri, Giulio Fattori, Matteo Ghetta, Federico Gianoli, Marco Grisolia, Italang, Luca76, Pipep, Valerio Pinna, Alberto Vallortigara, Salvatore Fiandaca (reporter), Giuseppe Mattiozzi (documentation)',
	'ja' => 'BABA Yoshihiko, Yoichi Kayama, Minoru Akagi, Takayuki Nuimura, Takayuki Mizutani, Norihiro Yamate, Kohei Tomita',
	'ka' => 'Shota Murtskhvaladze, George Machitidze',
	'km' => 'Khoem Sokhem',
	'ko' => 'OSGeo Korean Chapter',
	'ky' => 'Stéphane Henriod, Azamat Karypov, Salaidin Kamaldinov, Akylbek Chymyrov, Chinara Saparova, Almaz Abdiev, Nurlan Tokbaev, Tatygul Urmambetova, Adilet Bekturov, Nursultan Ismailov, Nurlan Zhusupov',
	'lo' => 'Anousak Souphavanh, Soukanh Lathsavong',
	'lv' => 'Maris Nartiss, Pēteris Brūns',
	'lt' => 'Paulius Litvinas, Tomas Straupis, Kestas M',
	'ml' => 'Vinayan Parameswaran',
	'mn' => 'Bayarmaa Enkhtur',
	'mr' => '',
	'nb' => 'James Stott, Maléne Peterson, Kjell Cato Heskjestad',
	'nl' => 'Richard Duivenvoorde, Raymond Nijssen, Carlo van Rijswijk, Diethard Jansen, Willem Hoffmans, Dick Groskamp',
	'pl' => 'Robert Szczepanek, Milena Nowotarska, Borys Jurgiel, Mateusz Łoskot, Tomasz Paul, Andrzej Świąder, Radosław Pasiok, Michał Kułach, Ewelina Krawczak, Michał Smoczyk, Jakub Bobrowski, Kuba Kiszkurno, Beata Baziak, Bartosz Mazurkiewcz, Tomasz Rychlicki',
	'pt_BR' => 'Sidney Schaberle Goveia, Arthur Nanni, Marcelo Soares Souza, Narcélio de Sá Pereira Filho, Leônidas Descovi Filho, Felipe Sodré Barros ',
	'pt_PT' => 'Giovanni Manghi, Joana Simões, Duarte Carreira, Alexandre Neto, Pedro Pereira, Pedro Palheiro, Nelson Silva, Ricardo Sena, Leandro Infantini, João Gaspar, José Macau',
	'ro' => 'Sorin Călinică, Tudor Bărăscu, Georgiana Ioanovici, Alex Bădescu, Lonut Losifescu-Enescu, Bogdan Pacurar',
	'ru' => 'Alexander Bruy, Artem Popov',
	'sc' => 'Valerio Pinna',
	'sk' => 'Lubos Balazovic, Jana Kormanikova, Ivan Mincik',
	'sl' => 'Jože Detečnik, Dejan Gregor, Jaka Kranjc',
	'sq' => '',
	'sr@latin' => 'Goran Ivanković',
	'sr' => 'Goran Ivanković',
	'sv' => 'Victor Axbom, Lars Luthman, Magnus Homann, Klas Karlsson, Isabelle J Wigren, Daniel Rosander, Anders Ekwall, Magnus Nilsson, Jonas Svensson, Christian Brinkenberg',
	'sw' => '',
	'ta' => '',
	'te' => '',
	'th' => 'Man Chao',
	'tl' => 'Kathrina Gregana',
	'tr' => 'Osman Yalçın YILMAZ, Omur Saygin',
	'uk' => 'Alexander Bruy, Daria Svidzinska, Svitlana Shulik, Alesya Shushova',
	'vi' => 'Phùng Văn Doanh, Bùi Hữu Mạnh, Nguyễn Văn Thanh, Nguyễn Hữu Phúc, Cao Minh Tu',
	'zh-Hant' => 'Calvin Ngei, Zhang Jun, Richard Xie, Dennis Raylin Chen',
	'zh-Hans' => 'Calvin Ngei, Lisashen, Wang Shuai, Xu Baocai',
};

my $maxn;

my $locale = Locales->new("en_US");

for my $i (<i18n/qgis_*.ts>) {
	my ($langcode) = $i =~ /i18n\/qgis_(.*).ts/;
	next if $langcode eq "en";

	my $translator = $translators->{$langcode} || "(orphaned)";

	my $charset = "";
	my $lc = $langcode;
	my $svg = $langcode;
	if( $langcode =~ /(.*)\@latin/ ) {
		$charset = " (latin)";
		$langcode = $1;
		$svg = $1;
	}
	if( $langcode =~ /(.*)\-Hans/ ) {
                $charset = " simplified";
                $langcode = $1;
        }
	if( $langcode =~ /(.*)\-Hant/ ) {
                $charset = " traditional";
                $langcode = $1;
        }

	my $name = $locale->get_language_from_code($langcode);
	$name .= $charset;

	open F, "LC_MESSAGES=C lrelease $i|";

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
		svg=>$svg,
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

print "<!-- created by scripts/tsstat.pl - Edits will be lost -->\n";
print "<style>";
print "body { font-family:sans-serif; background-color:#d3d3d3; }";
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
		. '<td align="center"><img src="qrc:/images/flags/%s.svg" height="20"></td><td>%s</td>'
		. '<td><div title="finished:%d unfinished:%d untranslated:%d" class="bartodo"><div class="bardone" style="width:%dpx">%.1f</div></div></td>'
		. '<td>%s</td>'
		. '</tr>',
		$l->{svg}, $l->{name},
		$l->{finished}, $l->{unfinished}, $l->{untranslated},
		$l->{percentage}, $l->{percentage},
		$l->{translator};
}
print "</table>\n";

my @ts;
for my $l (sort { $a->{code} cmp $b->{code} } @lang) {
	next if $l->{percentage} < 35;
	push @ts, $l->{origcode};
}

rename "i18n/CMakeLists.txt", "i18n/CMakeLists.txt.temp" || die "cannot rename i18n/CMakeLists.txt: $!";

open I, "i18n/CMakeLists.txt.temp";
open O, ">i18n/CMakeLists.txt";
while(<I>) {
	if( /^SET\(TS_FILES /i ) {
		print O "set(TS_FILES " . join( " ", map { "qgis_$_\.ts"; } @ts ) . ")\n";
	} else {
		print O;
	}
}
close O;
close I;

unlink "i18n/CMakeLists.txt.temp";
