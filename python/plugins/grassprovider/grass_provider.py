"""
***************************************************************************
    grass_provider.py
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

import json
from typing import List
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessingAlgorithm,
                       QgsProcessingProvider,
                       QgsVectorFileWriter,
                       QgsMessageLog,
                       QgsRuntimeProfiler)
from processing.core.ProcessingConfig import (ProcessingConfig, Setting)
from grassprovider.grass_utils import GrassUtils
from grassprovider.grass_algorithm import GrassAlgorithm


class GrassProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()

    def load(self):
        with QgsRuntimeProfiler.profile('Grass Provider'):
            ProcessingConfig.settingIcons[self.name()] = self.icon()
            ProcessingConfig.addSetting(Setting(
                self.name(),
                GrassUtils.GRASS_LOG_COMMANDS,
                self.tr('Log execution commands'), False))
            ProcessingConfig.addSetting(Setting(
                self.name(),
                GrassUtils.GRASS_LOG_CONSOLE,
                self.tr('Log console output'), False))
            ProcessingConfig.addSetting(Setting(
                self.name(),
                GrassUtils.GRASS_HELP_URL,
                self.tr('Location of GRASS docs'), ''))
            # Add settings for using r.external/v.external instead of r.in.gdal/v.in.ogr
            # but set them to False by default because the {r,v}.external implementations
            # have some bugs on windows + there are algorithms that can't be used with
            # external data (need a solid r.in.gdal/v.in.ogr).
            # For more info have a look at e.g. https://trac.osgeo.org/grass/ticket/3927
            ProcessingConfig.addSetting(Setting(
                self.name(),
                GrassUtils.GRASS_USE_REXTERNAL,
                self.tr('For raster layers, use r.external (faster) instead of r.in.gdal'),
                False))
            ProcessingConfig.addSetting(Setting(
                self.name(),
                GrassUtils.GRASS_USE_VEXTERNAL,
                self.tr('For vector layers, use v.external (faster) instead of v.in.ogr'),
                False))
            ProcessingConfig.readSettings()
            self.refreshAlgorithms()

        return True

    def unload(self):
        ProcessingConfig.removeSetting(GrassUtils.GRASS_LOG_COMMANDS)
        ProcessingConfig.removeSetting(GrassUtils.GRASS_LOG_CONSOLE)
        ProcessingConfig.removeSetting(GrassUtils.GRASS_HELP_URL)
        ProcessingConfig.removeSetting(GrassUtils.GRASS_USE_REXTERNAL)
        ProcessingConfig.removeSetting(GrassUtils.GRASS_USE_VEXTERNAL)

    def parse_algorithms(self) -> List[QgsProcessingAlgorithm]:
        """
        Parses all algorithm sources and returns a list of all GRASS
        algorithms.
        """
        algs = []
        for folder in GrassUtils.grassDescriptionFolders():
            if (folder / 'algorithms.json').exists():
                # fast approach -- use aggregated JSON summary of algorithms
                with open(folder / 'algorithms.json', 'rt', encoding='utf8') as f_in:
                    algorithm_strings = f_in.read()

                algorithms_json = json.loads(algorithm_strings)
                for algorithm_json in algorithms_json:
                    try:
                        alg = GrassAlgorithm(
                            json_definition=algorithm_json,
                            description_folder=folder)
                        if alg.name().strip() != '':
                            algs.append(alg)
                        else:
                            QgsMessageLog.logMessage(self.tr('Could not open GRASS GIS algorithm: {0}').format(algorithm_json.get('name')), self.tr('Processing'), Qgis.MessageLevel.Critical)
                    except Exception as e:
                        QgsMessageLog.logMessage(
                            self.tr('Could not open GRASS GIS algorithm: {0}\n{1}').format(algorithm_json.get('name'), e), self.tr('Processing'), Qgis.MessageLevel.Critical)
            else:
                # slow approach - pass txt files one by one
                for descriptionFile in folder.glob('*.txt'):
                    try:
                        alg = GrassAlgorithm(
                            description_file=descriptionFile)
                        if alg.name().strip() != '':
                            algs.append(alg)
                        else:
                            QgsMessageLog.logMessage(self.tr('Could not open GRASS GIS algorithm: {0}').format(descriptionFile), self.tr('Processing'), Qgis.MessageLevel.Critical)
                    except Exception as e:
                        QgsMessageLog.logMessage(
                            self.tr('Could not open GRASS GIS algorithm: {0}\n{1}').format(descriptionFile, e), self.tr('Processing'), Qgis.MessageLevel.Critical)
        return algs

    def loadAlgorithms(self):
        version = GrassUtils.installedVersion(True)
        if version is None:
            QgsMessageLog.logMessage(self.tr('Problem with GRASS installation: GRASS was not found or is not correctly installed'),
                                     self.tr('Processing'), Qgis.MessageLevel.Critical)
            return

        for a in self.parse_algorithms():
            self.addAlgorithm(a)

    def name(self):
        return 'GRASS'

    def longName(self):
        version = GrassUtils.installedVersion()
        return 'GRASS GIS ({})'.format(version) if version is not None else "GRASS GIS"

    def id(self):
        return 'grass'

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
        GRASS Provider doesn't support non file based outputs
        """
        return False

    def supportedOutputVectorLayerExtensions(self):
        # We use the same extensions than QGIS because:
        # - QGIS is using OGR like GRASS
        # - There are very chances than OGR version used in GRASS is
        # different from QGIS OGR version.
        return QgsVectorFileWriter.supportedFormatExtensions()

    def supportedOutputRasterLayerExtensions(self):
        return GrassUtils.getSupportedOutputRasterExtensions()

    def canBeActivated(self):
        return not bool(GrassUtils.checkGrassIsInstalled())

    def tr(self, string, context=''):
        if context == '':
            context = 'Grass7AlgorithmProvider'
        return QCoreApplication.translate(context, string)
