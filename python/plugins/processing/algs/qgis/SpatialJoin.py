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

__author__ = 'Joshua Arnott'
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Joshua Arnott'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
    GeoAlgorithmExecutionException
from processing.core.ProcessingLog import ProcessingLog

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SpatialJoin(GeoAlgorithm):
    TARGET = "TARGET"
    JOIN = "JOIN"
    SUMMARY = "SUMMARY"
    STATS = "STATS"
    KEEP = "KEEP"
    OUTPUT = "OUTPUT"

    SUMMARYS = [
        'Take attributes of the first located feature',
        'Take summary of intersecting features'
    ]

    KEEPS = [
        'Only keep matching records',
        'Keep all records (including non-matching target records)'
    ]

    def defineCharacteristics(self):
        self.name = "Join atributes by location"
        self.group = "Vector general tools"

        self.addParameter(ParameterVector(self.TARGET,
           'Target vector layer', [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.JOIN,
           'Join vector layer', [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterSelection(self.SUMMARY,
           'Attribute summary', self.SUMMARYS))
        self.addParameter(ParameterString(self.STATS,
           'Statistics for summary (comma separated)',
           'sum,mean,min,max,median'))
        self.addParameter(ParameterSelection(self.KEEP,
           'Output table', self.KEEPS))
        self.addOutput(OutputVector(self.OUTPUT, 'Output layer'))

    def processAlgorithm(self, progress):
        target = dataobjects.getObjectFromUri(
            self.getParameterValue(self.TARGET))
        join = dataobjects.getObjectFromUri(
            self.getParameterValue(self.JOIN))

        summary = self.getParameterValue(self.SUMMARY) == 1
        keep = self.getParameterValue(self.KEEP) == 0

        sumList = self.getParameterValue(self.STATS).lower().split(',')

        targetProvider = target.dataProvider()
        joinProvider = join.dataProvider()

        targetFields = targetProvider.fields()
        joinFields = joinProvider.fields()

        fieldList = QgsFields()

        if not summary:
            joinFields = vector.testForUniqueness(targetFields, joinFields)
            seq = range(0, len(targetFields) + len(joinFields))
            targetFields.extend(joinFields)
            targetFields = dict(zip(seq, targetFields))
        else:
            numFields = {}
            for j in xrange(len(joinFields)):
                if joinFields[j].type() in [QVariant.Int, QVariant.Double]:
                    numFields[j] = []
                    for i in sumList:
                        field = QgsField(i + unicode(joinFields[j].name()), QVariant.Double, '', 24, 16)
                        fieldList.append(field)
            field = QgsField('count', QVariant.Double, '', 24, 16)
            fieldList.append(field)
            joinFields = vector.testForUniqueness(targetFields, fieldList)
            targetFields.extend(fieldList)
            seq = range(0, len(targetFields))
            targetFields = dict(zip(seq, targetFields))

        fields = QgsFields()
        for f in targetFields.values():
            fields.append(f)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, targetProvider.geometryType(), targetProvider.crs())

        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inFeatB = QgsFeature()
        inGeom = QgsGeometry()

        index = vector.spatialindex(join)


        # cache all features from provider2 to avoid huge number
        # of feature requests in the inner loop
        mapP2 = {}
        features = vector.features(join)
        for f in features:
            mapP2[f.id()] = QgsFeature(f)

        features = vector.features(target)

        count = 0
        total = 100.0 / len(features)

        for f in features:
            inGeom = f.geometry()
            atMap1 = f.attributes()
            outFeat.setGeometry(inGeom)
            none = True
            joinList = []
            if inGeom.type() == QGis.Point:
                joinList = index.intersects(inGeom.buffer(10,2).boundingBox())
                if len(joinList) > 0:
                    check = 0
                else:
                    check = 1
            else:
                joinList = index.intersects(inGeom.boundingBox())
                if len(joinList) > 0:
                    check = 0
                else:
                    check = 1

            if check == 0:
                count = 0
                for i in joinList:
                    inFeatB = mapP2[i]  # cached feature from provider2
                    if inGeom.intersects(inFeatB.geometry()):
                        count += 1
                        none = False
                        atMap2 = inFeatB.attributes()
                        if not summary:
                            atMap = atMap1
                            atMap2 = atMap2
                            atMap.extend(atMap2)
                            atMap = dict(zip(seq, atMap))
                            break
                        else:
                            for j in numFields.keys():
                                numFields[j].append(atMap2[j])
                if summary and not none:
                    atMap = atMap1
                    for j in numFields.keys():
                        for k in sumList:
                            if k == 'sum':
                                atMap.append(sum(self._filter_null(numFields[j])))
                            elif k == 'mean':
                                try:
                                    nn_count = sum(1 for _ in self._filter_null(numFields[j]))
                                    atMap.append(sum(self._filter_null(numFields[j])) / nn_count)
                                except ZeroDivisionError:
                                    atMap.append(NULL)
                            elif k == 'min':
                                try:
                                    atMap.append(min(self._filter_null(numFields[j])))
                                except ValueError:
                                    atMap.append(NULL)
                            elif k == 'median':
                                atMap.append(self._myself(numFields[j]))
                            else:
                                try:
                                    atMap.append(max(self._filter_null(numFields[j])))
                                except ValueError:
                                    atMap.append(NULL)

                        numFields[j] = []
                    atMap.append(count)
                    atMap = dict(zip(seq, atMap))

            if none:
                outFeat.setAttributes(atMap1)
            else:
                outFeat.setAttributes(atMap.values())

            if keep: # keep all records
                writer.addFeature(outFeat)
            else: # keep only matching records
                if not none:
                    writer.addFeature(outFeat)

            count += 1
            progress.setPercentage(int(count * total))

        del writer

    def _filter_null(self, vals):
        """Takes an iterator of values and returns a new iterator
        returning the same values but skipping any NULL values"""
        return (v for v in vals if v != NULL)

    def _myself(self, L):
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
