# -*- coding: utf-8 -*-

"""
***************************************************************************
    slopearea.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Alexander Bruy
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
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from PyQt4.QtGui import QIcon

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithmExecutionException import \
    GeoAlgorithmExecutionException

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster

from TauDEMUtils import TauDEMUtils


class SlopeArea(GeoAlgorithm):

    SLOPE_GRID = 'SLOPE_GRID'
    AREA_GRID = 'AREA_GRID'
    SLOPE_EXPONENT = 'SLOPE_EXPONENT'
    AREA_EXPONENT = 'AREA_EXPONENT'

    SLOPE_AREA_GRID = 'SLOPE_AREA_GRID'

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/taudem.png')

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Slope Area Combination')
        self.cmdName = 'slopearea'
        self.group, self.i18n_group = self.trAlgorithm('Stream Network Analysis tools')

        self.addParameter(ParameterRaster(self.SLOPE_GRID,
            self.tr('Slope Grid'), False))
        self.addParameter(ParameterRaster(self.AREA_GRID,
            self.tr('Contributing Area Grid'), False))
        self.addParameter(ParameterNumber(self.SLOPE_EXPONENT,
            self.tr('Slope Exponent'), 0, None, 2))
        self.addParameter(ParameterNumber(self.AREA_EXPONENT,
            self.tr('Area Exponent'), 0, None, 1))

        self.addOutput(OutputRaster(self.SLOPE_AREA_GRID,
            self.tr('Slope Area Grid')))

    def processAlgorithm(self, progress):
        commands = []
        commands.append(os.path.join(TauDEMUtils.mpiexecPath(), 'mpiexec'))

        processNum = ProcessingConfig.getSetting(TauDEMUtils.MPI_PROCESSES)
        if processNum <= 0:
            raise GeoAlgorithmExecutionException(
                self.tr('Wrong number of MPI processes used. Please set '
                        'correct number before running TauDEM algorithms.'))

        commands.append('-n')
        commands.append(str(processNum))
        commands.append(os.path.join(TauDEMUtils.taudemPath(), self.cmdName))
        commands.append('-slp')
        commands.append(self.getParameterValue(self.SLOPE_GRID))
        commands.append('-sca')
        commands.append(self.getParameterValue(self.AREA_GRID))
        commands.append('-par')
        commands.append(str(self.getParameterValue(self.SLOPE_EXPONENT)))
        commands.append(str(self.getParameterValue(self.AREA_EXPONENT)))
        commands.append('-sa')
        commands.append(self.getOutputValue(self.SLOPE_AREA_GRID))

        TauDEMUtils.executeTauDEM(commands, progress)
