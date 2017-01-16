#!/usr/bin/perl -0 -i.sortinc -n
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

# Space around doxygen start blocks (force blank line before /**)
s#(?<!\n)(\n\h*\/\*\*(?!\*))#\n$1#g;

print;
