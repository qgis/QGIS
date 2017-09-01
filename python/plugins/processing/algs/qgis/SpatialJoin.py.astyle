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
from builtins import str
from builtins import zip
from builtins import range

__author__ = 'Joshua Arnott'
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Joshua Arnott'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import QgsFields, QgsField, QgsFeatureSink, QgsFeature, QgsGeometry, NULL, QgsWkbTypes, QgsProcessingUtils

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SpatialJoin(QgisAlgorithm):
    TARGET = "TARGET"
    JOIN = "JOIN"
    PREDICATE = "PREDICATE"
    PRECISION = 'PRECISION'
    SUMMARY = "SUMMARY"
    STATS = "STATS"
    KEEP = "KEEP"
    OUTPUT = "OUTPUT"

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'join_location.png'))

    def group(self):
        return self.tr('Vector general')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.predicates = (
            ('intersects', self.tr('intersects')),
            ('contains', self.tr('contains')),
            ('equals', self.tr('equals')),
            ('touches', self.tr('touches')),
            ('overlaps', self.tr('overlaps')),
            ('within', self.tr('within')),
            ('crosses', self.tr('crosses')))

        self.summarys = [
            self.tr('Take attributes of the first located feature'),
            self.tr('Take summary of intersecting features')
        ]

        self.keeps = [
            self.tr('Only keep matching records'),
            self.tr('Keep all records (including non-matching target records)')
        ]

        self.addParameter(ParameterVector(self.TARGET,
                                          self.tr('Target vector layer')))
        self.addParameter(ParameterVector(self.JOIN,
                                          self.tr('Join vector layer')))
        self.addParameter(ParameterSelection(self.PREDICATE,
                                             self.tr('Geometric predicate'),
                                             self.predicates,
                                             multiple=True))
        self.addParameter(ParameterNumber(self.PRECISION,
                                          self.tr('Precision'),
                                          0.0, None, 0.0))
        self.addParameter(ParameterSelection(self.SUMMARY,
                                             self.tr('Attribute summary'), self.summarys))
        self.addParameter(ParameterString(self.STATS,
                                          self.tr('Statistics for summary (comma separated)'),
                                          'sum,mean,min,max,median', optional=True))
        self.addParameter(ParameterSelection(self.KEEP,
                                             self.tr('Joined table'), self.keeps))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Joined layer')))

    def name(self):
        return 'joinattributesbylocation'

    def displayName(self):
        return self.tr('Join attributes by location')

    def processAlgorithm(self, parameters, context, feedback):
        target = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.TARGET), context)
        join = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.JOIN), context)
        predicates = self.getParameterValue(self.PREDICATE)
        precision = self.getParameterValue(self.PRECISION)

        summary = self.getParameterValue(self.SUMMARY) == 1
        keep = self.getParameterValue(self.KEEP) == 1

        sumList = self.getParameterValue(self.STATS).lower().split(',')

        targetFields = target.fields()
        joinFields = join.fields()

        fieldList = QgsFields()

        if not summary:
            joinFields = vector.testForUniqueness(targetFields, joinFields)
            seq = list(range(len(targetFields) + len(joinFields)))
            targetFields.extend(joinFields)
            targetFields = dict(list(zip(seq, targetFields)))
        else:
            numFields = {}
            for j in range(len(joinFields)):
                if joinFields[j].type() in [QVariant.Int, QVariant.Double, QVariant.LongLong, QVariant.UInt, QVariant.ULongLong]:
                    numFields[j] = []
                    for i in sumList:
                        field = QgsField(i + str(joinFields[j].name()), QVariant.Double, '', 24, 16)
                        fieldList.append(field)
            field = QgsField('count', QVariant.Double, '', 24, 16)
            fieldList.append(field)
            joinFields = vector.testForUniqueness(targetFields, fieldList)
            targetFields.extend(fieldList)
            seq = list(range(len(targetFields)))
            targetFields = dict(list(zip(seq, targetFields)))

        fields = QgsFields()
        for f in list(targetFields.values()):
            fields.append(f)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields, target.wkbType(), target.crs(), context)

        outFeat = QgsFeature()
        inFeatB = QgsFeature()
        inGeom = QgsGeometry()

        index = QgsProcessingUtils.createSpatialIndex(join, context)

        mapP2 = dict()
        features = QgsProcessingUtils.getFeatures(join, context)
        for f in features:
            mapP2[f.id()] = QgsFeature(f)

        features = QgsProcessingUtils.getFeatures(target, context)
        total = 100.0 / target.featureCount() if target.featureCount() else 0
        for c, f in enumerate(features):
            atMap1 = f.attributes()
            outFeat.setGeometry(f.geometry())
            inGeom = vector.snapToPrecision(f.geometry(), precision)
            none = True
            joinList = []
            if inGeom.type() == QgsWkbTypes.PointGeometry:
                bbox = inGeom.buffer(10, 2).boundingBox()
            else:
                bbox = inGeom.boundingBox()
            bbox.grow(0.51 * precision)
            joinList = index.intersects(bbox)
            if len(joinList) > 0:
                count = 0
                for i in joinList:
                    inFeatB = mapP2[i]
                    inGeomB = vector.snapToPrecision(inFeatB.geometry(), precision)

                    res = False
                    for predicate in predicates:
                        res = getattr(inGeom, predicate)(inGeomB)
                        if res:
                            break

                    if res:
                        count = count + 1
                        none = False
                        atMap2 = inFeatB.attributes()
                        if not summary:
                            atMap = atMap1
                            atMap2 = atMap2
                            atMap.extend(atMap2)
                            atMap = dict(list(zip(seq, atMap)))
                            break
                        else:
                            for j in list(numFields.keys()):
                                numFields[j].append(atMap2[j])

                if summary and not none:
                    atMap = atMap1
                    for j in list(numFields.keys()):
                        for k in sumList:
                            if k == 'sum':
                                atMap.append(sum(self._filterNull(numFields[j])))
                            elif k == 'mean':
                                try:
                                    nn_count = sum(1 for _ in self._filterNull(numFields[j]))
                                    atMap.append(sum(self._filterNull(numFields[j])) / nn_count)
                                except ZeroDivisionError:
                                    atMap.append(NULL)
                            elif k == 'min':
                                try:
                                    atMap.append(min(self._filterNull(numFields[j])))
                                except ValueError:
                                    atMap.append(NULL)
                            elif k == 'median':
                                atMap.append(self._median(numFields[j]))
                            else:
                                try:
                                    atMap.append(max(self._filterNull(numFields[j])))
                                except ValueError:
                                    atMap.append(NULL)

                        numFields[j] = []
                    atMap.append(count)
                    atMap = dict(list(zip(seq, atMap)))
            if none:
                outFeat.setAttributes(atMap1)
            else:
                outFeat.setAttributes(list(atMap.values()))

            if keep:
                writer.addFeature(outFeat, QgsFeatureSink.FastInsert)
            else:
                if not none:
                    writer.addFeature(outFeat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(c * total))
        del writer

    def _filterNull(self, values):
        """Takes an iterator of values and returns a new iterator
        returning the same values but skipping any NULL values"""
        return (v for v in values if v != NULL)

    def _median(self, data):
        count = len(data)
        if count == 1:
            return data[0]
        data.sort()

        median = 0
        if count > 1:
            if (count % 2) == 0:
                median = 0.5 * ((data[count / 2 - 1]) + (data[count / 2]))
            else:
                median = data[(count + 1) / 2 - 1]

        return median
