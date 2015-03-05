# -*- coding: utf-8 -*-

"""
***************************************************************************
    dinftranslimaccum2_multi.py
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
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputDirectory

from TauDEMUtils import TauDEMUtils


class DinfTransLimAccum2Multi(GeoAlgorithm):

    DINF_FLOW_DIR_GRID = 'DINF_FLOW_DIR_GRID'
    SUPPLY_GRID = 'SUPPLY_GRID'
    CAPACITY_GRID = 'CAPACITY_GRID'
    IN_CONCENTR_GRID = 'IN_CONCENTR_GRID'
    OUTLETS_SHAPE = 'OUTLETS_SHAPE'
    EDGE_CONTAM = 'EDGE_CONTAM'

    TRANSP_LIM_ACCUM_GRID = 'TRANSP_LIM_ACCUM_GRID'
    DEPOSITION_GRID = 'DEPOSITION_GRID'
    OUT_CONCENTR_GRID = 'OUT_CONCENTR_GRID'

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/taudem.png')

    def defineCharacteristics(self):
        self.name = 'D-Infinity Transport Limited Accumulation - 2 (multifile)'
        self.cmdName = 'dinftranslimaccum'
        self.group = 'Specialized Grid Analysis tools'

        self.addParameter(ParameterFile(self.DINF_FLOW_DIR_GRID,
            self.tr('D-Infinity Flow Direction Grid'), True, False))
        self.addParameter(ParameterFile(self.SUPPLY_GRID,
            self.tr('Supply Grid'), True, False))
        self.addParameter(ParameterFile(self.CAPACITY_GRID,
            self.tr('Transport Capacity Grid'), True, False))
        self.addParameter(ParameterFile(self.IN_CONCENTR_GRID,
            self.tr('Input Concentration Grid'), True, False))
        self.addParameter(ParameterVector(self.OUTLETS_SHAPE,
            self.tr('Outlets Shapefile'),
            [ParameterVector.VECTOR_TYPE_POINT], True))
        self.addParameter(ParameterBoolean(self.EDGE_CONTAM,
            self.tr('Check for edge contamination'), True))

        self.addOutput(OutputDirectory(self.TRANSP_LIM_ACCUM_GRID,
            self.tr('Transport Limited Accumulation Grid')))
        self.addOutput(OutputDirectory(self.DEPOSITION_GRID,
            self.tr('Deposition Grid')))
        self.addOutput(OutputDirectory(self.OUT_CONCENTR_GRID,
            self.tr('Output Concentration Grid')))

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
        commands.append('-tsup')
        commands.append(self.getParameterValue(self.SUPPLY_GRID))
        commands.append('-tc')
        commands.append(self.getParameterValue(self.CAPACITY_GRID))
        commands.append('-cs')
        commands.append(self.getParameterValue(self.IN_CONCENTR_GRID))
        param = self.getParameterValue(self.OUTLETS_SHAPE)
        if param is not None:
            commands.append('-o')
            commands.append(param)
        if not self.getParameterValue(self.EDGE_CONTAM):
            commands.append('-nc')

        commands.append('-tla')
        commands.append(self.getOutputValue(self.TRANSP_LIM_ACCUM_GRID))
        commands.append('-tdep')
        commands.append(self.getOutputValue(self.DEPOSITION_GRID))
        commands.append('-ctpt')
        commands.append(self.getOutputValue(self.OUT_CONCENTR_GRID))

        TauDEMUtils.executeTauDEM(commands, progress)
