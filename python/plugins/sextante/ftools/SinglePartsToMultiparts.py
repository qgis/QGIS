from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.core.QGisLayers import QGisLayers
from sextante.ftools import ftools_utils
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.outputs.OutputVector import OutputVector

class SinglePartsToMultiparts(GeoAlgorithm):

    INPUT = "INPUT"
    FIELD = "FIELD"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/single_to_multi.png")

    def processAlgorithm(self, progress):
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(SinglePartsToMultiparts.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        geomType = self.singleToMultiGeom(vprovider.geometryType())
        writer = self.getOutputFromName(SinglePartsToMultiparts.OUTPUT).getVectorWriter(fields, geomType, vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        field = self.getParameterValue(SinglePartsToMultiparts.FIELD)
        index = vprovider.fieldNameIndex(field)
        unique = ftools_utils.getUniqueValues( vprovider, int( index ) )
        nFeat = vprovider.featureCount() * len( unique )
        nElement = 0
        if not len( unique ) == vlayer.featureCount():
          for i in unique:
            vprovider.rewind()
            multi_feature= []
            first = True
            vprovider.select(allAttrs)
            while vprovider.nextFeature( inFeat ):
              atMap = inFeat.attributeMap()
              idVar = atMap[ index ]
              if idVar.toString().trimmed() == i.toString().trimmed():
                if first:
                  atts = atMap
                  first = False
                inGeom = QgsGeometry( inFeat.geometry() )
                vType = inGeom.type()
                feature_list = self.extractAsMulti( inGeom )
                multi_feature.extend( feature_list )
              nElement += 1
            outFeat.setAttributeMap( atts )
            outGeom = QgsGeometry( self.convertGeometry(multi_feature, vType) )
            outFeat.setGeometry(outGeom)
            writer.addFeature(outFeat)
          del writer
        else:
          raise GeoAlgorithmExecutionException("Invalid unique ID Field")

    def extractAsMulti( self, geom ):
        temp_geom = []
        if geom.type() == 0:
          if geom.isMultipart():
            return geom.asMultiPoint()
          else:
            return [ geom.asPoint() ]
        elif geom.type() == 1:
          if geom.isMultipart():
            return geom.asMultiPolyline()
          else:
            return [ geom.asPolyline() ]
        else:
          if geom.isMultipart():
            return geom.asMultiPolygon()
          else:
            return [ geom.asPolygon() ]

    def singleToMultiGeom(self, wkbType):
        try:
          if wkbType in (QGis.WKBPoint, QGis.WKBMultiPoint,
                         QGis.WKBPoint25D, QGis.WKBMultiPoint25D):
              return QGis.WKBMultiPoint
          elif wkbType in (QGis.WKBLineString, QGis.WKBMultiLineString,
                           QGis.WKBMultiLineString25D, QGis.WKBLineString25D):
              return QGis.WKBMultiLineString
          elif wkbType in (QGis.WKBPolygon, QGis.WKBMultiPolygon,
                           QGis.WKBMultiPolygon25D, QGis.WKBPolygon25D):
              return QGis.WKBMultiPolygon
          else:
              return QGis.WKBUnknown
        except Exception, err:
          print str(err)

    def convertGeometry( self, geom_list, vType ):
        if vType == 0:
          return QgsGeometry().fromMultiPoint( geom_list )
        elif vType == 1:
          return QgsGeometry().fromMultiPolyline( geom_list )
        else:
          return QgsGeometry().fromMultiPolygon( geom_list )


    def defineCharacteristics(self):
        self.name = "Singleparts to multipart"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(SinglePartsToMultiparts.INPUT, "Input layer"))
        self.addParameter(ParameterTableField(SinglePartsToMultiparts.FIELD,
                                              "Unique ID field", SinglePartsToMultiparts.INPUT))
        self.addOutput(OutputVector(SinglePartsToMultiparts.OUTPUT, "Output layer"))
    #=========================================================
