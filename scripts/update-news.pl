#!/usr/bin/perl
# updates the news file from changelog.qgis.org

# Copyright (C) 2016 Jürgen E. Fischer <jef@norbit.de>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

use strict;
use warnings;
use Pod::Usage;
use LWP::Simple;
use File::Temp qw/tempfile/;
use File::Copy qw/copy/;
use HTML::Entities qw/decode_entities/;

pod2usage(1) if @ARGV!=3;

my ($major,$minor,$releasename) = @ARGV;


my ($news,$tempfile) = tempfile();

open my $in, "doc/news.t2t";
while(<$in>) {
	print $news $_;
	last if /^Last Change/;
}

my $content = get("http://changelog.qgis.org/en/qgis/version/$major.$minor.0/gnu/" );
die "Couldn't get it!" unless defined $content;

print $news "\n= What's new in Version $major.$minor '$releasename'? =\n\n";
print $news "This release has following new features:\n\n";

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

copy($tempfile, "doc/news.t2t");

system "txt2tags -odoc/news.html -t html doc/news.t2t";
system "txt2tags -oNEWS -t txt doc/news.t2t";

=head1 NAME

update-news.pl - updates the news file from changelog.qgis.org

=head1 SYNOPSIS

update-news.pl major minor releasename

=cut
