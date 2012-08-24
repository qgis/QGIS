from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterVector import ParameterVector
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.core.QGisLayers import QGisLayers
import os
from PyQt4 import QtGui


class AddTableField(GeoAlgorithm):

    OUTPUT_LAYER = "OUTPUT_LAYER"
    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    FIELD_TYPE = "FIELD_TYPE"
    TYPE_NAMES = ["Integer", "Float", "String"]
    TYPES = [QVariant.Int, QVariant.Double, QVariant.String]

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def defineCharacteristics(self):
        self.name = "Add field to attributes table"
        self.group = "Algorithms for vector layers"
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterString(self.FIELD_NAME, "Field name"))
        self.addParameter(ParameterSelection(self.FIELD_TYPE, "Field type", self.TYPE_NAMES))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer"))

    def processAlgorithm(self, progress):
        fieldtype = self.getParameterValue(self.FIELD_TYPE)
        fieldname = self.getParameterValue(self.FIELD_NAME)
        output = self.getOutputFromName(self.OUTPUT_LAYER)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        fields[len(fields)] = QgsField(fieldname, self.TYPES[fieldtype])
        writer = output.getVectorWriter(fields, vprovider.geometryType(), vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        while vprovider.nextFeature(inFeat):
          progress.setPercentage(int((100 * nElement)/nFeat))
          nElement += 1
          inGeom = inFeat.geometry()
          outFeat.setGeometry( inGeom )
          atMap = inFeat.attributeMap()
          outFeat.setAttributeMap( atMap )
          outFeat.addAttribute( len(vprovider.fields()), QVariant() )
          writer.addFeature( outFeat )
        del writer

