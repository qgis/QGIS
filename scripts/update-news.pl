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

open my $in, "doc/NEWS.t2t";
while(<$in>) {
	print $news $_;
	last if /^Last Change/;
}

my $content = `curl -s http://changelog.qgis.org/en/qgis/version/$version/gnu/`;
die "Couldn't get it!" unless defined $content;

print $news "\n= What's new in Version $version '$releasename'? =\n\n";
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

copy($tempfile, "doc/NEWS.t2t");

system "txt2tags --encoding=utf-8 -odoc/NEWS.html -t html doc/NEWS.t2t";
system "txt2tags --encoding=utf-8 -oNEWS -t txt doc/NEWS.t2t";

=head1 NAME

update-news.pl - updates the news file from changelog.qgis.org

=head1 SYNOPSIS

update-news.pl version releasename

=cut
