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

from qgis.core import QgsFeatureRequest, QgsFeature, QgsGeometry
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class Union(GeoAlgorithm):

    INPUT = 'INPUT'
    INPUT2 = 'INPUT2'
    OUTPUT = 'OUTPUT'

    def processAlgorithm(self, progress):
        vlayerA = dataobjects.getObjectFromUri(self.getParameterValue(Union.INPUT))
        vlayerB = dataobjects.getObjectFromUri(self.getParameterValue(Union.INPUT2))
        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True
        vproviderA = vlayerA.dataProvider()

        fields = vector.combineVectorFields(vlayerA, vlayerB)
        names = [field.name() for field in fields]
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, str(names))
        writer = self.getOutputFromName(Union.OUTPUT).getVectorWriter(fields,
                vproviderA.geometryType(), vproviderA.crs())
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
                    raise GeoAlgorithmExecutionException(
                        self.tr('Feature exception while computing union'))
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
                            raise GeoAlgorithmExecutionException(
                                self.tr('Geometry exception while computing '
                                        'intersection'))
                        else:
                            int_geom = QgsGeometry(int_geom)

                        if int_geom.wkbType() == 0:
                            # Intersection produced different geomety types
                            temp_list = int_geom.asGeometryCollection()
                            for i in temp_list:
                                if i.type() == geom.type():
                                    int_geom = QgsGeometry(i)
                        try:
                            outFeat.setGeometry(int_geom)
                            attrs = []
                            attrs.extend(atMapA)
                            attrs.extend(atMapB)
                            outFeat.setAttributes(attrs)
                            writer.addFeature(outFeat)
                        except Exception, err:
                            raise GeoAlgorithmExecutionException(
                                self.tr('Feature exception while computing union'))

                try:
                        # the remaining bit of inFeatA's geometry
                        # if there is nothing left, this will just silently fail and we're good
                        diff_geom = QgsGeometry( geom )
                        if len(lstIntersectingB) != 0:
                            intB = QgsGeometry.unaryUnion(lstIntersectingB)
                            diff_geom = diff_geom.difference(intB)

                        if diff_geom.wkbType() == 0:
                            temp_list = diff_geom.asGeometryCollection()
                            for i in temp_list:
                                if i.type() == geom.type():
                                    diff_geom = QgsGeometry(i)
                        outFeat.setGeometry(diff_geom)
                        outFeat.setAttributes(atMapA)
                        writer.addFeature(outFeat)
                except Exception, err:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Feature exception while computing union'))

        length = len(vproviderA.fields())

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
                except Exception, err:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Feature exception while computing union'))
            else:
                for id in intersects:
                    request = QgsFeatureRequest().setFilterFid(id)
                    inFeatB = vlayerA.getFeatures(request).next()
                    atMapB = inFeatB.attributes()
                    tmpGeom = QgsGeometry(inFeatB.geometry())
                    try:
                        if diff_geom.intersects(tmpGeom):
                            add = True
                            diff_geom = QgsGeometry(
                                diff_geom.difference(tmpGeom))
                        else:
                            # Ihis only happends if the bounding box
                            # intersects, but the geometry doesn't
                            outFeat.setGeometry(diff_geom)
                            outFeat.setAttributes(atMap)
                            writer.addFeature(outFeat)
                    except Exception, err:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Geometry exception while computing intersection'))

            if add:
                try:
                    outFeat.setGeometry(diff_geom)
                    outFeat.setAttributes(atMap)
                    writer.addFeature(outFeat)
                except Exception, err:
                    raise err
                    FEATURE_EXCEPT = False
            nElement += 1

        del writer
        if not GEOS_EXCEPT:
            ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                self.tr('Geometry exception while computing intersection'))
        if not FEATURE_EXCEPT:
            ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                self.tr('Feature exception while computing intersection'))

    def defineCharacteristics(self):
        self.name = 'Union'
        self.group = 'Vector overlay tools'
        self.addParameter(ParameterVector(Union.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(Union.INPUT2,
            self.tr('Input layer 2'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(Union.OUTPUT, self.tr('Union')))
