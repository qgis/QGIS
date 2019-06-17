# -*- coding: utf-8 -*-

"""
***************************************************************************
    self.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Alexander Bruy
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
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterCrs,
                       QgsProcessingOutputRasterLayer)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class AssignProjection(GdalAlgorithm):

    INPUT = 'INPUT'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterCrs(self.CRS,
                                                    self.tr('Desired CRS')))

        self.addOutput(QgsProcessingOutputRasterLayer(self.OUTPUT,
                                                      self.tr('Layer with projection')))

    def name(self):
        return 'assignprojection'

    def displayName(self):
        return self.tr('Assign projection')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'projection-add.png'))

    def group(self):
        return self.tr('Raster projections')

    def groupId(self):
        return 'rasterprojections'

    def commandName(self):
        return 'gdal_edit'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        fileName = inLayer.source()

        crs = self.parameterAsCrs(parameters, self.CRS, context)

        arguments = []
        arguments.append('-a_srs')
        arguments.append(GdalUtils.gdal_crs_string(crs))

        arguments.append(fileName)

        if isWindows():
            commands = ["python3", "-m", self.commandName()]
        else:
            commands = [self.commandName() + '.py']

        commands.append(GdalUtils.escapeAndJoin(arguments))

        self.setOutputValue(self.OUTPUT, fileName)

        return commands
