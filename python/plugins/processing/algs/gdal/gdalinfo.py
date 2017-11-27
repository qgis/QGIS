# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdalinfo.py
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
from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingOutputHtml)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class gdalinfo(GdalAlgorithm):

    INPUT = 'INPUT'
    MIN_MAX = 'MIN_MAX'
    STATS = 'STATS'
    NO_GCP = 'NOGCP'
    NO_METADATA = 'NO_METADATA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBoolean(self.MIN_MAX,
                                                        self.tr('Force computation of the actual min/max values for each band'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.STATS,
                                                        self.tr('Read and display image statistics (force computation if necessary)'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.NO_GCP,
                                                        self.tr('Suppress GCP info'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.NO_METADATA,
                                                        self.tr('Suppress metadata info'),
                                                        defaultValue=False))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT,
                                                                self.tr('Layer information'),
                                                                self.tr('HTML files (*.html)')))
        self.addOutput(QgsProcessingOutputHtml(self.OUTPUT, self.tr('Layer information')))

    def name(self):
        return 'gdalinfo'

    def displayName(self):
        return self.tr('Raster information')

    def group(self):
        return self.tr('Raster miscellaneous')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'raster-info.png'))

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = []
        if self.parameterAsBool(parameters, self.MIN_MAX, context):
            arguments.append('-mm')
        if self.parameterAsBool(parameters, self.STATS, context):
            arguments.append('-stats')
        if self.parameterAsBool(parameters, self.NO_GCP, context):
            arguments.append('-nogcp')
        if self.parameterAsBool(parameters, self.NO_METADATA, context):
            arguments.append('-nomd')
        arguments.append(self.parameterAsRasterLayer(parameters, self.INPUT, context).source())
        return ['gdalinfo', GdalUtils.escapeAndJoin(arguments)]

    def processAlgorithm(self, parameters, context, feedback):
        GdalUtils.runGdal(self.getConsoleCommands(parameters, context, feedback), feedback)
        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)
        with open(output, 'w') as f:
            f.write('<pre>')
            for s in GdalUtils.getConsoleOutput()[1:]:
                f.write(str(s))
            f.write('</pre>')

        return {self.OUTPUT: output}
