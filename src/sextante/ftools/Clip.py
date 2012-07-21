from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.ftools import ftools_utils
from sextante.core.SextanteLog import SextanteLog
from sextante.parameters.ParameterBoolean import ParameterBoolean

class Clip(GeoAlgorithm):

    INPUT = "INPUT"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"
    USE_SELECTED = "USE_SELECTED"
    USE_SELECTED2 = "USE_SELECTED2"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/clip.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(Clip.OUTPUT)
        useSelection = self.getParameterValue(Clip.USE_SELECTED)
        useSelection2 = self.getParameterValue(Clip.USE_SELECTED2)
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Clip.INPUT))
        vlayerB = QGisLayers.getObjectFromUri(self.getParameterValue(Clip.INPUT2))
        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True
        vproviderA = vlayerA.dataProvider()
        allAttrsA = vproviderA.attributeIndexes()
        vproviderA.select( allAttrsA )
        vproviderB = vlayerB.dataProvider()
        allAttrsB = vproviderB.attributeIndexes()
        vproviderB.select( allAttrsB )
        # check for crs compatibility
        crsA = vproviderA.crs()
        crsB = vproviderB.crs()
        if not crsA.isValid() or not crsB.isValid():
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Intersection. Invalid CRS. Results might be unexpected")
        else:
            if not crsA != crsB:
                SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Intersection. Non-matching CRSs. Results might be unexpected")
        fields = vproviderA.fields()
        writer = QgsVectorFileWriter( output, systemEncoding,fields, vproviderA.geometryType(), vproviderA.crs() )

        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        index = ftools_utils.createIndex( vproviderB )
        vproviderA.rewind()
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
              geom = QgsGeometry( inFeatA.geometry() )
              int_geom = QgsGeometry( geom )
              atMap = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              found = False
              first = True
              for id in intersects:
                if id in selectionB:
                  vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                  tmpGeom = QgsGeometry( inFeatB.geometry() )
                  if tmpGeom.intersects( geom ):
                    found = True
                    if first:
                      outFeat.setGeometry( QgsGeometry( tmpGeom ) )
                      first = False
                    else:
                      try:
                        cur_geom = QgsGeometry( outFeat.geometry() )
                        new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                        outFeat.setGeometry( QgsGeometry( new_geom ) )
                      except:
                        GEOS_EXCEPT = False
                        break
              if found:
                try:
                  cur_geom = QgsGeometry( outFeat.geometry() )
                  new_geom = QgsGeometry( geom.intersection( cur_geom ) )
                  if new_geom.wkbType() == 7:
                    int_com = QgsGeometry( geom.combine( cur_geom ) )
                    int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                    new_geom = QgsGeometry( int_com.difference( int_sym ) )
                  try:
                    outFeat.setGeometry( new_geom )
                    outFeat.setAttributeMap( atMap )
                    writer.addFeature( outFeat )
                  except:
                    FEAT_EXCEPT = False
                    continue
                except:
                  GEOS_EXCEPT = False
                  continue
          # we have no selection in overlay layer
          else:
            for inFeatA in selectionA:
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              geom = QgsGeometry( inFeatA.geometry() )
              atMap = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              found = False
              first = True
              for id in intersects:
                vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                tmpGeom = QgsGeometry( inFeatB.geometry() )
                if tmpGeom.intersects( geom ):
                  found = True
                  if first:
                    outFeat.setGeometry( QgsGeometry( tmpGeom ) )
                    first = False
                  else:
                    try:
                      cur_geom = QgsGeometry( outFeat.geometry() )
                      new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                      outFeat.setGeometry( QgsGeometry( new_geom ) )
                    except:
                      GEOS_EXCEPT = False
                      break
              if found:
                try:
                  cur_geom = QgsGeometry( outFeat.geometry() )
                  new_geom = QgsGeometry( geom.intersection( cur_geom ) )
                  if new_geom.wkbType() == 7:
                    int_com = QgsGeometry( geom.combine( cur_geom ) )
                    int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                    new_geom = QgsGeometry( int_com.difference( int_sym ) )
                  try:
                    outFeat.setGeometry( new_geom )
                    outFeat.setAttributeMap( atMap )
                    writer.addFeature( outFeat )
                  except:
                    FEAT_EXCEPT = False
                    continue
                except:
                  GEOS_EXCEPT = False
                  continue
        # there is no selection in input layer
        else:
          nFeat = vproviderA.featureCount()
          # we have selection in overlay layer
          if useSelection2:
            selectionB = vlayerB.selectedFeaturesIds()
            while vproviderA.nextFeature( inFeatA ):
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              geom = QgsGeometry( inFeatA.geometry() )
              atMap = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              found = False
              first = True
              for id in intersects:
                if id in selectionB:
                  vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                  tmpGeom = QgsGeometry( inFeatB.geometry() )
                  if tmpGeom.intersects( geom ):
                    found = True
                    if first:
                      outFeat.setGeometry( QgsGeometry( tmpGeom ) )
                      first = False
                    else:
                      try:
                        cur_geom = QgsGeometry( outFeat.geometry() )
                        new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                        outFeat.setGeometry( QgsGeometry( new_geom ) )
                      except:
                        GEOS_EXCEPT = False
                        break
              if found:
                try:
                  cur_geom = QgsGeometry( outFeat.geometry() )
                  new_geom = QgsGeometry( geom.intersection( cur_geom ) )
                  if new_geom.wkbType() == 7:
                    int_com = QgsGeometry( geom.combine( cur_geom ) )
                    int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                    new_geom = QgsGeometry( int_com.difference( int_sym ) )
                  try:
                    outFeat.setGeometry( new_geom )
                    outFeat.setAttributeMap( atMap )
                    writer.addFeature( outFeat )
                  except:
                    FEAT_EXCEPT = False
                    continue
                except:
                  GEOS_EXCEPT = False
                  continue
          # we have no selection in overlay layer
          else:
            while vproviderA.nextFeature( inFeatA ):
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              geom = QgsGeometry( inFeatA.geometry() )
              atMap = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              first = True
              found = False
              if len( intersects ) > 0:
                for id in intersects:
                  vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                  tmpGeom = QgsGeometry( inFeatB.geometry() )
                  if tmpGeom.intersects( geom ):
                    found = True
                    if first:
                      outFeat.setGeometry( QgsGeometry( tmpGeom ) )
                      first = False
                    else:
                      try:
                        cur_geom = QgsGeometry( outFeat.geometry() )
                        new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                        outFeat.setGeometry( QgsGeometry( new_geom ) )
                      except:
                        GEOS_EXCEPT = False
                        break
                if found:
                  try:
                    cur_geom = QgsGeometry( outFeat.geometry() )
                    new_geom = QgsGeometry( geom.intersection( cur_geom ) )
                    if new_geom.wkbType() == 7:
                      int_com = QgsGeometry( geom.combine( cur_geom ) )
                      int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                      new_geom = QgsGeometry( int_com.difference( int_sym ) )
                    try:
                      outFeat.setGeometry( new_geom )
                      outFeat.setAttributeMap( atMap )
                      writer.addFeature( outFeat )
                    except:
                      FEAT_EXCEPT = False
                      continue
                  except:
                    GEOS_EXCEPT = False
                    continue
        del writer
        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing clip")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing clip")

    def defineCharacteristics(self):
        self.name = "Clip"
        self.group = "Geoprocessing tools"
        self.addParameter(ParameterVector(Clip.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(Clip.INPUT2, "Clip layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterBoolean(Clip.USE_SELECTED, "Use selected features (input)", False))
        self.addParameter(ParameterBoolean(Clip.USE_SELECTED2, "Use selected features (clip)", False))
        self.addOutput(OutputVector(Clip.OUTPUT, "Clipped"))
