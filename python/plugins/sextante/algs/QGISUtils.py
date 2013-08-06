# -*- coding: utf-8 -*-

"""
***************************************************************************
    QGISUtils.py
    ---------------------
    Date                 : August 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'August 2013'
__copyright__ = '(C) 2013, Alexander Bruy'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import math

from PyQt4.QtCore import *

from qgis.core import *


def mapToPixel(mX, mY, geoTransform):
    '''Convert map coordinates to pixel coordinates.

    @param mX              Input map X coordinate (double)
    @param mY              Input map Y coordinate (double)
    @param geoTransform    Input geotransform (six doubles)
    @return pX, pY         Output coordinates (two doubles)
    '''
    if geoTransform[2] + geoTransform[4] == 0:
        pX = (mX - geoTransform[0]) / geoTransform[1]
        pY = (mY - geoTransform[3]) / geoTransform[5]
    else:
        pX, pY = applyGeoTransform(mX, mY, invertGeoTransform(geoTransform))
    return int(pX), int(pY)

def pixelToMap(pX, pY, geoTransform):
    '''Convert pixel coordinates to map coordinates.

    @param pX              Input pixel X coordinate (double)
    @param pY              Input pixel Y coordinate (double)
    @param geoTransform    Input geotransform (six doubles)
    @return mX, mY         Output coordinates (two doubles)
    '''
    mX, mY = applyGeoTransform(pX + 0.5, pY + 0.5, geoTransform)
    return mX, mY

def applyGeoTransform(inX, inY, geoTransform):
    '''Apply a geotransform to coordinates.

    @param inX             Input coordinate (double)
    @param inY             Input coordinate (double)
    @param geoTransform    Input geotransform (six doubles)
    @return outX, outY     Output coordinates (two doubles)
    '''
    outX = geoTransform[0] + inX * geoTransform[1] + inY * geoTransform[2]
    outY = geoTransform[3] + inX * geoTransform[4] + inY * geoTransform[5]
    return outX, outY

def invertGeoTransform(geoTransform):
    '''Invert standard 3x2 set of geotransform coefficients.

    @param geoTransform        Input GeoTransform (six doubles - unaltered)
    @return outGeoTransform    Output GeoTransform (six doubles - updated)
                               on success, None if the equation is uninvertable
    '''
    # we assume a 3rd row that is [1 0 0]
    # compute determinate
    det = geoTransform[1] * geoTransform[5] - geoTransform[2] * geoTransform[4]

    if abs(det) < 0.000000000000001:
        return

    invDet = 1.0 / det

    # compute adjoint and divide by determinate
    outGeoTransform = [0, 0, 0, 0, 0, 0]
    outGeoTransform[1] = geoTransform[5] * invDet
    outGeoTransform[4] = -geoTransform[4] * invDet

    outGeoTransform[2] = -geoTransform[2] * invDet
    outGeoTransfrom[5] = geoTransform[1] * invDet

    outGeoTransform[0] = (geoTransform[2] * geoTransform[3] - geoTransform[0] * geoTransform[5]) * invDet
    outGeoTransform[3] = (-geoTransform[1] * geoTransform[3] + geoTransform[0] * geoTransform[4]) * invDet

    return outGeoTransform
