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

from PyQt4.QtCore import *
from qgis.core import *
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterVector import ParameterVector
from sextante.outputs.OutputVector import OutputVector
from sextante.algs.ftools import FToolsUtils as utils

class MeanCoords(GeoAlgorithm):

    POINTS = "POINTS"
    WEIGHT = "WEIGHT"
    OUTPUT = "OUTPUT"
    UID = "UID"
    WEIGHT = "WEIGHT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/mean.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Mean coordinate(s)"
        self.group = "Vector analysis tools"

        self.addParameter(ParameterVector(self.POINTS, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(self.WEIGHT, "Weight field", MeanCoords.POINTS, ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterTableField(self.UID, "Unique ID field", MeanCoords.POINTS, ParameterTableField.DATA_TYPE_NUMBER))

        self.addOutput(OutputVector(MeanCoords.OUTPUT, "Result"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.POINTS))
        weightField = self.getParameterValue(self.WEIGHT)
        uniqueField = self.getParameterValue(self.UID)

        weightIndex = layer.fieldNameIndex(weightField)
        uniqueIndex = layer.fieldNameIndex(uniqueField)

        if uniqueIndex <> -1:
            uniqueValues = utils.getUniqueValues(layer, uniqueIndex)
            single = False
        else:
            uniqueValues = [QVariant(1)]
            single = True

        fieldList = [QgsField("MEAN_X", QVariant.Double, "", 24, 15),
                     QgsField("MEAN_Y", QVariant.Double, "", 24, 15),
                     QgsField("UID", QVariant.String, "", 255)
                    ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldList,
                     QGis.WKBPoint, layer.crs())

        current = 0
        features = QGisLayers.features(layer)
        total = 100.0 / float(len(features) * len(uniqueValues))

        outFeat = QgsFeature()

        for j in uniqueValues:
            cx = 0.00
            cy = 0.00
            points = []
            weights = []
            for feat in features:
                current += 1
                progress.setPercentage(current * total)

                if single:
                    check = j.toString().trimmed()
                else:
                    check = feat.attributes()[uniqueIndex].toString().trimmed()

                if check == j.toString().trimmed():
                    cx = 0.00
                    cy = 0.00
                    if weightIndex == -1:
                        weight = 1.00
                    else:
                        try:
                            weight = float(feat.attributes()[weightIndex].toDouble()[0])
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
            outFeat.setAttributes([QVariant(cx), QVariant(cy), QVariant(j)])
            writer.addFeature(outFeat)

            if single:
                break

        del writer
