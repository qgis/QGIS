#!/usr/bin/perl
# creates a new release

# Copyright (C) 2014 Jürgen E. Fischer <jef@norbit.de>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;

my $newmajor;
my $newminor;
my $newreleasename;
my $help;
my $pointrelease;

my $result = GetOptions(
		"major=s" => \$newmajor,
		"minor=s" => \$newminor,
		"releasename=s" => \$newreleasename,
		"help" => \$help,
		"pointrelease" => \$pointrelease,
	);

pod2usage(1) if $help;

system("git fetch") == 0 or die "git fetch failed";

unless( $pointrelease ) {
	$newreleasename = shift @ARGV;
	pod2usage(1) unless $newreleasename;
	system("git checkout master" ) == 0 or die "git checkout master failed";
} else {
	my $latestrelease = `git branch -r | sed -ne "s#^ *origin/release-##p" | sort -rn | head -1`;
	print "Current release branch $latestrelease\n";
	system("git checkout release-$latestrelease") == 0 or die "git checkout release-$latestrelease";
}

pod2usage(1) if @ARGV;

my $major;
my $minor;
my $patch;
my $releasename;
open F, "CMakeLists.txt";
while(<F>) {
        if(/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/) {
                $major = $1;
        } elsif(/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/) {
                $minor = $1;
        } elsif(/SET\(CPACK_PACKAGE_VERSION_PATCH "(\d+)"\)/) {
                $patch = $1;
	} elsif(/SET\(RELEASE_NAME "(.+)"\)/) {
		$releasename = $1;
        }
}
close F;

my $newpatch;
unless( $pointrelease ) {
	if( defined $newmajor ) {
		die "New major must be equal or greater than old major $major" if $newmajor <= $major;
		$newminor = 0 unless defined $newminor;
	} else {
		$newmajor = $major;
	}

	if( defined $newminor ) {
		die "New minor must be greater than current minor $minor" if $newmajor==$major && $newminor<=$minor;
	}

	$newpatch = 0;
} else {
	die "-major not supported with point releases" if $newmajor;
	die "-minor not supported with point releases" if $newminor;

	$newmajor = $major;
	$newminor = $minor;
	$newpatch = $patch + 1;
}

die "No change." if $major==$newmajor && $minor==$newminor && $patch==$newpatch;

sub updateCMakeLists {
	my($major,$minor,$patch,$releasename) = @_;

	rename "CMakeLists.txt", "CMakeLists.txt.orig" or die "cannot rename CMakeLists.txt: $!";
	open I, "CMakeLists.txt.orig";
	open O, ">CMakeLists.txt" or die "cannot create CMakeLists.txt: $!";
	while(<I>) {
		s/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/SET(CPACK_PACKAGE_VERSION_MAJOR "$major")/;
		s/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/SET(CPACK_PACKAGE_VERSION_MINOR "$minor")/;
		s/SET\(CPACK_PACKAGE_VERSION_PATCH "(\d+)"\)/SET(CPACK_PACKAGE_VERSION_PATCH "$patch")/;
		s/SET\(RELEASE_NAME "(.+)"\)/SET(RELEASE_NAME "$releasename")/;
		print O;
	}
	close O;
	close I;
}

print "Last pull rebase...\n";
system("git pull --rebase") == 0 or die "git pull rebase failed";

my $release   = "$newmajor.$newminor";
my $relbranch = "release-${newmajor}_${newminor}";
my $reltag    = "final-${newmajor}_${newminor}_${newpatch}";

unless( $pointrelease ) {
	print "Creating branch...\n";
	system("git checkout -b $relbranch" ) == 0 or die "git creation of release branch failed";
	updateCMakeLists($newmajor,$newminor,$newpatch,$releasename);

	print "Updating branch...\n";
	system("dch -r ''" ) == 0 or die "dch failed";
	system( "dch --newversion $newmajor.$newminor.$newpatch 'Release of $release'" ) == 0 or die "dch failed";
	system( "cp debian/changelog /tmp" ) == 0 or die "backup changelog failed";
	system( "git commit -a -m 'Release of $release ($releasename)'" ) == 0 or die "release commit failed";

	system( "git tag $reltag -m 'Version $release'" ) == 0 or die "tag failed";

	$newminor++;

	print "Updating master...\n";
	system( "git checkout master" ) == 0 or die "checkout master failed";
	updateCMakeLists($newmajor,$newminor,$newpatch,"Master");
	system( "cp /tmp/changelog debian" ) == 0 or die "restore changelog failed";
	system("dch -r ''" ) == 0 or die "dch failed";
	system( "dch --newversion $newmajor.$newminor.$newpatch 'New development version $newmajor.$newminor after branch of $release'" ) == 0 or die "dch failed";
	system( "git commit -a -m 'Bump version to $newmajor.$newminor'" ) == 0 or die "bump version failed";
} else {
	print "New point release...\n";
	updateCMakeLists($newmajor,$newminor,$newpatch,$releasename);

	system( "dch --newversion $newmajor.$newminor.$newpatch 'New point release $release.$newpatch'" ) == 0 or die "dch failed";
	system( "git commit -a -m 'Point release of $release.$newpatch ($releasename)'" ) == 0 or die "release commit failed";
	system( "git tag $reltag -m 'Point release $release.$newpatch'" ) == 0 or die "tag failed";
}

print "Producing archive...\n";
system( "git archive --format tar --prefix=qgis-$release.$newpatch/ $reltag | bzip2 -c >qgis-$release.$newpatch.tar.bz2" ) == 0 or die "git archive failed";
system( "md5sum qgis-$newmajor.$newminor.$newpatch.tar.bz2 >qgis-$release.$newpatch.tar.bz2.md5" ) == 0 or die "md5sum failed";

print "Push dry-run...\n";
system( "git push -n origin master $relbranch $reltag" ) == 0 or die "git push -n failed";

print "Now manually push, update http://qgis.org/version.txt and upload the tarballs:\n";
print "\tgit push origin master $relbranch $reltag\n\trsync qgis-$release.$newpatch.tar.bz2* qgis.org:/var/www/downloads/\n\n";

=head1 NAME

release.pl - create a new release

=head1 SYNOPSIS

release.pl (-major=number | -minor=number |) releasename 	# create new major release
release.pl -pointrelease					# create new point release

  Options:
    -pointrelease       do a new point release (increment patch level, tag and create tarball)

  Major release options:
    -major=newmajor     new major number for release (defaults to current major version number)
    -minor=newminor     new minor number for release (defaults to current minor + 1, 0 if major was increased)
=cut
