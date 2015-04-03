# -*- coding: utf-8 -*-

"""
***************************************************************************
    Difference.py
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

from qgis.core import QgsFeatureRequest, QgsFeature, QgsGeometry
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class Difference(GeoAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'

    #==========================================================================
    #def getIcon(self):
    #   return QtGui.QIcon(os.path.dirname(__file__) + "/icons/difference.png")
    #==========================================================================

    def defineCharacteristics(self):
        self.name = 'Difference'
        self.group = 'Vector overlay tools'
        self.addParameter(ParameterVector(Difference.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(Difference.OVERLAY,
            self.tr('Difference layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(Difference.OUTPUT, self.tr('Difference')))

    def processAlgorithm(self, progress):
        layerA = dataobjects.getObjectFromUri(
            self.getParameterValue(Difference.INPUT))
        layerB = dataobjects.getObjectFromUri(
            self.getParameterValue(Difference.OVERLAY))

        GEOS_EXCEPT = True

        FEATURE_EXCEPT = True

        writer = self.getOutputFromName(
            Difference.OUTPUT).getVectorWriter(layerA.pendingFields(),
            layerA.dataProvider().geometryType(),
            layerA.dataProvider().crs())

        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()

        index = vector.spatialindex(layerB)

        selectionA = vector.features(layerA)

        current = 0
        total = 100.0 / float(len(selectionA))

        for inFeatA in selectionA:
            add = True
            geom = QgsGeometry(inFeatA.geometry())
            diff_geom = QgsGeometry(geom)
            attrs = inFeatA.attributes()
            intersections = index.intersects(geom.boundingBox())
            for i in intersections:
                request = QgsFeatureRequest().setFilterFid(i)
                inFeatB = layerB.getFeatures(request).next()
                tmpGeom = QgsGeometry(inFeatB.geometry())
                try:
                    if diff_geom.intersects(tmpGeom):
                        diff_geom = QgsGeometry(diff_geom.difference(tmpGeom))
                except:
                    GEOS_EXCEPT = False
                    add = False
                    break

            if add:
                try:
                    outFeat.setGeometry(diff_geom)
                    outFeat.setAttributes(attrs)
                    writer.addFeature(outFeat)
                except:
                    FEATURE_EXCEPT = False
                    continue

            current += 1
            progress.setPercentage(int(current * total))

        del writer

        if not GEOS_EXCEPT:
            ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                self.tr('Geometry exception while computing difference'))
        if not FEATURE_EXCEPT:
            ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                self.tr('Feature exception while computing difference'))
