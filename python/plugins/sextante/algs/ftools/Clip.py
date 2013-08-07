# -*- coding: utf-8 -*-

"""
***************************************************************************
    Clip.py
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteLog import SextanteLog

from sextante.parameters.ParameterVector import ParameterVector

from sextante.outputs.OutputVector import OutputVector

from sextante.algs.ftools import FToolsUtils as utils

class Clip(GeoAlgorithm):

    INPUT = "INPUT"
    OVERLAY = "OVERLAY"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/clip.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Clip"
        self.group = "Vector overlay tools"
        self.addParameter(ParameterVector(Clip.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(Clip.OVERLAY, "Clip layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(Clip.OUTPUT, "Clipped"))

    def processAlgorithm(self, progress):
        layerA = QGisLayers.getObjectFromUri(self.getParameterValue(Clip.INPUT))
        layerB = QGisLayers.getObjectFromUri(self.getParameterValue(Clip.OVERLAY))

        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(layerA.pendingFields(),
                     layerA.dataProvider().geometryType(), layerA.dataProvider().crs())

        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()

        index = utils.createSpatialIndex(layerB)

        selectionA = QGisLayers.features(layerA)

        current = 0
        total = 100.0 / float(len(selectionA))

        for inFeatA in selectionA:
            geom = QgsGeometry(inFeatA.geometry())
            attrs = inFeatA.attributes()
            intersections = index.intersects(geom.boundingBox())
            first = True
            found = False
            if len(intersections) > 0:
                for i in intersections:
                    request = QgsFeatureRequest().setFilterFid(i)
                    inFeatB = layerB.getFeatures(request).next()
                    tmpGeom = QgsGeometry(inFeatB.geometry())
                    if tmpGeom.intersects(geom):
                        found = True
                        if first:
                            outFeat.setGeometry(QgsGeometry(tmpGeom))
                            first = False
                        else:
                            try:
                                cur_geom = QgsGeometry(outFeat.geometry())
                                new_geom = QgsGeometry(cur_geom.combine(tmpGeom))
                                outFeat.setGeometry(QgsGeometry(new_geom))
                            except:
                                GEOS_EXCEPT = False
                                break
                if found:
                    try:
                        cur_geom = QgsGeometry(outFeat.geometry())
                        new_geom = QgsGeometry(geom.intersection(cur_geom))
                        if new_geom.wkbType() == QGis.WKBNoGeometry :
                            int_com = QgsGeometry(geom.combine(cur_geom))
                            int_sym = QgsGeometry(geom.symDifference(cur_geom))
                            new_geom = QgsGeometry(int_com.difference(int_sym))
                        try:
                            outFeat.setGeometry(new_geom)
                            outFeat.setAttributes(attrs)
                            writer.addFeature(outFeat)
                        except:
                            FEAT_EXCEPT = False
                            continue
                    except:
                        GEOS_EXCEPT = False
                        continue

            current += 1
            progress.setPercentage(int(current * total))

        del writer

        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing clip")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing clip")
