#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
***************************************************************************
    qgis.v.kernel.rast.py
    ---------------------
    Date                 : February 2010
    Copyright            : (C) 2010 by Radim Blazek
    Email                : radim dot blazek at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Radim Blazek'
__date__ = 'February 2010'
__copyright__ = '(C) 2010, Radim Blazek'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


############################################################################
#
# MODULE:       qgis.v.kernel.rast.py
# PURPOSE:      Export a vector to PostGIS (PostgreSQL) database table
#
#############################################################################

#%Module
#% description: Generates a raster density map from vector points data using a moving 2D isotropic Gaussian kernel.
#% keywords: vector, export, database
#%End

#%option
#% key: input
#% type: string
#% gisprompt: old,vector,vector
#% key_desc : name
#% description: Input vector with training points
#% required : yes
#%end

#%option
#% key: stddeviation
#% type: double
#% description: Standard deviation in map units
#% required : yes
#%end

#%option
#% key: output
#% type: string
#% gisprompt: new,cell,raster
#% key_desc : name
#% description: Output raster map
#% required : yes
#%end

import sys
import os
import string
try:
    from grass.script import core as grass
except ImportError:
    import grass
except:
    raise Exception ("Cannot find 'grass' Python module. Python is supported by GRASS from version >= 6.4" )

def main():
    input = options['input']
    output = options['output']
    stddeviation = options['stddeviation']

    if grass.run_command('v.kernel', input=input, stddeviation=stddeviation, output=output ) != 0:
         grass.fatal("Cannot run v.kernel.")

if __name__ == "__main__":
    options, flags = grass.parser()
    main()
