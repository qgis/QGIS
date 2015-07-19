#!/usr/bin/perl
# bootstrap osgeo4w environment for appveyor
# derived from creatensis.pl

# Copyright (C) 2015 Jürgen E. Fischer <jef@norbit.de>

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

my $verbose = 0;

my $root = "http://download.osgeo.org/osgeo4w";
my $ininame = "setup.ini";
my $arch;
my $help;

my $result = GetOptions(
		"verbose+" => \$verbose,
		"ininame=s" => \$ininame,
		"mirror=s" => \$root,
		"arch=s" => \$arch,
		"help" => \$help
	);

pod2usage(1) if $help || $arch !~ /^x86(_64)?$/;

my $wgetopt = $verbose ? "-nv" : "-q";

my $unpacked = $arch eq "x86_64" ? "c:/osgeo4w64" : "c:/osgeo4w";
my $packages = "/cygdrive/c/temp/$arch";

if(-d $unpacked) {
	print "$unpacked already exists.";
	exit 0;
}

mkdir "/cygdrive/c/temp", 0755 unless -d "/cygdrive/c/temp";
mkdir $packages, 0755 unless -d $packages;
chdir $packages;

my %dep;
my %file;
my %md5;
my $package;

system "wget $wgetopt -O setup.ini $root/$arch/$ininame";
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
	}
}
close F;

my %pkgs;

sub getDeps {
	my ($pkg) = @_;

	return if exists $pkgs{$pkg};
	return if $pkg =~ /^msvcrt..../;

	print " Including package $pkg\n" if $verbose;
	$pkgs{$pkg} = 1;

	foreach my $p ( @{ $dep{$pkg} } ) {
		getDeps($p);
	}
}

my @pkg = qw/fcgi gdal geos grass grass6 gsl-devel libspatialindex-devel oci openssl osg-dev osgearth-dev proj pyqt4 pyspatialite python python-qscintilla qscintilla qt4-devel sip spatialite sqlite3 zlib/;
push @pkg, "qwt-devel-qt4" if $arch eq "x86";
push @pkg, qw/qwt5-devel-qt4 oci-devel python-devel/ if $arch eq "x86_64";
getDeps($_) for @pkg;

foreach my $p ( keys %pkgs ) {
	my @f;
	unless( exists $file{$p} ) {
		print "No file for package $p found.\n" if $verbose;
		next;
	}
	push @f, "$root/$file{$p}";

	for my $f (@f) {
		$f =~ s/\/\.\//\//g;

		my($file) = $f =~ /([^\/]+)$/;

		next if -f $file;

		print "Downloading $file [$f]...\n" if $verbose;
		system "wget $wgetopt -c $f";
		die "download of $f failed" if $? or ! -f $file;

		if( exists $md5{$file} ) {
			my $md5;
			open F, "md5sum $file|";
			while(<F>) {
				if( /^(\S+)\s+\*?$file$/ ) {
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
#

my $taropt = "v" x $verbose;

mkdir "$unpacked", 0755;
mkdir "$unpacked/bin", 0755;
mkdir "$unpacked/etc", 0755;
mkdir "$unpacked/etc/setup", 0755;

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

	print "Unpacking $p...\n" if $verbose;
	system "bash -c 'tar $taropt -C $unpacked -xjvf $p | gzip -c >$unpacked/etc/setup/$pn.lst.gz && [ \${PIPESTATUS[0]} == 0 -a \${PIPESTATUS[1]} == 0 ]'";
	die "unpacking of $p failed" if $?;
}

#
# Create and run postinstall.bat
#

my $o4w = $unpacked;
$o4w =~ s#/#\\#g;

chdir $unpacked;
open F, ">postinstall.bat";

print F "\@echo off\r\n";
print F "set OSGEO4W_ROOT=$o4w\r\n";
print F "set OSGEO4W_ROOT_MSYS=$unpacked\r\n";
print F "set OSGEO4W_STARTMENU=$o4w\\Shortcuts\r\n";
print F "if \"%OSGEO4W_ROOT_MSYS:~1,1%\"==\":\" set OSGEO4W_ROOT_MSYS=/%OSGEO4W_ROOT_MSYS:~0,1%/%OSGEO4W_ROOT_MSYS:~3%\r\n";
print F "PATH %OSGEO4W_ROOT%\\bin;%PATH%\r\n";
print F "cd %OSGEO4W_ROOT%\r\n";

for my $p (<etc/postinstall/*.bat>) {
	next if $p =~ /msvcrt/;

	$p =~ s/\//\\/g;
	my($dir,$file) = $p =~ /^(.+)\\([^\\]+)$/;

	print F "echo Running postinstall $file...\r\n";
	print F "%COMSPEC% /c $p\r\n";
	print F "ren $p $file.done\r\n";
}

close F;

print " Running postinstall...\n" if $verbose;
my $cmd = "cmd /c postinstall.bat";
$cmd .= ">postinstall.log 2>&1" unless $verbose;
system $cmd;
print " postinstall.bat finished.\n" if $verbose;

system "tar -C $unpacked -xjf /cygdrive/c/src/qgis/ms-windows/osgeo4w/4127.tar.bz2";

print "PACKAGES $arch:";
system "du -sch $packages";

=head1 NAME

osgeo4w-appveyor.pl - create osgeo4w environment for appveyor

=head1 SYNOPSIS

osgeo4w-appveyor.pl [options] [packages...]

  Options:
    -verbose		increase verbosity
    -ininame=filename	name of the setup.ini (defaults to setup.ini)
    -mirror=s		default mirror (defaults to 'http://download.osgeo.org/osgeo4w')
    -arch=s		architecture (x86 or x86_64)
    -help		this help

=cut
