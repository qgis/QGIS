from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector

class ExtentFromLayer(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/layer_extent.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(ExtentFromLayer.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(ExtentFromLayer.INPUT))
        fields = {
            0 : QgsField( "MINX", QVariant.Double ),
            1 : QgsField( "MINY", QVariant.Double ),
            2 : QgsField( "MAXX", QVariant.Double ),
            3 : QgsField( "MAXY", QVariant.Double ),
            4 : QgsField( "CNTX", QVariant.Double ),
            5 : QgsField( "CNTY", QVariant.Double ),
            6 : QgsField( "AREA", QVariant.Double ),
            7 : QgsField( "PERIM", QVariant.Double ),
            8 : QgsField( "HEIGHT", QVariant.Double ),
            9 : QgsField( "WIDTH", QVariant.Double ) }

        writer = QgsVectorFileWriter(output, systemEncoding, fields, QGis.WKBPolygon, vlayer.crs() )
        rect = vlayer.extent()
        minx = rect.xMinimum()
        miny = rect.yMinimum()
        maxx = rect.xMaximum()
        maxy = rect.yMaximum()
        height = rect.height()
        width = rect.width()
        cntx = minx + ( width / 2.0 )
        cnty = miny + ( height / 2.0 )
        area = width * height
        perim = ( 2 * width ) + (2 * height )
        rect = [
            QgsPoint( minx, miny ),
            QgsPoint( minx, maxy ),
            QgsPoint( maxx, maxy ),
            QgsPoint( maxx, miny ),
            QgsPoint( minx, miny ) ]
        geometry = QgsGeometry().fromPolygon( [ rect ] )
        feat = QgsFeature()
        feat.setGeometry( geometry )
        feat.setAttributeMap( {
            0 : QVariant( minx ),
            1 : QVariant( miny ),
            2 : QVariant( maxx ),
            3 : QVariant( maxy ),
            4 : QVariant( cntx ),
            5 : QVariant( cnty ),
            6 : QVariant( area ),
            7 : QVariant( perim ),
            8 : QVariant( height ),
            9 : QVariant( width ) } )
        writer.addFeature( feat )
        del writer


    def defineCharacteristics(self):
        self.name = "Extent from layer"
        self.group = "Research tools"
        self.addParameter(ParameterVector(ExtentFromLayer.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addOutput(OutputVector(ExtentFromLayer.OUTPUT, "Extent layer"))
    #=========================================================
