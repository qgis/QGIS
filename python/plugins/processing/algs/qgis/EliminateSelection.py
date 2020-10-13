# -*- coding: utf-8 -*-

"""
***************************************************************************
    EliminateSelection.py
    ---------------------
    Date                 : January 2017
    Copyright         : (C) 2017 by Bernhard Ströbl
    Email                : bernhard.stroebl@jena.de
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
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Bernhard Ströbl'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsApplication,
                       QgsFeatureRequest,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsProcessingAlgorithm,
                       QgsProcessingException,
                       QgsProcessingUtils,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class EliminateSelection(QgisAlgorithm):
    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    MODE = 'MODE'

    MODE_LARGEST_AREA = 0
    MODE_SMALLEST_AREA = 1
    MODE_BOUNDARY = 2

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmDissolve.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmDissolve.svg")

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagNoThreading | QgsProcessingAlgorithm.FlagNotAvailableInStandaloneTool

    def initAlgorithm(self, config=None):
        self.modes = [self.tr('Largest Area'),
                      self.tr('Smallest Area'),
                      self.tr('Largest Common Boundary')]

        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT,
                                                            self.tr('Input layer'), [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterEnum(self.MODE,
                                                     self.tr('Merge selection with the neighbouring polygon with the'),
                                                     options=self.modes))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Eliminated'), QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'eliminateselectedpolygons'

    def displayName(self):
        return self.tr('Eliminate selected polygons')

    def processAlgorithm(self, parameters, context, feedback):
        inLayer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        boundary = self.parameterAsEnum(parameters, self.MODE, context) == self.MODE_BOUNDARY
        smallestArea = self.parameterAsEnum(parameters, self.MODE, context) == self.MODE_SMALLEST_AREA

        if inLayer.selectedFeatureCount() == 0:
            feedback.reportError(self.tr('{0}: (No selection in input layer "{1}")').format(self.displayName(), parameters[self.INPUT]))

        featToEliminate = []
        selFeatIds = inLayer.selectedFeatureIds()

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               inLayer.fields(), inLayer.wkbType(), inLayer.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        for aFeat in inLayer.getFeatures():
            if feedback.isCanceled():
                break

            if aFeat.id() in selFeatIds:
                # Keep references to the features to eliminate
                featToEliminate.append(aFeat)
            else:
                # write the others to output
                sink.addFeature(aFeat, QgsFeatureSink.FastInsert)

        del sink

        # Delete all features to eliminate in processLayer
        processLayer = QgsProcessingUtils.mapLayerFromString(dest_id, context)
        processLayer.startEditing()

        # ANALYZE
        if len(featToEliminate) > 0:  # Prevent zero division
            start = 20.00
            add = 80.00 / len(featToEliminate)
        else:
            start = 100

        feedback.setProgress(start)
        madeProgress = True

        # We go through the list and see if we find any polygons we can
        # merge the selected with. If we have no success with some we
        # merge and then restart the whole story.
        while madeProgress:  # Check if we made any progress
            madeProgress = False
            featNotEliminated = []

            # Iterate over the polygons to eliminate
            for i in range(len(featToEliminate)):
                if feedback.isCanceled():
                    break

                feat = featToEliminate.pop()
                geom2Eliminate = feat.geometry()
                bbox = geom2Eliminate.boundingBox()
                fit = processLayer.getFeatures(
                    QgsFeatureRequest().setFilterRect(bbox).setSubsetOfAttributes([]))
                mergeWithFid = None
                mergeWithGeom = None
                max = 0
                min = -1
                selFeat = QgsFeature()

                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(geom2Eliminate.constGet())
                engine.prepareGeometry()

                while fit.nextFeature(selFeat):
                    if feedback.isCanceled():
                        break

                    selGeom = selFeat.geometry()

                    if engine.intersects(selGeom.constGet()):
                        # We have a candidate
                        iGeom = geom2Eliminate.intersection(selGeom)

                        if not iGeom:
                            continue

                        if boundary:
                            selValue = iGeom.length()
                        else:
                            # area. We need a common boundary in
                            # order to merge
                            if 0 < iGeom.length():
                                selValue = selGeom.area()
                            else:
                                selValue = -1

                        if -1 != selValue:
                            useThis = True

                            if smallestArea:
                                if -1 == min:
                                    min = selValue
                                else:
                                    if selValue < min:
                                        min = selValue
                                    else:
                                        useThis = False
                            else:
                                if selValue > max:
                                    max = selValue
                                else:
                                    useThis = False

                            if useThis:
                                mergeWithFid = selFeat.id()
                                mergeWithGeom = QgsGeometry(selGeom)
                # End while fit

                if mergeWithFid is not None:
                    # A successful candidate
                    newGeom = mergeWithGeom.combine(geom2Eliminate)

                    if processLayer.changeGeometry(mergeWithFid, newGeom):
                        madeProgress = True
                    else:
                        raise QgsProcessingException(
                            self.tr('Could not replace geometry of feature with id {0}').format(mergeWithFid))

                    start = start + add
                    feedback.setProgress(start)
                else:
                    featNotEliminated.append(feat)

            # End for featToEliminate

            featToEliminate = featNotEliminated

        # End while
        if not processLayer.commitChanges():
            raise QgsProcessingException(self.tr('Could not commit changes'))

        for feature in featNotEliminated:
            if feedback.isCanceled():
                break

            processLayer.dataProvider().addFeature(feature, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}
