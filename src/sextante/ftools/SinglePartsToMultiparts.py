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
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(SinglePartsToMultiparts.INPUT))
        output = self.getOutputValue(SinglePartsToMultiparts.OUTPUT)
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        geomType = self.singleToMultiGeom(vprovider.geometryType())
        writer = QgsVectorFileWriter( output, systemEncoding,
            fields, geomType, vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        index = int(self.getParameterValue(SinglePartsToMultiparts.FIELD))
        unique = ftools_utils.getUniqueValues( vprovider, int( index ) )
        nFeat = vprovider.featureCount() * len( unique )
        nElement = 0
        #self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        #        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
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
              #self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ),  nElement )
            outFeat.setAttributeMap( atts )
            outGeom = QgsGeometry( self.convertGeometry(multi_feature, vType) )
            outFeat.setGeometry(outGeom)
            writer.addFeature(outFeat)
          del writer
        else:
          raise GeoAlgorithmExecutionException("Invalid unique ID Field")

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

    def defineCharacteristics(self):
        self.name = "Singleparts to multipart"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(SinglePartsToMultiparts.INPUT, "Input layer"))
        self.addParameter(ParameterTableField(SinglePartsToMultiparts.FIELD,
                                              "Unique ID field", SinglePartsToMultiparts.INPUT))
        self.addOutput(OutputVector(SinglePartsToMultiparts.OUTPUT, "Output layer"))
    #=========================================================
