# -*- coding: utf-8 -*-

"""
***************************************************************************
    Grass7AlgorithmProvider.py
    ---------------------
    Date                 : April 2014
    Copyright            : (C) 2014 by Victor Olaya
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
__date__ = 'April 2014'
__copyright__ = '(C) 2014, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessingProvider,
                       QgsVectorFileWriter,
                       QgsMessageLog,
                       QgsProcessingUtils)
from processing.core.ProcessingConfig import (ProcessingConfig, Setting)
from .Grass7Utils import Grass7Utils
from .Grass7Algorithm import Grass7Algorithm
from processing.tools.system import isWindows, isMac

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class Grass7AlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.algs = []

    def load(self):
        ProcessingConfig.settingIcons[self.name()] = self.icon()
        ProcessingConfig.addSetting(Setting(self.name(), 'ACTIVATE_GRASS7',
                                            self.tr('Activate'), True))
        if isMac():
            ProcessingConfig.addSetting(Setting(
                self.name(),
                Grass7Utils.GRASS_FOLDER, self.tr('GRASS7 folder'),
                Grass7Utils.grassPath(), valuetype=Setting.FOLDER))
        ProcessingConfig.addSetting(Setting(
            self.name(),
            Grass7Utils.GRASS_LOG_COMMANDS,
            self.tr('Log execution commands'), False))
        ProcessingConfig.addSetting(Setting(
            self.name(),
            Grass7Utils.GRASS_LOG_CONSOLE,
            self.tr('Log console output'), False))
        ProcessingConfig.addSetting(Setting(
            self.name(),
            Grass7Utils.GRASS_HELP_PATH,
            self.tr('Location of GRASS docs'),
            Grass7Utils.grassHelpPath()))
        # Add a setting for using v.external instead of v.in.ogr
        # But set it to False by default because some algorithms
        # can't be used with external data (need a solid v.in.ogr).
        ProcessingConfig.addSetting(Setting(
            self.name(),
            Grass7Utils.GRASS_USE_VEXTERNAL,
            self.tr('For vector layers, use v.external (faster) instead of v.in.ogr'),
            False))
        ProcessingConfig.readSettings()
        self.refreshAlgorithms()
        return True

    def unload(self):
        ProcessingConfig.removeSetting('ACTIVATE_GRASS7')
        if isMac():
            ProcessingConfig.removeSetting(Grass7Utils.GRASS_FOLDER)
        ProcessingConfig.removeSetting(Grass7Utils.GRASS_LOG_COMMANDS)
        ProcessingConfig.removeSetting(Grass7Utils.GRASS_LOG_CONSOLE)
        ProcessingConfig.removeSetting(Grass7Utils.GRASS_HELP_PATH)
        ProcessingConfig.removeSetting(Grass7Utils.GRASS_USE_VEXTERNAL)

    def isActive(self):
        return ProcessingConfig.getSetting('ACTIVATE_GRASS7')

    def setActive(self, active):
        ProcessingConfig.setSettingValue('ACTIVATE_GRASS7', active)

    def createAlgsList(self):
        algs = []
        folder = Grass7Utils.grassDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith('txt'):
                try:
                    alg = Grass7Algorithm(os.path.join(folder, descriptionFile))
                    if alg.name().strip() != '':
                        algs.append(alg)
                    else:
                        QgsMessageLog.logMessage(self.tr('Could not open GRASS GIS 7 algorithm: {0}').format(descriptionFile), self.tr('Processing'), Qgis.Critical)
                except Exception as e:
                    QgsMessageLog.logMessage(
                        self.tr('Could not open GRASS GIS 7 algorithm: {0}\n{1}').format(descriptionFile, str(e)), self.tr('Processing'), Qgis.Critical)
        return algs

    def loadAlgorithms(self):
        self.algs = self.createAlgsList()
        for a in self.algs:
            self.addAlgorithm(a)

    def name(self):
        return 'GRASS'

    def longName(self):
        version = Grass7Utils.installedVersion()
        return 'GRASS GIS ({})'.format(version) if version is not None else "GRASS GIS"

    def id(self):
        return 'grass7'

    def helpId(self):
        return 'grass7'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerGrass.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/providerGrass.svg")

    def defaultVectorFileExtension(self, hasGeometry=True):
        # By default,'gpkg', but if OGR has not been compiled with sqlite3, then
        # we take "SHP"
        if 'GPKG' in [o.driverName for o in
                      QgsVectorFileWriter.ogrDriverList()]:
            return 'gpkg'
        else:
            return 'shp' if hasGeometry else 'dbf'

    def supportsNonFileBasedOutput(self):
        """
        GRASS7 Provider doesn't support non file based outputs
        """
        return False

    def supportedOutputVectorLayerExtensions(self):
        # We use the same extensions than QGIS because:
        # - QGIS is using OGR like GRASS
        # - There are very chances than OGR version used in GRASS is
        # different from QGIS OGR version.
        return QgsVectorFileWriter.supportedFormatExtensions()

    def supportedOutputRasterLayerExtensions(self):
        return Grass7Utils.getSupportedOutputRasterExtensions()

    def canBeActivated(self):
        return not bool(Grass7Utils.checkGrassIsInstalled())

    def tr(self, string, context=''):
        if context == '':
            context = 'Grass7AlgorithmProvider'
        return QCoreApplication.translate(context, string)
