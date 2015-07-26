# -*- coding: utf-8 -*-

"""
***************************************************************************
    gridnet.py
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
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster

from TauDEMUtils import TauDEMUtils


class GridNet(GeoAlgorithm):

    D8_FLOW_DIR_GRID = 'D8_FLOW_DIR_GRID'
    OUTLETS_SHAPE = 'OUTLETS_SHAPE'
    MASK_GRID = 'MASK_GRID'
    THRESHOLD = 'THRESHOLD'

    LONGEST_LEN_GRID = 'LONGEST_LEN_GRID'
    TOTAL_LEN_GRID = 'TOTAL_LEN_GRID'
    STRAHLER_GRID = 'STRAHLER_GRID'

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/taudem.png')

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Grid Network')
        self.cmdName = 'gridnet'
        self.group, self.i18n_group = self.trAlgorithm('Basic Grid Analysis tools')

        self.addParameter(ParameterRaster(self.D8_FLOW_DIR_GRID,
            self.tr('D8 Flow Direction Grid'), False))
        self.addParameter(ParameterVector(self.OUTLETS_SHAPE,
            self.tr('Outlets Shapefile'),
            [ParameterVector.VECTOR_TYPE_POINT], True))
        self.addParameter(ParameterRaster(self.MASK_GRID,
            self.tr('Mask Grid'), True))
        self.addParameter(ParameterNumber(self.THRESHOLD,
            self.tr('Mask Threshold'), 0, None, 100))

        self.addOutput(OutputRaster(self.LONGEST_LEN_GRID,
            self.tr('Longest Upslope Length Grid')))
        self.addOutput(OutputRaster(self.TOTAL_LEN_GRID,
            self.tr('Total Upslope Length Grid')))
        self.addOutput(OutputRaster(self.STRAHLER_GRID,
            self.tr('Strahler Network Order Grid')))

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
        commands.append('-p')
        commands.append(self.getParameterValue(self.D8_FLOW_DIR_GRID))
        param = self.getParameterValue(self.OUTLETS_SHAPE)
        if param is not None:
            commands.append('-o')
            commands.append(param)
        param = self.getParameterValue(self.MASK_GRID)
        if param is not None:
            commands.append('-mask')
            commands.append(param)
            commands.append('-thresh')
            commands.append(self.getParameterValue(self.THRESHOLD))

        commands.append('-plen')
        commands.append(self.getOutputValue(self.LONGEST_LEN_GRID))
        commands.append('-tlen')
        commands.append(self.getOutputValue(self.TOTAL_LEN_GRID))
        commands.append('-gord')
        commands.append(self.getOutputValue(self.STRAHLER_GRID))

        TauDEMUtils.executeTauDEM(commands, progress)
