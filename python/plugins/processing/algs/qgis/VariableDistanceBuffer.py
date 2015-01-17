# -*- coding: utf-8 -*-

"""
***************************************************************************
    VariableDistanceBuffer.py
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

from PyQt4.QtCore import *

from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
import Buffer as buff
from processing.tools import dataobjects


class VariableDistanceBuffer(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'
    SEGMENTS = 'SEGMENTS'
    DISSOLVE = 'DISSOLVE'

    def defineCharacteristics(self):
        self.name = 'Variable distance buffer'
        self.group = 'Vector geometry tools'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
            self.tr('Distance field'), self.INPUT))
        self.addParameter(ParameterNumber(self.SEGMENTS,
            self.tr('Segments'), 1, default=5))
        self.addParameter(ParameterBoolean(self.DISSOLVE,
            self.tr('Dissolve result'), False))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Buffer')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
                self.getParameterValue(self.INPUT))
        dissolve = self.getParameterValue(self.DISSOLVE)
        field = self.getParameterValue(self.FIELD)
        segments = int(self.getParameterValue(self.SEGMENTS))

        writer = self.getOutputFromName(
                self.OUTPUT).getVectorWriter(layer.pendingFields().toList(),
                                             QGis.WKBPolygon, layer.crs())

        buff.buffering(progress, writer, 0, field, True, layer, dissolve,
                       segments)
