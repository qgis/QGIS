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

class MultipartToSingleparts(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/multi_to_single.png")

    def processAlgorithm(self, progress):
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        geomType = self.multiToSingleGeom(vprovider.geometryType())
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields, geomType, vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        while vprovider.nextFeature( inFeat ):
            nElement += 1
            progress.setPercentage((nElement*100)/nFeat)
            inGeom = inFeat.geometry()
            atMap = inFeat.attributeMap()
            featList = self.extractAsSingle( inGeom )
            outFeat.setAttributeMap( atMap )
            for i in featList:
                outFeat.setGeometry( i )
                writer.addFeature( outFeat )
        del writer


    def multiToSingleGeom(self, wkbType):
        try:
            if wkbType in (QGis.WKBPoint, QGis.WKBMultiPoint,
                         QGis.WKBPoint25D, QGis.WKBMultiPoint25D):
                return QGis.WKBPoint
            elif wkbType in (QGis.WKBLineString, QGis.WKBMultiLineString,
                           QGis.WKBMultiLineString25D, QGis.WKBLineString25D):
                return QGis.WKBLineString
            elif wkbType in (QGis.WKBPolygon, QGis.WKBMultiPolygon,
                           QGis.WKBMultiPolygon25D, QGis.WKBPolygon25D):
                return QGis.WKBPolygon
            else:
                return QGis.WKBUnknown
        except Exception, err:
            raise GeoAlgorithmExecutionException(str(err))


    def extractAsSingle( self, geom ):
        multi_geom = QgsGeometry()
        temp_geom = []
        if geom.type() == 0:
            if geom.isMultipart():
                multi_geom = geom.asMultiPoint()
                for i in multi_geom:
                    temp_geom.append( QgsGeometry().fromPoint ( i ) )
            else:
                temp_geom.append( geom )
        elif geom.type() == 1:
            if geom.isMultipart():
                multi_geom = geom.asMultiPolyline()
                for i in multi_geom:
                    temp_geom.append( QgsGeometry().fromPolyline( i ) )
            else:
                temp_geom.append( geom )
        elif geom.type() == 2:
            if geom.isMultipart():
                multi_geom = geom.asMultiPolygon()
                for i in multi_geom:
                    temp_geom.append( QgsGeometry().fromPolygon( i ) )
            else:
                temp_geom.append( geom )
        return temp_geom


    def defineCharacteristics(self):
        self.name = "Multipart to singleparts"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(self.INPUT, "Input layer"))
        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

