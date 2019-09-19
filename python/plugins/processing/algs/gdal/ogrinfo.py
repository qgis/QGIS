# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogrinfo.py
    ---------------------
    Date                 : November 2012
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
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'


from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFileDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class ogrinfo(GdalAlgorithm):

    INPUT = 'INPUT'
    SUMMARY_ONLY = 'SUMMARY_ONLY'
    NO_METADATA = 'NO_METADATA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBoolean(self.SUMMARY_ONLY,
                                                        self.tr('Summary output only'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterBoolean(self.NO_METADATA,
                                                        self.tr('Suppress metadata info'),
                                                        defaultValue=False))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT,
                                                                self.tr('Layer information'),
                                                                self.tr('HTML files (*.html)')))

    def name(self):
        return 'ogrinfo'

    def displayName(self):
        return self.tr('Vector information')

    def group(self):
        return self.tr('Vector miscellaneous')

    def groupId(self):
        return 'vectormiscellaneous'

    def commandName(self):
        return 'ogrinfo'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = ['-al']

        if self.parameterAsBoolean(parameters, self.SUMMARY_ONLY, context):
            arguments.append('-so')
        if self.parameterAsBoolean(parameters, self.NO_METADATA, context):
            arguments.append('-nomd')

        inLayer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        ogrLayer, layerName = self.getOgrCompatibleSource(self.INPUT, parameters, context, feedback, executing)
        arguments.append(ogrLayer)
        arguments.append(layerName)
        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]

    def processAlgorithm(self, parameters, context, feedback):
        GdalUtils.runGdal(self.getConsoleCommands(parameters, context, feedback), feedback)
        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)
        with open(output, 'w') as f:
            f.write('<pre>')
            for s in GdalUtils.getConsoleOutput()[1:]:
                f.write(str(s))
            f.write('</pre>')

        return {self.OUTPUT: output}
