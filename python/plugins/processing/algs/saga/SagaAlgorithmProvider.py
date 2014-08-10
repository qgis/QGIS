# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaAlgorithmProvider.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.ProcessingLog import ProcessingLog
from SagaAlgorithm import SagaAlgorithm
from SplitRGBBands import SplitRGBBands
from RasterCalculator import RasterCalculator
from SagaUtils import SagaUtils
from processing.tools.system import *


class SagaAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = True

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        if SagaUtils.findSagaFolder() is None:
            ProcessingConfig.addSetting(Setting(self.getDescription(),
                                        SagaUtils.SAGA_208,
                                        'Use SAGA 2.0.8 syntax', not isMac()))
            if isWindows() or isMac():
                ProcessingConfig.addSetting(Setting(self.getDescription(),
                                            SagaUtils.SAGA_FOLDER, 'SAGA folder', ''))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
                                    SagaUtils.SAGA_IMPORT_EXPORT_OPTIMIZATION,
                                    'Enable SAGA Import/Export optimizations',
                                    False))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
                                    SagaUtils.SAGA_LOG_COMMANDS,
                                    'Log execution commands', True))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
                                    SagaUtils.SAGA_LOG_CONSOLE,
                                    'Log console output', True))

    def unload(self):
        AlgorithmProvider.unload(self)
        if isWindows():
            ProcessingConfig.removeSetting(SagaUtils.SAGA_FOLDER)

        ProcessingConfig.removeSetting(SagaUtils.SAGA_LOG_CONSOLE)
        ProcessingConfig.removeSetting(SagaUtils.SAGA_LOG_COMMANDS)


    def _loadAlgorithms(self):
        self.algs = []
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith('txt'):
                if not SagaUtils.isSaga208():
                    if descriptionFile.startswith('2.0.8'):
                        continue
                else:
                    if descriptionFile.startswith('2.1'):
                        continue
                try:
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != '':
                        self.algs.append(alg)
                    else:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                'Could not open SAGA algorithm: '
                                + descriptionFile)
                except Exception, e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                            'Could not open SAGA algorithm: '
                            + descriptionFile + '\n' + str(e))
        self.algs.append(SplitRGBBands())
        self.algs.append(RasterCalculator())

    def getDescription(self):
        return 'SAGA'

    def getName(self):
        return 'saga'

    def getSupportedOutputVectorLayerExtensions(self):
        return ['shp']

    def getSupportedOutputRasterLayerExtensions(self):
        return ['tif']

    def getSupportedOutputTableLayerExtensions(self):
        return ['dbf']

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/saga.png')
