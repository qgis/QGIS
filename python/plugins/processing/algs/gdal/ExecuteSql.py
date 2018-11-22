# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExecuteSql.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterVectorDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class ExecuteSql(GdalAlgorithm):

    INPUT = 'INPUT'
    SQL = 'SQL'
    DIALECT = 'DIALECT'
    OPTIONS = 'OPTIONS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.dialects = ((self.tr('None'), ''),
                         (self.tr('OGR SQL'), 'ogrsql'),
                         (self.tr('SQLite'), 'sqlite'))

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterString(self.SQL,
                                                       self.tr('SQL expression'),
                                                       defaultValue=''))
        self.addParameter(QgsProcessingParameterEnum(self.DIALECT,
                                                     self.tr('SQL dialect'),
                                                     options=[i[0] for i in self.dialects],
                                                     allowMultiple=False,
                                                     defaultValue=0))

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(options_param)

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT,
                                                                  self.tr('SQL result')))

    def name(self):
        return 'executesql'

    def displayName(self):
        return self.tr('Execute SQL')

    def group(self):
        return self.tr('Vector miscellaneous')

    def groupId(self):
        return 'vectormiscellaneous'

    def commandName(self):
        return "ogr2ogr"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        ogrLayer, layerName = self.getOgrCompatibleSource(self.INPUT, parameters, context, feedback, executing)
        sql = self.parameterAsString(parameters, self.SQL, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        output, outputFormat = GdalUtils.ogrConnectionStringAndFormat(outFile, context)

        if not sql:
            raise QgsProcessingException(
                self.tr('Empty SQL. Please enter valid SQL expression and try again.'))

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append('-sql')
        arguments.append(sql)

        dialect = self.dialects[self.parameterAsEnum(parameters, self.DIALECT, context)][1]
        if dialect:
            arguments.append('-dialect')
            arguments.append(dialect)

        if options:
            arguments.append(options)

        if outputFormat:
            arguments.append('-f {}'.format(outputFormat))

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
