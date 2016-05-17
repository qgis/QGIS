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

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import QGis, QgsProject, QgsCoordinateTransform, QgsFeature, QgsGeometry, QgsField
from qgis.utils import iface

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExportGeometryInfo(GeoAlgorithm):

    INPUT = 'INPUT'
    METHOD = 'CALC_METHOD'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'export_geometry.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Export/Add geometry columns')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')

        self.calc_methods = [self.tr('Layer CRS'),
                             self.tr('Project CRS'),
                             self.tr('Ellipsoidal')]

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterSelection(self.METHOD,
                                             self.tr('Calculate using'), self.calc_methods, 0))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Added geom info')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        method = self.getParameterValue(self.METHOD)

        geometryType = layer.geometryType()
        fields = layer.pendingFields()

        if geometryType == QGis.Polygon:
            areaName = vector.createUniqueFieldName('area', fields)
            fields.append(QgsField(areaName, QVariant.Double))
            perimeterName = vector.createUniqueFieldName('perimeter', fields)
            fields.append(QgsField(perimeterName, QVariant.Double))
        elif geometryType == QGis.Line:
            lengthName = vector.createUniqueFieldName('length', fields)
            fields.append(QgsField(lengthName, QVariant.Double))
        else:
            xName = vector.createUniqueFieldName('xcoord', fields)
            fields.append(QgsField(xName, QVariant.Double))
            yName = vector.createUniqueFieldName('ycoord', fields)
            fields.append(QgsField(yName, QVariant.Double))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields.toList(), layer.dataProvider().geometryType(), layer.crs())

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
            mapCRS = iface.mapCanvas().mapSettings().destinationCrs()
            layCRS = layer.crs()
            coordTransform = QgsCoordinateTransform(layCRS, mapCRS)

        outFeat = QgsFeature()
        inGeom = QgsGeometry()

        outFeat.initAttributes(len(fields))
        outFeat.setFields(fields)

        features = vector.features(layer)
        total = 100.0 / len(features)
        for current, f in enumerate(features):
            inGeom = f.geometry()

            if method == 1:
                inGeom.transform(coordTransform)

            (attr1, attr2) = vector.simpleMeasure(inGeom, method, ellips, crs)

            outFeat.setGeometry(inGeom)
            attrs = f.attributes()
            attrs.append(attr1)
            if attr2 is not None:
                attrs.append(attr2)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)

            progress.setPercentage(int(current * total))

        del writer
