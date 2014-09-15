# -*- coding: utf-8 -*-

"""
***************************************************************************
    RegularPoints.py
    ---------------------
    Date                 : September 2014
    Copyright            : (C) 2014 by Alexander Bruy
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
__date__ = 'September 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from random import *

from PyQt4.QtCore import *
from qgis.core import *
from qgis.utils import iface

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector
from processing.tools import vector


class RegularPoints(GeoAlgorithm):

    EXTENT = 'EXTENT'
    SPACING = 'SPACING'
    INSET = 'INSET'
    RANDOMIZE = 'RANDOMIZE'
    IS_SPACING = 'IS_SPACING'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Regular points'
        self.group = 'Vector creation tools'

        self.addParameter(ParameterExtent(self.EXTENT, 'Input extent'))
        self.addParameter(ParameterNumber(self.SPACING,
            'Point spacing/count', 0.0001, 999999999.999999999, 0.0001))
        self.addParameter(ParameterNumber(self.INSET,
            'Initial inset from corner (LH side)', 0.0, 9999.9999, 0.0))
        self.addParameter(ParameterBoolean(self.RANDOMIZE,
            'Apply random offset to point spacing', False))
        self.addParameter(ParameterBoolean(self.IS_SPACING,
            'Use point spacing', True))
        self.addOutput(OutputVector(self.OUTPUT, 'Regular points'))

    def processAlgorithm(self, progress):
        extent = str(self.getParameterValue(self.EXTENT)).split(',')

        spacing = float(self.getParameterValue(self.SPACING))
        inset = float(self.getParameterValue(self.INSET))
        randomize = self.getParameterValue(self.RANDOMIZE)
        isSpacing = self.getParameterValue(self.IS_SPACING)

        extent = QgsRectangle(float(extent[0]), float(extent[2]),
                              float(extent[1]), float(extent[3]))

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        mapCRS = iface.mapCanvas().mapSettings().destinationCrs()

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QGis.WKBPoint, mapCRS)

        if randomize:
            seed()

        area = extent.width() * extent.height()
        if isSpacing:
            pSpacing = spacing
        else:
            pSpacing = sqrt(area / value)

        f = QgsFeature()
        f.initAttributes(1)
        f.setFields(fields)

        count = 0
        total = 100.00 / (area / pSpacing)
        y = extent.yMaximum() - inset
        while y >= extent.yMinimum():
            x = extent.xMinimum() + inset
            while x <= extent.xMaximum():
                if randomize:
                    geom = QgsGeometry().fromPoint(QgsPoint(
                        uniform(x - (pSpacing / 2.0), x + (pSpacing / 2.0)),
                        uniform(y - (pSpacing / 2.0), y + (pSpacing / 2.0))))
                else:
                    geom = QgsGeometry().fromPoint(QgsPoint(x, y))

                if geom.intersects(extent):
                    f.setAttribute('id', count)
                    f.setGeometry(geom)
                    writer.addFeature(f)
                    x += pSpacing
                    count += 1
                    progress.setPercentage(int(count* total))
            y = y - pSpacing
        del writer
