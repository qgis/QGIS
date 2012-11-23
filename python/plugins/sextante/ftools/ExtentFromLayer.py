# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtentFromLayer.py
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
from sextante.parameters.ParameterBoolean import ParameterBoolean

from sextante.outputs.OutputVector import OutputVector

class ExtentFromLayer(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    USE_SELECTION = "USE_SELECTION"
    BY_FEATURE = "BY_FEATURE"

    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/layer_extent.png")

    def defineCharacteristics(self):
        self.name = "Polygon from layer extent"
        self.group = "Research tools"

        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterBoolean(self.USE_SELECTION, "Use selection", False))
        self.addParameter(ParameterBoolean(self.BY_FEATURE, "Calculate extent for each feature separately", False))

        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        useSelection = self.getParameterValue(self.USE_SELECTION)
        byFeature = self.getParameterValue(self.BY_FEATURE)

        output = self.getOutputValue(self.OUTPUT)

        fields = {0 : QgsField("MINX", QVariant.Double),
                  1 : QgsField("MINY", QVariant.Double),
                  2 : QgsField("MAXX", QVariant.Double),
                  3 : QgsField("MAXY", QVariant.Double),
                  4 : QgsField("CNTX", QVariant.Double),
                  5 : QgsField("CNTY", QVariant.Double),
                  6 : QgsField("AREA", QVariant.Double),
                  7 : QgsField("PERIM", QVariant.Double),
                  8 : QgsField("HEIGHT", QVariant.Double),
                  9 : QgsField("WIDTH", QVariant.Double)
                 }

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                     QGis.WKBPolygon, layer.crs())

        if byFeature:
            self.featureExtent(layer, writer, useSelection, progress)
        else:
            self.layerExtent(layer, writer, progress)

        del writer

    def layerExtent(self, layer, writer, progress):
        rect = layer.extent()
        minx = rect.xMinimum()
        miny = rect.yMinimum()
        maxx = rect.xMaximum()
        maxy = rect.yMaximum()
        height = rect.height()
        width = rect.width()
        cntx = minx + (width / 2.0)
        cnty = miny + (height / 2.0)
        area = width * height
        perim = (2 * width) + (2 * height)

        rect = [QgsPoint(minx, miny),
                QgsPoint(minx, maxy),
                QgsPoint(maxx, maxy),
                QgsPoint(maxx, miny),
                QgsPoint(minx, miny)
               ]

        geometry = QgsGeometry().fromPolygon([rect])
        feat = QgsFeature()
        feat.setGeometry(geometry)
        feat.setAttributeMap({0 : QVariant(minx),
                              1 : QVariant(miny),
                              2 : QVariant(maxx),
                              3 : QVariant(maxy),
                              4 : QVariant(cntx),
                              5 : QVariant(cnty),
                              6 : QVariant(area),
                              7 : QVariant(perim),
                              8 : QVariant(height),
                              9 : QVariant(width)
                             })
        writer.addFeature(feat)

    def featureExtent(self, layer, writer, useSelection, progress):
        current = 0

        inFeat = QgsFeature()
        outFeat = QgsFeature()

        provider = layer.dataProvider()
        provider.select()

        if useSelection:
            total = 100.0 / float(layer.selectedFeatureCount())
            for inFeat in layer.selectedFeatures():
                rect = inFeat.geometry().boundingBox()
                minx = rect.xMinimum()
                miny = rect.yMinimum()
                maxx = rect.xMaximum()
                maxy = rect.yMaximum()
                height = rect.height()
                width = rect.width()
                cntx = minx + (width / 2.0)
                cnty = miny + (height / 2.0)
                area = width * height
                perim = (2 * width) + (2 * height)
                rect = [QgsPoint(minx, miny),
                        QgsPoint(minx, maxy),
                        QgsPoint(maxx, maxy),
                        QgsPoint(maxx, miny),
                        QgsPoint(minx, miny)
                       ]
                geometry = QgsGeometry().fromPolygon([rect])

                outFeat.setGeometry(geometry)
                outFeat.setAttributeMap({0 : QVariant(minx),
                                         1 : QVariant(miny),
                                         2 : QVariant(maxx),
                                         3 : QVariant(maxy),
                                         4 : QVariant(cntx),
                                         5 : QVariant(cnty),
                                         6 : QVariant(area),
                                         7 : QVariant(perim),
                                         8 : QVariant(height),
                                         9 : QVariant(width)
                                        })
                writer.addFeature(outFeat)
                current += 1
                progress.setPercentage(int(current * total))
        else:
            total = 100.0 / float(provider.featureCount())
            while provider.nextFeature(inFeat):
                rect = inFeat.geometry().boundingBox()
                minx = rect.xMinimum()
                miny = rect.yMinimum()
                maxx = rect.xMaximum()
                maxy = rect.yMaximum()
                height = rect.height()
                width = rect.width()
                cntx = minx + (width / 2.0)
                cnty = miny + (height / 2.0)
                area = width * height
                perim = (2 * width) + (2 * height)
                rect = [QgsPoint(minx, miny),
                        QgsPoint(minx, maxy),
                        QgsPoint(maxx, maxy),
                        QgsPoint(maxx, miny),
                        QgsPoint(minx, miny)
                       ]

                geometry = QgsGeometry().fromPolygon([rect])

                outFeat.setGeometry(geometry)
                outFeat.setAttributeMap({0 : QVariant(minx),
                                         1 : QVariant(miny),
                                         2 : QVariant(maxx),
                                         3 : QVariant(maxy),
                                         4 : QVariant(cntx),
                                         5 : QVariant(cnty),
                                         6 : QVariant(area),
                                         7 : QVariant(perim),
                                         8 : QVariant(height),
                                         9 : QVariant(width)
                                        })
                writer.addFeature(outFeat)
                current += 1
                progress.setPercentage(int(current * total))
