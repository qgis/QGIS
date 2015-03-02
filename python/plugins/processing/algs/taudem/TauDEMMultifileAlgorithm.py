# -*- coding: utf-8 -*-

"""
***************************************************************************
    TauDEMMultifileAlgorithm.py
    ---------------------
    Date                 : March 2015
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
__date__ = 'March 2015'
__copyright__ = '(C) 2015, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from PyQt4.QtGui import QIcon

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithmExecutionException import \
    GeoAlgorithmExecutionException

from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import getParameterFromString
from processing.core.outputs import getOutputFromString

from TauDEMUtils import TauDEMUtils


class TauDEMMultifileAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()

    def getCopy(self):
        newone = TauDEMMultifileAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/taudem.png')

    def defineCharacteristicsFromFile(self):
        lines = open(self.descriptionFile)
        line = lines.readline().strip('\n').strip()
        self.name = line
        line = lines.readline().strip('\n').strip()
        self.cmdName = line
        line = lines.readline().strip('\n').strip()
        self.group = line

        line = lines.readline().strip('\n').strip()
        while line != '':
            try:
                line = line.strip('\n').strip()
                if line.startswith('Parameter'):
                    param = getParameterFromString(line)
                    self.addParameter(param)
                else:
                    self.addOutput(getOutputFromString(line))
                line = lines.readline().strip('\n').strip()
            except Exception, e:
                ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                    self.tr('Could not load TauDEM algorithm: %s\n%s' % (self.descriptionFile, line)))
                raise e
        lines.close()

    def processAlgorithm(self, progress):
        commands = []
        commands.append(os.path.join(TauDEMUtils.mpiexecPath(), 'mpiexec'))

        processNum = int(ProcessingConfig.getSetting(TauDEMUtils.MPI_PROCESSES))
        if processNum <= 0:
            raise GeoAlgorithmExecutionException(
                self.tr('Wrong number of MPI processes used. Please set '
                        'correct number before running TauDEM algorithms.'))

        commands.append('-n')
        commands.append(str(processNum))
        commands.append(os.path.join(TauDEMUtils.taudemMultifilePath(), self.cmdName))

        for param in self.parameters:
            if param.value is None or param.value == '':
                continue
            if isinstance(param, ParameterNumber):
                commands.append(param.name)
                commands.append(str(param.value))
            if isinstance(param, (ParameterFile, ParameterVector)):
                commands.append(param.name)
                commands.append(param.value)
            elif isinstance(param, ParameterBoolean):
                if param.value and str(param.value).lower() == 'false':
                    commands.append(param.name)
            elif isinstance(param, ParameterString):
                commands.append(param.name)
                commands.append(str(param.value))

        for out in self.outputs:
            commands.append(out.name)
            commands.append(out.value)

        TauDEMUtils.executeTauDEM(commands, progress)
