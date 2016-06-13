# -*- coding: utf-8 -*-

"""
***************************************************************************
    Union.py
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


class Union(GeoAlgorithm):

    INPUT = 'INPUT'
    INPUT2 = 'INPUT2'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'union.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Union')
        self.group, self.i18n_group = self.trAlgorithm('Vector overlay tools')
        self.addParameter(ParameterVector(Union.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(Union.INPUT2,
                                          self.tr('Input layer 2'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(Union.OUTPUT, self.tr('Union')))

    def processAlgorithm(self, progress):
        vlayerA = dataobjects.getObjectFromUri(self.getParameterValue(Union.INPUT))
        vlayerB = dataobjects.getObjectFromUri(self.getParameterValue(Union.INPUT2))

        vproviderA = vlayerA.dataProvider()

        geomType = vproviderA.geometryType()
        fields = vector.combineVectorFields(vlayerA, vlayerB)
        writer = self.getOutputFromName(Union.OUTPUT).getVectorWriter(fields,
                                                                      geomType, vproviderA.crs())
        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        indexA = vector.spatialindex(vlayerB)
        indexB = vector.spatialindex(vlayerA)

        count = 0
        nElement = 0
        featuresA = vector.features(vlayerA)
        nFeat = len(featuresA)
        for inFeatA in featuresA:
            progress.setPercentage(nElement / float(nFeat) * 50)
            nElement += 1
            lstIntersectingB = []
            geom = QgsGeometry(inFeatA.geometry())
            atMapA = inFeatA.attributes()
            intersects = indexA.intersects(geom.boundingBox())
            if len(intersects) < 1:
                try:
                    outFeat.setGeometry(geom)
                    outFeat.setAttributes(atMapA)
                    writer.addFeature(outFeat)
                except:
                    # This really shouldn't happen, as we haven't
                    # edited the input geom at all
                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                           self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
            else:
                for id in intersects:
                    count += 1
                    request = QgsFeatureRequest().setFilterFid(id)
                    inFeatB = vlayerB.getFeatures(request).next()
                    atMapB = inFeatB.attributes()
                    tmpGeom = QgsGeometry(inFeatB.geometry())

                    if geom.intersects(tmpGeom):
                        int_geom = geom.intersection(tmpGeom)
                        lstIntersectingB.append(tmpGeom)

                        if int_geom is None:
                            # There was a problem creating the intersection
                            ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                                   self.tr('GEOS geoprocessing error: One or more input features have invalid geometry.'))
                            int_geom = QgsGeometry()
                        else:
                            int_geom = QgsGeometry(int_geom)

                        if int_geom.wkbType() == QGis.WKBUnknown or QgsWKBTypes.flatType(int_geom.geometry().wkbType()) == QgsWKBTypes.GeometryCollection:
                            # Intersection produced different geomety types
                            temp_list = int_geom.asGeometryCollection()
                            for i in temp_list:
                                if i.type() == geom.type():
                                    int_geom = QgsGeometry(i)
                                    try:
                                        outFeat.setGeometry(int_geom)
                                        outFeat.setAttributes(atMapA + atMapB)
                                        writer.addFeature(outFeat)
                                    except:
                                        ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                                               self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
                        else:
                            # Geometry list: prevents writing error
                            # in geometries of different types
                            # produced by the intersection
                            # fix #3549
                            if int_geom.wkbType() in wkbTypeGroups[wkbTypeGroups[int_geom.wkbType()]]:
                                try:
                                    outFeat.setGeometry(int_geom)
                                    outFeat.setAttributes(atMapA + atMapB)
                                    writer.addFeature(outFeat)
                                except:
                                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                                           self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))

                # the remaining bit of inFeatA's geometry
                # if there is nothing left, this will just silently fail and we're good
                diff_geom = QgsGeometry(geom)
                if len(lstIntersectingB) != 0:
                    intB = QgsGeometry.unaryUnion(lstIntersectingB)
                    diff_geom = diff_geom.difference(intB)
                    if diff_geom.isGeosEmpty() or not diff_geom.isGeosValid():
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                               self.tr('GEOS geoprocessing error: One or more input features have invalid geometry.'))

                if diff_geom.wkbType() == 0 or QgsWKBTypes.flatType(diff_geom.geometry().wkbType()) == QgsWKBTypes.GeometryCollection:
                    temp_list = diff_geom.asGeometryCollection()
                    for i in temp_list:
                        if i.type() == geom.type():
                            diff_geom = QgsGeometry(i)
                try:
                    outFeat.setGeometry(diff_geom)
                    outFeat.setAttributes(atMapA)
                    writer.addFeature(outFeat)
                except:
                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                           self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))

        length = len(vproviderA.fields())
        atMapA = [None] * length

        featuresA = vector.features(vlayerB)
        nFeat = len(featuresA)
        for inFeatA in featuresA:
            progress.setPercentage(nElement / float(nFeat) * 100)
            add = False
            geom = QgsGeometry(inFeatA.geometry())
            diff_geom = QgsGeometry(geom)
            atMap = [None] * length
            atMap.extend(inFeatA.attributes())
            intersects = indexB.intersects(geom.boundingBox())

            if len(intersects) < 1:
                try:
                    outFeat.setGeometry(geom)
                    outFeat.setAttributes(atMap)
                    writer.addFeature(outFeat)
                except:
                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                           self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
            else:
                for id in intersects:
                    request = QgsFeatureRequest().setFilterFid(id)
                    inFeatB = vlayerA.getFeatures(request).next()
                    atMapB = inFeatB.attributes()
                    tmpGeom = QgsGeometry(inFeatB.geometry())

                    if diff_geom.intersects(tmpGeom):
                        add = True
                        diff_geom = QgsGeometry(diff_geom.difference(tmpGeom))
                        if diff_geom.isGeosEmpty() or not diff_geom.isGeosValid():
                            ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                                   self.tr('GEOS geoprocessing error: One or more input features have invalid geometry.'))
                    else:
                        try:
                            # Ihis only happends if the bounding box
                            # intersects, but the geometry doesn't
                            outFeat.setGeometry(diff_geom)
                            outFeat.setAttributes(atMap)
                            writer.addFeature(outFeat)
                        except:
                            ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                                   self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))

            if add:
                try:
                    outFeat.setGeometry(diff_geom)
                    outFeat.setAttributes(atMap)
                    writer.addFeature(outFeat)
                except:
                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                           self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
            nElement += 1

        del writer
