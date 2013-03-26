# -*- coding: utf-8 -*-

"""
***************************************************************************
    Union.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.algs.ftools import FToolsUtils as utils
from sextante.core.SextanteLog import SextanteLog
from sextante.core.GeoAlgorithm import GeoAlgorithm

class   Union(GeoAlgorithm):

    INPUT = "INPUT"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/union.png")
    #===========================================================================

    def processAlgorithm(self, progress):
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Union.INPUT))
        vlayerB = QGisLayers.getObjectFromUri(self.getParameterValue(Union.INPUT2))
        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True
        vproviderA = vlayerA.dataProvider()

        fields = utils.combineVectorFields(vlayerA, vlayerB )
        names = [field.name() for field in fields]
        SextanteLog.addToLog(SextanteLog.LOG_INFO, str(names))
        writer = self.getOutputFromName(Union.OUTPUT).getVectorWriter(fields, vproviderA.geometryType(), vproviderA.crs() )
        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        indexA = utils.createSpatialIndex(vlayerB)
        indexB = utils.createSpatialIndex(vlayerA)

        count = 0
        nElement = 0
        featuresA = QGisLayers.features(vlayerA)
        nFeat = len(featuresA)
        for inFeatA in featuresA:
          progress.setPercentage(nElement/float(nFeat) * 50)
          nElement += 1
          found = False
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMapA = inFeatA.attributes()
          intersects = indexA.intersects( geom.boundingBox() )
          if len( intersects ) < 1:
            try:
              outFeat.setGeometry( geom )
              outFeat.setAttributes( atMapA )
              writer.addFeature( outFeat )
            except:
              # this really shouldn't happen, as we
              # haven't edited the input geom at all
              raise GeoAlgorithmExecutionException("Feature exception while computing union")
          else:
            for id in intersects:
                count += 1
                request = QgsFeatureRequest().setFilterFid(id)
                inFeatB = vlayerB.getFeatures(request).next()
                atMapB = inFeatB.attributes()
                tmpGeom = QgsGeometry( inFeatB.geometry() )

                if geom.intersects( tmpGeom ):
                  found = True
                  int_geom = geom.intersection( tmpGeom )

                  if int_geom is None:
                    # There was a problem creating the intersection
                    raise GeoAlgorithmExecutionException("Geometry exception while computing intersection")
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
                    attrs = []
                    attrs.extend(atMapA)
                    attrs.extend(atMapB)
                    outFeat.setAttributes(attrs)
                    writer.addFeature( outFeat )
                  except Exception, err:
                    raise GeoAlgorithmExecutionException("Feature exception while computing union")
                else:
                  # this only happends if the bounding box
                  # intersects, but the geometry doesn't
                  try:
                    outFeat.setGeometry( geom )
                    outFeat.setAttributes( atMapA )
                    writer.addFeature( outFeat )
                  except:
                    # also shoudn't ever happen
                    raise GeoAlgorithmExecutionException("Feature exception while computing union")


            if found:
              try:
                if diff_geom.wkbType() == 0:
                  temp_list = diff_geom.asGeometryCollection()
                  for i in temp_list:
                    if i.type() == geom.type():
                        diff_geom = QgsGeometry( i )
                outFeat.setGeometry( diff_geom )
                outFeat.setAttributes( atMapA )
                writer.addFeature( outFeat )
              except Exception, err:
                raise GeoAlgorithmExecutionException("Feature exception while computing union")

        length = len(vproviderA.fields())

        featuresA = QGisLayers.features(vlayerB)
        nFeat = len(featuresA)
        for inFeatA in featuresA:
            progress.setPercentage(nElement/float(nFeat) * 100)
            add = False
            geom = QgsGeometry( inFeatA.geometry() )
            diff_geom = QgsGeometry( geom )
            atMap = [None] * length
            atMap.extend(inFeatA.attributes())
            #atMap = dict( zip( range( length, length + len( atMap ) ), atMap ) )
            intersects = indexB.intersects( geom.boundingBox() )

            if len(intersects) < 1:
                try:
                    outFeat.setGeometry( geom )
                    outFeat.setAttributes( atMap )
                    writer.addFeature( outFeat )
                except Exception, err:
                    raise GeoAlgorithmExecutionException("Feature exception while computing union")
            else:
                for id in intersects:
                    request = QgsFeatureRequest().setFilterFid(id)
                    inFeatB = vlayerA.getFeatures(request).next()
                    atMapB = inFeatB.attributes()
                    tmpGeom = QgsGeometry( inFeatB.geometry() )
                    try:
                        if diff_geom.intersects( tmpGeom ):
                            add = True
                            diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
                        else:
                            # this only happends if the bounding box
                            # intersects, but the geometry doesn't
                            outFeat.setGeometry( diff_geom )
                            outFeat.setAttributes( atMap )
                            writer.addFeature( outFeat )
                    except Exception, err:
                        raise GeoAlgorithmExecutionException("Geometry exception while computing intersection")

            if add:
                try:
                    outFeat.setGeometry( diff_geom )
                    outFeat.setAttributes( atMapB )
                    writer.addFeature( outFeat )
                except Exception, err:
                    raise err
                    FEATURE_EXCEPT = False
            nElement += 1


        del writer
        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing intersection")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing interesection")

    def defineCharacteristics(self):
        self.name = "Union"
        self.group = "Vector overlay tools"
        self.addParameter(ParameterVector(Union.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(Union.INPUT2, "Input layer 2", ParameterVector.VECTOR_TYPE_ANY))
        self.addOutput(OutputVector(Union.OUTPUT, "Union"))
