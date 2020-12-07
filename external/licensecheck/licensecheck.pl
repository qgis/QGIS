#!/usr/bin/perl
# -*- tab-width: 8; indent-tabs-mode: t; cperl-indent-level: 4 -*-
# This script was originally based on the script of the same name from
# the KDE SDK (by dfaure@kde.org)
#
# This version is
#   Copyright (C) 2007, 2008 Adam D. Barratt
#   Copyright (C) 2012 Francesco Poli
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <https://www.gnu.org/licenses/>.

# Originally copied from Debian's devscripts. A more modern version of
# this can be found at
# https://anonscm.debian.org/git/pkg-perl/packages/licensecheck.git/

=head1 NAME

licensecheck - simple license checker for source files

=head1 SYNOPSIS

B<licensecheck> B<--help>|B<--version>

B<licensecheck> [B<--no-conf>] [B<--verbose>] [B<--copyright>]
[B<-l>|B<--lines=>I<N>] [B<-i>|B<--ignore=>I<regex>] [B<-c>|B<--check=>I<regex>]
[B<-m>|B<--machine>] [B<-r>|B<--recursive>]  [B<-e>|B<--encoding=>I<...>]
I<list of files and directories to check>

=head1 DESCRIPTION

B<licensecheck> attempts to determine the license that applies to each file
passed to it, by searching the start of the file for text belonging to
various licenses.

If any of the arguments passed are directories, B<licensecheck> will add
the files contained within to the list of files to process.

=head1 OPTIONS

=over 4

=item B<--verbose>, B<--no-verbose>

Specify whether to output the text being processed from each file before
the corresponding license information.

Default is to be quiet.

=item B<-l=>I<N>, B<--lines=>I<N>

Specify the number of lines of each file's header which should be parsed
for license information. (Default is 60).

=item B<--tail=>I<N>

By default, the last 5k bytes of each files are parsed to get license
information. You may use this option to set the size of this parsed chunk.
You may set this value to 0 to avoid parsing the end of the file.

=item B<-i=>I<regex>, B<--ignore=>I<regex>

When processing the list of files and directories, the regular
expression specified by this option will be used to indicate those which
should not be considered (e.g. backup files, VCS metadata).

=item B<-r>, B<--recursive>

Specify that the contents of directories should be added
recursively.

=item B<-c=>I<regex>, B<--check=>I<regex>

Specify a pattern against which filenames will be matched in order to
decide which files to check the license of.

The default includes common source files.

=item B<-s>, B<--skipped>

Specify whether to show skipped files, i.e. files found which do not
match the check regexp (see C<--check> option). Default is to not show
skipped files.

Note that ignored files (like C<.git> or C<.svn>) are not shown even when
this option is used.

=item B<--copyright>

Also display copyright text found within the file

=item B<-e> B<--encoding>

Specifies input encoding of source files. By default, input files are
not decoded. When encoding is specified, license and copyright
information are printed on STDOUT as utf8, or garbage if you got the
encoding wrong.

=item B<-m>, B<--machine>

Display the information in a machine readable way, i.e. in the form
<file><tab><license>[<tab><copyright>] so that it can be easily sorted
and/or filtered, e.g. with the B<awk> and B<sort> commands.
Note that using the B<--verbose> option will kill the readability.

=item B<--no-conf>, B<--noconf>

Do not read any configuration files. This can only be used as the first
option given on the command line.

=back

=head1 CONFIGURATION VARIABLES

The two configuration files F</etc/devscripts.conf> and
F<~/.devscripts> are sourced by a shell in that order to set
configuration variables.  Command line options can be used to override
configuration file settings.  Environment variable settings are
ignored for this purpose.  The currently recognised variables are:

=over 4

=item B<LICENSECHECK_VERBOSE>

If this is set to I<yes>, then it is the same as the B<--verbose> command
line parameter being used. The default is I<no>.

=item B<LICENSECHECK_PARSELINES>

If this is set to a positive number then the specified number of lines
at the start of each file will be read whilst attempting to determine
the license(s) in use.  This is equivalent to the B<--lines> command line
option.

=back

=head1 LICENSE

This code is copyright by Adam D. Barratt <I<adam@adam-barratt.org.uk>>,
all rights reserved; based on a script of the same name from the KDE
SDK, which is copyright by <I<dfaure@kde.org>>.
This program comes with ABSOLUTELY NO WARRANTY.
You are free to redistribute this code under the terms of the GNU
General Public License, version 2 or later.

=head1 AUTHOR

Adam D. Barratt <adam@adam-barratt.org.uk>

=cut

# see http://stackoverflow.com/questions/6162484/why-does-modern-perl-avoid-utf-8-by-default/6163129#6163129
use v5.14;
use utf8;

use strict;
use autodie;
use warnings;
use warnings    qw< FATAL  utf8     >;

use Getopt::Long qw(:config gnu_getopt);
use File::Basename;
use File::stat;
use IO::File;
use Fcntl qw/:seek/;

binmode STDOUT, ':utf8';

my $progname = basename($0);

# From dpkg-source
my $default_ignore_regex = qr!
# Ignore general backup files
~$|
# Ignore emacs recovery files
(?:^|/)\.#|
# Ignore vi swap files
(?:^|/)\..*\.swp$|
# Ignore baz-style junk files or directories
(?:^|/),,.*(?:$|/.*$)|
# File-names that should be ignored (never directories)
(?:^|/)(?:DEADJOE|\.cvsignore|\.arch-inventory|\.bzrignore|\.gitignore)$|
# File or directory names that should be ignored
(?:^|/)(?:CVS|RCS|\.pc|\.deps|\{arch\}|\.arch-ids|\.svn|\.hg|_darcs|\.git|
\.shelf|_MTN|\.bzr(?:\.backup|tags)?)(?:$|/.*$)
!x;

my $default_check_regex =
		qr!
		\.(                          # search for file suffix
				c(c|pp|xx)?              # c and c++
			 |h(h|pp|xx)?              # header files for c and c++
			 |S
			 |css|less                 # HTML css and similar
			 |f(77|90)?
			 |go
			 |groovy
			 |lisp
			 |scala
			 |clj
			 |p(l|m)?6?|t|xs|pod6?     # perl5 or perl6
			 |sh
			 |php
			 |py(|x)
			 |rb
			 |java
			 |js
			 |vala
			 |el
			 |sc(i|e)
			 |cs
			 |pas
			 |inc
			 |dtd|xsl
			 |mod
			 |m
			 |md|markdown
			 |tex
			 |mli?
			 |(c|l)?hs
		 )
		$
	 !x;

# also used to cleanup
my $copyright_indicator_regex
		= qr!
				 (?:copyright	# The full word
						|copr\.	# Legally-valid abbreviation
						|\xc2\xa9	# Unicode copyright sign encoded in iso8859
			|\x{00a9}	# Unicode character COPYRIGHT SIGN
			#|©		# Unicode character COPYRIGHT SIGN
						|\(c\)	# Legally-null representation of sign
				 )
				!lix;

my $copyright_indicator_regex_with_capture = qr!$copyright_indicator_regex(?::\s*|\s+)(\S.*)$!lix;

# avoid ditching things like <info@foo.com>
my $copyright_disindicator_regex
		= qr{
			\b(?:info(?:rmation)?(?!@)	# Discussing copyright information
						|(notice|statement|claim|string)s?	# Discussing the notice
						|is|in|to        # Part of a sentence
						|(holder|owner)s?       # Part of a sentence
						|ownership              # Part of a sentence
						)\b
				}ix;

my $copyright_predisindicator_regex
		= qr!(
						 ^[#]define\s+.*\(c\)    # #define foo(c) -- not copyright
				 )!ix;

my $modified_conf_msg;

my %OPT=(
		verbose        => '',
		lines          => '',
		noconf         => '',
		ignore         => '',
		check          => '',
		recursive      => 0,
		copyright      => 0,
		machine        => 0,
		text           => 0,
		skipped        => 0,
);

my $def_lines = 60;
my $def_tail = 5000; # roughly 60 lines of 80 chars

# Read configuration files and then command line
# This is boilerplate

if (@ARGV and $ARGV[0] =~ /^--no-?conf$/) {
		$modified_conf_msg = "  (no configuration files read)";
		shift;
} else {
		my @config_files = ('/etc/devscripts.conf', '~/.devscripts');
		my %config_vars = (
					 'LICENSECHECK_VERBOSE' => 'no',
					 'LICENSECHECK_PARSELINES' => $def_lines,
					);
		my %config_default = %config_vars;

		my $shell_cmd;
		# Set defaults
		foreach my $var (keys %config_vars) {
			$shell_cmd .= qq[$var="$config_vars{$var}";\n];
		}
		$shell_cmd .= 'for file in ' . join(" ", @config_files) . "; do\n";
		$shell_cmd .= '[ -f $file ] && . $file; done;' . "\n";
		# Read back values
		foreach my $var (keys %config_vars) { $shell_cmd .= "echo \$$var;\n" }
		my $shell_out = `/bin/bash -c '$shell_cmd'`;
		@config_vars{keys %config_vars} = split /\n/, $shell_out, -1;

		# Check validity
		$config_vars{'LICENSECHECK_VERBOSE'} =~ /^(yes|no)$/
			or $config_vars{'LICENSECHECK_VERBOSE'} = 'no';
		$config_vars{'LICENSECHECK_PARSELINES'} =~ /^[1-9][0-9]*$/
			or $config_vars{'LICENSECHECK_PARSELINES'} = $def_lines;

		foreach my $var (sort keys %config_vars) {
			if ($config_vars{$var} ne $config_default{$var}) {
				$modified_conf_msg .= "  $var=$config_vars{$var}\n";
			}
		}
		$modified_conf_msg ||= "  (none)\n";
		chomp $modified_conf_msg;

		$OPT{'verbose'} = $config_vars{'LICENSECHECK_VERBOSE'} eq 'yes' ? 1 : 0;
		$OPT{'lines'} = $config_vars{'LICENSECHECK_PARSELINES'};
}

GetOptions(\%OPT,
		"help|h",
		"check|c=s",
		"copyright",
		"encoding|e=s",
		"ignore|i=s",
		"lines|l=i",
		"machine|m",
		"noconf|no-conf",
		"recursive|r",
		"skipped|s",
		"tail",
		"text|t",
		"verbose!",
		"version|v",
) or die "Usage: $progname [options] filelist\nRun $progname --help for more details\n";

$OPT{'lines'} = $def_lines if $OPT{'lines'} !~ /^[1-9][0-9]*$/;
my $ignore_regex = length($OPT{ignore}) ? qr/$OPT{ignore}/ : $default_ignore_regex;

my $check_regex = $default_check_regex;
$check_regex = qr/$OPT{check}/ if length $OPT{check};

if ($OPT{'noconf'}) {
		fatal("--no-conf is only acceptable as the first command-line option!");
}
if ($OPT{'help'}) { help(); exit 0; }
if ($OPT{'version'}) { version(); exit 0; }

if ($OPT{text}) {
		warn "$0 warning: option -text is deprecated\n"; # remove -text end 2015
}

die "Usage: $progname [options] filelist\nRun $progname --help for more details\n" unless @ARGV;

$OPT{'lines'} = $def_lines if not defined $OPT{'lines'};

my @files = ();
my @find_args = ();
my $files_count = @ARGV;

push @find_args, qw(-maxdepth 1) unless $OPT{'recursive'};
push @find_args, qw(-follow -type f -print);

while (@ARGV) {
		my $file = shift @ARGV;

		if (-d $file) {
			open my $FIND, '-|', 'find', $file, @find_args
				or die "$progname: couldn't exec find: $!\n";

			while (my $found = <$FIND>) {
				chomp ($found);
				# Silently skip empty files or ignored files
				next if -z $found or $found =~ $ignore_regex;
				if ( not $check_regex or $found =~ $check_regex ) {
					# Silently skip empty files or ignored files
					push @files, $found ;
				}
				else {
					warn "skipped file $found\n" if $OPT{skipped};
				}
			}
			close $FIND;
		}
		elsif ($file =~ $ignore_regex) {
			# Silently skip ignored files
			next;
		}
		elsif ( $files_count == 1 or not $check_regex or $file =~ $check_regex ) {
			push @files, $file;
		}
		else {
			warn "skipped file $file\n" if $OPT{skipped};
		}
}

while (@files) {
		my $file = shift @files;
		my $content = '';
		my $copyright_match;
		my $copyright = '';

		my $st = stat $file;

		my $enc = $OPT{encoding} ;
		my $mode = $enc ? "<:encoding($enc)" : '<';
		# need to use "<" when encoding is unknown otherwise we break compatibility
		my $fh = IO::File->new ($file ,$mode) or die "Unable to access $file\n";

		while ( my $line = $fh->getline ) {
			last if ($fh->input_line_number > $OPT{'lines'});
			$content .= $line;
		}

		my %copyrights = extract_copyright($content);

		print qq(----- $file header -----\n$content----- end header -----\n\n)
		if $OPT{'verbose'};

		my $license = parselicense(clean_cruft_and_spaces(clean_comments($content)));
		$copyright = join(" / ", reverse sort values %copyrights);

		if ( not $copyright and $license eq 'UNKNOWN') {
			my $position = $fh->tell; # See IO::Seekable
			my $tail_size = $OPT{tail} // $def_tail;
			my $jump = $st->size - $tail_size;
			$jump = $position if $jump < $position;

			my $tail ;
			if ( $tail_size and $jump < $st->size) {
				$fh->seek($jump, SEEK_SET) ; # also IO::Seekable
				$tail .= join('',$fh->getlines);
			}

			print qq(----- $file tail -----\n$tail----- end tail -----\n\n)
			if $OPT{'verbose'};

			%copyrights = extract_copyright($tail);
			$license = parselicense(clean_cruft_and_spaces(clean_comments($tail)));
			$copyright = join(" / ", reverse sort values %copyrights);
		}

		$fh->close;

		if ($OPT{'machine'}) {
			print "$file\t$license";
			print "\t" . ($copyright or "*No copyright*") if $OPT{'copyright'};
			print "\n";
		} else {
			print "$file: ";
			print "*No copyright* " unless $copyright;
			print $license . "\n";
			print "  [Copyright: " . $copyright . "]\n"
			if $copyright and $OPT{'copyright'};
				print "\n" if $OPT{'copyright'};
		}
}

sub extract_copyright {
		my $content = shift;
		my @c = split /\n/, clean_comments($content);

		my %copyrights;
		my $lines_after_copyright_block = 0;

		my $in_copyright_block = 0;
		while (@c) {
		my $line = shift @c ;
		my $copyright_match = parse_copyright($line, \$in_copyright_block) ;
		if ($copyright_match) {
			while (@c and $copyright_match =~ /\d[,.]?\s*$/) {
				# looks like copyright end with a year, assume the owner is on next line(s)
				$copyright_match .= ' '. shift @c;
			}
			$copyright_match =~ s/\s+/ /g;
			$copyright_match =~ s/\s*$//;
			$copyrights{lc("$copyright_match")} = "$copyright_match";
		}
		elsif (scalar keys %copyrights) {
			# skip remaining lines if a copyright blocks was found more than 5 lines ago.
			# so a copyright block may contain up to 5 blank lines, but no more
			last if $lines_after_copyright_block++ > 5;
		}
	}
	return %copyrights;
}

sub parse_copyright {
		my $data = shift ;
		my $in_copyright_block_ref = shift;
		my $copyright = '';
		my $match;

		if ( $data !~ $copyright_predisindicator_regex) {
		#print "match against ->$data<-\n";
				if ($data =~ $copyright_indicator_regex_with_capture) {
						$match = $1;
						$$in_copyright_block_ref = 1;
						# Ignore lines matching "see foo for copyright information" etc.
						if ($match !~ $copyright_disindicator_regex) {
								# De-cruft
								$match =~ s/$copyright_indicator_regex//igx;
								$match =~ s/^\s+//;
								$match =~ s/\s*\bby\b\s*/ /;
								$match =~ s/([,.])?\s*$//;
								$match =~ s/\s{2,}/ /g;
								$match =~ s/\\//g; # de-cruft nroff files
								$match =~ s/\s*[*#]\s*$//;
								$copyright = $match;
						}
				}
				elsif ($$in_copyright_block_ref and $data =~ /^\d{2,}[,\s]+/) {
						# following lines beginning with a year are supposed to be
						# continued copyright blocks
						$copyright = $data;
				}
				else {
						$$in_copyright_block_ref = 0;
				}
		}

		return $copyright;
}

sub clean_comments {
		local $_ = shift or return q{};

		# Remove generic comments: look for 4 or more lines beginning with
		# regular comment pattern and trim it. Fall back to old algorithm
		# if no such pattern found.
		my @matches = m/^\s*((?:[^a-zA-Z0-9\s]{1,3}|\bREM\b))\s\w/mg;
		if (@matches >= 4) {
				my $comment_re = qr/\s*[\Q$matches[0]\E]{1,3}\s*/;
				s/^$comment_re//mg;
		}

		# Remove Fortran comments
		s/^[cC] //gm;

		# Remove C / C++ comments
		s#(\*/|/[/*])##g;

		return $_;
}

sub clean_cruft_and_spaces {
		local $_ = shift or return q{};

		tr/\t\r\n/ /;

		# this also removes quotes
		tr% A-Za-z.+,@:;0-9\(\)/-%%cd;
		tr/ //s;

		return $_;
}

sub help {
	 print <<"EOF";
Usage: $progname [options] filename [filename ...]
Valid options are:
	 --help, -h             Display this message
	 --version, -v          Display version and copyright info
	 --no-conf, --noconf    Don't read devscripts config files; must be
													the first option given
	 --verbose              Display the header of each file before its
														license information
	 --skipped, -s          Show skipped files
	 --lines, -l            Specify how many lines of the file header
														should be parsed for license information
														(Default: $def_lines)
	 --tail                 Specify how many bytes to parse at end of file
														(Default: $def_tail)
	 --check, -c            Specify a pattern indicating which files should
														 be checked
														 (Default: '$default_check_regex')
	 --machine, -m          Display in a machine readable way (good for awk)
	 --recursive, -r        Add the contents of directories recursively
	 --copyright            Also display the file's copyright
	 --ignore, -i           Specify that files / directories matching the
														regular expression should be ignored when
														checking files
														(Default: '$default_ignore_regex')

Default settings modified by devscripts configuration files:
$modified_conf_msg
EOF
}

sub version {
		print <<"EOF";
This is $progname, from the Debian devscripts package, version 2.16.2
Copyright (C) 2007, 2008 by Adam D. Barratt <adam\@adam-barratt.org.uk>; based
on a script of the same name from the KDE SDK by <dfaure\@kde.org>.

This program comes with ABSOLUTELY NO WARRANTY.
You are free to redistribute this code under the terms of the
GNU General Public License, version 2, or (at your option) any
later version.
EOF
}

sub parselicense {
		my ($licensetext) = @_;

		my $gplver = "";
		my $extrainfo = "";
		my $license = "";

		if ($licensetext =~ /version ([^ ]+)(?: of the License)?,? or(?: \(at your option\))? version (\d(?:[.-]\d+)*)/) {
			$gplver = " (v$1 or v$2)";
		} elsif ($licensetext =~ /version ([^, ]+?)[.,]? (?:\(?only\)?.? )?(?:of the GNU (Affero )?(Lesser |Library )?General Public License )?(as )?published by the Free Software Foundation/i or
			$licensetext =~ /GNU (?:Affero )?(?:Lesser |Library )?General Public License (?:as )?published by the Free Software Foundation[;,] version ([^, ]+?)[.,]? /i) {

			$gplver = " (v$1)";
		} elsif ($licensetext =~ /GNU (?:Affero )?(?:Lesser |Library )?General Public License\s*(?:[(),GPL]+)\s*version (\d+(?:\.\d+)?)[ \.]/i) {
			$gplver = " (v$1)";
		} elsif ($licensetext =~ /either version ([^ ]+)(?: of the License)?, or (?:\(at your option\) )?any later version/) {
			$gplver = " (v$1 or later)";
		} elsif ($licensetext =~ /GPL\sas\spublished\sby\sthe\sFree\sSoftware\sFoundation,\sversion\s([\d.]+)/i ) {
			$gplver = " (v$1)";
		} elsif ($licensetext =~ /SPDX-License-Identifier:\s+GPL-([1-9])\.0-or-later/i ){
			$gplver = " (v$1 or later)";
		} elsif ($licensetext =~ /SPDX-License-Identifier:\s+GPL-([1-9])\.0[^+]/i ) {
			$gplver = " (v$1)";
		} elsif ($licensetext =~ /SPDX-License-Identifier:\s+GPL-([1-9])\.0\+/i ) {
			$gplver = " (v$1 or later)";
		} elsif ($licensetext =~ /SPDX-License-Identifier:\s+LGPL-([1-9])\.[0-1]\-or-later/i ) {
			$gplver = " (v$1 or later)";
		}

		if ($licensetext =~ /(?:675 Mass Ave|59 Temple Place|51 Franklin Steet|02139|02111-1307)/i) {
			$extrainfo = " (with incorrect FSF address)$extrainfo";
		}

		if ($licensetext =~ /permission (?:is (also granted|given))? to link (the code of )?this program with (any edition of )?(Qt|the Qt library)/i) {
			$extrainfo = " (with Qt exception)$extrainfo"
		}

		if ($licensetext =~ /As a special exception, you may create a larger work that contains part or all of the Bison parser skeleton and distribute that work under terms of your choice/) {
			$extrainfo = " (with Bison parser exception)$extrainfo";
		}

		# exclude blurb found in boost license text
		if ($licensetext =~ /(All changes made in this file will be lost|DO NOT (EDIT|delete this file)|Generated (automatically|by|from)|generated.*file)/i
				and $licensetext !~  /unless such copies or derivative works are solely in the form of machine-executable object code generated by a source language processor/) {
			$license = "GENERATED FILE";
		}

		if ($licensetext =~ /(are made available|(is free software.? )?you can redistribute (it|them) and(?:\/|\s+)or modify (it|them)|is licensed) under the terms of (version [^ ]+ of )?the (GNU (Library |Lesser )General Public License|LGPL)/i) {
			$license = "LGPL$gplver$extrainfo $license";
		}
		# For Perl modules handled by Dist::Zilla
		elsif ($licensetext =~ /this is free software,? licensed under:? (?:the )?(?:GNU (?:Library |Lesser )General Public License|LGPL),? version ([\d\.]+)/i) {
			$license = "LGPL (v$1) $license";
		}

		if ($licensetext =~ /is free software.? you can redistribute (it|them) and(?:\/|\s+)or modify (it|them) under the terms of the (GNU Affero General Public License|AGPL)/i) {
			$license = "AGPL$gplver$extrainfo $license";
		}

		if ($licensetext =~ /(is free software.? )?you (can|may) redistribute (it|them) and(?:\/|\s+)or modify (it|them) under the terms of (?:version [^ ]+ (?:\(?only\)? )?of )?the GNU General Public License/i) {
			$license = "GPL$gplver$extrainfo $license";
		}

		if ($licensetext =~ /is distributed under the terms of the GNU General Public License,/
				and length $gplver) {
			$license = "GPL$gplver$extrainfo $license";
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+GPL/i and length $gplver) {
			$license = "GPL$gplver$extrainfo $license";
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+GPL-2.0-or-later/i and length $gplver) {
			$license = "GPL$gplver$extrainfo";
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+LGPL/i and length $gplver) {
			$license = "LGPL$gplver$extrainfo $license";
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+Zlib/i) {
			$license = "zlib/libpng $license";
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+BSD-3-Clause/i) {
			$license = 'BSD (3 clause)';
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+BSD-2-Clause/i) {
			$license = 'BSD (2 clause)';
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+BSD-1-Clause/i) {
			$license = 'BSD';
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+MIT/i) {
			$license = 'MIT/X11 (BSD like)';
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+ISC/i) {
			$license = 'ISC';
		}

		if ($licensetext =~ /(?:is|may be)\s(?:(?:distributed|used).*?terms|being\s+released).*?\b(L?GPL)\b/) {
			my $v = $gplver || ' (unversioned/unknown version)';
			$license = "$1$v $license";
		}

		if ($licensetext =~ /the rights to distribute and use this software as governed by the terms of the Lisp Lesser General Public License|\bLLGPL\b/ ) {
			$license = "LLGPL $license";
		}

		if ($licensetext =~ /This file is part of the .*Qt GUI Toolkit. This file may be distributed under the terms of the Q Public License as defined/) {
			$license = "QPL (part of Qt) $license";
		} elsif ($licensetext =~ /may (be distributed|redistribute it) under the terms of the Q Public License/) {
			$license = "QPL $license";
		}

		if ($licensetext =~ /opensource\.org\/licenses\/mit-license\.php/) {
			$license = "MIT/X11 (BSD like) $license";
		} elsif ($licensetext =~ /Permission is hereby granted, free of charge, to any person obtaining a copy of this software and(\/or)? associated documentation files \(the (Software|Materials)\), to deal in the (Software|Materials)/) {
			$license = "MIT/X11 (BSD like) $license";
		} elsif ($licensetext =~ /Permission is hereby granted, without written agreement and without license or royalty fees, to use, copy, modify, and distribute this software and its documentation for any purpose/) {
			$license = "MIT/X11 (BSD like) $license";
		}

		if ($licensetext  =~ /Permission to use, copy, modify, and(\/or)? distribute this software for any purpose with or without fee is hereby granted, provided.*copyright notice.*permission notice.*all copies/) {
			$license = "ISC $license";
		}

		if ($licensetext =~ /THIS SOFTWARE IS PROVIDED .*AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY/) {
			if ($licensetext =~ /All advertising materials mentioning features or use of this software must display the following acknowledge?ment.*This product includes software developed by/i) {
					$license = "BSD (4 clause) $license";
			} elsif ($licensetext =~ /(The name(?:\(s\))? .*? may not|Neither the (names? .*?|authors?) nor the names of( (its|their|other|any))? contributors may) be used to endorse or promote products derived from this software/i) {
					$license = "BSD (3 clause) $license";
			} elsif ($licensetext =~ /Redistributions in binary form must reproduce the above copyright notice/i) {
					$license = "BSD (2 clause) $license";
			} else {
					$license = "BSD $license";
			}
		}

		if ($licensetext =~ /Mozilla Public License,? (?:(?:Version|v\.)\s+)?(\d+(?:\.\d+)?)/) {
			$license = "MPL (v$1) $license";
		}
		elsif ($licensetext =~ /Mozilla Public License,? \((?:Version|v\.) (\d+(?:\.\d+)?)\)/) {
			$license = "MPL (v$1) $license";
		}

		# match when either:
		# - the text *begins* with "The Artistic license v2.0" which is (hopefully) the actual artistic license v2.0 text.
		# - a license grant is found. i.e something like "this is free software, licensed under the artistic license v2.0"
		if ($licensetext =~ /(?:^\s*|(?:This is free software, licensed|Released|be used|use and modify this (?:module|software)) under (?:the terms of )?)[Tt]he Artistic License ([v\d.]*\d)/) {
			$license = "Artistic (v$1) $license";
		}

		if ($licensetext =~ /is free software under the Artistic [Ll]icense/) {
			$license = "Artistic $license";
		}

		if ($licensetext =~ /This program is free software; you can redistribute it and\/or modify it under the same terms as Perl itself/) {
			$license = "Perl $license";
		}

		if ($licensetext =~ /under the Apache License, Version ([^ ]+)/) {
			$license = "Apache (v$1) $license";
		}

		if ($licensetext =~ /(THE BEER-WARE LICENSE)/i) {
			$license = "Beerware $license";
		}

		if ($licensetext =~ /distributed under the terms of the FreeType project/i) {
			$license = "FreeType $license"; # aka FTL see http://www.freetype.org/license.html
		}

		if ($licensetext =~ /This source file is subject to version ([^ ]+) of the PHP license/) {
			$license = "PHP (v$1) $license";
		}

		if ($licensetext =~ /under the terms of the CeCILL /) {
			$license = "CeCILL $license";
		}

		if ($licensetext =~ /under the terms of the CeCILL-([^ ]+) /) {
			$license = "CeCILL-$1 $license";
		}

		if ($licensetext =~ /under the SGI Free Software License B/) {
			$license = "SGI Free Software License B $license";
		}

		if ($licensetext =~ /is in the public domain/i) {
			$license = "Public domain $license";
		}

		if ($licensetext =~ /terms of the Common Development and Distribution License(, Version ([^(]+))? \(the License\)/) {
			$license = "CDDL " . ($1 ? "(v$2) " : '') . $license;
		}

		if ($licensetext =~ /Microsoft Permissive License \(Ms-PL\)/) {
			$license = "Ms-PL $license";
		}

		if ($licensetext =~ /Licensed under the Academic Free License version ([\d.]+)/) {
			$license = $1 ? "AFL-$1" : "AFL";
		}

		if ($licensetext =~ /This program and the accompanying materials are made available under the terms of the Eclipse Public License v?([\d.]+)/) {
			$license = $1 ? "EPL-$1" : "EPL";
		}

		# quotes were removed by clean_comments function
		if ($licensetext =~ /Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and accompanying documentation covered by this license \(the Software\)/ or
			$licensetext =~ /Boost Software License([ ,-]+Version ([^ ]+)?(\.))/i) {
			$license = "BSL " . ($1 ? "(v$2) " : '') . $license;
		}

		if ($licensetext =~ /PYTHON SOFTWARE FOUNDATION LICENSE (VERSION ([^ ]+))/i) {
			$license = "PSF " . ($1 ? "(v$2) " : '') . $license;
		}

		if ($licensetext =~ /The origin of this software must not be misrepresented.*Altered source versions must be plainly marked as such.*This notice may not be removed or altered from any source distribution/ or
				$licensetext =~ /see copyright notice in zlib\.h/) {
			$license = "zlib/libpng $license";
		} elsif ($licensetext =~ /This code is released under the libpng license/) {
			$license = "libpng $license";
		}

		if ($licensetext =~ /Do What The Fuck You Want To Public License, Version ([^, ]+)/i) {
			$license = "WTFPL (v$1) $license";
		}

		if ($licensetext =~ /Do what The Fuck You Want To Public License/i) {
			$license = "WTFPL $license";
		}

		if ($licensetext =~ /(License WTFPL|Under (the|a) WTFPL)/i) {
			$license = "WTFPL $license";
		}

		if ($licensetext =~ /SPDX-License-Identifier:\s+\(([a-zA-Z0-9-\.]+)\s+OR\s+([a-zA-Z0-9-\.]+)\)/i) {
		  # print STDERR "OK ---$1---$2---";
			# print "PIPPO " . parselicense("SPDX-License-Identifier: $1") . " E " . parselicense("SPDX-License-Identifier: $2");
			if ($1 and $2) {
				$license = parselicense("SPDX-License-Identifier: $1") . " " . parselicense("SPDX-License-Identifier: $2");
			}
		}

		$license = "UNKNOWN" if (!length($license));

		# Remove trailing spaces.
		$license =~ s/\s+$//;

		return $license;
}

sub fatal {
		my ($pack,$file,$line);
		($pack,$file,$line) = caller();
		(my $msg = "$progname: fatal error at line $line:\n@_\n") =~ tr/\0//d;
		$msg =~ s/\n\n$/\n/;
		die $msg;
}
