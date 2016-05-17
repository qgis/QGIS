# -*- coding: utf-8 -*-

"""
***************************************************************************
    FixedDistanceBuffer.py
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

from qgis.core import QGis

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from . import Buffer as buff
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class FixedDistanceBuffer(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'
    DISTANCE = 'DISTANCE'
    SEGMENTS = 'SEGMENTS'
    DISSOLVE = 'DISSOLVE'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'buffer.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Fixed distance buffer')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterNumber(self.DISTANCE,
                                          self.tr('Distance'), default=10.0))
        self.addParameter(ParameterNumber(self.SEGMENTS,
                                          self.tr('Segments'), 1, default=5))
        self.addParameter(ParameterBoolean(self.DISSOLVE,
                                           self.tr('Dissolve result'), False))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Buffer')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        distance = self.getParameterValue(self.DISTANCE)
        dissolve = self.getParameterValue(self.DISSOLVE)
        segments = int(self.getParameterValue(self.SEGMENTS))

        writer = self.getOutputFromName(
            self.OUTPUT).getVectorWriter(layer.pendingFields().toList(),
                                         QGis.WKBPolygon, layer.crs())

        buff.buffering(progress, writer, distance, None, False, layer,
                       dissolve, segments)
