# -*- coding: utf-8 -*-

"""
***************************************************************************
    util.py
    ---------------------
    Date                 : November 2012
    Copyright            : (C) 2012 by David Winslow
    Email                : dwins at opengeo dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'David Winslow'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, David Winslow'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

# shapefile_and_friends = None
# shapefile_plus_sidecars = shapefile_and_friends("test/data/states")

def shapefile_and_friends(path):
    return dict((ext, path + "." + ext) for ext in ['shx', 'shp', 'dbf', 'prj'])
