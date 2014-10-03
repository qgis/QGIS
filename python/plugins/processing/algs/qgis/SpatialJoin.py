# -*- coding: utf-8 -*-

"""
***************************************************************************
    SpatialJoin.py
    ---------------------
    Date                 : October 2013
    Copyright            : (C) 2013 by Joshua Arnott
    Email                : josh at snorfalorpagus dot net
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
__author__ = 'Joshua Arnott'
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Joshua Arnott'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from processing.core.GeoAlgorithm import GeoAlgorithm
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterString
from processing.tools import dataobjects
from processing.core.outputs import OutputVector
from processing.tools import vector
from processing.core.ProcessingLog import ProcessingLog
import os

def myself(L):
    #median computation
    nVal = len(L)
    if nVal == 1:
        return L[0]
    L.sort()
    #test for list length
    medianVal = 0
    if nVal > 1:
        if ( nVal % 2 ) == 0:
            #index begin at 0
            #remove 1 to index in standard median computation
            medianVal = 0.5 * ( (L[ (nVal) / 2  - 1]) + (L[ (nVal) / 2 ] ))
        else:
            medianVal = L[ (nVal + 1) / 2 - 1]
    return medianVal

class SpatialJoin(GeoAlgorithm):
    '''
    Join by location

    Port of the spatial join algorithm from fTools to the Processing Toolbox.
    '''

    INPUT1 = "INPUT1"
    INPUT2 = "INPUT2"
    SUMMARY = "SUMMARY"
    STATS = "STATS"
    GEOMETRY = "GEOMETRY"
    KEEP = "KEEP"
    OUTPUT = "OUTPUT"

    SUMMARYS = [
        'Take attributes of the first located feature',
        'Take summary of intersecting features'
    ]

    GEOMETRYS = [
        'Use geometry from target layer',
        'Use geometry from joined layer (multipart if summary)'
    ]

    KEEPS = [
        'Only keep matching records',
        'Keep all records (including non-matching target records)'
    ]

    #===========================================================================
    # def getIcon(self):
    #    return QIcon(os.path.dirname(__file__) + "/icons/join_location.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Join by location"
        self.group = "Vector general tools"
        self.addParameter(ParameterVector(SpatialJoin.INPUT1, "Target vector layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(SpatialJoin.INPUT2, "Join vector layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterSelection(self.SUMMARY, "Attribute summary", self.SUMMARYS, 0))
        self.addParameter(ParameterString(self.STATS, "Statistics for summary (comma separated)", "sum,mean,min,max,median"))
        self.addParameter(ParameterSelection(self.GEOMETRY, "Output geometry", self.GEOMETRYS, 0))
        self.addParameter(ParameterSelection(self.KEEP, "Output table", self.KEEPS, 0))
        self.addOutput(OutputVector(SpatialJoin.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        progress.setText('Analysing inputs...')

        summary = self.getParameterValue(self.SUMMARY) == 1
        sumList = self.getParameterValue(self.STATS).upper().replace(' ','').split(',')
        use_geom = self.getParameterValue(self.GEOMETRY)
        keep = self.getParameterValue(self.KEEP) == 1

        input1 = self.getParameterValue(self.INPUT1)
        layer1 = dataobjects.getObjectFromUri(input1)
        provider1 = layer1.dataProvider()
        fieldList1 = provider1.fields()

        input2 = self.getParameterValue(self.INPUT2)
        layer2 = dataobjects.getObjectFromUri(input2)
        provider2 = layer2.dataProvider()
        fieldList2 = provider2.fields()

        fieldList = QgsFields()
        if not summary:
            fieldList2 = vector.testForUniqueness(fieldList1, fieldList2)
            seq = range(0, len(fieldList1) + len(fieldList2))
            fieldList1.extend(fieldList2)
            fieldList1 = dict(zip(seq, fieldList1))
        else:
            numFields = {}
            for j in xrange(len(fieldList2)):
                if fieldList2[j].type() == QVariant.Int or fieldList2[j].type() == QVariant.Double:
                    numFields[j] = []
                    for i in sumList:
                        field = QgsField(i + unicode(fieldList2[j].name()), QVariant.Double, "real", 24, 16, "Summary field" )
                        fieldList.append(field)
            field = QgsField("COUNT", QVariant.Double, "real", 24, 0, "Summary field" )
            fieldList.append(field)
            fieldList2 = vector.testForUniqueness(fieldList1, fieldList)
            fieldList1.extend(fieldList)
            seq = range(0, len(fieldList1))
            fieldList1 = dict(zip(seq, fieldList1))

        progress.setPercentage(13)
        fields = QgsFields()
        for f in fieldList1.values():
            fields.append(f)
        output = self.getOutputFromName(self.OUTPUT)

        if use_geom == 0:
            # from target layer
            crs = provider1.crs()
            geometry_type = provider1.geometryType()
        else:
            # from joined layer
            crs = provider2.crs()
            if summary:
                geometry_type = self.singleToMultiGeom(provider2.geometryType())
            else:
                geometry_type = provider2.geometryType()

        writer = output.getVectorWriter(fields, geometry_type, crs)

        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inFeatB = QgsFeature()
        inGeom = QgsGeometry()

        progress.setPercentage(15)
        start = 15.00
        add = 85.00 / provider1.featureCount()

        progress.setText('Creating spatial index...')
        index = vector.spatialindex(layer2)
        progress.setText('Processing spatial join...')
        fit1 = provider1.getFeatures()
        while fit1.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            atMap1 = inFeat.attributes()
            if use_geom == 0:
                outFeat.setGeometry(inGeom)
            none = True
            joinList = []
            if inGeom.type() == QGis.Point:
                joinList = index.intersects( inGeom.buffer(10,2).boundingBox() )
                if len(joinList) > 0: check = 0
                else: check = 1
            else:
                joinList = index.intersects( inGeom.boundingBox() )
                if len(joinList) > 0: check = 0
                else: check = 1
            if check == 0:
                count = 0
                multi_feature = []
                for i in joinList:
                    provider2.getFeatures( QgsFeatureRequest().setFilterFid( int(i) ) ).nextFeature( inFeatB )
                    if inGeom.intersects(inFeatB.geometry()):
                        count = count + 1
                        atMap2 = inFeatB.attributes()
                        if not summary:
                            # first located feature
                            atMap = atMap1
                            atMap2 = atMap2
                            atMap.extend(atMap2)
                            atMap = dict(zip(seq, atMap))
                            if use_geom == 1:
                                outFeat.setGeometry(inFeatB.geometry())
                            none = False
                            break
                        else:
                            for j in numFields.keys():
                                numFields[j].append(atMap2[j])
                            if use_geom == 0:
                                if none:
                                    outFeat.setGeometry(inGeom)
                            else:
                                feature_list = self.extractAsMulti(inFeatB.geometry())
                                multi_feature.extend(feature_list)
                            none = False
                if summary and not none:
                    atMap = atMap1
                    for j in numFields.keys():
                        for k in sumList:
                            if k == "SUM": atMap.append(sum(numFields[j]))
                            elif k == "MEAN": atMap.append(sum(numFields[j]) / count)
                            elif k == "MIN": atMap.append(min(numFields[j]))
                            elif k == "MEDIAN": atMap.append(myself(numFields[j]))
                            else: atMap.append(max(numFields[j]))
                        numFields[j] = []
                    atMap.append(count)
                    atMap = dict(zip(seq, atMap))
                    if use_geom == 1:
                        outGeom = QgsGeometry(self.convertGeometry(multi_feature, geometry_type))
                        outFeat.setGeometry(outGeom)
            if none:
                outFeat.setAttributes(atMap1)
            else:
                outFeat.setAttributes(atMap.values())
            if keep: # keep all records
                writer.addFeature(outFeat)
            else: # keep only matching records
                if not none:
                    writer.addFeature(outFeat)
            start = start + add
            progress.setPercentage(start)

        del writer

    def singleToMultiGeom(self, wkbType):
        try:
            if wkbType in (QGis.WKBPoint, QGis.WKBMultiPoint,
                           QGis.WKBPoint25D, QGis.WKBMultiPoint25D):
                return QGis.WKBMultiPoint
            elif wkbType in (QGis.WKBLineString, QGis.WKBMultiLineString,
                             QGis.WKBMultiLineString25D,
                             QGis.WKBLineString25D):

                return QGis.WKBMultiLineString
            elif wkbType in (QGis.WKBPolygon, QGis.WKBMultiPolygon,
                             QGis.WKBMultiPolygon25D, QGis.WKBPolygon25D):

                return QGis.WKBMultiPolygon
            else:
                return QGis.WKBUnknown
        except Exception, err:
            pass

    def extractAsMulti(self, geom):
        if geom.type() == QGis.Point:
            if geom.isMultipart():
                return geom.asMultiPoint()
            else:
                return [geom.asPoint()]
        elif geom.type() == QGis.Line:
            if geom.isMultipart():
                return geom.asMultiPolyline()
            else:
                return [geom.asPolyline()]
        else:
            if geom.isMultipart():
                return geom.asMultiPolygon()
            else:
                return [geom.asPolygon()]

    def convertGeometry(self, geom_list, vType):
        if vType == QGis.Point:
            return QgsGeometry().fromMultiPoint(geom_list)
        elif vType == QGis.Line:
            return QgsGeometry().fromMultiPolyline(geom_list)
        else:
            return QgsGeometry().fromMultiPolygon(geom_list)
