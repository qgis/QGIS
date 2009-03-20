#!/usr/bin/perl

use strict;
use Locale::Language;
use Locale::Country;

print "||'''Language'''||'''Count'''||'''Translated'''||'''Translation finished'''||'''Translated unfinished'''||'''Untranslated'''||'''Percentage'''||\n";

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

        print "||'''$name'''||$n||$translations||$finished||$unfinished||$untranslated||" . sprintf("%.1f%", ($n-$untranslated)/$n*100) . "||\n";
}
