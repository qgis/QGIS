# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterCalculator.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2014 by Victor Olaya
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
from processing.core.parameters import ParameterMultipleInput
from processing.algs.saga.SagaAlgorithm import SagaAlgorithm
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterString
from processing.algs.saga.SagaGroupNameDecorator import SagaGroupNameDecorator

__author__ = 'Victor Olaya'
__date__ = 'May 2014'
__copyright__ = '(C) 2014, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4 import QtGui
from processing.core.parameters import ParameterRaster
from processing.core.outputs import OutputRaster
from processing.tools.system import *


class RasterCalculator(SagaAlgorithm):


    FORMULA = "FORMULA"
    GRIDS = 'GRIDS'
    XGRIDS = 'XGRIDS'
    RESULT = "RESULT"

    def __init__(self):
        self.allowUnmatchingGridExtents = True
        self.hardcodedStrings = []
        GeoAlgorithm.__init__(self)

    def getCopy(self):
        newone = RasterCalculator()
        newone.provider = self.provider
        return newone

    def defineCharacteristics(self):
        self.name = 'Raster calculator'
        self.cmdname = 'Grid Calculator'
        self.undecoratedGroup = "grid_calculus"
        self.group = SagaGroupNameDecorator.getDecoratedName(self.undecoratedGroup)
        self.addParameter(ParameterRaster(self.GRIDS, 'Main input layers'))
        self.addParameter(ParameterMultipleInput(self.XGRIDS, 'Additional layers',
                          ParameterMultipleInput.TYPE_RASTER, True))
        self.addParameter(ParameterString(self.FORMULA, "Formula"))
        self.addOutput(OutputRaster(self.RESULT, "Result"))


    #===========================================================================
    # def processAlgorithm(self, progress):
    #     xgrids = self.getParameterValue(self.XGRIDS)
    #     layers = xgrids.split(';')
    #     grid = layers[0]
    #     self.setParameterValue(self.GRIDS, grid)
    #     xgrids = ";".join(layers[1:])
    #     if xgrids == "": xgrids = None
    #     self.setParameterValue(self.XGRIDS, xgrids)
    #     SagaAlgorithm.processAlgorithm(self, progress)
    #===========================================================================
