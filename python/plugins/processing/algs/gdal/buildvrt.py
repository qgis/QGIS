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
                                                               self.tr('Input layers'),
                                                               QgsProcessing.TypeRaster))
        self.addParameter(QgsProcessingParameterEnum(self.RESOLUTION,
                                                     self.tr('Resolution'),
                                                     options=self.RESOLUTION_OPTIONS,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterBoolean(self.SEPARATE,
                                                        self.tr('Layer stack'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterBoolean(self.PROJ_DIFFERENCE,
                                                        self.tr('Allow projection difference'),
                                                        defaultValue=False))
        self.addParameter(ParameterVrtDestination(self.OUTPUT, self.tr('Virtual')))

    def name(self):
        return 'buildvirtualraster'

    def displayName(self):
        return self.tr('Build Virtual Raster')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'vrt.png'))

    def group(self):
        return self.tr('Raster miscellaneous')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = []
        arguments.append('-resolution')
        arguments.append(self.RESOLUTION_OPTIONS[self.parameterAsEnum(parameters, self.RESOLUTION, context)])
        if self.parameterAsBool(parameters, buildvrt.SEPARATE, context):
            arguments.append('-separate')
        if self.parameterAsBool(parameters, buildvrt.PROJ_DIFFERENCE, context):
            arguments.append('-allow_projection_difference')
        # Always write input files to a text file in case there are many of them and the
        # length of the command will be longer then allowed in command prompt
        listFile = os.path.join(QgsProcessingUtils.tempFolder(), 'buildvrtInputFiles.txt')
        with open(listFile, 'w') as f:
            layers = []
            for l in self.parameterAsLayerList(parameters, self.INPUT, context):
                layers.append(l.source())
            f.write('\n'.join(layers))
        arguments.append('-input_file_list')
        arguments.append(listFile)

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        # Ideally the file extensions should be limited to just .vrt but I'm not sure how
        # to do it simply so instead a check is performed.
        _, ext = os.path.splitext(out)
        if not ext.lower() == '.vrt':
            out = out[:-len(ext)] + '.vrt'
            if isinstance(parameters[self.OUTPUT], QgsProcessingOutputLayerDefinition):
                output_def = QgsProcessingOutputLayerDefinition(parameters[self.OUTPUT])
                output_def.sink = QgsProperty.fromValue(out)
                self.setOutputValue(self.OUTPUT, output_def)
            else:
                self.setOutputValue(self.OUTPUT, out)
        arguments.append(out)

        return ['gdalbuildvrt', GdalUtils.escapeAndJoin(arguments)]
