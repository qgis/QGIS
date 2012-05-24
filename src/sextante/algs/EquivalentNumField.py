from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4.QtCore import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
import os
from PyQt4 import QtGui
from sextante.parameters.ParameterTableField import ParameterTableField

class AutoincrementalField(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def processAlgorithm(self, progress):
        field_index = self.getParameterValue(self.FIELD)
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(self.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        fields[len(fields)] = QgsField("NUM_FIELD", QVariant.Int)
        writer = QgsVectorFileWriter( output, systemEncoding,fields, vprovider.geometryType(), vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        classes = {}
        while vprovider.nextFeature(inFeat):
          progress.setPercentage(int((100 * nElement)/nFeat))
          nElement += 1
          atMap = inFeat.attributeMap()
          clazz = atMap[field_index]
          if not clazz in classes.keys:
              classes[clazz] = len(classes.keys())
        while vprovider.nextFeature(inFeat):
          progress.setPercentage(int((100 * nElement)/nFeat))
          nElement += 1
          inGeom = inFeat.geometry()
          outFeat.setGeometry( inGeom )
          atMap = inFeat.attributeMap()
          outFeat.setAttributeMap( atMap )
          outFeat.addAttribute( len(vprovider.fields()), QVariant(nElement) )
          writer.addFeature( outFeat )
        del writer

    def defineCharacteristics(self):
        self.name = "Add autoincremental field"
        self.group = "Algorithms for vector layers"
        self.addParameter(ParameterVector(self.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(self.FIELD, "Unique ID Field", self.INPUT))
        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

