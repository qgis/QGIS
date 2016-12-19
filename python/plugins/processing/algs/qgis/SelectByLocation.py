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
from builtins import next

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsGeometry, QgsFeatureRequest

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SelectByLocation(GeoAlgorithm):

    INPUT = 'INPUT'
    INTERSECT = 'INTERSECT'
    PREDICATE = 'PREDICATE'
    PRECISION = 'PRECISION'
    METHOD = 'METHOD'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'select_location.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Select by location')
        self.group, self.i18n_group = self.trAlgorithm('Vector selection tools')

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

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)
        inputLayer = dataobjects.getObjectFromUri(filename)
        method = self.getParameterValue(self.METHOD)
        filename2 = self.getParameterValue(self.INTERSECT)
        selectLayer = dataobjects.getObjectFromUri(filename2)
        predicates = self.getParameterValue(self.PREDICATE)
        precision = self.getParameterValue(self.PRECISION)

        oldSelection = set(inputLayer.selectedFeatureIds())
        inputLayer.removeSelection()
        index = vector.spatialindex(inputLayer)

        if 'disjoint' in predicates:
            disjoinSet = []
            for feat in vector.features(inputLayer):
                disjoinSet.append(feat.id())

        geom = QgsGeometry()
        selectedSet = []
        features = vector.features(selectLayer)
        total = 100.0 / len(features)
        for current, f in enumerate(features):
            geom = vector.snapToPrecision(f.geometry(), precision)
            bbox = vector.bufferedBoundingBox(geom.boundingBox(), 0.51 * precision)
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

            progress.setPercentage(int(current * total))

        if 'disjoint' in predicates:
            selectedSet = selectedSet + disjoinSet

        if method == 1:
            selectedSet = list(oldSelection.union(selectedSet))
        elif method == 2:
            selectedSet = list(oldSelection.difference(selectedSet))

        inputLayer.selectByIds(selectedSet)
        self.setOutputValue(self.OUTPUT, filename)
