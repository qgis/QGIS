# -*- coding: utf-8 -*-

"""
***************************************************************************
    SelectByLocation.py
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

from qgis.core import QgsGeometry, QgsFeatureRequest, QgsProcessingUtils

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SelectByLocation(QgisAlgorithm):

    INPUT = 'INPUT'
    INTERSECT = 'INTERSECT'
    PREDICATE = 'PREDICATE'
    PRECISION = 'PRECISION'
    METHOD = 'METHOD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'select_location.png'))

    def group(self):
        return self.tr('Vector selection')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.predicates = (
            ('intersects', self.tr('intersects')),
            ('contains', self.tr('contains')),
            ('disjoint', self.tr('disjoint')),
            ('equals', self.tr('equals')),
            ('touches', self.tr('touches')),
            ('overlaps', self.tr('overlaps')),
            ('within', self.tr('within')),
            ('crosses', self.tr('crosses')))

        self.methods = [self.tr('creating new selection'),
                        self.tr('adding to current selection'),
                        self.tr('removing from current selection')]

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Layer to select from')))
        self.addParameter(ParameterVector(self.INTERSECT,
                                          self.tr('Additional layer (intersection layer)')))
        self.addParameter(ParameterSelection(self.PREDICATE,
                                             self.tr('Geometric predicate'),
                                             self.predicates,
                                             multiple=True))
        self.addParameter(ParameterNumber(self.PRECISION,
                                          self.tr('Precision'),
                                          0.0, None, 0.0))
        self.addParameter(ParameterSelection(self.METHOD,
                                             self.tr('Modify current selection by'),
                                             self.methods, 0))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Selected (location)'), True))

    def name(self):
        return 'selectbylocation'

    def displayName(self):
        return self.tr('Select by location')

    def processAlgorithm(self, parameters, context, feedback):
        filename = self.getParameterValue(self.INPUT)
        inputLayer = QgsProcessingUtils.mapLayerFromString(filename, context)
        method = self.getParameterValue(self.METHOD)
        filename2 = self.getParameterValue(self.INTERSECT)
        selectLayer = QgsProcessingUtils.mapLayerFromString(filename2, context)
        predicates = self.getParameterValue(self.PREDICATE)
        precision = self.getParameterValue(self.PRECISION)

        oldSelection = set(inputLayer.selectedFeatureIds())
        inputLayer.removeSelection()
        index = QgsProcessingUtils.createSpatialIndex(inputLayer, context)

        if 'disjoint' in predicates:
            disjoinSet = []
            for feat in QgsProcessingUtils.getFeatures(inputLayer, context):
                disjoinSet.append(feat.id())

        geom = QgsGeometry()
        selectedSet = []
        features = QgsProcessingUtils.getFeatures(selectLayer, context)
        total = 100.0 / selectLayer.featureCount() if selectLayer.featureCount() else 0
        for current, f in enumerate(features):
            geom = vector.snapToPrecision(f.geometry(), precision)
            bbox = geom.boundingBox()
            bbox.grow(0.51 * precision)
            intersects = index.intersects(bbox)

            request = QgsFeatureRequest().setFilterFids(intersects).setSubsetOfAttributes([])
            for feat in inputLayer.getFeatures(request):
                tmpGeom = vector.snapToPrecision(feat.geometry(), precision)

                res = False
                for predicate in predicates:
                    if predicate == 'disjoint':
                        if tmpGeom.intersects(geom):
                            try:
                                disjoinSet.remove(feat.id())
                            except:
                                pass  # already removed
                    else:
                        res = getattr(tmpGeom, predicate)(geom)
                        if res:
                            selectedSet.append(feat.id())
                            break

            feedback.setProgress(int(current * total))

        if 'disjoint' in predicates:
            selectedSet = selectedSet + disjoinSet

        if method == 1:
            selectedSet = list(oldSelection.union(selectedSet))
        elif method == 2:
            selectedSet = list(oldSelection.difference(selectedSet))

        inputLayer.selectByIds(selectedSet)
        self.setOutputValue(self.OUTPUT, filename)
