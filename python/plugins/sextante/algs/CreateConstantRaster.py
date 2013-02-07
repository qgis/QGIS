from sextante.core.SextanteRasterWriter import SextanteRasterWriter
from sextante.parameters.ParameterRaster import ParameterRaster



# -*- coding: utf-8 -*-

"""
***************************************************************************
    AutoincrementalField.py
    ---------------------
    Date                 : August 2012
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4.QtCore import *
from qgis.core import *
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.outputs.OutputRaster import OutputRaster
from sextante.core.QGisLayers import QGisLayers

class CreateConstantRaster(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NUMBER = "NUMBER"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")
    #===========================================================================

    def processAlgorithm(self, progress):
        output = self.getOutputFromName(self.OUTPUT)
        value = self.getOutputValue(self.NUMBER)
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        cellsize = (layer.extent().xMaximum() - layer.extent().xMinimum())/layer.width()
        w = SextanteRasterWriter(output.getCompatibleFileName(self), layer.extent().xMinimum(), layer.extent().yMinimum(), layer.extent().xMaximum(),
                                 layer.extent().yMaximum(), cellsize, 1, self.crs)
        w.matrix[:] = value
        w.close()

    def defineCharacteristics(self):
        self.name = "Create constant raster layer"
        self.group = "Raster tools"
        self.addParameter(ParameterRaster(self.INPUT, "Reference layer"))
        self.addParameter(ParameterNumber(self.NUMBER, "Constant value", default = 1.0));
        self.addOutput(OutputRaster(self.OUTPUT, "Output layer"))


