# -*- coding: utf-8 -*-

"""
***************************************************************************
    Intersection.py
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

from qgis.core import (QgsFeatureRequest,
                       QgsFeature,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsMessageLog,
                       QgsProcessingUtils)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]

wkbTypeGroups = {
    'Point': (QgsWkbTypes.Point, QgsWkbTypes.MultiPoint, QgsWkbTypes.Point25D, QgsWkbTypes.MultiPoint25D,),
    'LineString': (QgsWkbTypes.LineString, QgsWkbTypes.MultiLineString, QgsWkbTypes.LineString25D, QgsWkbTypes.MultiLineString25D,),
    'Polygon': (QgsWkbTypes.Polygon, QgsWkbTypes.MultiPolygon, QgsWkbTypes.Polygon25D, QgsWkbTypes.MultiPolygon25D,),
}
for key, value in list(wkbTypeGroups.items()):
    for const in value:
        wkbTypeGroups[const] = key


class Intersection(GeoAlgorithm):

    INPUT = 'INPUT'
    INPUT2 = 'INPUT2'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'intersect.png'))

    def group(self):
        return self.tr('Vector overlay tools')

    def name(self):
        return 'intersection'

    def displayName(self):
        return self.tr('Intersection')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer')))
        self.addParameter(ParameterVector(self.INPUT2,
                                          self.tr('Intersect layer')))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Intersection')))

    def processAlgorithm(self, context, feedback):
        vlayerA = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))
        vlayerB = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT2))

        geomType = QgsWkbTypes.multiType(vlayerA.wkbType())
        fields = vector.combineVectorFields(vlayerA, vlayerB)
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     geomType, vlayerA.crs())
        outFeat = QgsFeature()
        index = vector.spatialindex(vlayerB)
        selectionA = QgsProcessingUtils.getFeatures(vlayerA, context)
        total = 100.0 / QgsProcessingUtils.featureCount(vlayerA, context)
        for current, inFeatA in enumerate(selectionA):
            feedback.setProgress(int(current * total))
            geom = inFeatA.geometry()
            atMapA = inFeatA.attributes()
            intersects = index.intersects(geom.boundingBox())
            request = QgsFeatureRequest().setFilterFids(intersects)

            engine = None
            if len(intersects) > 0:
                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(geom.geometry())
                engine.prepareGeometry()

            for inFeatB in vlayerB.getFeatures(request):
                tmpGeom = inFeatB.geometry()
                if engine.intersects(tmpGeom.geometry()):
                    atMapB = inFeatB.attributes()
                    int_geom = QgsGeometry(geom.intersection(tmpGeom))
                    if int_geom.wkbType() == QgsWkbTypes.Unknown or QgsWkbTypes.flatType(int_geom.geometry().wkbType()) == QgsWkbTypes.GeometryCollection:
                        int_com = geom.combine(tmpGeom)
                        int_geom = QgsGeometry()
                        if int_com:
                            int_sym = geom.symDifference(tmpGeom)
                            int_geom = QgsGeometry(int_com.difference(int_sym))
                    if int_geom.isEmpty() or not int_geom.isGeosValid():
                        QgsMessageLog.logMessage(self.tr('GEOS geoprocessing error: One or '
                                                         'more input features have invalid '
                                                         'geometry.'), self.tr('Processing'), QgsMessageLog.CRITICAL)
                    try:
                        if int_geom.wkbType() in wkbTypeGroups[wkbTypeGroups[int_geom.wkbType()]]:
                            outFeat.setGeometry(int_geom)
                            attrs = []
                            attrs.extend(atMapA)
                            attrs.extend(atMapB)
                            outFeat.setAttributes(attrs)
                            writer.addFeature(outFeat)
                    except:
                        QgsMessageLog.logMessage(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'), self.tr('Processing'), QgsMessageLog.INFO)
                        continue

        del writer
