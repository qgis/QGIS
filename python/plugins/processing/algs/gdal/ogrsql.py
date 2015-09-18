# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogrsql.py
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

from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalUtils import GdalUtils
from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm


class OgrSql(OgrAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    SQL = 'SQL'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Execute SQL')
        self.group, self.i18n_group = self.trAlgorithm('[OGR] Miscellaneous')

        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer'),
                          [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterString(self.SQL, self.tr('SQL'), ''))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('SQL result')))

    def getConsoleCommands(self):
        sql = self.getParameterValue(self.SQL)
        if sql == '':
            raise GeoAlgorithmExecutionException(
                self.tr('Empty SQL. Please enter valid SQL expression and try again.'))

        arguments = []
        arguments.append('-sql')
        arguments.append(sql)

        output = self.getOutputFromName(self.OUTPUT)
        outFile = output.value
        arguments.append(outFile)

        layer = self.getParameterValue(self.INPUT)
        conn = self.ogrConnectionString(layer)
        arguments.append(conn)

        return ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]
