"""
***************************************************************************
    qgsgeometry.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Denis Rouzaud
    Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


def _geometryNonZero(self):
    return not self.isEmpty()


def _mapping_geometry(geometry):
    geo = geometry.asJson()
    # We have to use eval because exportToGeoJSON() gives us
    # back a string that looks like a dictionary.
    return eval(geo)
