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

mkdir "packages", 0755 unless -d "packages";
chdir "packages";

my $root = "http://download.osgeo.org/osgeo4w";

system "wget -q -c http://nsis.sourceforge.net/mediawiki/images/9/9d/Untgz.zip" unless -f "Untgz.zip";
system "wget -q -c http://www.nirsoft.net/utils/nircmd.zip" unless -f "nircmd.zip";

my %dep;
my %file;

system "wget -q -c $root/setup.ini";
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

	$pkgs{$pkg} = 1;

	foreach my $p ( @{ $dep{$pkg} } ) {
		getDeps($p);
	}
}

getDeps("qgis-dev");

if(-f "../addons/bin/NCSEcw.dll") {
	print "Enabling ECW support...\n";
	getDeps("gdal16-ecw")
}

if(-f "../addons/bin/lti_dsdk_dll.dll") {
	print "Enabling MrSID support...\n";
	getDeps("gdal16-mrsid")
}

delete $pkgs{"qgis-dev"};


foreach my $p ( keys %pkgs ) {
	$f = "$root/$file{$p}";
	$f =~ s/\/\.\//\//g;

	my($file) = $f =~ /([^\/]+)$/;

	next if -f $file;
	
	print "Downloading $file [$f]...\n";
	system "wget -q -c $f";
}

chdir "..";

#
# Unpack them
# Add nircmd
# Add addons
#


system "rm -rf unpacked" if -d "unpacked" && !grep(/^-k$/, @ARGV);

unless(-d "unpacked") {
	mkdir "unpacked", 0755;

	for my $p (<packages/*.tar.bz2>) {
		print "Unpacking $p...\n";
		system "tar -C unpacked -xjf $p";
	}

	chdir "unpacked";

	mkdir "bin", 0755;
	mkdir "apps", 0755;
	mkdir "apps/nircmd", 0755;

	system "cd apps/nircmd; unzip ../../../packages/nircmd.zip && mv nircmd.exe ../../bin";

	system "tar -C ../addons -cf . | tar -xf -" if -d "../addons";

	chdir "..";
}

#
# Create postinstall.bat
#

unless(-f "../Installer-Files/postinstall.bat") {
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
}

unless(-f "../Installer-Files/preremove.bat") {
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
}

my($major, $minor, $patch, $release, $revision);

open F, "../../CMakeLists.txt";
while(<F>) {
	print;
	if(/SET\(CPACK_PACKAGE_VERSION_MAJOR "(\d+)"\)/) {
		$major = $1;
	} elsif(/SET\(CPACK_PACKAGE_VERSION_MINOR "(\d+)"\)/) {
		$minor = $1;
	} elsif(/SET\(CPACK_PACKAGE_VERSION_PATCH "(\d+)"\)/) {
		$patch = $1;
	} elsif(/SET\(RELEASE_NAME "(.+)"\)/) {
		$release = $1;
	}
}
close F;

open F, "svnversion|";
$revision = <F>;
$revision =~ s/\D+$//g;
close F;

chdir "..";

my $cmd = "makensis";
$cmd .= " -DVERSION_NUMBER='$major.$minor.$patch'";
$cmd .= " -DVERSION_NAME='$release'";
$cmd .= " -DSVN_REVISION='$revision'";
$cmd .= " -DQGIS_BASE='Quantum GIS $release'";
$cmd .= " -DINSTALLER_NAME='QGIS-OSGeo4W-$major.$minor.$patch-$revision-Setup.exe'";
$cmd .= " -DDISPLAYED_NAME='Quantum GIS OSGeo4W ($release)'";
$cmd .= " -DBINARY_REVISION=1";
$cmd .= " -DINSTALLER_TYPE=OSGeo4W";
$cmd .= " -DPACKAGE_FOLDER=osgeo4w/unpacked";
$cmd .= " QGIS-Installer.nsi";

system $cmd;
