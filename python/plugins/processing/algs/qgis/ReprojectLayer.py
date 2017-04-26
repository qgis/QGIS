# -*- coding: utf-8 -*-

"""
***************************************************************************
    ReprojectLayer.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Alexander Bruy
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
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import QgsCoordinateReferenceSystem, QgsCoordinateTransform, QgsFeature, QgsProcessingUtils
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterCrs
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector
from qgis.PyQt.QtGui import QIcon

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ReprojectLayer(GeoAlgorithm):

    INPUT = 'INPUT'
    TARGET_CRS = 'TARGET_CRS'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'warp.png'))

    def tags(self):
        return self.tr('transform,reproject,crs,srs,warp').split(',')

    def group(self):
        return self.tr('Vector general tools')

    def name(self):
        return 'reprojectlayer'

    def displayName(self):
        return self.tr('Reproject layer')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer')))
        self.addParameter(ParameterCrs(self.TARGET_CRS,
                                       self.tr('Target CRS'), 'EPSG:4326'))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Reprojected')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(self.getParameterValue(self.INPUT))
        crsId = self.getParameterValue(self.TARGET_CRS)
        targetCrs = QgsCoordinateReferenceSystem()
        targetCrs.createFromUserInput(crsId)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields().toList(), layer.wkbType(), targetCrs)

        layerCrs = layer.crs()
        crsTransform = QgsCoordinateTransform(layerCrs, targetCrs)

        outFeat = QgsFeature()
        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        for current, f in enumerate(features):
            geom = f.geometry()
            geom.transform(crsTransform)
            outFeat.setGeometry(geom)
            outFeat.setAttributes(f.attributes())
            writer.addFeature(outFeat)

            feedback.setProgress(int(current * total))

        del writer

        self.crs = targetCrs
