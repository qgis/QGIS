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

class   Union(GeoAlgorithm):

    INPUT = "INPUT"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/union.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(Union.OUTPUT)
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Union.INPUT))
        vlayerB = QGisLayers.getObjectFromUri(self.getParameterValue(Union.INPUT2))
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
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Union. Invalid CRS. Results might be unexpected")
        else:
            if not crsA != crsB:
                SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Union. Non-matching CRSs. Results might be unexpected")
        fields = ftools_utils.combineVectorFields(vlayerA, vlayerB )
        longNames = ftools_utils.checkFieldNameLength( fields )
        if not longNames.isEmpty():
            raise GeoAlgorithmExecutionException("Following field names are longer than 10 characters:\n" +  longNames.join('\n') )
        writer = QgsVectorFileWriter( output, systemEncoding, fields, vproviderA.geometryType(), vproviderA.crs() )
        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        indexA = ftools_utils.createIndex( vproviderB )
        indexB = ftools_utils.createIndex( vproviderA )
        nFeat = vproviderA.featureCount() * vproviderB.featureCount()
        vproviderA.rewind()
        count = 0
        nElement = 0

        while vproviderA.nextFeature( inFeatA ):
          progress.setPercentage(int(nElement/nFeat * 100))
          nElement += 1
          found = False
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMapA = inFeatA.attributeMap()
          intersects = indexA.intersects( geom.boundingBox() )
          if len( intersects ) < 1:
            try:
              outFeat.setGeometry( geom )
              outFeat.setAttributeMap( atMapA )
              writer.addFeature( outFeat )
            except:
              # this really shouldn't happen, as we
              # haven't edited the input geom at all
              FEATURE_EXCEPT = False
          else:
            for id in intersects:
              count += 1
              vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
              atMapB = inFeatB.attributeMap()
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              try:
                if geom.intersects( tmpGeom ):
                  found = True
                  int_geom = geom.intersection( tmpGeom )

                  if int_geom is None:
                    # There was a problem creating the intersection
                    GEOS_EXCEPT = False
                    int_geom = QgsGeometry()
                  else:
                    int_geom = QgsGeometry(int_geom)

                  if diff_geom.intersects( tmpGeom ):
                    diff_geom = diff_geom.difference( tmpGeom )
                    if diff_geom is None:
                      # It's possible there was an error here?
                      diff_geom = QgsGeometry()
                    else:
                      diff_geom = QgsGeometry(diff_geom)

                  if int_geom.wkbType() == 0:
                    # intersection produced different geomety types
                    temp_list = int_geom.asGeometryCollection()
                    for i in temp_list:
                      if i.type() == geom.type():
                          int_geom = QgsGeometry( i )
                  try:
                    outFeat.setGeometry( int_geom )
                    outFeat.setAttributeMap( ftools_utils.combineVectorAttributes( atMapA, atMapB ) )
                    writer.addFeature( outFeat )
                  except Exception, err:
                    FEATURE_EXCEPT = False
                else:
                  # this only happends if the bounding box
                  # intersects, but the geometry doesn't
                  try:
                    outFeat.setGeometry( geom )
                    outFeat.setAttributeMap( atMapA )
                    writer.addFeature( outFeat )
                  except:
                    # also shoudn't ever happen
                    FEATURE_EXCEPT = False
              except Exception, err:
                GEOS_EXCEPT = False
                found = False

            if found:
              try:
                if diff_geom.wkbType() == 0:
                  temp_list = diff_geom.asGeometryCollection()
                  for i in temp_list:
                    if i.type() == geom.type():
                        diff_geom = QgsGeometry( i )
                outFeat.setGeometry( diff_geom )
                outFeat.setAttributeMap( atMapA )
                writer.addFeature( outFeat )
              except Exception, err:
                FEATURE_EXCEPT = False

        length = len( vproviderA.fields().values() )
        vproviderB.rewind()

        while vproviderB.nextFeature( inFeatA ):
          add = False
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMap = inFeatA.attributeMap().values()
          atMap = dict( zip( range( length, length + len( atMap ) ), atMap ) )
          intersects = indexB.intersects( geom.boundingBox() )

          if len(intersects) < 1:
            try:
              outFeat.setGeometry( geom )
              outFeat.setAttributeMap( atMap )
              writer.addFeature( outFeat )
            except Exception, err:
              FEATURE_EXCEPT = False
          else:
            for id in intersects:
              vproviderA.featureAtId( int( id ), inFeatB , True, allAttrsA )
              atMapB = inFeatB.attributeMap()
              tmpGeom = QgsGeometry( inFeatB.geometry() )

              try:
                if diff_geom.intersects( tmpGeom ):
                  add = True
                  diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
                else:
                  # this only happends if the bounding box
                  # intersects, but the geometry doesn't
                  outFeat.setGeometry( diff_geom )
                  outFeat.setAttributeMap( atMap )
                  writer.addFeature( outFeat )
              except Exception, err:
                add = False
                GEOS_EXCEPT = False

            if add:
              try:
                outFeat.setGeometry( diff_geom )
                outFeat.setAttributeMap( atMapB )
                writer.addFeature( outFeat )
              except Exception, err:
                FEATURE_EXCEPT = False
          progress.setPercentage(int(nElement/nFeat * 100))
          nElement += 1
          del writer
          if not GEOS_EXCEPT:
              SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing intersection")
          if not FEATURE_EXCEPT:
              SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing interesection")

    def defineCharacteristics(self):
        self.name = "Union"
        self.group = "Geoprocessing tools"
        self.addParameter(ParameterVector(Union.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(Union.INPUT2, "Input layer 2", ParameterVector.VECTOR_TYPE_ANY))
        self.addOutput(OutputVector(Union.OUTPUT, "Intersection"))
