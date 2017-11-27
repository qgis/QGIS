#!/usr/bin/perl
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

while ($LINE_IDX < $LINE_COUNT){
    my $new_line = $INPUT_LINES[$LINE_IDX];
    my $is_blank_line = ( $new_line =~ m/^\s*$/ ) ? 1 : 0;

    if ( $new_line =~ m/^(\s*)\/\/!\s*(.*?)$/ ){
       #found a //! comment
       my $identation = $1;
       my $comment = $2;
       #check next line to see if it begins with //!
       if ( $INPUT_LINES[$LINE_IDX+1] =~ m/\s*\/\/!\s*(.*?)$/){
          #we are in a multiline //! comment block, convert to /** block
          if ( !$PREVIOUS_WAS_BLANKLINE ){
            print $out_handle "\n";
          }
          print $out_handle $identation."/**\n";
          print $out_handle $identation." * ".$comment."\n";
          while ( $INPUT_LINES[$LINE_IDX+1] =~ m/\s*\/\/!\s*(.*?)$/ ){
            if ($1 ne ''){
              print $out_handle $identation." * ".$1."\n";
            }
            else {
              print $out_handle $identation." *\n";
            }
            $LINE_IDX++;
          }
          print $out_handle $identation." */\n";
       }
       else {
          print $out_handle $new_line."\n";
       }
    }
    else {
        if ( $new_line =~ m/^(\s*)\/\*\*(?!\*)\s*(.*)$/ ){
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
        }
        else {
            print $out_handle $new_line."\n";
        }
    }
    $LINE_IDX++;
    $PREVIOUS_WAS_BLANKLINE = $is_blank_line;
}
close $out_handle;
