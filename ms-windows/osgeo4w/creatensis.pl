#!/usr/bin/perl
# creates a NSIS installer from OSGeo4W packages
# note: works also on Unix

# Copyright (C) 2010 Jürgen E. Fischer <jef@norbit.de>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.         

#
# Download OSGeo4W packages
#

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;

my $keep = 0;
my $verbose = 0;

my $packagename;
my $releasename;
my $shortname;
my $version;
my $binary;
my $ininame = "setup.ini";
my $help;

my $result = GetOptions(
		"verbose+" => \$verbose,
		"keep" => \$keep,
		"releasename=s" => \$releasename,
		"version=s" => \$version,
		"binary=i" => \$binary,
		"packagename=s" => \$packagename,
		"shortname=s" => \$shortname,
		"ininame=s" => \$ininame,
		"help" => \$help
	);

if( $help ) {
	pod2usage(1);
}

my $wgetopt = $verbose ? "" : "-q";

unless(-f "nsis/System.dll") {
	mkdir "nsis", 0755 unless -d "nsis";
	system "wget $wgetopt -Onsis/System.dll http://qgis.org/downloads/System.dll";
}

mkdir "packages", 0755 unless -d "packages";
chdir "packages";

my $root = "http://download.osgeo.org/osgeo4w";

system "wget $wgetopt -c http://nsis.sourceforge.net/mediawiki/images/9/9d/Untgz.zip" unless -f "Untgz.zip";
system "wget $wgetopt -c http://www.nirsoft.net/utils/nircmd.zip" unless -f "nircmd.zip";

my %dep;
my %file;
my $package;

system "wget $wgetopt -O setup.ini -c $root/$ininame";
open F, "setup.ini" || die "setup.ini not found";
while(<F>) {
	chop;
	if(/^@ (\S+)/) {
		$package = $1;
	} elsif( /^requires: (.*)$/ ) {
		@{$dep{$package}} = split / /, $1;
	} elsif( /^install:\s+(\S+)\s+/) {
		$file{$package} = $1 unless exists $file{$package};
	}
}
close F;

my %pkgs;

sub getDeps {
	my ($pkg) = @_;

	return if exists $pkgs{$pkg};

	print " Including package $pkg\n" if $verbose;
	$pkgs{$pkg} = 1;

	foreach my $p ( @{ $dep{$pkg} } ) {
		getDeps($p);
	}
}

unless(@ARGV) {
	print "Defaulting to qgis-full package...\n" if $verbose;
	push @ARGV, "qgis-full";
}

getDeps($_) for @ARGV;

if(-f "../addons/bin/NCSEcw4_RO.dll") {
	print "Enabling ECW support...\n" if $verbose;
	getDeps("gdal-ecw")
}

foreach my $p ( keys %pkgs ) {
	my $f = "$root/$file{$p}";
	$f =~ s/\/\.\//\//g;

	my($file) = $f =~ /([^\/]+)$/;

	next if -f $file;
	
	print "Downloading $file [$f]...\n" if $verbose;
	system "wget $wgetopt -c $f";
}

chdir "..";

#
# Unpack them
# Add nircmd
# Add addons
#

if( -d "unpacked" ) {
	unless( $keep ) {
		print "Removing unpacked directory\n" if $verbose;
		system "rm -rf unpacked";
	} else {
		print "Keeping unpacked directory\n" if $verbose;
	}
}

my $taropt = "v" x $verbose;

unless(-d "unpacked") {
	mkdir "unpacked", 0755;

	foreach my $p ( keys %pkgs ) {
		$p = $file{$p};
		$p =~ s#^.*/#packages/#;

		print "Unpacking $p...\n" if $verbose;
		system "tar $taropt -C unpacked -xjf $p";
	}

	chdir "unpacked";

	mkdir "bin", 0755;
	mkdir "apps", 0755;
	mkdir "apps/nircmd", 0755;

	system "cd apps/nircmd; unzip ../../../packages/nircmd.zip && mv nircmd.exe ../../bin";

	if( -d "../addons" ) {
		print " Including addons...\n" if $verbose;
		system "tar -C ../addons -cf - . | tar $taropt -xf -";
	}

	chdir "..";
}

#
# Create postinstall.bat
#

open F, ">../Installer-Files/postinstall.bat";

print F "\@echo off\r\n";
print F "del postinstall.log>>postinstall.log\r\n";
print F "echo OSGEO4W_ROOT=%OSGEO4W_ROOT%>>postinstall.log 2>&1\r\n";
print F "echo OSGEO4W_STARTMENU=%OSGEO4W_STARTMENU%>>postinstall.log 2>&1\r\n";
print F "set OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT:\\=/%\r\n";
print F "if \"%OSGEO4W_ROOT_MSYS:~1,1%\"==\":\" set OSGEO4W_ROOT_MSYS=/%OSGEO4W_ROOT_MSYS:~0,1%/%OSGEO4W_ROOT_MSYS:~3%\r\n";
print F "echo OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT_MSYS%>>postinstall.log 2>&1\r\n";
print F "PATH %OSGEO4W_ROOT%\\bin;%PATH%>>postinstall.log 2>&1\r\n";
print F "cd %OSGEO4W_ROOT%>>postinstall.log 2>&1\r\n";

chdir "unpacked";
for my $p (<etc/postinstall/*.bat>) {
	$p =~ s/\//\\/g;
	my($dir,$file) = $p =~ /^(.+)\\([^\\]+)$/;

	print F "echo Running postinstall $file...\r\n";
	print F "%COMSPEC% /c $p>>postinstall.log 2>&1\r\n";
	print F "ren $p $file.done>>postinstall.log 2>&1\r\n";
}
chdir "..";

print F "ren postinstall.bat postinstall.bat.done\r\n";

close F;

open F, ">../Installer-Files/preremove.bat";

print F "\@echo off\r\n";
print F "del preremove.log>>preremove.log\r\n";
print F "echo OSGEO4W_ROOT=%OSGEO4W_ROOT%>>preremove.log 2>&1\r\n";
print F "echo OSGEO4W_STARTMENU=%OSGEO4W_STARTMENU%>>preremove.log 2>&1\r\n";
print F "set OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT:\\=/%\r\n";
print F "if \"%OSGEO4W_ROOT_MSYS:~1,1%\"==\":\" set OSGEO4W_ROOT_MSYS=/%OSGEO4W_ROOT_MSYS:~0,1%/%OSGEO4W_ROOT_MSYS:~3%\r\n";
print F "echo OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT_MSYS%>>preremove.log 2>&1\r\n";
print F "PATH %OSGEO4W_ROOT%\\bin;%PATH%>>preremove.log 2>&1\r\n";
print F "cd %OSGEO4W_ROOT%>>preremove.log 2>&1\r\n";

chdir "unpacked";
for my $p (<etc/preremove/*.bat>) {
	$p =~ s/\//\\/g;
	my($dir,$file) = $p =~ /^(.+)\\([^\\]+)$/;

	print F "echo Running preremove $file...\r\n";
	print F "%COMSPEC% /c $p>>preremove.log 2>&1\r\n";
	print F "ren $p $file.done>>preremove.log 2>&1\r\n";
}
chdir "..";

print F "ren preremove.bat preremove.bat.done\r\n";

close F;

my($major, $minor, $patch);

open F, "../../CMakeLists.txt";
while(<F>) {
	if(/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/) {
		$major = $1;
	} elsif(/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/) {
		$minor = $1;
	} elsif(/SET\(CPACK_PACKAGE_VERSION_PATCH "(\d+)"\)/) {
		$patch = $1;
	} elsif(/SET\(RELEASE_NAME "(.+)"\)/) {
		$releasename = $1 unless defined $releasename;
	}
}
close F;

$version = "$major.$minor.$patch" unless defined $version;

unless( defined $binary ) {
	if( -f "binary-$version" ) {
		open P, "binary-$version";
		$binary = <P>;
		close P;
		$binary++;
	} else {
		$binary = 1;
	}
}

system "unzip packages/Untgz.zip" unless -d "untgz";

chdir "..";

$packagename = "QGIS" unless defined $packagename;
$shortname = "qgis" unless defined $shortname;

my $cmd = "makensis";
$cmd .= " -V$verbose";
$cmd .= " -DVERSION_NAME='$releasename'";
$cmd .= " -DVERSION_NUMBER='$version'";
$cmd .= " -DBINARY_REVISION=$binary";
$cmd .= sprintf( " -DVERSION_INT='%d%02d%02d%02d'", $major, $minor, $patch, $binary );
$cmd .= " -DQGIS_BASE='$packagename $releasename'";
$cmd .= " -DINSTALLER_NAME='QGIS-OSGeo4W-$version-$binary-Setup.exe'";
$cmd .= " -DDISPLAYED_NAME='$packagename \'$releasename\' ($version)'";
$cmd .= " -DSHORTNAME='$shortname'";
$cmd .= " -DINSTALLER_TYPE=OSGeo4W";
$cmd .= " -DPACKAGE_FOLDER=osgeo4w/unpacked";
$cmd .= " QGIS-Installer.nsi";

system $cmd;

open P, ">osgeo4w/binary-$version";
print P $binary;
close P;


__END__

=head1 NAME

creatensis.pl - create NSIS package from OSGeo4W packages

=head1 SYNOPSIS

creatensis.pl [options] [packages...]

  Options:
    -verbose		increase verbosity
    -releasename=name	name of release (defaults to CMakeLists.txt setting)
    -keep		don't start with a fresh unpacked directory
    -version=m.m.p	package version (defaults to CMakeLists.txt setting)
    -binary=b		binary version of package
    -ininame=filename	name of the setup.ini (defaults to setup.ini)
    -packagename=s	name of package (defaults to 'QGIS')
    -shortname=s	shortname used for batch file (defaults to 'qgis')
    -help		this help

  If no packages are given 'qgis-full' and it's dependencies will be retrieved
  and packaged.

=cut
