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
from processing.core.parameters import ParameterVector
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
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Intersection')))

    def processAlgorithm(self, progress):
        vlayerA = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        vlayerB = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT2))
        vproviderA = vlayerA.dataProvider()

        geomType = vproviderA.geometryType()
        fields = vector.combineVectorFields(vlayerA, vlayerB)
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     geomType, vproviderA.crs())
        outFeat = QgsFeature()
        index = vector.spatialindex(vlayerB)
        selectionA = vector.features(vlayerA)
        total = 100.0 / len(selectionA)
        for current, inFeatA in enumerate(selectionA):
            progress.setPercentage(int(current * total))
            geom = QgsGeometry(inFeatA.geometry())
            atMapA = inFeatA.attributes()
            intersects = index.intersects(geom.boundingBox())
            for i in intersects:
                request = QgsFeatureRequest().setFilterFid(i)
                inFeatB = vlayerB.getFeatures(request).next()
                tmpGeom = QgsGeometry(inFeatB.geometry())
                if geom.intersects(tmpGeom):
                    atMapB = inFeatB.attributes()
                    int_geom = QgsGeometry(geom.intersection(tmpGeom))
                    if int_geom.wkbType() == QGis.WKBUnknown or QgsWKBTypes.flatType(int_geom.geometry().wkbType()) == QgsWKBTypes.GeometryCollection:
                        int_com = geom.combine(tmpGeom)
                        int_sym = geom.symDifference(tmpGeom)
                        int_geom = QgsGeometry(int_com.difference(int_sym))
                    if int_geom.isGeosEmpty() or not int_geom.isGeosValid():
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
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
                        ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                               self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
                        continue

        del writer
