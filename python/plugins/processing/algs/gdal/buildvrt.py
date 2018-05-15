# -*- coding: utf-8 -*-

"""
***************************************************************************
    merge.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Radoslaw Guzinski
    Email                : rmgu at dhi-gras dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Radoslaw Guzinski'
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Radoslaw Guzinski'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessing,
                       QgsProperty,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingUtils)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class buildvrt(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    RESOLUTION = 'RESOLUTION'
    SEPARATE = 'SEPARATE'
    PROJ_DIFFERENCE = 'PROJ_DIFFERENCE'

    RESOLUTION_OPTIONS = ['average', 'highest', 'lowest']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        class ParameterVrtDestination(QgsProcessingParameterRasterDestination):

            def __init__(self, name, description):
                super().__init__(name, description)

            def clone(self):
                copy = ParameterVrtDestination(self.name(), self.description())
                return copy

            def type(self):
                return 'vrt_destination'

            def defaultFileExtension(self):
                return 'vrt'

        self.addParameter(QgsProcessingParameterMultipleLayers(self.INPUT,
                                                               QCoreApplication.translate("ParameterVrtDestination", 'Input layers'),
                                                               QgsProcessing.TypeRaster))
        self.addParameter(QgsProcessingParameterEnum(self.RESOLUTION,
                                                     QCoreApplication.translate("ParameterVrtDestination", 'Resolution'),
                                                     options=self.RESOLUTION_OPTIONS,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterBoolean(self.SEPARATE,
                                                        QCoreApplication.translate("ParameterVrtDestination", 'Layer stack'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterBoolean(self.PROJ_DIFFERENCE,
                                                        QCoreApplication.translate("ParameterVrtDestination", 'Allow projection difference'),
                                                        defaultValue=False))
        self.addParameter(ParameterVrtDestination(self.OUTPUT, QCoreApplication.translate("ParameterVrtDestination", 'Virtual')))

    def name(self):
        return 'buildvirtualraster'

    def displayName(self):
        return QCoreApplication.translate("buildvrt", 'Build Virtual Raster')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'vrt.png'))

    def group(self):
        return QCoreApplication.translate("buildvrt", 'Raster miscellaneous')

    def groupId(self):
        return 'rastermiscellaneous'

    def commandName(self):
        return "gdalbuildvrt"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = []
        arguments.append('-resolution')
        arguments.append(self.RESOLUTION_OPTIONS[self.parameterAsEnum(parameters, self.RESOLUTION, context)])
        if self.parameterAsBool(parameters, buildvrt.SEPARATE, context):
            arguments.append('-separate')
        if self.parameterAsBool(parameters, buildvrt.PROJ_DIFFERENCE, context):
            arguments.append('-allow_projection_difference')
        # Always write input files to a text file in case there are many of them and the
        # length of the command will be longer then allowed in command prompt
        list_file = GdalUtils.writeLayerParameterToTextFile(filename='buildvrtInputFiles.txt', alg=self, parameters=parameters, parameter_name=self.INPUT, context=context, executing=executing, quote=False)
        arguments.append('-input_file_list')
        arguments.append(list_file)

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
