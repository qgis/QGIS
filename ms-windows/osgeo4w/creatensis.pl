#!/usr/bin/env perl
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

BEGIN {
	# ignore requireAdministrator execution level while producing the
	# uninstaller
	$ENV{"__COMPAT_LAYER"} = 'RUNASINVOKER';
}

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;

my $keep = 0;
my $verbose = 0;

my $packagename = "QGIS";
my $releasename;
my $shortname = "qgis";
my $version;
my $binary;
my $root = "http://download.osgeo.org/osgeo4w";
my $ininame = "setup.ini";
my $arch = "x86_64";
my $signwith;
my $signpass;
my $help;

my $result = GetOptions(
		"verbose+" => \$verbose,
		"keep" => \$keep,
		"signwith=s" => \$signwith,
		"signpass=s" => \$signpass,
		"releasename=s" => \$releasename,
		"version=s" => \$version,
		"binary=i" => \$binary,
		"packagename=s" => \$packagename,
		"shortname=s" => \$shortname,
		"ininame=s" => \$ininame,
		"mirror=s" => \$root,
		"arch=s" => \$arch,
		"help" => \$help
	);

die "certificate not found" if defined $signwith && ! -f $signwith;

pod2usage(1) if $help;

my $wgetopt = $verbose ? "" : "-nv";

unless(-f "nsis/System.dll") {
	mkdir "nsis", 0755 unless -d "nsis";
	system "wget $wgetopt -Onsis/System.dll http://qgis.org/downloads/System.dll";
	die "download of System.dll failed" if $?;
}

my $archpath    = $arch eq "" ? "" : "/$arch";
my $archpostfix = $arch eq "" ? "" : "-$arch";
my $unpacked    = "unpacked" . ($arch eq "" ? "" : "-$arch");
my $packages    = "packages" . ($arch eq "" ? "" : "-$arch");

mkdir $packages, 0755 unless -d $packages;
chdir $packages;

system "wget $wgetopt -c http://qgis.org/downloads/Untgz.zip" unless -f "Untgz.zip";
die "download of Untgz.zip failed" if $?;

system "wget $wgetopt -c https://qgis.org/downloads/Inetc.zip" unless -f "Inetc.zip";
die "download of Inetc.zip failed" if $?;

my %dep;
my %file;
my %lic;
my %sdesc;
my %md5;
my $package;

system "wget $wgetopt -O setup.ini $root$archpath/$ininame";
die "download of setup.ini failed" if $?;
open F, "setup.ini" || die "setup.ini not found";
while(<F>) {
	my $file;
	my $md5;

	chop;
	if(/^@ (\S+)/) {
		$package = $1;
	} elsif( /^requires: (.*)$/ ) {
		@{$dep{$package}} = split / /, $1;
	} elsif( ($file,$md5) = /^install:\s+(\S+)\s+.*\s+(\S+)$/) {
		$file{$package} = $file unless exists $file{$package};
		$file =~ s/^.*\///;
		$md5{$file} = $md5 unless exists $md5{$file};
	} elsif( ($file,$md5) = /^license:\s+(\S+)\s+.*\s+(\S+)$/) {
		$lic{$package} = $file unless exists $lic{$package};
		$file =~ s/^.*\///;
		$md5{$file} = $md5 unless exists $md5{$file};
	} elsif( /^sdesc:\s*"(.*)"\s*$/) {
		$sdesc{$package} = $1 unless exists $sdesc{$package};
	}
}
close F;

my %pkgs;

sub getDeps {
	my $pkg = shift;

	my $deponly = $pkg =~ /-$/;
	$pkg =~ s/-$//;

	unless($deponly) {
		return if exists $pkgs{$pkg};
		print " Including package $pkg\n" if $verbose;
		$pkgs{$pkg} = 1;
	} elsif( exists $pkgs{$pkg} ) {
		print " Excluding package $pkg\n" if $verbose;
		delete $pkgs{$pkg};
		return;
	} else {
		print " Including dependencies of package $pkg\n" if $verbose;
	}

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

my @lic;
my @desc;
foreach my $p ( keys %pkgs ) {
	my @f;
	unless( exists $file{$p} ) {
		print "No file for package $p found.\n" if $verbose;
		next;
	}
	push @f, "$root/$file{$p}";

	if( exists $lic{$p} ) {
		push @f, "$root/$lic{$p}";
		my($l) = $lic{$p} =~ /([^\/]+)$/;
		push @lic, $l;
		push @desc, $sdesc{$p};
	}

	for my $f (@f) {
		$f =~ s/\/\.\//\//g;

		my($file) = $f =~ /([^\/]+)$/;

		next if -f $file;

		print "Downloading $file [$f]...\n" if $verbose;
		system "wget $wgetopt -c $f";
		die "download of $f failed" if $? or ! -f $file;

		if( exists $md5{$file} ) {
			my $md5;
			open F, "md5sum '$file'|";
			while(<F>) {
				if( /^(\S+)\s+\*?(.*)$/ && $2 eq $file ) {
					$md5 = $1;
				}
			}
			close F;

			die "No md5sum of $p determined [$file]" unless defined $md5;
			if( $md5 eq $md5{$file} ) {
				print "md5sum of $file verified.\n" if $verbose;
			} else {
				die "md5sum mismatch for $file [$md5 vs $md5{$file{$p}}]"
			}
		}
		else
		{
			die "md5sum for $file not found.\n";
		}
	}
}

chdir "..";

#
# Unpack them
# Add nircmd
# Add addons
#

if( -d $unpacked ) {
	unless( $keep ) {
		print "Removing $unpacked directory\n" if $verbose;
		system "rm -rf $unpacked";
	} else {
		print "Keeping $unpacked directory\n" if $verbose;
	}
}

my $taropt = "v" x $verbose;

unless(-d $unpacked ) {
	mkdir "$unpacked", 0755;
	mkdir "$unpacked/bin", 0755;
	mkdir "$unpacked/etc", 0755;
	mkdir "$unpacked/etc/setup", 0755;

	# Create package database
	open O, ">$unpacked/etc/setup/installed.db";
	print O "INSTALLED.DB 2\n";

	foreach my $pn ( keys %pkgs ) {
		my $p = $file{$pn};
		unless( defined $p ) {
			print "No package found for $pn\n" if $verbose;
			next;
		}

		$p =~ s#^.*/#$packages/#;

		unless( -r $p ) {
			print "Package $p not found.\n" if $verbose;
			next;
		}

		print O "$pn $p 0\n";

		print "Unpacking $p...\n" if $verbose;
		system "bash -c 'tar $taropt -C $unpacked -xjvf $p | gzip -c >$unpacked/etc/setup/$pn.lst.gz && [ \${PIPESTATUS[0]} == 0 -a \${PIPESTATUS[1]} == 0 ]'";
		die "unpacking of $p failed" if $?;
	}

	close O;

	chdir $unpacked;

	mkdir "bin", 0755;

	unless( -f "bin/nircmd.exe" ) {
		unless( -f "../$packages/nircmd.zip" ) {
			system "cd ../$packages; wget $wgetopt -c http://www.nirsoft.net/utils/nircmd.zip";
			die "download of nircmd.zip failed" if $?;
		}

		mkdir "apps", 0755;
		mkdir "apps/nircmd", 0755;
		system "cd apps/nircmd; unzip ../../../$packages/nircmd.zip && mv nircmd.exe nircmdc.exe ../../bin";
		die "unpacking of nircmd failed" if $?;
	}

	if( -d "../addons" ) {
		print " Including addons...\n" if $verbose;
		system "tar -C ../addons -cf - . | tar $taropt -xf -";
		die "copying of addons failed" if $?;
	}

	chdir "..";
}

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

my($pmajor,$pminor,$ppatch) = $version =~ /^(\d+)\.(\d+)\.(\d+)$/;
die "Invalid version $version" unless defined $ppatch;

unless( defined $binary ) {
	if( -f "binary$archpostfix-$version" ) {
		open P, "binary$archpostfix-$version";
		$binary = <P>;
		close P;
		$binary++;
	} else {
		$binary = 1;
	}
}

#
# Create postinstall.bat
#

open F, ">../Installer-Files/postinstall.bat";

my $r = ">>postinstall.log 2>&1\r\n";

print F "\@echo off\r\n";
print F "if exist postinstall.log del postinstall.log\r\n";
print F "set OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT:\\=/%$r";
print F "if \"%OSGEO4W_ROOT_MSYS:~1,1%\"==\":\" set OSGEO4W_ROOT_MSYS=/%OSGEO4W_ROOT_MSYS:~0,1%/%OSGEO4W_ROOT_MSYS:~3%$r";

print F "del preremove-conf.bat$r";
my $c = ">>preremove-conf.bat\r\n";
print F "echo set OSGEO4W_ROOT=%OSGEO4W_ROOT%$c";
print F "echo set OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT_MSYS%$c";
print F "echo set OSGEO4W_STARTMENU=%OSGEO4W_STARTMENU%$c";
print F "echo set OSGEO4W_DESKTOP=%OSGEO4W_DESKTOP%$c";

print F "echo OSGEO4W_ROOT=%OSGEO4W_ROOT%$r";
print F "echo OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT_MSYS%$r";
print F "echo OSGEO4W_STARTMENU=%OSGEO4W_STARTMENU%$r";
print F "echo OSGEO4W_DESKTOP=%OSGEO4W_DESKTOP%$r";
print F "PATH %OSGEO4W_ROOT%\\bin;%PATH%$r";
print F "cd /d %OSGEO4W_ROOT%$r";

chdir $unpacked;
for my $p (<etc/postinstall/*.bat>) {
	$p =~ s/\//\\/g;
	my($dir,$file) = $p =~ /^(.+)\\([^\\]+)$/;

	print F "echo Running postinstall $file...$r";
	print F "%COMSPEC% /c $p$r";
	print F "ren $p $file.done$r";
}
chdir "..";

print F "ren postinstall.bat postinstall.bat.done$r";

close F;

open F, ">../Installer-Files/preremove.bat";

$r = ">>%TEMP%\\$packagename-OSGeo4W-$version-$binary-preremove.log 2>&1\r\n";

print F "\@echo off\r\n";
print F "call \"%~dp0\\preremove-conf.bat\"$r";
print F "echo OSGEO4W_ROOT=%OSGEO4W_ROOT%$r";
print F "echo OSGEO4W_STARTMENU=%OSGEO4W_STARTMENU%$r";
print F "echo OSGEO4W_DESKTOP=%OSGEO4W_DESKTOP%$r";
print F "set OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT:\\=/%$r";
print F "if \"%OSGEO4W_ROOT_MSYS:~1,1%\"==\":\" set OSGEO4W_ROOT_MSYS=/%OSGEO4W_ROOT_MSYS:~0,1%/%OSGEO4W_ROOT_MSYS:~3%$r";
print F "echo OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT_MSYS%$r";
print F "PATH %OSGEO4W_ROOT%\\bin;%PATH%$r";
print F "cd /d \"%OSGEO4W_ROOT%\"$r";

chdir $unpacked;
for my $p (<etc/preremove/*.bat>) {
	$p =~ s/\//\\/g;
	my($dir,$file) = $p =~ /^(.+)\\([^\\]+)$/;

	print F "echo Running preremove $file...$r";
	print F "%COMSPEC% /c $p$r";
	print F "ren $p $file.done$r";
}
chdir "..";

print F "ren preremove.bat preremove.bat.done$r";

close F;

unless(-d "untgz") {
	system "unzip $packages/Untgz.zip";
	die "unpacking Untgz.zip failed" if $?;
}

unless(-d "inetc") {
	mkdir "inetc", 0755;
	system "unzip -p $packages/Inetc.zip Plugins/x86-ansi/INetC.dll >inetc/INetC.dll";
	die "unpacking Inetc.zip failed" if $?;
}

chdir "..";


print "Creating license file\n" if $verbose;
open O, ">license.tmp";
my $lic;
for my $l ( ( "osgeo4w/$unpacked/apps/$shortname/doc/LICENSE", "../COPYING", "./Installer-Files/LICENSE.txt" ) ) {
	next unless -f $l;
	$lic = $l;
	last;
}

die "no license found" unless defined $lic;

my $i = 0;
if( @lic ) {
	print O "License overview:\n";
	print O "1. QGIS\n";
	$i = 1;
	for my $l ( @desc ) {
		print O ++$i . ". $l\n";
	}
	$i = 0;
	print O "\n\n----------\n\n" . ++$i . ". License of 'QGIS'\n\n";
}

print " Including QGIS license $lic\n" if $verbose;
open I, $lic;
while(<I>) {
	s/\s*$/\n/;
	print O;
}
close I;

for my $l (@lic) {
	print " Including license $l\n" if $verbose;

	open I, "osgeo4w/$packages/$l" or die "License $l not found.";
	print O "\n\n----------\n\n" . ++$i . ". License of '" . shift(@desc) . "'\n\n";
	while(<I>) {
		s/\s*$/\n/;
		print O;
	}
	close I;
}

close O;

my $license = "license.tmp";
if( -f "osgeo4w/$unpacked/apps/$shortname/doc/LICENSE" ) {
	open O, ">osgeo4w/$unpacked/apps/$shortname/doc/LICENSE";
	open I, $license;
	while(<I>) {
		print O;
	}
	close O;
	close I;

	$license = "osgeo4w/$unpacked/apps/$shortname/doc/LICENSE";
}


print "Running NSIS\n" if $verbose;

my $installerbase = "$packagename-OSGeo4W-$version-$binary-Setup$archpostfix";

my $run;
my $instdest;

if($^O eq "cygwin") {
	$run = "cygstart ";
	$instdest = `cygpath -w \$PWD`;
} else {
	$run = "wine ";
	$instdest = `winepath -w \$PWD`;
}

$instdest =~ s/\s+$//;
$instdest =~ s/\\/\\\\/g;


my $args = "";
$args .= " -V$verbose";
$args .= " -DVERSION_NAME='$releasename'";
$args .= " -DVERSION_NUMBER='$version'";
$args .= " -DBINARY_REVISION=$binary";
$args .= sprintf( " -DVERSION_INT='%d%02d%02d%02d'", $pmajor, $pminor, $ppatch, $binary );
$args .= sprintf( " -DQGIS_BASE='$packagename %d.%d'", $pmajor, $pminor );
$args .= " -DDISPLAYED_NAME=\"$packagename $version '$releasename'\"";
$args .= " -DPACKAGE_FOLDER=osgeo4w/$unpacked";
$args .= " -DLICENSE_FILE='$license'";
$args .= " -DARCH='$arch'";
$args .= " QGIS-Installer.nsi";

sub sign {
	my $base = shift;

	my $cmd = "osslsigncode sign";
	$cmd .= " -pkcs12 \"$signwith\"";
	$cmd .= " -pass \"$signpass\"" if defined $signpass;
	$cmd .= " -n \"$packagename $version '$releasename'\"";
	$cmd .= " -h sha256";
	$cmd .= " -i \"https://qgis.org\"";
	$cmd .= " -t \"http://timestamp.digicert.com\"";
	$cmd .= " -in \"$base.exe\"";
	$cmd .= " $base-signed.exe";
	system $cmd;
	die "signing failed [$cmd]" if $?;

	rename("$base-signed.exe", "$base.exe") or die "rename failed: $!";
}

my $cmd;
unlink "makeuinst.exe";
$cmd = "makensis -DINNER=1 -DUNINSTALLERDEST='$instdest' -DINSTALLER_NAME='makeuinst.exe' $args";
system $cmd;
die "running makensis failed [$cmd]" if $?;
die "makeuinst.exe not created" unless -f "makeuinst.exe";

unlink "uninstall.exe";
chmod 0755, "makeuinst.exe";
system "${run}makeuinst.exe";
sleep 5;
die "uninstall.exe not created" unless -f "uninstall.exe";
unlink "makeuinst.exe";

sign "uninstall" if $signwith;

$cmd = "makensis -DINSTALLER_NAME='$installerbase.exe' $args";
system $cmd;
die "running makensis failed [$cmd]" if $?;

sign "$installerbase" if $signwith;

open P, ">osgeo4w/binary$archpostfix-$version";
print P $binary;
close P;

system "md5sum $installerbase.exe >$installerbase.exe.md5sum";

__END__

=head1 NAME

creatensis.pl - create NSIS package from OSGeo4W packages

=head1 SYNOPSIS

creatensis.pl [options] [packages...]

  Options:
    -verbose		increase verbosity
    -releasename=name	name of release (defaults to CMakeLists.txt setting)
    -keep		don't start with a fresh unpacked directory
    -signwith=cert.p12	optionall sign package with certificate (requires osslsigncode)
    -signpass=password	password of certificate
    -version=m.m.p	package version (defaults to CMakeLists.txt setting)
    -binary=b		binary version of package
    -ininame=filename	name of the setup.ini (defaults to setup.ini)
    -packagename=s	name of package (defaults to 'QGIS')
    -shortname=s	shortname used for batch file (defaults to 'qgis')
    -mirror=s		default mirror (defaults to 'http://download.osgeo.org/osgeo4w')
    -arch=s		architecture (x86 or x86_64; defaults to 'x86_64')
    -help		this help

  If no packages are given 'qgis-full' and it's dependencies will be retrieved
  and packaged.

  Packages with a appended '-' are excluded, but their dependencies are included.
=cut
