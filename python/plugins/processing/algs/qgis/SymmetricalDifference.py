# -*- coding: utf-8 -*-

"""
***************************************************************************
    SymetricalDifference.py
    ---------------------
    Date                 : September 2014
    Copyright            : (C) 2014 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsFeature, QgsGeometry, QgsFeatureRequest, NULL
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SymmetricalDifference(GeoAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Symmetrical difference')
        self.group, self.i18n_group = self.trAlgorithm('Vector overlay tools')
        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.OVERLAY,
            self.tr('Difference layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(self.OUTPUT,
            self.tr('Symetrical difference')))

    def processAlgorithm(self, progress):
        layerA = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        layerB = dataobjects.getObjectFromUri(
            self.getParameterValue(self.OVERLAY))

        providerA = layerA.dataProvider()
        providerB = layerB.dataProvider()

        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True

        fields = vector.combineVectorFields(layerA, layerB)
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, providerA.geometryType(), providerA.crs())

        featB = QgsFeature()
        outFeat = QgsFeature()

        indexA = vector.spatialindex(layerB)
        indexB = vector.spatialindex(layerA)

        featuresA = vector.features(layerA)
        featuresB = vector.features(layerB)

        total = 100.0 / (len(featuresA) * len(featuresB))
        count = 0

        for featA in featuresA:
            add = True
            geom = QgsGeometry(featA.geometry())
            diffGeom = QgsGeometry(geom)
            attrs = featA.attributes()
            intersects = indexA.intersects(geom.boundingBox())
            for i in intersects:
                providerB.getFeatures(QgsFeatureRequest().setFilterFid(i)).nextFeature(featB)
                tmpGeom = QgsGeometry(featB.geometry())
                try:
                    if diffGeom.intersects(tmpGeom):
                        diffGeom = QgsGeometry(diffGeom.difference(tmpGeom))
                except:
                    add = False
                    GEOS_EXCEPT = False
                    break
            if add:
                try:
                    outFeat.setGeometry(diffGeom)
                    outFeat.setAttributes(attrs)
                    writer.addFeature(outFeat)
                except:
                    FEATURE_EXCEPT = False
                    continue

            count += 1
            progress.setPercentage(int(count * total))

        length = len(providerA.fields())

        for featA in featuresB:
            add = True
            geom = QgsGeometry(featA.geometry())
            diffGeom = QgsGeometry(geom)
            attrs = featA.attributes()
            attrs = [NULL] * length + attrs
            intersects = indexB.intersects(geom.boundingBox())
            for i in intersects:
                providerA.getFeatures(QgsFeatureRequest().setFilterFid(i)).nextFeature(featB)
                tmpGeom = QgsGeometry(featB.geometry())
                try:
                    if diffGeom.intersects(tmpGeom):
                        diffGeom = QgsGeometry(diffGeom.difference(tmpGeom))
                except:
                    add = False
                    GEOS_EXCEPT = False
                    break
            if add:
                try:
                    outFeat.setGeometry(diffGeom)
                    outFeat.setAttributes(attrs)
                    writer.addFeature(outFeat)
                except:
                    FEATURE_EXCEPT = False
                    continue

            count += 1
            progress.setPercentage(int(count * total))

        del writer

        if not GEOS_EXCEPT:
            ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                self.tr('Geometry exception while computing symetrical difference'))
        if not FEATURE_EXCEPT:
            ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                self.tr('Feature exception while computing symetrical difference'))
