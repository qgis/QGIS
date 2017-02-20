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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from qgis.PyQt.QtGui import QIcon
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import isWindows, isMac

from .SagaAlgorithm import SagaAlgorithm
from .SplitRGBBands import SplitRGBBands
from . import SagaUtils

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class SagaAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        super().__init__()
        self.activate = True

    def initializeSettings(self):
        if (isWindows() or isMac()):
            ProcessingConfig.addSetting(Setting("SAGA",
                                                SagaUtils.SAGA_FOLDER, self.tr('SAGA folder'),
                                                '',
                                                valuetype=Setting.FOLDER))
        ProcessingConfig.addSetting(Setting("SAGA",
                                            SagaUtils.SAGA_IMPORT_EXPORT_OPTIMIZATION,
                                            self.tr('Enable SAGA Import/Export optimizations'), False))
        ProcessingConfig.addSetting(Setting("SAGA",
                                            SagaUtils.SAGA_LOG_COMMANDS,
                                            self.tr('Log execution commands'), True))
        ProcessingConfig.addSetting(Setting("SAGA",
                                            SagaUtils.SAGA_LOG_CONSOLE,
                                            self.tr('Log console output'), True))
        ProcessingConfig.settingIcons["SAGA"] = self.icon()
        ProcessingConfig.addSetting(Setting("SAGA", "ACTIVATE_SAGA",
                                            self.tr('Activate'), self.activate))

    def unload(self):
        AlgorithmProvider.unload(self)
        if (isWindows() or isMac()):
            ProcessingConfig.removeSetting(SagaUtils.SAGA_FOLDER)

        ProcessingConfig.removeSetting(SagaUtils.SAGA_LOG_CONSOLE)
        ProcessingConfig.removeSetting(SagaUtils.SAGA_LOG_COMMANDS)

    def _loadAlgorithms(self):
        self.algs = []
        version = SagaUtils.getInstalledVersion(True)
        if version is None:
            ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                   self.tr('Problem with SAGA installation: SAGA was not found or is not correctly installed'))
            return

        if not version.startswith('2.3.'):
            ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                   self.tr('Problem with SAGA installation: unsupported SAGA version found.'))
            return

        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith('txt'):
                f = os.path.join(folder, descriptionFile)
                try:
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != '':
                        self.algs.append(alg)
                    else:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                               self.tr('Could not open SAGA algorithm: {}'.format(descriptionFile)))
                except Exception as e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                           self.tr('Could not open SAGA algorithm: {}\n{}'.format(descriptionFile, str(e))))

        self.algs.append(SplitRGBBands())

    def name(self):
        version = SagaUtils.getInstalledVersion()
        return 'SAGA ({})'.format(version) if version is not None else 'SAGA'

    def id(self):
        return 'saga'

    def getSupportedOutputVectorLayerExtensions(self):
        return ['shp']

    def getSupportedOutputRasterLayerExtensions(self):
        return ['sdat']

    def getSupportedOutputTableLayerExtensions(self):
        return ['dbf']

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'saga.png'))
