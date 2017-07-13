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
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsSpatialIndex,
                       QgsProcessingUtils)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]

wkbTypeGroups = {
    'Point': (QgsWkbTypes.Point, QgsWkbTypes.MultiPoint, QgsWkbTypes.Point25D, QgsWkbTypes.MultiPoint25D,),
    'LineString': (QgsWkbTypes.LineString, QgsWkbTypes.MultiLineString, QgsWkbTypes.LineString25D, QgsWkbTypes.MultiLineString25D,),
    'Polygon': (QgsWkbTypes.Polygon, QgsWkbTypes.MultiPolygon, QgsWkbTypes.Polygon25D, QgsWkbTypes.MultiPolygon25D,),
}
for key, value in list(wkbTypeGroups.items()):
    for const in value:
        wkbTypeGroups[const] = key


class Intersection(QgisAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'intersect.png'))

    def group(self):
        return self.tr('Vector overlay tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.OVERLAY,
                                                              self.tr('Intersection layer')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Intersection')))

    def name(self):
        return 'intersection'

    def displayName(self):
        return self.tr('Intersection')

    def processAlgorithm(self, parameters, context, feedback):
        sourceA = self.parameterAsSource(parameters, self.INPUT, context)
        sourceB = self.parameterAsSource(parameters, self.OVERLAY, context)

        geomType = QgsWkbTypes.multiType(sourceA.wkbType())
        fields = vector.combineFields(sourceA.fields(), sourceB.fields())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, geomType, sourceA.sourceCrs())

        outFeat = QgsFeature()
        indexB = QgsSpatialIndex(sourceB.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(sourceA.sourceCrs())))

        total = 100.0 / sourceA.featureCount() if sourceA.featureCount() else 1
        count = 0

        for featA in sourceA.getFeatures():
            if feedback.isCanceled():
                break

            geom = featA.geometry()
            atMapA = featA.attributes()
            intersects = indexB.intersects(geom.boundingBox())

            request = QgsFeatureRequest().setFilterFids(intersects)
            request.setDestinationCrs(sourceA.sourceCrs())

            engine = None
            if len(intersects) > 0:
                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(geom.geometry())
                engine.prepareGeometry()

            for featB in sourceB.getFeatures(request):
                if feedback.isCanceled():
                    break

                tmpGeom = featB.geometry()
                if engine.intersects(tmpGeom.geometry()):
                    atMapB = featB.attributes()
                    int_geom = QgsGeometry(geom.intersection(tmpGeom))
                    if int_geom.wkbType() == QgsWkbTypes.Unknown or QgsWkbTypes.flatType(int_geom.geometry().wkbType()) == QgsWkbTypes.GeometryCollection:
                        int_com = geom.combine(tmpGeom)
                        int_geom = QgsGeometry()
                        if int_com:
                            int_sym = geom.symDifference(tmpGeom)
                            int_geom = QgsGeometry(int_com.difference(int_sym))
                    if int_geom.isEmpty() or not int_geom.isGeosValid():
                        raise QgsProcessingException(
                            self.tr('GEOS geoprocessing error: One or '
                                    'more input features have invalid '
                                    'geometry.'))
                    try:
                        if int_geom.wkbType() in wkbTypeGroups[wkbTypeGroups[int_geom.wkbType()]]:
                            outFeat.setGeometry(int_geom)
                            attrs = []
                            attrs.extend(atMapA)
                            attrs.extend(atMapB)
                            outFeat.setAttributes(attrs)
                            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                    except:
                        raise QgsProcessingException(
                            self.tr('Feature geometry error: One or more '
                                    'output features ignored due to invalid '
                                    'geometry.'))

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}
