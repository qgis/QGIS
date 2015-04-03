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

from qgis.core import QGis, QgsFeatureRequest, QgsFeature, QgsGeometry
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

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

    def processAlgorithm(self, progress):
        vlayerA = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        vlayerB = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT2))
        vproviderA = vlayerA.dataProvider()

        fields = vector.combineVectorFields(vlayerA, vlayerB)
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                vproviderA.geometryType(), vproviderA.crs())
        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        index = vector.spatialindex(vlayerB)
        nElement = 0
        selectionA = vector.features(vlayerA)
        nFeat = len(selectionA)
        for inFeatA in selectionA:
            nElement += 1
            progress.setPercentage(nElement / float(nFeat) * 100)
            geom = QgsGeometry(inFeatA.geometry())
            atMapA = inFeatA.attributes()
            intersects = index.intersects(geom.boundingBox())
            for i in intersects:
                request = QgsFeatureRequest().setFilterFid(i)
                inFeatB = vlayerB.getFeatures(request).next()
                tmpGeom = QgsGeometry(inFeatB.geometry())
                try:
                    if geom.intersects(tmpGeom):
                        atMapB = inFeatB.attributes()
                        int_geom = QgsGeometry(geom.intersection(tmpGeom))
                        if int_geom.wkbType() == QGis.WKBUnknown:
                            int_com = geom.combine(tmpGeom)
                            int_sym = geom.symDifference(tmpGeom)
                            int_geom = QgsGeometry(int_com.difference(int_sym))
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
                except:
                    break

        del writer

    def defineCharacteristics(self):
        self.name = 'Intersection'
        self.group = 'Vector overlay tools'
        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.INPUT2,
            self.tr('Intersect layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Intersection')))
