# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterFactory.py
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

from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterMultipleInput import ParameterMultipleInput
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterString import ParameterString
from processing.parameters.ParameterTable import ParameterTable
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterTableField import ParameterTableField
from processing.parameters.ParameterSelection import ParameterSelection
from processing.parameters.ParameterRange import ParameterRange
from processing.parameters.ParameterFixedTable import ParameterFixedTable
from processing.parameters.ParameterExtent import ParameterExtent
from processing.parameters.ParameterFile import ParameterFile
from processing.parameters.ParameterCrs import ParameterCrs

class ParameterFactory():

    @staticmethod
    def getFromString(s):
        classes = [ParameterBoolean, ParameterMultipleInput,ParameterNumber,
                   ParameterRaster, ParameterString, ParameterVector, ParameterTableField,
                   ParameterTable, ParameterSelection, ParameterRange, ParameterFixedTable,
                   ParameterExtent, ParameterFile, ParameterCrs]
        for clazz in classes:
            if s.startswith(clazz().parameterName()):
                return clazz().deserialize(s)