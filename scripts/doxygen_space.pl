#!/usr/bin/perl -0 -i.sortinc -n
###########################################################################
#    unify_includes.pl
#    ---------------------
#    begin                : June 2015
#    copyright            : (C) 2015 by Juergen E. Fischer
#    email                : jef at norbit dot de
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

# this is slurped in whole-file mode, as opposed to unify_incudes.pl
# which slurps in per-line mode

use strict;
use warnings;

our %inc;
our @inc;

END { die "header files not empty" if @inc; }

# Space around doxygen start blocks (force blank line before /**)
s#(?<!\n)(\n\h*/\*\*)#\n$1#g;

print;
