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
from sextante.ftools import ftools_utils
from sextante.core.SextanteLog import SextanteLog
from sextante.parameters.ParameterBoolean import ParameterBoolean

class Difference(GeoAlgorithm):

    INPUT = "INPUT"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"
    USE_SELECTED = "USE_SELECTED"
    USE_SELECTED2 = "USE_SELECTED2"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/difference.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(Difference.OUTPUT)
        useSelection = self.getParameterValue(Difference.USE_SELECTED)
        useSelection2 = self.getParameterValue(Difference.USE_SELECTED2)
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Difference.INPUT))
        vlayerB = QGisLayers.getObjectFromUri(self.getParameterValue(Difference.INPUT2))
        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True
        vproviderA = vlayerA.dataProvider()
        allAttrsA = vproviderA.attributeIndexes()
        vproviderA.select( allAttrsA )
        vproviderB = vlayerB.dataProvider()
        allAttrsB = vproviderB.attributeIndexes()
        vproviderB.select( allAttrsB )
        fields = vproviderA.fields()
        # check for crs compatibility
        crsA = vproviderA.crs()
        crsB = vproviderB.crs()
        if not crsA.isValid() or not crsB.isValid():
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Difference. Invalid CRS. Results might be unexpected")
        else:
            if not crsA != crsB:
                SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Difference. Non-matching CRSs. Results might be unexpected")
        writer = QgsVectorFileWriter(output, systemEncoding, fields, vproviderA.geometryType(), vproviderA.crs() )
        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        index = ftools_utils.createIndex( vproviderB )
        nElement = 0
        # there is selection in input layer
        if useSelection:
          nFeat = vlayerA.selectedFeatureCount()
          selectionA = vlayerA.selectedFeatures()
          # we have selection in overlay layer
          if useSelection2:
            selectionB = vlayerB.selectedFeaturesIds()
            for inFeatA in selectionA:
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              add = True
              geom = QgsGeometry( inFeatA.geometry() )
              diff_geom = QgsGeometry( geom )
              atMap = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              for id in intersects:
                # is intersect feature in selection
                if id in selectionB:
                  vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                  tmpGeom = QgsGeometry( inFeatB.geometry() )
                  try:
                    if diff_geom.intersects( tmpGeom ):
                      diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
                  except:
                    GEOS_EXCEPT = False
                    add = False
                    break
              if add:
                try:
                  outFeat.setGeometry( diff_geom )
                  outFeat.setAttributeMap( atMap )
                  writer.addFeature( outFeat )
                except:
                  FEATURE_EXCEPT = False
                  continue
          # we have no selection in overlay layer
          else:
            for inFeatA in selectionA:
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              add = True
              geom = QgsGeometry( inFeatA.geometry() )
              diff_geom = QgsGeometry( geom )
              atMap = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              for id in intersects:
                vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                tmpGeom = QgsGeometry( inFeatB.geometry() )
                try:
                  if diff_geom.intersects( tmpGeom ):
                    diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
                except:
                  GEOS_EXCEPT = False
                  add = False
                  break
              if add:
                try:
                  outFeat.setGeometry( diff_geom )
                  outFeat.setAttributeMap( atMap )
                  writer.addFeature( outFeat )
                except:
                  FEATURE_EXCEPT = False
                  continue
        # there is no selection in input layer
        else:
          nFeat = vproviderA.featureCount()
          vproviderA.rewind()
          # we have selection in overlay layer
          if useSelection2:
            selectionB = vlayerB.selectedFeaturesIds()
            while vproviderA.nextFeature( inFeatA ):
              nElement += 1
              add = True
              progress.setPercentage(int(nElement/nFeat * 100))
              geom = QgsGeometry( inFeatA.geometry() )
              diff_geom = QgsGeometry( geom )
              atMap = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              for id in intersects:
                # now check if id in selection
                if id in selectionB:
                  vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                  tmpGeom = QgsGeometry( inFeatB.geometry() )
                  try:
                    if diff_geom.intersects( tmpGeom ):
                      diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
                  except:
                    GEOS_EXCEPT = False
                    add = False
                    break
              if add:
                try:
                  outFeat.setGeometry( diff_geom )
                  outFeat.setAttributeMap( atMap )
                  writer.addFeature( outFeat )
                except:
                  FEATURE_EXCEPT = False
                  continue
          # we have no selection in overlay layer
          else:
            while vproviderA.nextFeature( inFeatA ):
              nElement += 1
              add = True
              progress.setPercentage(int(nElement/nFeat * 100))
              geom = QgsGeometry( inFeatA.geometry() )
              diff_geom = QgsGeometry( geom )
              atMap = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              for id in intersects:
                vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                tmpGeom = QgsGeometry( inFeatB.geometry() )
                try:
                  if diff_geom.intersects( tmpGeom ):
                    diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
                except:
                  GEOS_EXCEPT = False
                  add = False
                  break
              if add:
                try:
                  outFeat.setGeometry( diff_geom )
                  outFeat.setAttributeMap( atMap )
                  writer.addFeature( outFeat )
                except:
                  FEATURE_EXCEPT = False
                  continue
        del writer
        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing difference")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing difference")

    def defineCharacteristics(self):
        self.name = "Difference"
        self.group = "Geoprocessing tools"
        self.addParameter(ParameterVector(Difference.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(Difference.INPUT2, "Difference layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterBoolean(Difference.USE_SELECTED, "Use selected features (input)", False))
        self.addParameter(ParameterBoolean(Difference.USE_SELECTED2, "Use selected features (difference)", False))
        self.addOutput(OutputVector(Difference.OUTPUT, "Difference"))
