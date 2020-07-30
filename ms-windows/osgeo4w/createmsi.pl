#!/usr/bin/env perl
# creates a MSI installer from OSGeo4W packages
# note: works also on Linux using wine and wine mono

# Copyright (C) 2020 Jürgen E. Fischer <jef@norbit.de>

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
use Data::UUID;
use RTF::Writer;
use File::Copy;

my $ug = Data::UUID->new;

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
my $manufacturer = "QGIS.org";

my $result = GetOptions(
		"verbose+" => \$verbose,
		"keep" => \$keep,
		"signwith=s" => \$signwith,
		"signpass=s" => \$signpass,
		"releasename=s" => \$releasename,
		"version=s" => \$version,
		"binary=i" => \$binary,
		"packagename=s" => \$packagename,
		"manufacturer=s" => \$manufacturer,
		"shortname=s" => \$shortname,
		"ininame=s" => \$ininame,
		"mirror=s" => \$root,
		"arch=s" => \$arch,
		"help" => \$help
	);

die "certificate not found" if defined $signwith && ! -f $signwith;

pod2usage(1) if $help;

my $wgetopt = $verbose ? "" : "-nv";

my $archpath    = $arch eq "" ? "" : "/$arch";
my $archpostfix = $arch eq "" ? "" : "-$arch";
my $unpacked    = "unpacked" . ($arch eq "" ? "" : "-$arch");
my $packages    = "packages" . ($arch eq "" ? "" : "-$arch");

mkdir $packages, 0755 unless -d $packages;
chdir $packages;

unless(-d "wix") {
	system "wget $wgetopt -c https://github.com/wixtoolset/wix3/releases/download/wix3111rtm/wix311-binaries.zip";
	die "download of wix failed" if $?;

	mkdir "wix", 0755;
	chdir "wix";
	system "unzip ../wix311-binaries.zip";
	die "unzip of wix failed" if $?;
	chdir "..";
}

if($^O ne "cygwin") {
	unless(-f "wine-mono-5.1.0-x86.msi") {
		system "wget $wgetopt -c https://dl.winehq.org/wine/wine-mono/5.1.0/wine-mono-5.1.0-x86.msi";
		die "download of wine-mono failed" if $?;
		system "wine msiexec /i wine-mono-5.1.0-x86.msi";
		die "install of wine-mono failed" if $?;
	}
}

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

sub getuuid {
	my($file) = shift;
	my $uuid;

	if(-f $file) {
		open F, "$file" or die "cannot open $file: $!";
		$uuid = <F>;
		close F;
	} else {
		$uuid = $ug->to_string($ug->create());
		open F, ">$file" or die "cannot open $file: $!";
		print F $uuid;
		close F;
	}
	return $uuid;
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
		die "removal of $unpacked failed" if $?;
	} else {
		print "Keeping $unpacked directory\n" if $verbose;
	}
}

if( -f "$packages/files.wxs") {
	unless( $keep ) {
		print "Removing files.wxs\n" if $verbose;
		system "rm -f $packages/files.wxs";
		die "removal of $packages/files failed" if $?;
	} else {
		print "Keeping files.wxs\n" if $verbose;
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

open F, ">$packages/postinstall.bat";

# my $r = ">>\"%OSGEO4W_ROOT%\\postinstall.log\" 2>&1\r\n";
my $l = "\"%TEMP%\\$packagename-OSGeo4W-$version-$binary-postinstall.log\"";
my $r = ">>$l 2>&1\r\n";

#print F "\@echo off\r\n";
print F "set OSGEO4W_ROOT=%~dp0\r\n";
print F "if %OSGEO4W_ROOT:~-1%==\\ set OSGEO4W_ROOT=%OSGEO4W_ROOT:~0,-1%\r\n";
print F "set OSGEO4W_STARTMENU=%~1\r\n";
print F "if %OSGEO4W_STARTMENU~-1%==\\ set OSGEO4W_STARTMENU=%OSGEO4W_STARTMENU~0,-1%\r\n";
print F "set OSGEO4W_DESKTOP=%~2\r\n";
print F "if %OSGEO4W_DESKTOP~-1%==\\ set OSGEO4W_DESKTOP=%OSGEO4W_DESKTOP~0,-1%\r\n";
print F "if exist $l del $l\r\n";
print F "set OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT:\\=/%\r\n";
print F "if \"%OSGEO4W_ROOT_MSYS:~1,1%\"==\":\" set OSGEO4W_ROOT_MSYS=/%OSGEO4W_ROOT_MSYS:~0,1%/%OSGEO4W_ROOT_MSYS:~3%$r";
print F "set OSGEO4W_MENU_LINKS=1\r\n";
print F "set OSGEO4W_DESKTOP_LINKS=1\r\n";

my $b = "\"%OSGEO4W_ROOT%\\preremove-conf.bat\"";
print F "if exist $b del $b$r";

my $c = ">>$b\r\n";
print F "(\r\n";
print F "echo set OSGEO4W_ROOT=%OSGEO4W_ROOT%\r\n";
print F "echo set OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT_MSYS%\r\n";
print F "echo set OSGEO4W_STARTMENU=%OSGEO4W_STARTMENU%\r\n";
print F "echo set OSGEO4W_DESKTOP=%OSGEO4W_DESKTOP%\r\n";
print F "echo set OSGEO4W_MENU_LINKS=%OSGEO4W_MENU_LINKS%\r\n";
print F "echo set OSGEO4W_DESKTOP_LINKS=%OSGEO4W_DESKTOP_LINKS%\r\n";
print F ")$c";

print F "(\r\n";
print F "echo OSGEO4W_ROOT=%OSGEO4W_ROOT%\r\n";
print F "echo OSGEO4W_ROOT_MSYS=%OSGEO4W_ROOT_MSYS%\r\n";
print F "echo OSGEO4W_STARTMENU=%OSGEO4W_STARTMENU%\r\n";
print F "echo OSGEO4W_DESKTOP=%OSGEO4W_DESKTOP%\r\n";
print F "echo OSGEO4W_MENU_LINKS=%OSGEO4W_MENU_LINKS%\r\n";
print F "echo OSGEO4W_DESKTOP_LINKS=%OSGEO4W_DESKTOP_LINKS%\r\n";

print F "PATH %OSGEO4W_ROOT%\\bin;%PATH%\r\n";
print F "cd /d %OSGEO4W_ROOT%\r\n";
print F ")$r";

chdir $unpacked;
for my $p (<etc/postinstall/*.bat>) {
	$p =~ s/\//\\/g;
	my($dir,$file) = $p =~ /^(.+)\\([^\\]+)$/;

	print F "echo Running postinstall $file...$r";
	print F "%COMSPEC% /c \"%OSGEO4W_ROOT%\\$p\"$r";
	print F "ren \"%OSGEO4W_ROOT%\\$p\" $file.done$r";
}
chdir "..";

print F "ren postinstall.bat postinstall.bat.done$r";
print F "exit /b 0\r\n";

close F;

open F, ">$packages/preremove.bat";

$r = ">>\"%TEMP%\\$packagename-OSGeo4W-$version-$binary-preremove.log\" 2>&1\r\n";

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

print F "del preremove.bat$r";
print F "del postinstall.bat.done$r";
print F "del preremove-conf.bat$r";

close F;

print "Creating license file\n" if $verbose;

my $lic;
for my $l ( ( "$unpacked/apps/$shortname/doc/LICENSE", "../COPYING", "./Installer-Files/LICENSE.txt" ) ) {
	next unless -f $l;
	$lic = $l;
	last;
}

warn "no QGIS license found" unless defined $lic;

my $rtf = RTF::Writer->new_to_file("$packages/license.temp");
open O, ">$packages/license.txt";

sub out {
	my $m = shift;
	$rtf->print($m);
	print O $m;
}

out("Lizenz\n");

my $i = 0;
if( @lic ) {
	out("License overview:\n");
	out("1. QGIS\n") if defined $lic;
	$i = defined $lic ? 1 : 0;
	for my $l ( @desc ) {
		out(++$i . ". $l\n");
	}
	$i = 0;
	out("\n\n----------\n\n");
	out(++$i . ". License of 'QGIS'\n\n") if defined $lic;
}

if(defined $lic) {
	print " Including QGIS license $lic\n" if $verbose;
	open I, $lic;
	while(<I>) {
		s/\s*$/\n/;
		out($_);
	}
	close I;
}

for my $l (@lic) {
	print " Including license $l\n" if $verbose;

	open I, "$packages/$l" or die "License $l not found.";
	out("\n\n----------\n\n" . ++$i . ". License of '" . shift(@desc) . "'\n\n");
	while(<I>) {
		s/\s*$/\n/;
		out($_);
	}
	close I;
}

$rtf->close();
undef $rtf;
system "cp $packages/license.temp $packages/license.rtf";
close O;

my $license = "$packages/license.txt";
if( -f "$unpacked/apps/$shortname/doc/LICENSE" ) {
	open O, ">$unpacked/apps/$shortname/doc/LICENSE";
	open I, "$packages/license.txt";
	while(<I>) {
		print O;
	}
	close O;
	close I;

	$license = "$unpacked/apps/$shortname/doc/LICENSE";
}

my $installer = "$packagename-OSGeo4W-$version-$binary$archpostfix";

my $run = $^O eq "cygwin" ? "" : "wine";

my $productuuid        = getuuid(".$shortname.$version.product");
my $upgradeuuid        = getuuid(".$shortname.$version.upgrade");
my $postinstalluuid    = getuuid(".$shortname.$version.postinstall");
my $preremoveuuid      = getuuid(".$shortname.$version.preremove");
my $linkfolders        = getuuid(".$shortname.$version.linkfolders");
my $programfilesfolder = $arch eq "x86_64" ? "ProgramFiles64Folder" : "ProgramFiles";
my $WixQuietExec       = $arch eq "x86_64" ? "WixQuietExec64" : "WixQuietExec";

my $fn = 0;
unless($keep && -f "$packages/files1.wxs") {
	print "Harvesting files...\n" if $verbose;

	# Harvest ourselves - candle/light doesn't cope well with huge wxses (light doesn't handle symlinks either, so also resolve those)
	# system "$run $packages/wix/heat.exe dir $unpacked -nologo -var env.UNPACKEDDIR -sw HEAT5150 -cg INSTALLDIR -dr INSTALLDIR -gg -sfrag -srd -template fragment -out $packages\\\\files.wxs";
	# die "harvesting failed" if $?;

	my $f;
	my $fi = 0;
	my $indent = 3;
	my @lelements;   # current <Directory> path
	my @components;  # collect components for <ComponentGroup>

	sub wclose {
		my $f = shift;

		while(@lelements) {
			my $item = pop @lelements;
			printf $f "%*s</Directory> <!-- %s -->\n", --$indent, " ", $item;
		}

		print $f <<EOF;
  </DirectoryRef>
 </Fragment>
 <Fragment>
   <ComponentGroup Id="INSTALLDIR$fn">
EOF

		while(@components) {
			my $c = shift @components;
			print $f <<EOF
   <ComponentRef Id="$c" />
EOF
		}

		print $f <<EOF;
  </ComponentGroup> <!-- INSTALLDIR$fn -->
 </Fragment>
</Wix>
EOF

		close $f if defined $f;

	}

	chdir $unpacked;

	open F, "find . -print|";
	while(<F>) {
		#	print;
		if($fi++ % 5000 == 0) {
			wclose($f) if defined $f;
			open $f, ">../$packages/files" . ++$fn . ".wxs";

			print $f <<EOF;
<?xml version="1.0" encoding="utf-8"?>
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
 <Fragment>
  <DirectoryRef Id="INSTALLDIR">
EOF
		}

		s/^\.\///;
		chomp;

		my @elements = split /\//;

		pop @elements unless -d;

		my $i;
		for($i = 0; $i < @elements && $i < @lelements && $elements[$i] eq $lelements[$i]; $i++ ) {
		}

		while(@lelements > $i) {
			my $item = pop @lelements;
			printf $f "%*s</Directory> <!-- %s -->\n", --$indent, " ", $item;
		}

		while(@elements > @lelements) {
			my $item = $elements[$i++];
			my $did = "dir" .  $ug->to_string($ug->create());
			$did =~ s/-//g;

			printf $f "%*s<Directory Id=\"%s\" Name=\"%s\">\n",
				$indent++, " ", $did, $item;
			push @lelements, $item;
		}

		next if -d $_;

		if(-l $_) {
                	my $d = readlink($_);
			my $p = $_;
			$p =~ s#/[^/]+$#/$d#;
                	unlink $_ or die "Cannot unlink $_: $!";
                	copy($p,$_) or die "Copy $d to $_ failed: $!";
			print " Replacing symlink $_\n" if $verbose;
        	}

		s/\//\\/g;

		my $c = "cmp" . $ug->to_string($ug->create());
		$c =~ s/-//g;
		push @components, $c;

		my $fid = "fil" . $ug->to_string($ug->create());
		$fid =~ s/-//g;

		printf $f "%*s<Component Id=\"%s\" Guid=\"{%s}\">\n%*s<File Id=\"%s\" KeyPath=\"yes\" Source=\"\$(env.UNPACKEDDIR)\\%s\" />\n%*s</Component>\n",
			$indent, " ", $c, $ug->to_string($ug->create()),
			$indent+1, " ", $fid, $_,
			$indent, " ";
	}

	wclose($f) if defined $f;

	chdir "..";
} else {
	$fn = 1;
	while(-f "$packages/files$fn.wxs") {
		$fn++;
	}
	$fn--;
}

open F, ">$packages/installer.wxs";
print F <<EOF;
<?xml version="1.0" encoding="windows-1252"?>
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
  <Product Name="$packagename $version"
     Manufacturer="$manufacturer"
     Id='$productuuid'
     UpgradeCode="$upgradeuuid"
     Language="1033" Codepage="1252" Version="$version">

    <Package Id="*" Keywords="Installer" Description="$packagename $version Installer"
      Comments="QGIS is a registered trademark of QGIS.org"
      Manufacturer="$manufacturer"
      InstallerVersion="200"
      Languages="1033"
      Compressed="yes"
      SummaryCodepage="1252" />

    <Media Id="1" EmbedCab="yes" Cabinet="application.cab" />
    <Property Id="DiskPrompt" Value="$packagename $version Installation [1]" />
    <Property Id="WIXUI_INSTALLDIR" Value="INSTALLDIR" />

    <UIRef Id="WixUI_InstallDir" />
    <UIRef Id="WixUI_ErrorProgressText" />

    <WixVariable Id="WixUILicenseRtf" Value="license.rtf"/>
    <WixVariable Id="WixUIBannerBmp" Value="..\\..\\Installer-Files\\WelcomeFinishPage.bmp" />
    <WixVariable Id="WixUIDialogBmp" Value="..\\..\\Installer-Files\\WelcomeFinishPage.bmp" />

    <Directory Id="TARGETDIR" Name="SourceDir">
      <Directory Id="$programfilesfolder">
        <Directory Id="INSTALLDIR" Name="$packagename $version">
          <Component Id="postinstall.bat" Guid="$postinstalluuid">
            <File Id="postinstall.bat" Name="postinstall.bat" Source="postinstall.bat" />
          </Component>
          <Component Id="preremove.bat" Guid="$preremoveuuid">
            <File Id="preremove.bat" Name="preremove.bat" Source="preremove.bat" />
          </Component>
        </Directory>
	<Directory Id="ProgramMenuFolder">
            <Directory Id="ApplicationProgramsFolder" Name="$packagename $version"/>
        </Directory>
	<Directory Id="DesktopFolder">
            <Directory Id="ApplicationDesktopFolder" Name="$packagename $version"/>
	</Directory>
      </Directory>
    </Directory>

    <Feature Id="$packagename" Title="$packagename" Level="1">
      <ComponentRef Id="postinstall.bat" />
EOF

for(my $i=1; $i <= $fn; $i++) {
	print F "      <ComponentGroupRef Id=\"INSTALLDIR$i\" />\n";
}

print F <<EOF;
      <ComponentRef Id="preremove.bat" />
    </Feature>

    <SetProperty Id="postinstall" Value="&quot;[INSTALLDIR]\\postinstall.bat&quot; &quot;[ApplicationProgramMenuFolder]&quot; &quot;[ApplicationDesktopFolder]&quot;" Before="postinstall" Sequence='execute' />
    <CustomAction Id="postinstall" BinaryKey="WixCA" DllEntry="$WixQuietExec" Execute="deferred" Return="check" Impersonate="no" />
    
    <SetProperty Id="preremove" Value="&quot;[INSTALLDIR]\\preremove.bat&quot;" Before="preremove" Sequence='execute' />
    <CustomAction Id="preremove" BinaryKey="WixCA" DllEntry="$WixQuietExec" Execute="deferred" Return="check" Impersonate="no" />

    <InstallExecuteSequence>
      <Custom Action="postinstall" After="InstallFiles">(NOT Installed) AND (NOT REMOVE)</Custom>
      <Custom Action="preremove" After="InstallInitialize">(NOT UPGRADINGPRODUCTCODE) AND (REMOVE="ALL")</Custom>
    </InstallExecuteSequence>
  </Product>
</Wix>
EOF
close F;

chdir $packages;

$ENV{'UNPACKEDDIR'} = "..\\$unpacked";

my $msiarch = "-arch " . ($arch eq "x86" ? "x86" : "x64");
	
print "Running candle...\n" if $verbose;
system "$run wix/candle.exe -nologo $msiarch installer.wxs";
die "candle failed" if $?;

my @files;
for(my $i=1; $i<=$fn; $i++) {
	system "$run wix/candle.exe -nologo $msiarch files$i.wxs";
	die "candle failed" if $?;
	push @files, "files$i.wixobj"
}

print "Running light...\n" if $verbose;
# ICE09, ICE32, ICE61 produce:
# produce "light.exe : error LGHT0217 : Error executing ICE action 'ICExx'. The most common cause of this kind of ICE failure is an incorrectly registered scripting engine. See http://wixtoolset.org/documentation/error217/ for
# details and how to solve this problem. The following string format was not expected by the external UI message logger: "The installer has encountered an unexpected error installing this package. This may indicate a
# problem with this package. The error code is 2738. ".
#
# ICE61 produces following warning for font files:
# warning LGHT1076 : ICE60: The file filXXX is not a Font, and its version is not a companion file reference. It should have a language specified in the Language column.
#
# ICE64: complains about the desktop and start menu folder
my $cmd = "$run wix/light.exe -nologo -ext WixUIExtension -ext WixUtilExtension -out $installer.msi -sice:ICE09 -sice:ICE32 -sice:ICE60 -sice:ICE61 -sice:ICE64 -b ../$unpacked installer.wixobj " . join(" ", @files);
print "EXEC: $cmd\n" if $verbose;
system $cmd;
die "light failed" if $?;

sub sign {
	my $base = shift;

	my $cmd = "osslsigncode sign";
	$cmd .= " -pkcs12 \"$signwith\"";
	$cmd .= " -pass \"$signpass\"" if defined $signpass;
	$cmd .= " -n \"$packagename $version '$releasename'\"";
	$cmd .= " -h sha256";
	$cmd .= " -i \"https://qgis.org\"";
	$cmd .= " -t \"http://timestamp.digicert.com\"";
	$cmd .= " -in \"$base.msi\"";
	$cmd .= " \"$base-signed.msi\"";
	system $cmd;
	die "signing failed [$cmd]" if $?;

	rename("$base-signed.msi", "$base.msi") or die "rename failed: $!";
}

sign "$installer" if $signwith;

open P, ">../binary$archpostfix-$version";
print P $binary;
close P;

system "sha256sum $installer.msi >$installer.sha256sum";

__END__

=head1 NAME

createmsi.pl - create MSI package from OSGeo4W packages

=head1 SYNOPSIS

creatensis.pl [options] [packages...]

  Options:
    -verbose		increase verbosity
    -releasename=name	name of release (defaults to CMakeLists.txt setting)
    -keep		don't start with a fresh unpacked directory
    -signwith=cert.p12	optionally sign package with certificate (requires osslsigncode)
    -signpass=password	password of certificate
    -version=m.m.p	package version (defaults to CMakeLists.txt setting)
    -binary=b		binary version of package
    -ininame=filename	name of the setup.ini (defaults to setup.ini)
    -packagename=s	name of package (defaults to 'QGIS')
    -manufacturer=s     name of manufacturer (defaults to 'QGIS.org')
    -shortname=s	shortname used for batch file (defaults to 'qgis')
    -mirror=s		default mirror (defaults to 'http://download.osgeo.org/osgeo4w')
    -arch=s		architecture (x86 or x86_64; defaults to 'x86_64')
    -help		this help

  If no packages are given 'qgis-full' and it's dependencies will be retrieved
  and packaged.

  Packages with a appended '-' are excluded, but their dependencies are included.
=cut
