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

from qgis.core import (QgsFeatureRequest,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsSpatialIndex)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Union(QgisAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'union.png'))

    def group(self):
        return self.tr('Vector overlay')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.OVERLAY,
                                                              self.tr('Union layer')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Union')))

    def name(self):
        return 'union'

    def displayName(self):
        return self.tr('Union')

    def processAlgorithm(self, parameters, context, feedback):
        sourceA = self.parameterAsSource(parameters, self.INPUT, context)
        sourceB = self.parameterAsSource(parameters, self.OVERLAY, context)

        geomType = QgsWkbTypes.multiType(sourceA.wkbType())
        fields = QgsProcessingUtils.combineFields(sourceA.fields(), sourceB.fields())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, geomType, sourceA.sourceCrs())

        featA = QgsFeature()
        featB = QgsFeature()
        outFeat = QgsFeature()

        indexA = QgsSpatialIndex(sourceA, feedback)
        indexB = QgsSpatialIndex(sourceB.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(sourceA.sourceCrs())), feedback)

        total = 100.0 / (sourceA.featureCount() * sourceB.featureCount()) if sourceA.featureCount() and sourceB.featureCount() else 1
        count = 0

        for featA in sourceA.getFeatures():
            if feedback.isCanceled():
                break

            lstIntersectingB = []
            geom = featA.geometry()
            atMapA = featA.attributes()
            intersects = indexB.intersects(geom.boundingBox())
            if len(intersects) < 1:
                try:
                    geom.convertToMultiType()
                    outFeat.setGeometry(geom)
                    outFeat.setAttributes(atMapA)
                    sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                except:
                    # This really shouldn't happen, as we haven't
                    # edited the input geom at all
                    feedback.pushInfo(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
            else:
                request = QgsFeatureRequest().setFilterFids(intersects).setSubsetOfAttributes([])
                request.setDestinationCrs(sourceA.sourceCrs())

                engine = QgsGeometry.createGeometryEngine(geom.constGet())
                engine.prepareGeometry()

                for featB in sourceB.getFeatures(request):
                    atMapB = featB.attributes()
                    tmpGeom = featB.geometry()

                    if engine.intersects(tmpGeom.constGet()):
                        int_geom = geom.intersection(tmpGeom)
                        lstIntersectingB.append(tmpGeom)

                        if not int_geom:
                            # There was a problem creating the intersection
                            feedback.pushInfo(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
                            int_geom = QgsGeometry()
                        else:
                            int_geom = QgsGeometry(int_geom)

                        if int_geom.wkbType() == QgsWkbTypes.Unknown or QgsWkbTypes.flatType(int_geom.wkbType()) == QgsWkbTypes.GeometryCollection:
                            # Intersection produced different geomety types
                            temp_list = int_geom.asGeometryCollection()
                            for i in temp_list:
                                if i.type() == geom.type():
                                    int_geom = QgsGeometry(i)
                                    try:
                                        int_geom.convertToMultiType()
                                        outFeat.setGeometry(int_geom)
                                        outFeat.setAttributes(atMapA + atMapB)
                                        sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                                    except:
                                        feedback.pushInfo(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
                        else:
                            # Geometry list: prevents writing error
                            # in geometries of different types
                            # produced by the intersection
                            # fix #3549
                            if QgsWkbTypes.geometryType(int_geom.wkbType()) == QgsWkbTypes.geometryType(geomType):
                                try:
                                    int_geom.convertToMultiType()
                                    outFeat.setGeometry(int_geom)
                                    outFeat.setAttributes(atMapA + atMapB)
                                    sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                                except:
                                    feedback.pushInfo(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))

                # the remaining bit of featA's geometry
                # if there is nothing left, this will just silently fail and we're good
                diff_geom = QgsGeometry(geom)
                if len(lstIntersectingB) != 0:
                    intB = QgsGeometry.unaryUnion(lstIntersectingB)
                    diff_geom = diff_geom.difference(intB)

                if diff_geom.wkbType() == QgsWkbTypes.Unknown or QgsWkbTypes.flatType(diff_geom.wkbType()) == QgsWkbTypes.GeometryCollection:
                    temp_list = diff_geom.asGeometryCollection()
                    for i in temp_list:
                        if i.type() == geom.type():
                            diff_geom = QgsGeometry(i)
                try:
                    diff_geom.convertToMultiType()
                    outFeat.setGeometry(diff_geom)
                    outFeat.setAttributes(atMapA)
                    sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                except:
                    feedback.pushInfo(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))

            count += 1
            feedback.setProgress(int(count * total))

        length = len(sourceA.fields())
        atMapA = [None] * length

        for featA in sourceB.getFeatures(QgsFeatureRequest().setDestinationCrs(sourceA.sourceCrs())):
            if feedback.isCanceled():
                break

            add = False
            geom = featA.geometry()
            diff_geom = QgsGeometry(geom)
            atMap = [None] * length
            atMap.extend(featA.attributes())
            intersects = indexA.intersects(geom.boundingBox())

            if len(intersects) < 1:
                try:
                    geom.convertToMultiType()
                    outFeat.setGeometry(geom)
                    outFeat.setAttributes(atMap)
                    sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                except:
                    feedback.pushInfo(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))
            else:
                request = QgsFeatureRequest().setFilterFids(intersects).setSubsetOfAttributes([])
                request.setDestinationCrs(sourceA.sourceCrs())

                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(diff_geom.constGet())
                engine.prepareGeometry()

                for featB in sourceA.getFeatures(request):
                    atMapB = featB.attributes()
                    tmpGeom = featB.geometry()

                    if engine.intersects(tmpGeom.constGet()):
                        add = True
                        diff_geom = QgsGeometry(diff_geom.difference(tmpGeom))
                    else:
                        try:
                            # Ihis only happens if the bounding box
                            # intersects, but the geometry doesn't
                            diff_geom.convertToMultiType()
                            outFeat.setGeometry(diff_geom)
                            outFeat.setAttributes(atMap)
                            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                        except:
                            feedback.pushInfo(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))

            if add:
                try:
                    diff_geom.convertToMultiType()
                    outFeat.setGeometry(diff_geom)
                    outFeat.setAttributes(atMap)
                    sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                except:
                    feedback.pushInfo(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'))

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}
