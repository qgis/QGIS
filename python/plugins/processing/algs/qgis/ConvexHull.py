# -*- coding: utf-8 -*-

"""
***************************************************************************
    ConvexHull.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsField,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsFields,
                       NULL,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessing,
                       QgsProcessingException)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ConvexHull(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'convex_hull.png'))

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Field (optional, set if creating convex hulls by classes)'),
                                                      parentLayerParameterName=self.INPUT, optional=True))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Convex hull'), QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'convexhull'

    def displayName(self):
        return self.tr('Convex hull')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        fieldName = self.parameterAsString(parameters, self.FIELD, context)
        useField = bool(fieldName)

        field_index = None
        f = QgsField('value', QVariant.String, '', 255)
        if useField:
            field_index = source.fields().lookupField(fieldName)
            fType = source.fields()[field_index].type()
            if fType in [QVariant.Int, QVariant.UInt, QVariant.LongLong, QVariant.ULongLong]:
                f.setType(fType)
                f.setLength(20)
            elif fType == QVariant.Double:
                f.setType(QVariant.Double)
                f.setLength(20)
                f.setPrecision(6)
            else:
                f.setType(QVariant.String)
                f.setLength(255)

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 20))
        fields.append(f)
        fields.append(QgsField('area', QVariant.Double, '', 20, 6))
        fields.append(QgsField('perim', QVariant.Double, '', 20, 6))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Polygon, source.sourceCrs())

        outFeat = QgsFeature()
        outGeom = QgsGeometry()

        fid = 0
        val = None
        if useField:
            unique = source.uniqueValues(field_index)
            current = 0
            total = 100.0 / (source.featureCount() * len(unique)) if source.featureCount() else 1
            for i in unique:
                if feedback.isCanceled():
                    break

                first = True
                hull = []
                features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([field_index]))
                for f in features:
                    if feedback.isCanceled():
                        break

                    idVar = f.attributes()[field_index]
                    if str(idVar).strip() == str(i).strip():
                        if first:
                            val = idVar
                            first = False

                        inGeom = f.geometry()
                        points = vector.extractPoints(inGeom)
                        hull.extend(points)
                    current += 1
                    feedback.setProgress(int(current * total))

                if len(hull) >= 3:
                    tmpGeom = QgsGeometry(outGeom.fromMultiPoint(hull))
                    try:
                        outGeom = tmpGeom.convexHull()
                        if outGeom:
                            area = outGeom.geometry().area()
                            perim = outGeom.geometry().perimeter()
                        else:
                            area = NULL
                            perim = NULL
                        outFeat.setGeometry(outGeom)
                        outFeat.setAttributes([fid, val, area, perim])
                        sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                    except:
                        raise QgsProcessingException(
                            self.tr('Exception while computing convex hull'))
                fid += 1
        else:
            hull = []
            total = 100.0 / source.featureCount() if source.featureCount() else 1
            features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]))
            for current, f in enumerate(features):
                if feedback.isCanceled():
                    break

                inGeom = f.geometry()
                points = vector.extractPoints(inGeom)
                hull.extend(points)
                feedback.setProgress(int(current * total))

            tmpGeom = QgsGeometry(outGeom.fromMultiPoint(hull))
            try:
                outGeom = tmpGeom.convexHull()
                if outGeom:
                    area = outGeom.geometry().area()
                    perim = outGeom.geometry().perimeter()
                else:
                    area = NULL
                    perim = NULL
                outFeat.setGeometry(outGeom)
                outFeat.setAttributes([0, 'all', area, perim])
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            except:
                raise QgsProcessingException(
                    self.tr('Exception while computing convex hull'))

        return {self.OUTPUT: dest_id}
