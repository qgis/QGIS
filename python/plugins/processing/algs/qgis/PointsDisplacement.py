# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsDisplacement.py
    ---------------------
    Date                 : July 2013
    Copyright            : (C) 2013 by Alexander Bruy
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
__date__ = 'July 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import math
from qgis.core import QgsFeatureRequest, QgsFeature, QgsGeometry, QgsPoint
from processing.tools import dataobjects, vector
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector


class PointsDisplacement(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    DISTANCE = 'DISTANCE'
    HORIZONTAL = 'HORIZONTAL'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def defineCharacteristics(self):
        self.name = 'Points displacement'
        self.group = 'Vector geometry tools'

        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POINT]))
        self.addParameter(ParameterNumber(self.DISTANCE,
            self.tr('Displacement distance'),
            0.00001, 999999999.999990, 0.00015))
        self.addParameter(ParameterBoolean(self.HORIZONTAL,
            self.tr('Horizontal distribution for two point case')))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Displaced')))

    def processAlgorithm(self, progress):
        radius = self.getParameterValue(self.DISTANCE)
        horizontal = self.getParameterValue(self.HORIZONTAL)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))

        provider = layer.dataProvider()
        writer = output.getVectorWriter(provider.fields(),
                provider.geometryType(), provider.crs())

        features = vector.features(layer)

        current = 0
        total = 100.0 / len(features)

        duplicates = dict()
        for f in features:
            wkt = f.geometry().exportToWkt()
            if wkt not in duplicates:
                duplicates[wkt] = [f.id()]
            else:
                duplicates[wkt].extend([f.id()])

            current += 1
            progress.setPercentage(int(current * total))

        current = 0
        total = 100.0 / len(duplicates)
        progress.setPercentage(0)

        fullPerimeter = 2 * math.pi

        request = QgsFeatureRequest()
        for (geom, fids) in duplicates.iteritems():
            count = len(fids)
            if count == 1:
                f = layer.getFeatures(request.setFilterFid(fids[0])).next()
                writer.addFeature(f)
            else:
                angleStep = fullPerimeter / count
                if count == 2 and horizontal:
                    currentAngle = math.pi / 2
                else:
                    currentAngle = 0

                old_point = QgsGeometry.fromWkt(geom).asPoint()
                for fid in fids:
                    sinusCurrentAngle = math.sin(currentAngle)
                    cosinusCurrentAngle = math.cos(currentAngle)
                    dx = radius * sinusCurrentAngle
                    dy = radius * cosinusCurrentAngle

                    f = layer.getFeatures(request.setFilterFid(fid)).next()

                    new_point = QgsPoint(old_point.x() + dx, old_point.y()
                            + dy)
                    out_feature = QgsFeature()
                    out_feature.setGeometry(QgsGeometry.fromPoint(new_point))
                    out_feature.setAttributes(f.attributes())

                    writer.addFeature(out_feature)
                    currentAngle += angleStep

            current += 1
            progress.setPercentage(int(current * total))

        del writer
