# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteRasterWriter.py
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
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import *
import numpy
from PyQt4.QtCore import *

class SextanteRasterWriter:

    NODATA = -99999.0
    
    def __init__(self, fileName, minx, miny, maxx, maxy, cellsize, nbands, crs):
        self.fileName = fileName
        self.nx = (maxx - minx) / float(cellsize)
        self.ny = (maxy - miny) / float(cellsize)
        self.matrix = numpy.empty(shape=(self.nx, self.ny, nbands))
        self.matrix[:] = self.NODATA
        self.cellsize = cellsize
        self.crs = crs    
        
    def setValue(self, value, x, y, band = 0):
        try:
            self.matrix[x, y, band] = value
        except IndexError:
            pass
        
    def getValue(self, x, y, band = 0):        
        try:
            return matrix[x, y, band]
        except IndexError:
            return self.NODATA        
        
    def close(self):
        #todo
        pass
