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

from qgis.core import QGis, QgsGeometry, QgsFeatureRequest
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterGeometryPredicate
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SelectByLocation(GeoAlgorithm):

    INPUT = 'INPUT'
    INTERSECT = 'INTERSECT'
    PREDICATE = 'PREDICATE'
    METHOD = 'METHOD'
    OUTPUT = 'OUTPUT'

    METHODS = ['creating new selection',
               'adding to current selection',
               'removing from current selection']

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Select by location')
        self.group, self.i18n_group = self.trAlgorithm('Vector selection tools')
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Layer to select from'),
                                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.INTERSECT,
                                          self.tr('Additional layer (intersection layer)'),
                                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterGeometryPredicate(self.PREDICATE,
                                                     self.tr('Geometric predicate'),
                                                     left=self.INPUT, right=self.INTERSECT))
        self.addParameter(ParameterSelection(self.METHOD,
                                             self.tr('Modify current selection by'),
                                             self.METHODS, 0))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Selected (location)'), True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)
        inputLayer = dataobjects.getObjectFromUri(filename)
        method = self.getParameterValue(self.METHOD)
        filename2 = self.getParameterValue(self.INTERSECT)
        selectLayer = dataobjects.getObjectFromUri(filename2)
        predicates = self.getParameterValue(self.PREDICATE)

        oldSelection = set(inputLayer.selectedFeaturesIds())
        inputLayer.removeSelection()
        index = vector.spatialindex(inputLayer)

        if 'disjoint' in predicates:
            disjoinSet = []
            for feat in vector.features(inputLayer):
                disjoinSet.append(feat.id())

        geom = QgsGeometry()
        selectedSet = []
        current = 0
        features = vector.features(selectLayer)
        total = 100.0 / float(len(features))
        for f in features:
            geom = QgsGeometry(f.geometry())

            intersects = index.intersects(geom.boundingBox())
            for i in intersects:
                request = QgsFeatureRequest().setFilterFid(i)
                feat = inputLayer.getFeatures(request).next()
                tmpGeom = QgsGeometry(feat.geometry())
                res = False
                for predicate in predicates:
                    if predicate == 'disjoint':
                        if tmpGeom.intersects(geom):
                            try:
                                disjoinSet.remove(feat.id())
                            except:
                                pass  # already removed
                    else:
                        if predicate == 'intersects':
                            res = tmpGeom.intersects(geom)
                        elif predicate == 'contains':
                            res = tmpGeom.contains(geom)
                        elif predicate == 'equals':
                            res = tmpGeom.equals(geom)
                        elif predicate == 'touches':
                            res = tmpGeom.touches(geom)
                        elif predicate == 'overlaps':
                            res = tmpGeom.overlaps(geom)
                        elif predicate == 'within':
                            res = tmpGeom.within(geom)
                        elif predicate == 'crosses':
                            res = tmpGeom.crosses(geom)
                        if res:
                            selectedSet.append(feat.id())
                            break

            current += 1
            progress.setPercentage(int(current * total))

        if 'disjoint' in predicates:
            selectedSet = selectedSet + disjoinSet

        if method == 1:
            selectedSet = list(oldSelection.union(selectedSet))
        elif method == 2:
            selectedSet = list(oldSelection.difference(selectedSet))

        inputLayer.setSelectedFeatures(selectedSet)
        self.setOutputValue(self.OUTPUT, filename)
