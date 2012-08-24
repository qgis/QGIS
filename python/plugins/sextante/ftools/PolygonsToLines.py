from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.outputs.OutputVector import OutputVector

class PolygonsToLines(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/to_lines.png")

    def processAlgorithm(self, progress):
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(PolygonsToLines.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields, QGis.WKBLineString, vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        while vprovider.nextFeature(inFeat):
          multi = False
          nElement += 1
          progress.setPercentage(int(nElement/nFeat * 100))
          inGeom = inFeat.geometry()
          if inGeom.isMultipart():
            multi = True
          atMap = inFeat.attributeMap()
          lineList = self.extractAsLine( inGeom )
          outFeat.setAttributeMap( atMap )
          for h in lineList:
            outFeat.setGeometry( outGeom.fromPolyline( h ) )
            writer.addFeature( outFeat )
        del writer

    def extractAsLine( self, geom ):
        multi_geom = QgsGeometry()
        temp_geom = []
        if geom.type() == 2:
          if geom.isMultipart():
            multi_geom = geom.asMultiPolygon()
            for i in multi_geom:
              temp_geom.extend(i)
          else:
            multi_geom = geom.asPolygon()
            temp_geom = multi_geom
          return temp_geom
        else:
          return []

    def defineCharacteristics(self):
        self.name = "Polygons to lines"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(PolygonsToLines.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_POLYGON))
        self.addOutput(OutputVector(PolygonsToLines.OUTPUT, "Output layer"))
    #=========================================================
