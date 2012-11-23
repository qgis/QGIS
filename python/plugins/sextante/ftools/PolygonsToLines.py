# -*- coding: utf-8 -*-

"""
***************************************************************************
    PolygonsToLines.py
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

import os.path

from PyQt4 import QtGui
from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers

from sextante.parameters.ParameterVector import ParameterVector

from sextante.outputs.OutputVector import OutputVector

class PolygonsToLines(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/to_lines.png")

    def defineCharacteristics(self):
        self.name = "Polygons to lines"
        self.group = "Geometry tools"

        self.addParameter(ParameterVector(self.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_POLYGON))

        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        output = self.getOutputValue(self.OUTPUT)

        provider = layer.dataProvider()
        layer.select(layer.pendingAllAttributesList())

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(layer.pendingFields(),
                     QGis.WKBLineString, provider.crs())

        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()

        current = 0
        total = 100.0 / float(provider.featureCount())

        while layer.nextFeature(inFeat):
            multi = False
            inGeom = inFeat.geometry()
            if inGeom.isMultipart():
                multi = True
            atMap = inFeat.attributeMap()
            lineList = self.extractAsLine(inGeom)
            outFeat.setAttributeMap(atMap)
            for h in lineList:
              outFeat.setGeometry(outGeom.fromPolyline(h))
              writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(int(current * total))

        del writer

    def extractAsLine(self, geom):
        multiGeom = QgsGeometry()
        lines = []
        if geom.type() == QGis.Polygon:
          if geom.isMultipart():
              multiGeom = geom.asMultiPolygon()
              for i in multiGeom:
                  lines.extend(i)
          else:
              multiGeom = geom.asPolygon()
              lines = multiGeom
          return lines
        else:
            return []
