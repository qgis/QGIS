#!/usr/bin/perl

# use QgsDebugMsg instead of iostream for debugging output

# EXAMPLE:
#
#   #include <iostream>
#
#   #ifdef QGISDEBUG
#   std::cout << "A " << a << " B " << b << " A " << a << std::endl;
#   // std::cout << "commented out" << std::endl;
#   #endif
#
# becomes
#
#   #include "qgslogger.h"
#
#   QgsDebugMsg(QString("A %1 B %2 A %1").arg(a).arg(b));
#   QgsDebugMsgLevel("commented out", 3);
#
#
# append // OK to keep it as it is.

use strict;
use warnings;

for my $file (@ARGV) {
	my $output;
	my $F;
	my @file;

	open $F, $file;
	 
	my $externc = 0;
	my $ifdef = 0;
	my $loggerseen=0;
	my $lastinclude=0;
	my $modified = 0;
	my $i=0;
	my $le;
	while(<$F>) {
		$i++;

		($le) = /([\r\n]+)$/ unless defined $le;

		if(/\s*#\s*include\s*<iostream>/) {
			next;
		}

		if(/\s*#\s*include\s*qgslogger\.h/) {
			$loggerseen=1;
		}

		$externc=1 if /extern\s+\"C\"\s*{/;
		$externc=0 if $externc && /^\s*}\s*$/;
		$ifdef++ if /^\s*#\s*if/;
		$ifdef-- if /^\s*#\s*endif/;

		if($externc==0 && $ifdef==0 && /\s*#\s*include/) {
			$lastinclude = scalar(@file)+1;
		}

		if(/(std::)?(cout|cerr)/) {
			die "nested? [$file]" if defined $output;
			$output = "";
		}

		if(defined $output) {
			$output .= $_;
			if(/;/) {
				($le) = ($output =~ /([\r\n]+)$/);
				$output =~ s/$le/\n/g;

				my $level = 0;
				if($output =~ /^\s*\/\/\s*((std::)?(cout|cerr))/) {
					$level = 3;
					$output =~ s/^\s*\/\///;
					$output =~ s/\n\s*\/\//\n /g;
				}

				my @arr = split /\s*<<\s*/, $output;
				my ($indent) = ($arr[0] =~ /^(\s*)/);
				$arr[0] =~ s/^\s+//;

				if($arr[0] =~ /^\/\// || $arr[-1] =~ /\/\/ OK$/) {
					# commented out
					push @file, "$output\n";
					undef $output;
					next;
				}

				unless( $arr[0] =~ /^(std::)?(cout|cerr)$/ ) {
					die "std::(cerr|cout) expected [$file]: |" . $arr[0] . "|";
				}

				$arr[-1] =~ s/\s*;\s*$/;/;

				if( $arr[-1] =~ /\\n";$/) {
					$arr[-1] =~ s/\\n";/"/;
					push @arr, "std::endl;";
				} elsif( $arr[-1] =~ /'\\n';$/) {
					$arr[-1] = "std::endl;";
				}

				if( $arr[-1] =~ /^(std::)?flush;$/ &&
				    $arr[-2] =~ /^(std::)?endl$/ ) {
					pop @arr;
					pop @arr;
					push @arr, "std::endl;";
				}

				unless( $arr[-1] =~ /^(std::)?endl;$/ ) {
					die "std::endl; expected [$file]: |" . $arr[-1] . "|";
				} 

				shift @arr;
				pop @arr;

				my $str;
				my %args;
				my @args;
				my $fmt = "";
				foreach(@arr) {
					if(/^"(.*)"$/) {
						$fmt .= $1;
					} else {
						if(/^QString::number\s*\(\s*([^,]*)\s*\)$/) {
							$_ = $1;
						}
		
						s/\.toLocal8Bit\(\).data\(\)$//;
						s/\.toUtf8\(\).data\(\)$//;
						s/\.ascii\(\)$//;

						if(exists $args{$_}) {
							my $n = $args{$_};
							$fmt .= "%$n";
						} else {
							push @args, $_;
							$args{$_} = scalar(@args);
							$fmt .= "%" . scalar(@args);
						}
					}
				}

				if(@args>0) {
					if(@args==1 && $fmt eq "%1") {
						$str = $args[0];
					} else {
						$str = "QString(\"$fmt\").arg(" . join(").arg(", @args) . ")";
					}
				} else {
					$str = "\"$fmt\"";
				}

				if($level == 3) {
#					push @file, $indent . "QgsDebugMsgLevel($str, 3);$le";
					push @file, $indent . "// QgsDebugMsg($str);$le";
				} else {
					push @file, $indent . "QgsDebugMsg($str);$le";
				}

				$modified=1;

				undef $output;
			}
		} else {
			push @file, $_;
		}
	}
	close $F;


	if($modified) {
	  if(!$loggerseen) {
	    die "no includes? [$file]" unless defined $lastinclude;
	    splice @file, $lastinclude, 0, "#include \"qgslogger.h\"$le";
	  }

	  #print "MODIFIED: $file\n";

	  my @filtered;
	  my @output;
	  my $ifdef_seen=0;

	  foreach(@file) {
	    if($ifdef_seen) {
	      if(/^\s*#\s*if/) {
		die "nested #if? [$file]";
	      } elsif(/^\s*QgsDebugMsg/) {
		push @output, $_;
	      } elsif(/^\s*#\s*endif/) {
		push @filtered, $_ foreach @output;
		undef @output;
		$ifdef_seen=0;
	      } else {
		push @filtered, "#ifdef QGISDEBUG$le";
		push @filtered, $_ foreach @output;
		push @filtered, $_;
		undef @output;
		$ifdef_seen=0;
	      }
	    } elsif(/^\s*#\s*ifdef\s+QGISDEBUG\s*$/) {
	      die "output pending" if @output;
	      $ifdef_seen=1;
	    } else {
	      push @filtered, $_;
	    }
	  }

	  die "output pending" if @output;

	  link $file, "$file.iostream" unless -f "$file.iostream";
	  unlink $file;

	  open $F, ">$file";
	  foreach (@filtered) {
	    print $F $_;
	  }
	  close $F;
	}
}

# vim: set ts=8 noet:
