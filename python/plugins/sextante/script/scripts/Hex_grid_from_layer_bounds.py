# -*- coding: utf-8 -*-

"""
***************************************************************************
    Hex_grid_from_layer_bounds.py
    ---------------------
    Date                 : November 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

##[Example scripts]=group
##input=vector
##cellsize=number 1000.0
##grid=output vector
from sextante.core.QGisLayers import QGisLayers

input = QGisLayers.getObjectFromUri(input)
centerx = (input.extent().xMinimum() + input.extent().xMaximum()) / 2
centery = (input.extent().yMinimum() + input.extent().yMaximum()) / 2
width = (input.extent().xMaximum() - input.extent().xMinimum())
height = (input.extent().yMaximum() - input.extent().yMinimum())
sextante.runalg("qgis:creategrid", cellsize, cellsize, width, height, centerx, centery, 3, grid)
