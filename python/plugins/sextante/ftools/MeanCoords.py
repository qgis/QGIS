# -*- coding: utf-8 -*-

"""
***************************************************************************
    MeanCoords.py
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

from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterVector import ParameterVector

from sextante.outputs.OutputVector import OutputVector

from sextante.ftools import FToolsUtils as utils


class MeanCoords(GeoAlgorithm):

    POINTS = "POINTS"
    WEIGHT = "WEIGHT"
    OUTPUT = "OUTPUT"
    UID = "UID"
    WEIGHT = "WEIGHT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/mean.png")

    def defineCharacteristics(self):
        self.name = "Mean coordinate(s)"
        self.group = "Analysis tools"

        self.addParameter(ParameterVector(self.POINTS, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(self.WEIGHT, "Weight field", MeanCoords.POINTS, ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterTableField(self.UID, "Unique ID field", MeanCoords.POINTS, ParameterTableField.DATA_TYPE_NUMBER))

        self.addOutput(OutputVector(MeanCoords.OUTPUT, "Result"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.POINTS))
        weightField = self.getParameterValue(self.WEIGHT)
        uniqueField = self.getParameterValue(self.UID)

        output = self.getOutputValue(self.OUTPUT)

        provider = layer.dataProvider()
        weightIndex = layer.fieldNameIndex(weightField)
        uniqueIndex = layer.fieldNameIndex(uniqueField)

        if uniqueIndex <> -1:
            uniqueValues = layer.uniqueValues(uniqueIndex)
            single = False
        else:
            uniqueValues = [QVariant(1)]
            single = True

        fieldList = {0 : QgsField("MEAN_X", QVariant.Double, "", 24, 15),
                     1 : QgsField("MEAN_Y", QVariant.Double, "", 24, 15),
                     2 : QgsField("UID", QVariant.String, "", 255)
                    }

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldList,
                     QGis.WKBPoint, provider.crs())

        current = 0
        total = 100.0 / float(provider.featureCount() * len(uniqueValues))

        feat = QgsFeature()
        outFeat = QgsFeature()

        for j in uniqueValues:
            provider.rewind()
            layer.select([weightIndex, uniqueIndex])
            cx = 0.00
            cy = 0.00
            points = []
            weights = []
            while layer.nextFeature(feat):
                current += 1
                progress.setPercentage(current * total)

                if single:
                    check = j.toString().trimmed()
                else:
                    check = feat.attributeMap()[uniqueIndex].toString().trimmed()

                if check == j.toString().trimmed():
                    cx = 0.00
                    cy = 0.00
                    if weightIndex == -1:
                        weight = 1.00
                    else:
                        try:
                            weight = float(feat.attributeMap()[weightIndex].toDouble()[0])
                        except:
                            weight = 1.00

                    geom = QgsGeometry(feat.geometry())
                    geom = utils.extractPoints(geom)
                    for i in geom:
                        cx += i.x()
                        cy += i.y()
                    points.append(QgsPoint((cx / len(geom)), (cy / len(geom))))
                    weights.append(weight)

            sumWeight = sum(weights)
            cx = 0.00
            cy = 0.00
            item = 0
            for item, i in enumerate(points):
                cx += i.x() * weights[item]
                cy += i.y() * weights[item]

            cx = cx / sumWeight
            cy = cy / sumWeight
            meanPoint = QgsPoint(cx, cy)

            outFeat.setGeometry(QgsGeometry.fromPoint(meanPoint))
            outFeat.addAttribute(0, QVariant(cx))
            outFeat.addAttribute(1, QVariant(cy))
            outFeat.addAttribute(2, QVariant(j))
            writer.addFeature(outFeat)

            if single:
                break

        del writer
