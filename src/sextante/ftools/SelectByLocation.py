from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.ftools import ftools_utils

class SelectByLocation(GeoAlgorithm):

    INPUT = "INPUT"
    INTERSECT = "INTERSECT"
    METHOD = "METHOD"
    USE_SELECTED = "USE_SELECTED"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/select_location.png")

    def processAlgorithm(self, progress):
        method = self.getParameterValue(self.METHOD)
        selection = self.getParameterValue(self.USE_SELECTED)
        filename = self.getParameterValue(SelectByLocation.INPUT)
        inputLayer = QGisLayers.getObjectFromUri(filename)
        filename = self.getParameterValue(SelectByLocation.INTERSECT)
        selectLayer = QGisLayers.getObjectFromUri(filename)
        inputProvider = inputLayer.dataProvider()
        allAttrs = inputProvider.attributeIndexes()
        inputProvider.select(allAttrs, QgsRectangle())
        selectProvider = selectLayer.dataProvider()
        allAttrs = selectProvider.attributeIndexes()
        selectProvider.select(allAttrs, QgsRectangle())
        feat = QgsFeature()
        infeat = QgsFeature()
        geom = QgsGeometry()
        selectedSet = []
        index = ftools_utils.createIndex(inputProvider)
        if selection:
            features = selectLayer.selectedFeatures()
            featurescount = len(features)
            i = 0
            for feat in features:
                geom = QgsGeometry(feat.geometry())
                intersects = index.intersects(geom.boundingBox())
                for id in intersects:
                    inputProvider.featureAtId(int(id), infeat, True)
                    tmpGeom = QgsGeometry(infeat.geometry())
                    if geom.intersects(tmpGeom):
                        selectedSet.append(infeat.id())
                i += 1
                progress.setPercentage(i/featurescount * 100)
        else:
            featurescount = selectProvider.featureCount()
            while selectProvider.nextFeature(feat):
                geom = QgsGeometry(feat.geometry())
                intersects = index.intersects(geom.boundingBox())
                i = 0
                for iid in intersects:
                    inputProvider.featureAtId(int(iid), infeat, True)
                    tmpGeom = QgsGeometry( infeat.geometry() )
                    if geom.intersects(tmpGeom):
                        selectedSet.append(infeat.id())
                i += 1
                progress.setPercentage(i/featurescount * 100)
        if method == 1:
            selectedSet = list(set(inputLayer.selectedFeaturesIds()).union(selectedSet))
        elif method == 2:
            selectedSet = list(set(inputLayer.selectedFeaturesIds()).difference(selectedSet))
        inputLayer.setSelectedFeatures(selectedSet)
        self.setOutputValue(self.OUTPUT, filename)


    def defineCharacteristics(self):
        self.name = "Select by location"
        self.group = "Research tools"
        self.addParameter(ParameterVector(SelectByLocation.INPUT, "Layer to select from", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(SelectByLocation.INTERSECT, "Additional layer (intersection layer)", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterSelection(SelectByLocation.METHOD, "Modify current selection by", ["creating new selection","adding to current selection", "removing from current selection"]))
        self.addParameter(ParameterBoolean(SelectByLocation.USE_SELECTED, "Use only selected features", False))
        self.addOutput(OutputVector(SelectByLocation.OUTPUT, "Selection", True))
