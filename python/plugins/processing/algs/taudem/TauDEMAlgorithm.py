# -*- coding: utf-8 -*-

"""
***************************************************************************
    TauDEMAlgorithm.py
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
from future import standard_library
standard_library.install_aliases()
from builtins import str

__author__ = 'Alexander Bruy'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.PyQt.QtGui import QIcon

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithmExecutionException import \
    GeoAlgorithmExecutionException

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import getParameterFromString
from processing.core.outputs import getOutputFromString

from .TauDEMUtils import TauDEMUtils

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class TauDEMAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()
        self._icon = None

    def getCopy(self):
        newone = TauDEMAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        if self._icon is None:
            self._icon = QIcon(os.path.join(pluginPath, 'images', 'taudem.svg'))
        return self._icon

    def defineCharacteristicsFromFile(self):
        with codecs.open(self.descriptionFile, encoding='utf-8') as f:
            line = f.readline().strip('\n').strip()
            self.name = line
            self.i18n_name = self.tr(line)
            line = f.readline().strip('\n').strip()
            self.cmdName = line
            line = f.readline().strip('\n').strip()
            self.group = line
            self.i18n_group = self.tr(line)

            line = f.readline().strip('\n').strip()
            while line != '':
                try:
                    line = line.strip('\n').strip()
                    if line.startswith('Parameter'):
                        param = getParameterFromString(line)
                        self.addParameter(param)
                    else:
                        self.addOutput(getOutputFromString(line))
                    line = f.readline().strip('\n').strip()
                except Exception as e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                           self.tr('Could not load TauDEM algorithm: {}\n{}'.format(self.descriptionFile, line)))
                    raise e

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
        commands.append(os.path.join(TauDEMUtils.taudemPath(), self.cmdName))

        for param in self.parameters:
            if param.value is None or param.value == '':
                continue
            if isinstance(param, ParameterNumber):
                commands.append(param.name)
                commands.append(str(param.value))
            if isinstance(param, (ParameterRaster, ParameterVector)):
                commands.append(param.name)
                commands.append(param.value)
            elif isinstance(param, ParameterBoolean):
                if not param.value:
                    commands.append(param.name)
            elif isinstance(param, ParameterString):
                commands.append(param.name)
                commands.append(str(param.value))

        for out in self.outputs:
            commands.append(out.name)
            commands.append(out.value)

        TauDEMUtils.executeTauDEM(commands, progress)
