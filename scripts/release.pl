#!/usr/bin/env perl
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
use POSIX;

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
		s/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/set(CPACK_PACKAGE_VERSION_MAJOR "$major")/i;
		s/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/set(CPACK_PACKAGE_VERSION_MINOR "$minor")/i;
		s/SET\(CPACK_PACKAGE_VERSION_PATCH "(\d+)"\)/set(CPACK_PACKAGE_VERSION_PATCH "$patch")/i;
		s/SET\(RELEASE_NAME "(.+)"\)/set(RELEASE_NAME "$release")/i;
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
my $doltr = 0;
my $dopremajor = 0;
my $skipts = 0;

my $result = GetOptions(
		"major" => \$domajor,
		"minor" => \$dominor,
		"point:i" => \$dopoint,
		"releasename=s" => \$newreleasename,
		"help" => \$help,
		"ltr" => \$doltr,
		"dryrun" => \$dryrun,
		"premajor" => \$dopremajor,
		"skipts" => \$skipts,
	);

pod2usage(1) if $help;

my $i = 0;
$i++ if defined $domajor;
$i++ if defined $dominor;
$i++ if defined $dopoint;
pod2usage("Exactly one of -major, -minor or -point expected") if $i!=1;
pod2usage("Release name for major and minor releases expected") if !defined $dopoint && !defined $newreleasename;
pod2usage("Pre-major releases can only be minor releases") if $dopremajor && !$dominor;
pod2usage("No CMakeLists.txt in current directory") unless -r "CMakeLists.txt";

my $major;
my $minor;
my $patch;
my $releasename;
open F, "CMakeLists.txt";
while(<F>) {
        if(/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/i) {
                $major = $1;
        } elsif(/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/i) {
                $minor = $1;
        } elsif(/SET\(CPACK_PACKAGE_VERSION_PATCH "(\d+)"\)/i) {
                $patch = $1;
	} elsif(/SET\(RELEASE_NAME \"(.*)\"\)/i) {
		$releasename = $1;
        }
}
close F;

my $branch = `git rev-parse --abbrev-ref HEAD 2>/dev/null`;
$branch =~ s/\s+$//;
pod2usage("Not on a branch") unless $branch;
pod2usage("Current branch is $branch. master or a release branch expected") if $branch !~ /^(master.*|release-(\d+)_(\d+))$/;
pod2usage("Version mismatch ($2.$3) in branch $branch vs. $major.$minor in CMakeLists.txt)") if $branch !~ /master/ && ( $major != $2 || $minor != $3 );
pod2usage("Release name Master expected on master branch" ) if $branch =~ /^master/ && $releasename ne "Master";

if( $branch =~ /^master.*/ ) {
	pod2usage("No point releases on master branch") if defined $dopoint;
	pod2usage("No new release name for major/minor release") unless $newreleasename || $newreleasename eq $releasename;
} else {
	pod2usage("Only point releases on release branches") if !defined $dopoint;
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
} elsif( defined $dopoint ) {
	$newmajor = $major;
	$newminor = $minor;
	pod2usage("Given point release number <= $patch") if $dopoint && $dopoint <= $patch;
	$newpatch = $dopoint ? $dopoint : $patch + 1;
} else {
	pod2usage("No version change");
}

my $splashwidth;
unless( defined $dopoint ) {
	pod2usage("Splash images/splash/splash-$newmajor.$newminor.png not found") unless -r "images/splash/splash-$newmajor.$newminor.png";
}

print "Last pull rebase...\n";
run( "git pull --rebase", "git pull rebase failed" );

my $release = "$newmajor.$newminor";
my $version = "$release.$newpatch";
my $relbranch = "release-${newmajor}_${newminor}";
my $ltrtag = $doltr ? "ltr-${newmajor}_${newminor}" : "";
my $reltag = "final-${newmajor}_${newminor}_${newpatch}";

unless( $skipts ) {
	print "Pulling transifex translations...\n";
	run( "scripts/pull_ts.sh", "pull_ts.sh failed" );
	run( "git add i18n/*.ts", "adding translations failed" );
	run( "git commit -n -a -m \"translation update for $version from transifex\"", "could not commit translation updates" );
} else {
	print "TRANSIFEX UPDATE SKIPPED!\n";
}

print "Updating changelog...\n";
run( "scripts/create_changelog.sh", "create_changelog.sh failed" );
run( "perl -i -pe 's#<releases>#<releases>\n    <release version=\"$newmajor.$newminor.$newpatch\" date=\"" . strftime("%Y-%m-%d", localtime) . "\" />#' linux/org.qgis.qgis.appdata.xml.in", "appdata update failed" );

unless( defined $dopoint ) {
	run( "scripts/update_news.pl $newmajor.$newminor '$newreleasename'", "could not update news" ) if $major>2 || ($major==2 && $minor>14);

	run( "git commit -n -a -m \"changelog and news update for $release\"", "could not commit changelog and news update" );

	print "Creating and checking out branch...\n";
	run( "git checkout -b $relbranch", "git checkout release branch failed" );
}

updateCMakeLists($newmajor,$newminor,$newpatch,$newreleasename);

print "Updating version...\n";
run( "dch -r ''", "dch failed" );
run( "dch --newversion $version 'Release of $version'", "dch failed" );
run( "cp debian/changelog /tmp", "backup changelog failed" );

unless( defined $dopoint ) {
	run( "perl -i -pe 's/qgis-dev-deps/qgis-ltr-deps/;' INSTALL.md", "could not update osgeo4w deps package" ) if $doltr;
	run( "perl -i -pe 's/qgis-dev-deps/qgis-rel-deps/;' INSTALL.md", "could not update osgeo4w deps package" ) unless $doltr;
	run( "cp -v images/splash/splash-$newmajor.$newminor.png images/splash/splash.png", "splash png switch failed" );
	run( "git commit -n -a -m 'Release of $release ($newreleasename)'", "release commit failed" );
	run( "git tag $reltag -m 'Version $release'", "release tag failed" );
	run( "for i in \$(seq 20); do tx push -s -b $relbranch && exit 0; echo \"Retry \$i/20...\"; done; exit 1", "push translation for $relbranch branch" );
} else {
	run( "git commit -n -a -m 'Release of $version'", "release commit failed" );
	run( "git tag $reltag -m 'Version $version'", "tag failed" );
}

run( "git tag $ltrtag -m 'Long term release $release'", "ltr tag failed" ) if $doltr;

print "Producing archive...\n";
run( "git archive --format tar --prefix=qgis-$version/ $reltag | bzip2 -c >qgis-$version.tar.bz2", "git archive failed" );
run( "sha256sum qgis-$version.tar.bz2 >qgis-$version.tar.bz2.sha256", "sha256sum failed" );

my @topush;
unless( defined $dopoint ) {
	$newminor++;

	print "Updating master...\n";
	run( "git checkout $branch", "checkout master failed" );

	if($dopremajor) {
		print " Creating master_$newmajor...\n";
		run( "git checkout -b master_$newmajor", "checkout master_$newmajor failed" );
		updateCMakeLists($newmajor,$newminor,0,"Master");
		run( "cp /tmp/changelog debian", "restore changelog failed" );
		run( "dch -r ''", "dch failed" );
		run( "dch --newversion $newmajor.$newminor.0 'New development version $newmajor.$newminor after branch of $release'", "dch failed" );
		run( "git commit -n -a -m 'New development branch for interim $newmajor.x releases'", "bump version failed" );

		push @topush, "master_$newmajor";

		run( "git checkout master", "checkout master failed" );
		$newminor=99;
	}

	updateCMakeLists($newmajor,$newminor,0,"Master");
	run( "cp /tmp/changelog debian", "restore changelog failed" );
	run( "dch -r ''", "dch failed" );
	run( "dch --newversion $newmajor.$newminor.0 'New development version $newmajor.$newminor after branch of $release'", "dch failed" );
	run( "git commit -n -a -m 'Bump version to $newmajor.$newminor'", "bump version failed" );

	push @topush, $branch;
}

push @topush, $relbranch;
my $topush = join(" ", @topush);

print "Push dry-run...\n";
run( "git push -n --follow-tags origin $topush", "push dry run failed" );
print "Now manually push and upload the tar balls:\n\tgit push --follow-tags origin $topush\n\trsync qgis-$version.tar.bz2* ssh.qgis.org:/var/www/downloads/\n";
print "Update version-ltr.txt rewrite rule on website\n" if $doltr;
unless($dopoint) {
	print "Create new transifex branch and push the translations.\n";
	print "Update the versions and release name in release spreadsheet.\n";
	print "Package and update the website afterwards.\n";
}
print "WARNING: TRANSIFEX UPDATE SKIPPED!\n" if $skipts;


=head1 NAME

release.pl - create a new release

=head1 SYNOPSIS

release.pl {{-major|-minor [-premajor]} [-skipts] -releasename=releasename|-point[=version]} [-ltr]

  Options:
    -major              do a new major release
    -minor              do a new minor release
    -point[=number]     do a new point release with an optional number
    -releasename=name   new release name for master/minor release
    -ltr                new release is a long term release
    -dryrun             just echo but don't run any commands
    -skipts		skip transifex update
    -premajor           branch off a second "master" branch before
			a major release

  Major and minor releases also require a new splash screen
  images/splash/splash-M.N.png.

  A pre-major minor release also produces a second branch
  master_$currentmajor to allow more interim minor releases
  while the new major version is being developed in master.
  For that the minor version of the master branch leading
  to the next major release is bumped to 99.
=cut
