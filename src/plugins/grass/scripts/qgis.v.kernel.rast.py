#!/usr/bin/env python

############################################################################
#
# MODULE:       qgis.v.kernel.rast.py
# AUTHOR(S):    Radim Blazek
#
# PURPOSE:      Export a vectore to PostGIS (PostgreSQL) database table
# COPYRIGHT:    (C) 2009 by Radim Blazek
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with GRASS
#               for details.
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
