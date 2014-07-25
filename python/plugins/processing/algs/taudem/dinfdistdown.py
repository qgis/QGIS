# -*- coding: utf-8 -*-

"""
***************************************************************************
    dinfdistdown.py
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

from PyQt4.QtGui import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithmExecutionException import \
    GeoAlgorithmExecutionException

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputRaster

from processing.tools.system import *

from TauDEMUtils import TauDEMUtils


class DinfDistDown(GeoAlgorithm):

    DINF_FLOW_DIR_GRID = 'DINF_FLOW_DIR_GRID'
    PIT_FILLED_GRID = 'PIT_FILLED_GRID'
    STREAM_GRID = 'STREAM_GRID'
    WEIGHT_PATH_GRID = 'WEIGHT_PATH_GRID'
    STAT_METHOD = 'STAT_METHOD'
    DIST_METHOD = 'DIST_METHOD'
    EDGE_CONTAM = 'EDGE_CONTAM'

    DIST_DOWN_GRID = 'DIST_DOWN_GRID'

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
        self.name = 'D-Infinity Distance Down'
        self.cmdName = 'dinfdistdown'
        self.group = 'Specialized Grid Analysis tools'

        self.addParameter(ParameterRaster(self.DINF_FLOW_DIR_GRID,
                          'D-Infinity Flow Direction Grid', False))
        self.addParameter(ParameterRaster(self.PIT_FILLED_GRID,
                          'Pit Filled Elevation Grid', False))
        self.addParameter(ParameterRaster(self.STREAM_GRID,
                          'Stream Raster Grid', False))
        self.addParameter(ParameterRaster(self.WEIGHT_PATH_GRID,
                          'Weight Path Grid', True))
        self.addParameter(ParameterSelection(self.STAT_METHOD,
                          'Statistical Method', self.STATISTICS, 2))
        self.addParameter(ParameterSelection(self.DIST_METHOD,
                          'Distance Method', self.DISTANCE, 1))
        self.addParameter(ParameterBoolean(self.EDGE_CONTAM,
                          'Check for edge contamination', True))

        self.addOutput(OutputRaster(self.DIST_DOWN_GRID,
                       'D-Infinity Drop to Stream Grid'))

    def processAlgorithm(self, progress):
        commands = []
        commands.append(os.path.join(TauDEMUtils.mpiexecPath(), 'mpiexec'))

        processNum = ProcessingConfig.getSetting(TauDEMUtils.MPI_PROCESSES)
        if processNum <= 0:
            raise GeoAlgorithmExecutionException('Wrong number of MPI \
                processes used.\nPlease set correct number before running \
                TauDEM algorithms.')

        commands.append('-n')
        commands.append(str(processNum))
        commands.append(os.path.join(TauDEMUtils.taudemPath(), self.cmdName))
        commands.append('-ang')
        commands.append(self.getParameterValue(self.DINF_FLOW_DIR_GRID))
        commands.append('-fel')
        commands.append(self.getParameterValue(self.PIT_FILLED_GRID))
        commands.append('-src')
        commands.append(self.getParameterValue(self.STREAM_GRID))
        wg = self.getParameterValue(self.WEIGHT_PATH_GRID)
        if wg is not None:
            commands.append('-wg')
            commands.append(self.getParameterValue(self.WEIGHT_PATH_GRID))
        commands.append('-m')
        commands.append(str(self.STAT_DICT[self.getParameterValue(
            self.STAT_METHOD)]))
        commands.append(str(self.DIST_DICT[self.getParameterValue(
            self.DIST_METHOD)]))
        if str(self.getParameterValue(self.EDGE_CONTAM)).lower() == 'false':
            commands.append('-nc')
        commands.append('-dd')
        commands.append(self.getOutputValue(self.DIST_DOWN_GRID))

        loglines = []
        loglines.append('TauDEM execution command')
        for line in commands:
            loglines.append(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)

        TauDEMUtils.executeTauDEM(commands, progress)
