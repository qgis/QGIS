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

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SelectByLocation(GeoAlgorithm):

    INPUT = 'INPUT'
    INTERSECT = 'INTERSECT'
    TOUCHES = 'TOUCHES'
    OVERLAPS = 'OVERLAPS'
    WITHIN = 'WITHIN'
    METHOD = 'METHOD'
    OUTPUT = 'OUTPUT'

    METHODS = ['creating new selection', 'adding to current selection',
               'removing from current selection']
    opFlags = 0
    operators = {'TOUCHES':1,'OVERLAPS':2,'WITHIN':4}


    def defineCharacteristics(self):
        self.name = 'Select by location'
        self.group = 'Vector selection tools'
        self.addParameter(ParameterVector(self.INPUT, 'Layer to select from',
                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.INTERSECT,
                          'Additional layer (intersection layer)',
                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterBoolean(self.TOUCHES,
                          'Include input features that touch the selection features',
                          [True]))
        self.addParameter(ParameterBoolean(self.OVERLAPS,
                          'Include input features that overlap/cross the selection features',
                          [True]))
        self.addParameter(ParameterBoolean(self.WITHIN,
                          'Include input features completely within the selection features',
                          [True]))
        self.addParameter(ParameterSelection(self.METHOD,
                          'Modify current selection by', self.METHODS, 0))
        self.addOutput(OutputVector(self.OUTPUT, 'Selection', True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)
        inputLayer = dataobjects.getObjectFromUri(filename)
        method = self.getParameterValue(self.METHOD)
        filename = self.getParameterValue(self.INTERSECT)
        selectLayer = dataobjects.getObjectFromUri(filename)

        oldSelection = set(inputLayer.selectedFeaturesIds())
        inputLayer.removeSelection()
        index = vector.spatialindex(inputLayer)

        def _points_op(geomA,geomB):
            return geomA.intersects(geomB)

        def _poly_lines_op(geomA,geomB):
            if geomA.disjoint(geomB):
                return False
            intersects = False
            if self.opFlags & self.operators['TOUCHES']:
                intersects |= geomA.touches(geomB)
            if not intersects and (self.opFlags & self.operators['OVERLAPS']):
                if geomB.type() == QGis.Line or geomA.type() == QGis.Line:
                    intersects |= geomA.crosses(geomB)
                else:
                    intersects |= geomA.overlaps(geomB)
            if not intersects and (self.opFlags & self.operators['WITHIN']):
                intersects |= geomA.contains(geomB)
            return intersects

        def _sp_operator():
            if inputLayer.geometryType() == QGis.Point:
                return _points_op
            else:
                return _poly_lines_op

        self.opFlags = 0
        if self.getParameterValue(self.TOUCHES):
            self.opFlags |= self.operators['TOUCHES']
        if self.getParameterValue(self.OVERLAPS):
            self.opFlags |= self.operators['OVERLAPS']
        if self.getParameterValue(self.WITHIN):
            self.opFlags |= self.operators['WITHIN']

        sp_operator = _sp_operator()

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
                if sp_operator(geom,tmpGeom):
                    selectedSet.append(feat.id())
            current += 1
            progress.setPercentage(int(current * total))

        if method == 1:
            selectedSet = list(oldSelection.union(selectedSet))
        elif method == 2:
            selectedSet = list(oldSelection.difference(selectedSet))

        inputLayer.setSelectedFeatures(selectedSet)
        self.setOutputValue(self.OUTPUT, filename)
