# -*- coding: utf-8 -*-

"""
***************************************************************************
    dropanalysis_multi.py
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
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputFile

from TauDEMUtils import TauDEMUtils


class DropAnalysisMulti(GeoAlgorithm):

    PIT_FILLED_GRID = 'PIT_FILLED_GRID'
    D8_CONTRIB_AREA_GRID = 'D8_CONTRIB_AREA_GRID'
    D8_FLOW_DIR_GRID = 'D8_FLOW_DIR_GRID'
    ACCUM_STREAM_SOURCE_GRID = 'ACCUM_STREAM_SOURCE_GRID'
    OUTLETS_SHAPE = 'OUTLETS_SHAPE'
    MIN_TRESHOLD = 'MIN_TRESHOLD'
    MAX_THRESHOLD = 'MAX_THRESHOLD'
    TRESHOLD_NUM = 'TRESHOLD_NUM'
    STEP_TYPE = 'STEP_TYPE'

    DROP_ANALYSIS_FILE = 'DROP_ANALYSIS_FILE'

    STEPS = ['Logarithmic', 'Linear']

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/taudem.png')

    def defineCharacteristics(self):
        self.name = 'Stream Drop Analysis (multifile)'
        self.cmdName = 'dropanalysis'
        self.group = 'Stream Network Analysis tools'

        self.addParameter(ParameterFile(self.D8_CONTRIB_AREA_GRID,
            self.tr('D8 Contributing Area Grid'), True, False))
        self.addParameter(ParameterFile(self.D8_FLOW_DIR_GRID,
            self.tr('D8 Flow Direction Grid'), True, False))
        self.addParameter(ParameterFile(self.PIT_FILLED_GRID,
            self.tr('Pit Filled Elevation Grid'), True, False))
        self.addParameter(ParameterFile(self.ACCUM_STREAM_SOURCE_GRID,
            self.tr('Accumulated Stream Source Grid'), True, False))
        self.addParameter(ParameterVector(self.OUTLETS_SHAPE,
            self.tr('Outlets Shapefile'),
            [ParameterVector.VECTOR_TYPE_POINT], False))
        self.addParameter(ParameterNumber(self.MIN_TRESHOLD,
            self.tr('Minimum Threshold'), 0, None, 5))
        self.addParameter(ParameterNumber(self.MAX_THRESHOLD,
            self.tr('Maximum Threshold'), 0, None, 500))
        self.addParameter(ParameterNumber(self.TRESHOLD_NUM,
            self.tr('Number of Threshold Values'), 0, None, 10))
        self.addParameter(ParameterSelection(self.STEP_TYPE,
            self.tr('Spacing for Threshold Values'), self.STEPS, 0))

        self.addOutput(OutputFile(self.DROP_ANALYSIS_FILE,
            self.tr('D-Infinity Drop to Stream Grid')))

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
        commands.append('-ad8')
        commands.append(self.getParameterValue(self.D8_CONTRIB_AREA_GRID))
        commands.append('-p')
        commands.append(self.getParameterValue(self.D8_FLOW_DIR_GRID))
        commands.append('-fel')
        commands.append(self.getParameterValue(self.PIT_FILLED_GRID))
        commands.append('-ssa')
        commands.append(self.getParameterValue(self.ACCUM_STREAM_SOURCE_GRID))
        commands.append('-o')
        commands.append(self.getParameterValue(self.OUTLETS_SHAPE))
        commands.append('-par')
        commands.append(str(self.getParameterValue(self.MIN_TRESHOLD)))
        commands.append(str(self.getParameterValue(self.MAX_THRESHOLD)))
        commands.append(str(self.getParameterValue(self.TRESHOLD_NUM)))
        commands.append(str(self.getParameterValue(self.STEPS)))
        commands.append('-drp')
        commands.append(self.getOutputValue(self.DROP_ANALYSIS_FILE))

        TauDEMUtils.executeTauDEM(commands, progress)
