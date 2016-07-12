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

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import Qgis, QgsFeature, QgsGeometry, QgsFeatureRequest, QgsWKBTypes

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Clip(GeoAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'clip.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Clip')
        self.group, self.i18n_group = self.trAlgorithm('Vector overlay tools')
        self.addParameter(ParameterVector(Clip.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(Clip.OVERLAY,
                                          self.tr('Clip layer'), [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addOutput(OutputVector(Clip.OUTPUT, self.tr('Clipped')))

    def processAlgorithm(self, progress):
        sourceLayer = dataobjects.getObjectFromUri(
            self.getParameterValue(Clip.INPUT))
        maskLayer = dataobjects.getObjectFromUri(
            self.getParameterValue(Clip.OVERLAY))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            sourceLayer.pendingFields(),
            sourceLayer.dataProvider().geometryType(),
            sourceLayer.dataProvider().crs())

        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()

        # first build up a list of clip geometries
        clip_geoms = []
        for maskFeat in vector.features(maskLayer, QgsFeatureRequest().setSubsetOfAttributes([])):
            clip_geoms.append(maskFeat.geometry())

        if len(clip_geoms) > 1:
            combined_clip_geom = QgsGeometry.unaryUnion(clip_geoms)
        else:
            combined_clip_geom = clip_geoms[0]

        # use prepared geometries for faster insection tests
        engine = QgsGeometry.createGeometryEngine(combined_clip_geom.geometry())
        engine.prepareGeometry()

        input_features = [f for f in vector.features(sourceLayer, QgsFeatureRequest().setFilterRect(combined_clip_geom.boundingBox()))]
        total = 100.0 / len(input_features)
        for current, in_feat in enumerate(input_features):
            if not in_feat.geometry():
                continue

            if not engine.intersects(in_feat.geometry().geometry()):
                continue

            if not engine.contains(in_feat.geometry().geometry()):
                cur_geom = in_feat.geometry()
                new_geom = combined_clip_geom.intersection(cur_geom)
                if new_geom.wkbType() == Qgis.WKBUnknown or QgsWKBTypes.flatType(new_geom.geometry().wkbType()) == QgsWKBTypes.GeometryCollection:
                    int_com = in_feat.geometry().combine(new_geom)
                    int_sym = in_feat.geometry().symDifference(new_geom)
                    new_geom = int_com.difference(int_sym)
                    if new_geom.isGeosEmpty() or not new_geom.isGeosValid():
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                               self.tr('GEOS geoprocessing error: One or more '
                                                       'input features have invalid geometry.'))
            else:
                # clip geometry totally contains feature geometry, so no need to perform intersection
                new_geom = in_feat.geometry()

            try:
                outFeat = QgsFeature()
                outFeat.setGeometry(new_geom)
                outFeat.setAttributes(in_feat.attributes())
                writer.addFeature(outFeat)
            except:
                ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                       self.tr('Feature geometry error: One or more '
                                               'output features ignored due to '
                                               'invalid geometry.'))
                continue

            progress.setPercentage(int(current * total))

        del writer
