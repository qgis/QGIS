# -*- coding: utf-8 -*-

"""
***************************************************************************
    hillshade.py
    ---------------------
    Date                 : October 2013
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
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from PyQt4.QtGui import *

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import *


class hillshade(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    ZEVENBERGEN = 'ZEVENBERGEN'
    Z_FACTOR = 'Z_FACTOR'
    SCALE = 'SCALE'
    AZIMUTH = 'AZIMUTH'
    ALTITUDE = 'ALTITUDE'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Hillshade'
        self.group = '[GDAL] Analysis'
        self.addParameter(ParameterRaster(self.INPUT, 'Input layer'))
        self.addParameter(ParameterNumber(self.BAND, 'Band number', 1, 99, 1))
        self.addParameter(ParameterBoolean(self.COMPUTE_EDGES, 'Compute edges',
                          False))
        self.addParameter(ParameterBoolean(self.ZEVENBERGEN,
                "Use Zevenbergen&Thorne formula (instead of the Horn's one)",
                False))
        self.addParameter(ParameterNumber(self.Z_FACTOR,
                          'Z factor (vertical exaggeration)', 0.0,
                          99999999.999999, 1.0))
        self.addParameter(ParameterNumber(self.SCALE,
                          'Scale (ratio of vert. units to horiz.)', 0.0,
                          99999999.999999, 1.0))
        self.addParameter(ParameterNumber(self.AZIMUTH,
                          'Azimuth of the light', 0.0, 359.0, 315.0))
        self.addParameter(ParameterNumber(self.ALTITUDE,
                          'Altitude of the light', 0.0, 99999999.999999, 45.0))

        self.addOutput(OutputRaster(self.OUTPUT, 'Output file'))

    def processAlgorithm(self, progress):
        arguments = ['hillshade']
        arguments.append(unicode(self.getParameterValue(self.INPUT)))
        arguments.append(unicode(self.getOutputValue(self.OUTPUT)))

        arguments.append('-b')
        arguments.append(str(self.getParameterValue(self.BAND)))
        arguments.append('-z')
        arguments.append(str(self.getParameterValue(self.Z_FACTOR)))
        arguments.append('-s')
        arguments.append(str(self.getParameterValue(self.SCALE)))
        arguments.append('-az')
        arguments.append(str(self.getParameterValue(self.AZIMUTH)))
        arguments.append('-alt')
        arguments.append(str(self.getParameterValue(self.ALTITUDE)))

        if self.getParameterValue(self.COMPUTE_EDGES):
            arguments.append('-compute_edges')

        if self.getParameterValue(self.ZEVENBERGEN):
            arguments.append('-alg')
            arguments.append('ZevenbergenThorne')

        GdalUtils.runGdal(['gdaldem',
                          GdalUtils.escapeAndJoin(arguments)], progress)
