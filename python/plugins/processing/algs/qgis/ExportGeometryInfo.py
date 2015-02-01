# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExportGeometryInfo.py
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

from qgis.core import QGis, QgsProject, QgsCoordinateTransform, QgsFeature, QgsGeometry
from qgis.utils import iface
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class ExportGeometryInfo(GeoAlgorithm):

    INPUT = 'INPUT'
    METHOD = 'CALC_METHOD'
    OUTPUT = 'OUTPUT'

    CALC_METHODS = ['Layer CRS', 'Project CRS', 'Ellipsoidal']

    def defineCharacteristics(self):
        self.name = 'Export/Add geometry columns'
        self.group = 'Vector table tools'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterSelection(self.METHOD,
            self.tr('Calculate using'), self.CALC_METHODS, 0))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output layer')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        method = self.getParameterValue(self.METHOD)

        geometryType = layer.geometryType()

        idx1 = -1
        idx2 = -1
        fields = layer.pendingFields()

        if geometryType == QGis.Polygon:
            (idx1, fields) = vector.findOrCreateField(layer, fields, 'area',
                    21, 6)
            (idx2, fields) = vector.findOrCreateField(layer, fields,
                    'perimeter', 21, 6)
        elif geometryType == QGis.Line:
            (idx1, fields) = vector.findOrCreateField(layer, fields, 'length',
                    21, 6)
            idx2 = idx1
        else:
            (idx1, fields) = vector.findOrCreateField(layer, fields, 'xcoord',
                    21, 6)
            (idx2, fields) = vector.findOrCreateField(layer, fields, 'ycoord',
                    21, 6)

        writer = self.getOutputFromName(
            self.OUTPUT).getVectorWriter(fields.toList(),
            layer.dataProvider().geometryType(), layer.crs())

        ellips = None
        crs = None
        coordTransform = None

        # Calculate with:
        # 0 - layer CRS
        # 1 - project CRS
        # 2 - ellipsoidal

        if method == 2:
            ellips = QgsProject.instance().readEntry('Measure', '/Ellipsoid',
                    'NONE')[0]
            crs = layer.crs().srsid()
        elif method == 1:
            mapCRS = iface.mapCanvas().mapRenderer().destinationCrs()
            layCRS = layer.crs()
            coordTransform = QgsCoordinateTransform(layCRS, mapCRS)

        outFeat = QgsFeature()
        inGeom = QgsGeometry()

        outFeat.initAttributes(len(fields))
        outFeat.setFields(fields)

        current = 0
        features = vector.features(layer)
        total = 100.0 / float(len(features))
        for f in features:
            inGeom = f.geometry()

            if method == 1:
                inGeom.transform(coordTransform)

            (attr1, attr2) = vector.simpleMeasure(inGeom, method, ellips, crs)

            outFeat.setGeometry(inGeom)
            attrs = f.attributes()
            attrs.insert(idx1, attr1)
            if attr2 is not None:
                attrs.insert(idx2, attr2)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(int(current * total))

        del writer
