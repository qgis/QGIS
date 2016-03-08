#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
***************************************************************************
    qgis.v.upgrade.py
    ---------------------
    Date                 : October 2015
    Copyright            : (C) 2015 by Radim Blazek
    Email                : radim.blazek@gmail.com
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
__date__ = 'October 2015'
__copyright__ = '(C) 2015 by Radim Blazek'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


############################################################################
#
# MODULE:       qgis.v.upgrade.py
# AUTHOR(S):    Radim Blazek
#
# PURPOSE:      Upgrade all vectors from GRASS 6 to GRASS 7
#
# COPYRIGHT:    (C) 2015 by Radim Blazek
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with GRASS
#               for details.
#
#############################################################################

#%Module
#% description: Upgrade all vectors from GRASS 6 to GRASS 7
#% keywords: vector, upgrade
#%End

import os
try:
    from grass.script import core as grass
except ImportError:
    import grass
except:
    raise Exception("Cannot find 'grass' Python module. Python is supported by GRASS from version >= 6.4")


def main():
    # see https://grasswiki.osgeo.org/wiki/Convert_all_GRASS_6_vector_maps_to_GRASS_7
    grass.message('Building topology')
    if grass.run_command('v.build.all') != 0:
        grass.warning('Cannot build topology')

    grass.message('Creating new DB connection')
    if grass.run_command('db.connect', flags='d') != 0:
        grass.warning('Cannot create new DB connection')
        return

    grass.message('Transferring tables to the new DB')
    if grass.run_command('v.db.reconnect.all', flags='cd') != 0:
        grass.warning('Cannot transfer tables')

if __name__ == "__main__":
    options, flags = grass.parser()
    main()
