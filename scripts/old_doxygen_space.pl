#!/usr/bin/env perl
###########################################################################
#    doxygen_space.pl
#    ---------------------
#    begin                : October 2016
#    copyright            : (C) 2016 by Nyall Dawson
#    email                : nyall dot dawson at gmail dot com
#
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

# adapted from scripts/sort_includes.sh

# this is slurped in whole-file mode, as opposed to unify_includes.pl
# which slurps in per-line mode

use strict;
use warnings;

my $file = $ARGV[0];

# read file
open(my $handle, "<", $file) || die "Couldn't open '".$file."' for reading because: ".$!;
chomp(my @INPUT_LINES = <$handle>);
close $handle;


my $LINE_IDX = 0;
my $LINE;
my @OUTPUT = ();
my $LINE_COUNT = @INPUT_LINES;

open(my $out_handle, ">", $file) || die "Couldn't open '".$file."' for writing because: ".$!;

my $PREVIOUS_WAS_BLANKLINE = 0;
my $INSIDE_DOX_BLOCK = 0;
my $INSIDE_DOX_LIST = 0;
my $PREVIOUS_WAS_DOX_BLANKLINE = 0;
my $JUST_FINISHED_A_LIST = 0;
my $BUFFERED_LINE;

while ($LINE_IDX < $LINE_COUNT){
    my $new_line = $INPUT_LINES[$LINE_IDX];
    my $is_blank_line = ( $new_line =~ m/^\s*$/ ) ? 1 : 0;

    if ( $new_line =~ m/^(\s*)(?:#ifdef|#ifndef|#else|#endif)/ ){
      print $out_handle $new_line."\n";
    }
    elsif ( $new_line =~ m/^(\s*)\/\/!\s*(.*?)$/ ){
       #found a //! comment
       my $indentation = $1;
       my $comment = $2;
       #check next line to see if it begins with //!
       if ( $INPUT_LINES[$LINE_IDX+1] =~ m/\s*\/\/!\s*(.*?)$/){
          #we are in a multiline //! comment block, convert to /** block
          if ( !$PREVIOUS_WAS_BLANKLINE ){
            print $out_handle "\n";
          }
          print $out_handle $indentation."/**\n";
          print $out_handle $indentation." * ".$comment."\n";
          while ( $INPUT_LINES[$LINE_IDX+1] =~ m/\s*\/\/!\s*(.*?)$/ ){
            if ($1 ne ''){
              print $out_handle $indentation." * ".$1."\n";
            }
            else {
              print $out_handle $indentation." *\n";
            }
            $LINE_IDX++;
          }
          print $out_handle $indentation." */\n";
       }
       else {
          print $out_handle $new_line."\n";
       }
    }
    elsif ( $INSIDE_DOX_BLOCK ){
      # replace "* abc" style doxygen lists with correct "- abc" formatting
      $new_line =~ s/^(\s+)\*\s{1,10}\*/$1\* \-/;

      if ( $new_line =~ m/^\s*\*\s*$/ ){
        # print $out_handle "blank line!\n";
        $PREVIOUS_WAS_DOX_BLANKLINE = 1;
        if ( $INSIDE_DOX_LIST ) {
          $INSIDE_DOX_LIST = 0;
          $JUST_FINISHED_A_LIST = 1;
          $BUFFERED_LINE = $new_line;
          # print $out_handle "end list\n";
        }
        else {
          print $out_handle $new_line . "\n";
        }
      }
      elsif ( $new_line =~ m/^(\s*)\*\s*\-(?![-\d>])(?: )*(.*)/ ){
        if ( !$INSIDE_DOX_LIST && !$PREVIOUS_WAS_DOX_BLANKLINE ){
          print $out_handle "$1*\n";
        }
        if ($JUST_FINISHED_A_LIST ){
          # print $out_handle "just finished a list, continuing the same one!!\n";
          $BUFFERED_LINE = "";
        }
        # print $out_handle "start list\n";
        print $out_handle "$1* - $2\n";
        $INSIDE_DOX_LIST = 1;
        $JUST_FINISHED_A_LIST = 0;
      }
      elsif ($INSIDE_DOX_LIST && $new_line =~ m/^(\s*)\*\s{2,}(.*)$/ ){
        # print $out_handle "list continuation\n";
        print $out_handle "$1*   $2\n";
      }
      elsif ($INSIDE_DOX_LIST && $new_line =~ m/^(\s*)\*(?!\/)/ ){
        $INSIDE_DOX_LIST = 0;
        # print $out_handle "end list without line break\n";
        print $out_handle "$1*\n";
        print $out_handle $new_line."\n";
        $JUST_FINISHED_A_LIST = 1;
      }
      elsif ( $new_line =~ m/^(\s*)\*\/\s*$/ ){
        $INSIDE_DOX_BLOCK = 0;
        $INSIDE_DOX_LIST = 0;
        $JUST_FINISHED_A_LIST = 0;
        if ( $BUFFERED_LINE ) {
          print $out_handle $BUFFERED_LINE."\n";
          $BUFFERED_LINE = "";
        }
        print $out_handle $new_line."\n";
        # print $out_handle "end_block\n";
      }
      else {
        if ( $BUFFERED_LINE ) {
          print $out_handle $BUFFERED_LINE."\n";
          $BUFFERED_LINE = "";
        }

        if ($new_line !~ m/^\s*[#\*]/ && $new_line =~ m/^(\s*?)(\s?)(.+?)$/ )
        {
          $new_line = "$1* $3";
        }

        print $out_handle $new_line."\n";
        # print $out_handle "normal dox\n";
        $PREVIOUS_WAS_DOX_BLANKLINE = 0;
        $JUST_FINISHED_A_LIST = 0;
      }
    }
    elsif ( $new_line =~ m/^(\s*)\/\*\*(?!\*)\s*(.*)$/ ){
          # Space around doxygen start blocks (force blank line before /**)
      if ( !$PREVIOUS_WAS_BLANKLINE ){
          print $out_handle "\n";
      }
      if ( $2 ne '' ){
          # new line after /** begin block
          print $out_handle "$1\/**\n$1 * $2\n";
      }
      else {
          print $out_handle $new_line."\n";
      }
      $INSIDE_DOX_BLOCK = 1;
      # print $out_handle "start_block\n";
    }
    else {
      if ( $BUFFERED_LINE ) {
        print $out_handle $BUFFERED_LINE."\n";
        $BUFFERED_LINE = "";
      }
        print $out_handle $new_line."\n";
    }
    $LINE_IDX++;
    $PREVIOUS_WAS_BLANKLINE = $is_blank_line;
}
close $out_handle;
