from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog

def buffering(progress, output, distance, field, useSelection, useField, layer, dissolve, segments ):
    settings = QSettings()
    systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = layer.dataProvider()
    allAttrs = vproviderA.attributeIndexes()
    vproviderA.select( allAttrs )
    fields = vproviderA.fields()
    if useField:
        field = vproviderA.fieldNameIndex(field)
    writer = QgsVectorFileWriter(output, systemEncoding,  fields, QGis.WKBPolygon, vproviderA.crs() )
    # check if writer was created properly, if not, return with error
    if writer.hasError():
      raise GeoAlgorithmExecutionException("Could not create output file");
    outFeat = QgsFeature()
    inFeat = QgsFeature()
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()
    nElement = 0
    # there is selection in input layer
    if useSelection:
      nFeat = layer.selectedFeatureCount()
      selectionA = layer.selectedFeatures()
      # with dissolve
      if dissolve:
        first = True
        for inFeat in selectionA:
          atMap = inFeat.attributeMap()
          if useField:
            value = atMap[ field ].doDouble()[ 0 ]
          else:
            value = distance
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), segments)
            if first:
              tempGeom = QgsGeometry( outGeom )
              first = False
            else:
              try:
                tempGeom = tempGeom.combine( outGeom )
              except:
                GEOS_EXCEPT = False
                continue
          except:
            GEOS_EXCEPT = False
            continue
          nElement += 1
          progress.setPercentage(int(nElement/nFeat * 100))
        try:
          outFeat.setGeometry( tempGeom )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
      # without dissolve
      else:
        for inFeat in selectionA:
          atMap = inFeat.attributeMap()
          if useField:
            value = atMap[ self.myParam ].toDouble()[ 0 ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), segments )
            try:
              outFeat.setGeometry( outGeom )
              outFeat.setAttributeMap( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
          except:
            GEOS_EXCEPT = False
            continue
          nElement += 1
          progress.setPercentage(int(nElement/nFeat * 100))
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      # with dissolve
      if dissolve:
        first = True
        while vproviderA.nextFeature( inFeat ):
          atMap = inFeat.attributeMap()
          if useField:
            value = atMap[ field ].toDouble()[ 0 ]
          else:
            value = distance
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), segments)
            if first:
              tempGeom = QgsGeometry( outGeom )
              first = False
            else:
              try:
                tempGeom = tempGeom.combine( outGeom )
              except:
                GEOS_EXCEPT = False
                continue
          except:
            GEOS_EXCEPT = False
            continue
          nElement += 1
          progress.setPercentage(int(nElement/nFeat * 100))
        try:
          outFeat.setGeometry( tempGeom )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
      # without dissolve
      else:
        vproviderA.rewind()
        while vproviderA.nextFeature( inFeat ):
          atMap = inFeat.attributeMap()
          if useField:
            value = atMap[ field ].toDouble()[ 0 ]
          else:
            value = distance
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ),segments )
            try:
              outFeat.setGeometry( outGeom )
              outFeat.setAttributeMap( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
          except:
            GEOS_EXCEPT = False
            continue
          nElement += 1
          progress.setPercentage(int(nElement/nFeat * 100))
    del writer
    if not GEOS_EXCEPT:
        SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing buffer")
    if not FEATURE_EXCEPT:
        SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing buffer")



