#!/usr/bin/env perl
# updates the news file from changelog.qgis.org

# Copyright (C) 2016 Jürgen E. Fischer <jef@norbit.de>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

use strict;
use warnings;
use Pod::Usage;
use File::Temp qw/tempfile/;
use File::Copy qw/copy/;
use HTML::Entities qw/decode_entities/;

pod2usage(1) if @ARGV!=2;

my ($version,$releasename) = @ARGV;


my ($news,$tempfile) = tempfile();

open my $in, "NEWS.md";
while(<$in>) {
	print $news $_;
	last if /^Change history for the QGIS Project/;
}

my $content = `curl -A Mozilla -s https://changelog.qgis.org/en/qgis/version/$version/gnu/`;
die "Couldn't get it!" unless defined $content;

print $news "\n# What's new in Version $version '$releasename'?\n\n";
print $news "This release has following new features:\n\n";

die "Invalid changelog" unless $content =~ /^Changelog for QGIS/;

for $_ (split /\n/, $content) {
	next if /^Changelog /;
	next if /^------/;
	next if /^\s*$/;

	$_ = decode_entities($_);

	s/^\*\s+/- /;
	s/ : /: /;
	s/\s+$//;

	print $news "$_\n";
}

print $news "-\n\n";

while(<$in>) {
	print $news $_;
}

close $news;
close $in;

copy($tempfile, "doc/NEWS.md");

system "pandoc --table-of-contents --toc-depth=1 -s -o doc/NEWS.html NEWS.md --metadata title=\"QGIS News\"";

=head1 NAME

update_news.pl - updates the news file from changelog.qgis.org

=head1 SYNOPSIS

update_news.pl version releasename

=cut
