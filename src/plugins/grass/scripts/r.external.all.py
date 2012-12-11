#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
***************************************************************************
    r.external.all.py
    ---------------------
    Date                 : July 2009
    Copyright            : (C) 2009 by Lorenzo Masini
    Email                : rugginoso at develer dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Lorenzo Masini'
__date__ = 'July 2009'
__copyright__ = '(C) 2009, Lorenzo Masini'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


############################################################################
#
# MODULE:       qgis.r.external.all.py
# AUTHOR(S):    Lorenzo Masini
#
# PURPOSE:      Link all GDAL supported raster files into a directory
#		to binary raster map layers.
# COPYRIGHT:    (C) 2009 by Lorenzo Masini
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with GRASS
#               for details.
#
#############################################################################

#%Module
#% description: Link all GDAL supported raster files into a directory to binary raster map layers.
#% keywords: raster, import
#%End

#%option
#% key: input
#% type: string
#% gisprompt: input
#% key_desc : name
#% description: Directory containing raster files
#% required : yes
#%end

#%option
#% key: band
#% type: integer
#% description: Band to select
#% answer: 1
#% required : no
#%end

#%flag
#% key: o
#% description: Override projection (use location's projection)
#%end

#%flag
#% key: e
#% description: Extend location extents based on new dataset
#%end

#%flag
#% key: r
#% description: Recursively scan subdirectories

import sys
import os
try:
    from grass.script import core as grass
except ImportError:
    import grass
except:
    raise Exception ("Cannot find 'grass' Python module. Python is supported by GRASS from version >= 6.4" )


def import_directory_of_rasters(directory, recursive):
    for dir, dirnames, filenames in os.walk(directory):
	for filename in filenames:
		if grass.run_command('r.external', flags=flags_string, input=os.path.join(dir, filename), band=options['band'], output=filename[:-4], title=filename[:-4]) != 0:
			grass.warning('Cannot import file' + filename)
	if not recursive:
		break
	for dirname in dirnames:
		import_directory_of_rasters(dirname, recursive)

def main():
    input = options['input']
    recursive = flags['r']

    import_directory_of_rasters(input, recursive)

if __name__ == "__main__":
    options, flags = grass.parser()
    flags_string = "".join([k for k in flags.keys() if flags[k] and k != 'r'])
    main()

