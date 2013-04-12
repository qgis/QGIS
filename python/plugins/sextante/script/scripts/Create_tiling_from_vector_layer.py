# -*- coding: utf-8 -*-

"""
***************************************************************************
    Create_tiling_from_vector_layer.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'January 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

##[Example scripts]=group
##input=vector
##numpolygons=number 10
##polygons=output vector

input = sextante.getobject(input)
centerx = (input.extent().xMinimum() + input.extent().xMaximum()) / 2
centery = (input.extent().yMinimum() + input.extent().yMaximum()) / 2
width = (input.extent().xMaximum() - input.extent().xMinimum())
cellsize = width / numpolygons
height = (input.extent().yMaximum() - input.extent().yMinimum())
sextante.runalg("qgis:creategrid", cellsize, height, width, height, centerx, centery, 1, polygons)