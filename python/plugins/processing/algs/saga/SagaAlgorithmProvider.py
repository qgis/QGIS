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
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (QgsProcessingProvider,
                       QgsProcessingUtils,
                       QgsMessageLog)
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.tools.system import isWindows, isMac

from .SagaAlgorithm import SagaAlgorithm
from .SplitRGBBands import SplitRGBBands
from . import SagaUtils

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class SagaAlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.algs = []

    def load(self):
        ProcessingConfig.settingIcons[self.name()] = self.icon()
        ProcessingConfig.addSetting(Setting("SAGA", 'ACTIVATE_SAGA',
                                            self.tr('Activate'), True))
        ProcessingConfig.addSetting(Setting("SAGA",
                                            SagaUtils.SAGA_IMPORT_EXPORT_OPTIMIZATION,
                                            self.tr('Enable SAGA Import/Export optimizations'), False))
        ProcessingConfig.addSetting(Setting("SAGA",
                                            SagaUtils.SAGA_LOG_COMMANDS,
                                            self.tr('Log execution commands'), True))
        ProcessingConfig.addSetting(Setting("SAGA",
                                            SagaUtils.SAGA_LOG_CONSOLE,
                                            self.tr('Log console output'), True))
        ProcessingConfig.readSettings()
        self.refreshAlgorithms()
        return True

    def unload(self):
        ProcessingConfig.removeSetting('ACTIVATE_SAGA')
        ProcessingConfig.removeSetting(SagaUtils.SAGA_LOG_CONSOLE)
        ProcessingConfig.removeSetting(SagaUtils.SAGA_LOG_COMMANDS)

    def isActive(self):
        return ProcessingConfig.getSetting('ACTIVATE_SAGA')

    def setActive(self, active):
        ProcessingConfig.setSettingValue('ACTIVATE_SAGA', active)

    def loadAlgorithms(self):
        version = SagaUtils.getInstalledVersion(True)
        if version is None:
            QgsMessageLog.logMessage(self.tr('Problem with SAGA installation: SAGA was not found or is not correctly installed'),
                                     self.tr('Processing'), QgsMessageLog.CRITICAL)
            return

        if not version.startswith('2.3.'):
            QgsMessageLog.logMessage(self.tr('Problem with SAGA installation: unsupported SAGA version found.'),
                                     self.tr('Processing'),
                                     QgsMessageLog.CRITICAL)
            return

        self.algs = []
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith('txt'):
                try:
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name().strip() != '':
                        self.algs.append(alg)
                    else:
                        QgsMessageLog.logMessage(self.tr('Could not open SAGA algorithm: {}'.format(descriptionFile)),
                                                 self.tr('Processing'), QgsMessageLog.CRITICAL)
                except Exception as e:
                    QgsMessageLog.logMessage(self.tr('Could not open SAGA algorithm: {}\n{}'.format(descriptionFile, str(e))),
                                             self.tr('Processing'), QgsMessageLog.CRITICAL)

        self.algs.append(SplitRGBBands())
        for a in self.algs:
            self.addAlgorithm(a)

    def name(self):
        version = SagaUtils.getInstalledVersion()
        return 'SAGA ({})'.format(version) if version is not None else 'SAGA'

    def id(self):
        return 'saga'

    def defaultVectorFileExtension(self, hasGeometry=True):
        return 'shp'

    def defaultRasterFileExtension(self):
        return 'sdat'

    def supportedOutputTableExtensions(self):
        return ['dbf']

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'saga.png'))

    def tr(self, string, context=''):
        if context == '':
            context = 'SagaAlgorithmProvider'
        return QCoreApplication.translate(context, string)
