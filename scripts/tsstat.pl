#!/usr/bin/perl

use strict;
use Locale::Language;
use Locale::Country;

my @lang;

print "|Language|Count|Translated|Translation finished|Translation unfinished|Untranslated|Percentage|\n";

for my $i (<i18n/qgis_*.ts>) {
        my ($langcode) = $i =~ /i18n\/qgis_(.*).ts/;

        my $name;
        if($langcode =~ /(.*)_(.*)/) {
                my $lang = code2language(lc $1);
                my $country = code2country(lc $2);
                $name = "$lang ($country)";
        } else {
                $name = code2language(lc $langcode);
        }


        open F, "lrelease $i|";

        my($translations,$finished,$unfinished,$untranslated);

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

        push @lang, { name=>$name, n=>$n, translations=>$translations, finished=>$finished, unfinished=>$unfinished, untranslated=>$untranslated, percentage=>($n-$untranslated)/$n*100 };
}


for my $l (sort { $b->{percentage} <=> $a->{percentage} } @lang) {
        print "|", $l->{name}, "|", join("|", $l->{n}, $l->{translations}, $l->{finished}, $l->{unfinished}, $l->{untranslated}, sprintf("%.1f", $l->{percentage}) ), "|\n";
}
