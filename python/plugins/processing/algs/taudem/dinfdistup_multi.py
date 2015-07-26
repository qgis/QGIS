# -*- coding: utf-8 -*-

"""
***************************************************************************
    dinfdistup_multi.py
    ---------------------
    Date                 : March 2015
    Copyright            : (C) 2015 by Alexander Bruy
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
__date__ = 'March 2015'
__copyright__ = '(C) 2015, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from PyQt4.QtGui import QIcon

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithmExecutionException import \
    GeoAlgorithmExecutionException

from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputDirectory

from TauDEMUtils import TauDEMUtils


class DinfDistUpMulti(GeoAlgorithm):

    DINF_FLOW_DIR_GRID = 'DINF_FLOW_DIR_GRID'
    PIT_FILLED_GRID = 'PIT_FILLED_GRID'
    SLOPE_GRID = 'SLOPE_GRID'
    THRESHOLD = 'THRESHOLD'
    STAT_METHOD = 'STAT_METHOD'
    DIST_METHOD = 'DIST_METHOD'
    EDGE_CONTAM = 'EDGE_CONTAM'

    DIST_UP_GRID = 'DIST_UP_GRID'

    STATISTICS = ['Minimum', 'Maximum', 'Average']
    STAT_DICT = {0: 'min', 1: 'max', 2: 'ave'}

    DISTANCE = ['Pythagoras', 'Horizontal', 'Vertical', 'Surface']
    DIST_DICT = {
        0: 'p',
        1: 'h',
        2: 'v',
        3: 's',
    }

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/taudem.png')

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('D-Infinity Distance Up (multifile)')
        self.cmdName = 'dinfdistup'
        self.group, self.i18n_group = self.trAlgorithm('Specialized Grid Analysis tools')

        self.addParameter(ParameterFile(self.DINF_FLOW_DIR_GRID,
            self.tr('D-Infinity Flow Direction Grid'), True, False))
        self.addParameter(ParameterFile(self.PIT_FILLED_GRID,
            self.tr('Pit Filled Elevation Grid'), True, False))
        self.addParameter(ParameterFile(self.SLOPE_GRID,
            self.tr('Slope Grid'), True, False))
        self.addParameter(ParameterSelection(self.STAT_METHOD,
            self.tr('Statistical Method'), self.STATISTICS, 2))
        self.addParameter(ParameterSelection(self.DIST_METHOD,
            self.tr('Distance Method'), self.DISTANCE, 1))
        self.addParameter(ParameterNumber(self.THRESHOLD,
            self.tr('Proportion Threshold'), 0, None, 0.5))
        self.addParameter(ParameterBoolean(self.EDGE_CONTAM,
            self.tr('Check for edge contamination'), True))

        self.addOutput(OutputDirectory(self.DIST_UP_GRID,
            self.tr('D-Infinity Distance Up')))

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
        commands.append(os.path.join(TauDEMUtils.taudemMultifilePath(), self.cmdName))
        commands.append('-ang')
        commands.append(self.getParameterValue(self.DINF_FLOW_DIR_GRID))
        commands.append('-fel')
        commands.append(self.getParameterValue(self.PIT_FILLED_GRID))
        commands.append('-m')
        commands.append(str(self.STAT_DICT[self.getParameterValue(
            self.STAT_METHOD)]))
        commands.append(str(self.DIST_DICT[self.getParameterValue(
            self.DIST_METHOD)]))
        commands.append('-thresh')
        commands.append(str(self.getParameterValue(self.THRESHOLD)))
        if not self.getParameterValue(self.EDGE_CONTAM):
            commands.append('-nc')
        commands.append('-du')
        commands.append(self.getOutputValue(self.DIST_UP_GRID))

        TauDEMUtils.executeTauDEM(commands, progress)
