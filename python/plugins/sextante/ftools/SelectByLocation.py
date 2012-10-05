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

import os.path

from PyQt4 import QtGui
from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers

from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean

from sextante.outputs.OutputVector import OutputVector

from sextante.ftools import FToolsUtils as utils

class SelectByLocation(GeoAlgorithm):

    INPUT = "INPUT"
    INTERSECT = "INTERSECT"
    METHOD = "METHOD"
    USE_SELECTED = "USE_SELECTED"
    OUTPUT = "OUTPUT"

    METHODS = ["creating new selection",
               "adding to current selection",
               "removing from current selection"]

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/select_location.png")

    def defineCharacteristics(self):
        self.name = "Select by location"
        self.group = "Research tools"

        self.addParameter(ParameterVector(self.INPUT, "Layer to select from", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(self.INTERSECT, "Additional layer (intersection layer)", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterSelection(self.METHOD, "Modify current selection by", self.METHODS, 0))
        self.addParameter(ParameterBoolean(self.USE_SELECTED, "Use only selected features", False))

        self.addOutput(OutputVector(self.OUTPUT, "Selection", True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)
        inputLayer = QGisLayers.getObjectFromUri(filename)
        method = self.getParameterValue(self.METHOD)
        selection = self.getParameterValue(self.USE_SELECTED)

        filename = self.getParameterValue(self.INTERSECT)
        selectLayer = QGisLayers.getObjectFromUri(filename)

        inputProvider = inputLayer.dataProvider()
        selectProvider = selectLayer.dataProvider()

        index = utils.createSpatialIndex(inputProvider)

        inputProvider.select()
        selectProvider.select()

        feat = QgsFeature()
        infeat = QgsFeature()
        geom = QgsGeometry()
        selectedSet = []

        if selection:
            features = selectLayer.selectedFeatures()
            total = 100.0 / float(len(features))
            current = 0
            for feat in features:
                geom = QgsGeometry(feat.geometry())
                intersects = index.intersects(geom.boundingBox())
                for i in intersects:
                    inputProvider.featureAtId(i, infeat, True)
                    tmpGeom = QgsGeometry(infeat.geometry())
                    if geom.intersects(tmpGeom):
                        selectedSet.append(infeat.id())
                current += 1
                progress.setPercentage(int(current * total))
        else:
            total = 100.0 / float(selectProvider.featureCount())
            current = 0
            while selectProvider.nextFeature(feat):
                geom = QgsGeometry(feat.geometry())
                intersects = index.intersects(geom.boundingBox())
                for i in intersects:
                    inputProvider.featureAtId(i, infeat, True)
                    tmpGeom = QgsGeometry( infeat.geometry() )
                    if geom.intersects(tmpGeom):
                        selectedSet.append(infeat.id())
                current += 1
                progress.setPercentage(int(current * total))

        if method == 1:
            selectedSet = list(set(inputLayer.selectedFeaturesIds()).union(selectedSet))
        elif method == 2:
            selectedSet = list(set(inputLayer.selectedFeaturesIds()).difference(selectedSet))

        inputLayer.setSelectedFeatures(selectedSet)
        self.setOutputValue(self.OUTPUT, filename)
