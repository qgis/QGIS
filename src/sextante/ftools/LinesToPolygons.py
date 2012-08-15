from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector

class LinesToPolygons(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/to_lines.png")

    def processAlgorithm(self, progress):
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(LinesToPolygons.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        writer = self.getOutputFromName(LinesToPolygons.OUTPUT).getVectorWriter(fields, QGis.WKBPolygon, vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        while vprovider.nextFeature(inFeat):
          outGeomList = []
          multi = False
          nElement += 1
          progress.setPercentage(int(nElement/nFeat * 100))
          if inFeat.geometry().isMultipart():
            outGeomList = inFeat.geometry().asMultiPolyline()
            multi = True
          else:
            outGeomList.append( inFeat.geometry().asPolyline() )
          polyGeom = self.remove_bad_lines( outGeomList )
          if len(polyGeom) <> 0:
            outFeat.setGeometry( QgsGeometry.fromPolygon( polyGeom ) )
            atMap = inFeat.attributeMap()
            outFeat.setAttributeMap( atMap )
            writer.addFeature( outFeat )
        del writer


    def remove_bad_lines( self, lines ):
        temp_geom = []
        if len(lines)==1:
          if len(lines[0]) > 2:
            temp_geom = lines
          else:
            temp_geom = []
        else:
          temp_geom = [elem for elem in lines if len(elem) > 2]
        return temp_geom


    def defineCharacteristics(self):
        self.name = "Lines to polygons"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(LinesToPolygons.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_LINE))
        self.addOutput(OutputVector(LinesToPolygons.OUTPUT, "Output layer"))
    #=========================================================
