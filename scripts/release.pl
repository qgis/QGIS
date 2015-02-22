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

my $dryrun;

sub updateCMakeLists($$$$) {
	my($major,$minor,$patch,$release) = @_;

	if( $dryrun ) {
		print "DRYRUN: Update CMakeLists.txt to $major.$minor.$patch ($release)\n";
		return;
	}

	rename "CMakeLists.txt", "CMakeLists.txt.orig" or die "cannot rename CMakeLists.txt: $!";
	open I, "CMakeLists.txt.orig";
	open O, ">CMakeLists.txt" or die "cannot create CMakeLists.txt: $!";
	while(<I>) {
		s/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/SET(CPACK_PACKAGE_VERSION_MAJOR "$major")/;
		s/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/SET(CPACK_PACKAGE_VERSION_MINOR "$minor")/;
		s/SET\(CPACK_PACKAGE_VERSION_PATCH "(\d+)"\)/SET(CPACK_PACKAGE_VERSION_PATCH "$patch")/;
		s/SET\(RELEASE_NAME "(.+)"\)/SET(RELEASE_NAME "$release")/;
		print O;
	}
	close O;
	close I;
}

sub run($$) {
	my($cmd, $errmsg) = @_;
	if( $dryrun ) {
		print "DRYRUN: $cmd\n";
	} elsif( system($cmd) != 0 ) {
		print STDERR "error: $errmsg [$?]";
		exit 1;
	}
}

my $newreleasename;
my $help;
my $domajor;
my $dominor;
my $dopoint;

my $result = GetOptions(
		"major" => \$domajor,
		"minor" => \$dominor,
		"point" => \$dopoint,
		"releasename=s" => \$newreleasename,
		"help" => \$help,
		"dryrun" => \$dryrun,
	);

pod2usage(1) if $help;

my $i = 0;
$i++ if defined $domajor;
$i++ if defined $dominor;
$i++ if defined $dopoint;
pod2usage("Exactly one of -major, -minor or -point expected") if $i!=1;
pod2usage("Release name for major and minor releases expected") if !$dopoint && !defined $newreleasename;
pod2usage("No CMakeLists.txt in current directory") unless -r "CMakeLists.txt";

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
	} elsif(/SET\(RELEASE_NAME \"(.*)\"\)/) {
		$releasename = $1;
        }
}
close F;

my $branch = `git rev-parse --abbrev-ref HEAD 2>/dev/null`;
$branch =~ s/\s+$//;
pod2usage("Not on a branch") unless $branch;
pod2usage("Current branch is $branch. master or a release branch expected") if $branch !~ /^(master|release-(\d+)_(\d+))$/;
pod2usage("Version mismatch ($2.$3 in branch $branch vs. $major.$minor in CMakeLists.txt)") if $branch ne "master" && ( $major != $2 || $minor != $3 );
pod2usage("Release name Master expected on master branch" ) if $branch eq "master" && $releasename ne "Master";

if( $branch eq "master" ) {
	pod2usage("No point releases on master branch") if $dopoint;
	pod2usage("No new release name for major/minor release") unless $newreleasename || $newreleasename eq $releasename;
} else {
	pod2usage("Only point releases on release branches") if !$dopoint;
	pod2usage("New release names only for new minor releases") if $newreleasename;
	$newreleasename = $releasename;
}

my $newmajor;
my $newminor;
my $newpatch;
if( $domajor ) {
	$newmajor = $major + 1;
	$newminor = 0;
	$newpatch = 0;
} elsif( $dominor ) {
	$newmajor = $major;
	$newminor = $minor + 1;
	$newpatch = 0;
} elsif( $dopoint ) {
	$newmajor = $major;
	$newminor = $minor;
	$newpatch = $patch + 1;
} else {
	pod2usage("No version change");
}

pod2usage("Splash images/splash/splash-$newmajor.$newminor.png not found") unless -r "images/splash/splash-$newmajor.$newminor.png";
pod2usage("NSIS image ms-windows/Installer-Files/WelcomeFinishPage-$newmajor.$newminor.bmp not found") unless -r "ms-windows/Installer-Files/WelcomeFinishPage-$newmajor.$newminor.bmp";

print "Last pull rebase...\n";
run( "git pull --rebase", "git pull rebase failed" );

my $release = "$newmajor.$newminor";
my $version = "$release.$newpatch";
my $relbranch = "release-${newmajor}_${newminor}";
my $reltag = "final-${newmajor}_${newminor}_${newpatch}";

unless( $dopoint ) {
	print "Pulling transifex translations...\n";
	run( "scripts/pull_ts.sh", "pull_ts.sh failed" );
	run( "git add i18n/*.ts", "adding translations failed" );
	run( "git commit -a -m \"translation update for $release from transifex\"", "could not commit translation updates" );
}

print "Updating changelog...\n";
run( "scripts/create_changelog.sh", "create_changelog.sh failed" );

unless( $dopoint ) {
	run( "git commit -a -m \"changelog update for $release\"", "could not commit changelog update" );

	print "Creating and checking out branch...\n";
	run( "git checkout -b $relbranch", "git checkout release branch failed" );
}

updateCMakeLists($newmajor,$newminor,$newpatch,$releasename);

print "Updating version...\n";
run( "dch -r ''", "dch failed" );
run( "dch --newversion $version 'Release of $version'", "dch failed" );
run( "cp debian/changelog /tmp", "backup changelog failed" );

unless( $dopoint ) {
	run( "cp -v images/splash/splash-$newmajor.$newminor.png images/splash/splash.png", "splash png switch failed" );
	run( "cp -v ms-windows/Installer-Files/WelcomeFinishPage-$newmajor.$newminor.bmp ms-windows/Installer-Files/WelcomeFinishPage.bmp", "installer bitmap switch failed" );

	if( -f "images/splash/splash-release.xcf.bz2" ) {
		run( "cp -v images/splash/splash-$newmajor.$newminor.xcf.bz2 images/splash/splash.xcf.bz2", "splash xcf switch failed" );
	} else {
		print "WARNING: NO images/splash/splash-release.xcf.bz2\n";
	}

	run( "git commit -a -m 'Release of $release ($releasename)'", "release commit failed" );
	run( "git tag $reltag -m 'Version $release'", "tag failed" );
} else {
	run( "git commit -a -m 'Release of $version'", "release commit failed" );
	run( "git tag $reltag -m 'Version $version'", "tag failed" );
}

print "Producing archive...\n";
run( "git archive --format tar --prefix=qgis-$version/ $reltag | bzip2 -c >qgis-$version.tar.bz2", "git archive failed" );
run( "md5sum qgis-$version.tar.bz2 >qgis-$version.tar.bz2.md5", "md5sum failed" );

unless( $dopoint ) {
	$newminor++;

	print "Updating master...\n";
	run( "git checkout master", "checkout master failed" );
	updateCMakeLists($newmajor,$newminor,0,"Master");
	run( "cp /tmp/changelog debian", "restore changelog failed" );
	run( "dch -r ''", "dch failed" );
	run( "dch --newversion $newmajor.$newminor.0 'New development version $newmajor.$newminor after branch of $release'", "dch failed" );
	run( "git commit -a -m 'Bump version to $newmajor.$newminor'", "bump version failed" );
}

my $topush = ($dopoint ? "" : "master ") . "$relbranch $reltag";

print "Push dry-run...\n";
run( "git push -n origin $topush", "push dry run failed" );
print "Now manually push and upload the tarballs :\n\tgit push origin $topush\n\trsync qgis-$version.tar.bz2* qgis.org:/var/www/downloads/\n\n";


=head1 NAME

release.pl - create a new release

=head1 SYNOPSIS

release.pl {{-major|-minor} -releasename=releasename|-point}

  Options:
    -major              do a new major release
    -minor              do a new minor release
    -point              do a new point release
    -releasename=name   new release name for master/minor release
    -dryrun             just echo but don't run any commands
=cut
