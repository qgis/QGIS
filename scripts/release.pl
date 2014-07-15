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
my $releasename;
my $help;

my $result = GetOptions(
		"major" => \$newmajor,
		"minor" => \$newminor,
		"releasename=s" => \$releasename,
		"help" => \$help
	);

$releasename = shift @ARGV;

pod2usage(1) if $help or !defined $releasename;

my $major;
my $minor;
open F, "CMakeLists.txt";
while(<F>) {
        if(/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/) {
                $major = $1;
        } elsif(/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/) {
                $minor = $1;
        }
}
close F;

if( defined $newmajor ) {
	die "New major must be equal or greater than old major $major" if $newmajor <= $major;
	$newminor = 0 unless defined $newminor;
} else {
	$newmajor = $major;
}

if( defined $newminor ) {
	die "New minor must be greater than current minor $minor" if $newmajor==$major && $newminor<=$minor;
} else {
	$newminor = $minor + 1;
}

sub updateCMakeLists {
	my($major,$minor,$release) = @_;

	rename "CMakeLists.txt", "CMakeLists.txt.orig" or die "cannot rename CMakeLists.txt: $!";
	open I, "CMakeLists.txt.orig";
	open O, ">CMakeLists.txt" or die "cannot create CMakeLists.txt: $!";
	while(<I>) {
		s/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/SET(CPACK_PACKAGE_VERSION_MAJOR "$major")/;
		s/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/SET(CPACK_PACKAGE_VERSION_MINOR "$minor")/;
		s/SET\(RELEASE_NAME "(.+)"\)/SET(RELEASE_NAME "$release")/;
		print O;
	}
	close O;
	close I;
}

print "Last pull rebase...\n";
system( "git pull --rebase" ) == 0 or die "git pull rebase failed";

my $release = "$newmajor.$newminor";
my $relbranch = "release-${newmajor}_${newminor}";
my $reltag = "final-${newmajor}_${newminor}_0";

print "Creating branch...\n";
system( "git checkout -b $relbranch" ) == 0 or die "git checkout release branch failed";
updateCMakeLists($newmajor,$newminor,$releasename);

print "Updating branch...\n";
system( "dch -r ''" ) == 0 or die "dch failed";
system( "dch --newversion $newmajor.$newminor.0 'Release of $release'" ) == 0 or die "dch failed";
system( "cp debian/changelog /tmp" ) == 0 or die "backup changelog failed";

if( -f "images/splash/splash-release.png" ) {
	system( "cp -v images/splash/splash-release.png images/splash/splash.png" ) == 0 or die "splash png switch failed";
} else {
	print "WARNING: NO images/splash/splash-release.png\n";
}

if( -f "images/splash/splash-release.xcf.bz2" ) {
	system( "cp -v images/splash/splash-release.xcf.bz2 images/splash/splash.xcf.bz2" ) == 0 or die "splash xcf switch failed";
} else {
	print "WARNING: NO images/splash/splash-release.xcf.bz2\n";
}

if( -f "ms-windows/Installer-Files/WelcomeFinishPage-release.bmp" ) {
	system( "cp -v ms-windows/Installer-Files/WelcomeFinishPage-release.bmp ms-windows/Installer-Files/WelcomeFinishPage.bmp" ) == 0 or die "installer bitmap switch failed";
} else {
	print "WARNING: NO ms-windows/Installer-Files/WelcomeFinishPage-release.bmp\n";
}

system( "git commit -a -m 'Release of $release ($releasename)'" ) == 0 or die "release commit failed";
system( "git tag $reltag -m 'Version $release'" ) == 0 or die "tag failed";

print "Producing archive...\n";
system( "git archive --format tar --prefix=qgis-$release.0/ $reltag | bzip2 -c >qgis-$release.0.tar.bz2" ) == 0 or die "git archive failed";
system( "md5sum qgis-$newmajor.$newminor.0.tar.bz2 >qgis-$release.0.tar.bz2.md5" ) == 0 or die "md5sum failed";

$newminor++;

print "Updating master...\n";
system( "git checkout master" ) == 0 or die "checkout master failed";
updateCMakeLists($newmajor,$newminor,"Master");
system( "cp /tmp/changelog debian" ) == 0 or die "restore changelog failed";
system("dch -r ''" ) == 0 or die "dch failed";
system( "dch --newversion $newmajor.$newminor.0 'New development version $newmajor.$newminor after branch of $release'" ) == 0 or die "dch failed";
system( "git commit -a -m 'Bump version to $newmajor.$newminor'" ) == 0 or die "bump version failed";

print "Push dry-run...\n";
system( "git push -n origin master $relbranch $reltag" ) == 0 or die "git push -n failed";

print "Now manually push and upload the tarballs :\n\tgit push origin master $relbranch $reltag\n\trsync qgis-$release.0.tar.bz2* qgis.org:/var/www/downloads/\n\n";

=head1 NAME

release.pl - create a new release

=head1 SYNOPSIS

release.pl [options] releasename

  Options:
    -major=newmajor     new major number for release (defaults to current major version number)
    -minor=newminor     new minor number for release (defaults to current minor + 1 or 0 when major version number was increased)
			next master will become minor + 1
=cut
