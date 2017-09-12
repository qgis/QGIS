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

from qgis.core import QGis, QgsFeatureRequest, QgsFeature, QgsGeometry, QgsWKBTypes

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector, ParameterBoolean
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]

wkbTypeGroups = {
    'Point': (QGis.WKBPoint, QGis.WKBMultiPoint, QGis.WKBPoint25D, QGis.WKBMultiPoint25D,),
    'LineString': (QGis.WKBLineString, QGis.WKBMultiLineString, QGis.WKBLineString25D, QGis.WKBMultiLineString25D,),
    'Polygon': (QGis.WKBPolygon, QGis.WKBMultiPolygon, QGis.WKBPolygon25D, QGis.WKBMultiPolygon25D,),
}
for key, value in wkbTypeGroups.items():
    for const in value:
        wkbTypeGroups[const] = key


class Intersection(GeoAlgorithm):

    INPUT = 'INPUT'
    INPUT2 = 'INPUT2'
    IGNORE_NULL = 'IGNORE_NULL'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'intersect.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Intersection')
        self.group, self.i18n_group = self.trAlgorithm('Vector overlay tools')
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.INPUT2,
                                          self.tr('Intersect layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterBoolean(Intersection.IGNORE_NULL,
                                           self.tr('Ignore NULL geometries'),
                                           False, True))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Intersection')))

    def processAlgorithm(self, progress):
        vlayerA = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        vlayerB = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT2))
        ignoreNull = self.getParameterValue(Intersection.IGNORE_NULL)

        geomType = QgsWKBTypes.multiType(QGis.fromOldWkbType(vlayerA.wkbType()))
        fields = vector.combineVectorFields(vlayerA, vlayerB)
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     geomType, vlayerA.crs())
        outFeat = QgsFeature()
        index = vector.spatialindex(vlayerB)
        selectionA = vector.features(vlayerA)
        total = 100.0 / len(selectionA) if len(selectionA) > 0 else 1
        for current, inFeatA in enumerate(selectionA):
            progress.setPercentage(int(current * total))
            geom = inFeatA.geometry()
            if not geom:
                if ignoreNull:
                    continue
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Input layer A contains NULL geometries. '
                                'Please check "Ignore NULL geometries" '
                                'if you want to run this algorithm anyway.'))
            if not geom.isGeosValid():
                raise GeoAlgorithmExecutionException(
                    self.tr('Input layer A contains invalid geometries '
                            '(feature {}). Unable to complete intersection '
                            'algorithm.'.format(inFeatA.id())))
            atMapA = inFeatA.attributes()
            intersects = index.intersects(geom.boundingBox())
            for inFeatB in vlayerB.getFeatures(QgsFeatureRequest().setFilterFids(intersects)):
                tmpGeom = QgsGeometry(inFeatB.geometry())
                if not geom:
                    if ignoreNull:
                        continue
                    else:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Input layer B contains NULL geometries. '
                                    'Please check "Ignore NULL geometries" '
                                    'if you want to run this algorithm anyway.'))
                if not geom.isGeosValid():
                    raise GeoAlgorithmExecutionException(
                        self.tr('Input layer B contains invalid geometries '
                                '(feature {}). Unable to complete intersection '
                                'algorithm.'.format(inFeatB.id())))

                if geom.intersects(tmpGeom):
                    atMapB = inFeatB.attributes()
                    int_geom = QgsGeometry(geom.intersection(tmpGeom))
                    if int_geom.wkbType() == QGis.WKBUnknown or QgsWKBTypes.flatType(int_geom.geometry().wkbType()) == QgsWKBTypes.GeometryCollection:
                        int_com = geom.combine(tmpGeom)
                        int_geom = QgsGeometry()
                        if int_com is not None:
                            int_sym = geom.symDifference(tmpGeom)
                            if int_sym:
                                diff_geom = int_com.difference(int_sym)
                                int_geom = QgsGeometry(diff_geom)

                    if int_geom.isGeosEmpty() or not int_geom.isGeosValid():
                        raise GeoAlgorithmExecutionException(
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
                            writer.addFeature(outFeat)
                    except:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Feature geometry error: one or '
                                    'more output features ignored due '
                                    'to invalid geometry.'))

        del writer
