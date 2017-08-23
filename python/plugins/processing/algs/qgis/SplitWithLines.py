# -*- coding: utf-8 -*-

"""
***************************************************************************
    SplitWithLines.py
    ---------------------
    Date                 : November 2014
    Revised              : November 2016
    Copyright            : (C) 2014 by Bernhard Ströbl
    Email                : bernhard dot stroebl at jena dot de
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Bernhard Ströbl'
__date__ = 'November 2014'
__copyright__ = '(C) 2014, Bernhard Ströbl'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsFeatureRequest,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsSpatialIndex,
                       QgsWkbTypes,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector


class SplitWithLines(QgisAlgorithm):

    INPUT = 'INPUT'
    LINES = 'LINES'

    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector overlay')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer, single geometries only'), [QgsProcessing.TypeVectorLine,
                                                                                                               QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterFeatureSource(self.LINES,
                                                              self.tr('Split layer'), [QgsProcessing.TypeVectorLine]))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Split')))

    def name(self):
        return 'splitwithlines'

    def displayName(self):
        return self.tr('Split with lines')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        line_source = self.parameterAsSource(parameters, self.LINES, context)

        sameLayer = parameters[self.INPUT] == parameters[self.LINES]

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), QgsWkbTypes.multiType(source.wkbType()), source.sourceCrs())

        spatialIndex = QgsSpatialIndex()
        splitGeoms = {}
        request = QgsFeatureRequest()
        request.setSubsetOfAttributes([])
        request.setDestinationCrs(source.sourceCrs())

        for aSplitFeature in line_source.getFeatures(request):
            if feedback.isCanceled():
                break

            splitGeoms[aSplitFeature.id()] = aSplitFeature.geometry()
            spatialIndex.insertFeature(aSplitFeature)
            # honor the case that user has selection on split layer and has setting "use selection"

        outFeat = QgsFeature()
        features = source.getFeatures()

        total = 100.0 / source.featureCount() if source.featureCount() else 100

        for current, inFeatA in enumerate(features):
            if feedback.isCanceled():
                break

            inGeom = inFeatA.geometry()
            attrsA = inFeatA.attributes()
            outFeat.setAttributes(attrsA)

            if inGeom.isMultipart():
                inGeoms = []

                for g in inGeom.asGeometryCollection():
                    inGeoms.append(g)
            else:
                inGeoms = [inGeom]

            lines = spatialIndex.intersects(inGeom.boundingBox())

            if len(lines) > 0:  # has intersection of bounding boxes
                splittingLines = []

                engine = QgsGeometry.createGeometryEngine(inGeom.geometry())
                engine.prepareGeometry()

                for i in lines:
                    try:
                        splitGeom = splitGeoms[i]
                    except:
                        continue

                    # check if trying to self-intersect
                    if sameLayer:
                        if inFeatA.id() == i:
                            continue

                    if engine.intersects(splitGeom.geometry()):
                        splittingLines.append(splitGeom)

                if len(splittingLines) > 0:
                    for splitGeom in splittingLines:
                        splitterPList = None
                        outGeoms = []

                        split_geom_engine = QgsGeometry.createGeometryEngine(splitGeom.geometry())
                        split_geom_engine.prepareGeometry()

                        while len(inGeoms) > 0:
                            if feedback.isCanceled():
                                break

                            inGeom = inGeoms.pop()

                            if inGeom.isNull():  # this has been encountered and created a run-time error
                                continue

                            if split_geom_engine.intersects(inGeom.geometry()):
                                inPoints = vector.extractPoints(inGeom)
                                if splitterPList is None:
                                    splitterPList = vector.extractPoints(splitGeom)

                                try:
                                    result, newGeometries, topoTestPoints = inGeom.splitGeometry(splitterPList, False)
                                except:
                                    feedback.reportError(self.tr('Geometry exception while splitting'))
                                    result = 1

                                # splitGeometry: If there are several intersections
                                # between geometry and splitLine, only the first one is considered.
                                if result == 0:  # split occurred
                                    if inPoints == vector.extractPoints(inGeom):
                                        # bug in splitGeometry: sometimes it returns 0 but
                                        # the geometry is unchanged
                                        outGeoms.append(inGeom)
                                    else:
                                        inGeoms.append(inGeom)

                                        for aNewGeom in newGeometries:
                                            inGeoms.append(aNewGeom)
                                else:
                                    outGeoms.append(inGeom)
                            else:
                                outGeoms.append(inGeom)

                        inGeoms = outGeoms

            parts = []

            for aGeom in inGeoms:
                if feedback.isCanceled():
                    break

                passed = True

                if QgsWkbTypes.geometryType(aGeom.wkbType()) == QgsWkbTypes.LineGeometry:
                    numPoints = aGeom.geometry().numPoints()

                    if numPoints <= 2:
                        if numPoints == 2:
                            passed = not aGeom.geometry().isClosed()  # tests if vertex 0 = vertex 1
                        else:
                            passed = False
                            # sometimes splitting results in lines of zero length

                if passed:
                    parts.append(aGeom)

            if len(parts) > 0:
                outFeat.setGeometry(QgsGeometry.collectGeometry(parts))
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))
        return {self.OUTPUT: dest_id}
