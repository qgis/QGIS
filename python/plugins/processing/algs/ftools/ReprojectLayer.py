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

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.QGisLayers import QGisLayers
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterCrs import ParameterCrs
from processing.outputs.OutputVector import OutputVector

class ReprojectLayer(GeoAlgorithm):

    INPUT = "INPUT"
    TARGET_CRS = "TARGET_CRS"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/reproject.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Reproject layer"
        self.group = "Vector general tools"

        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterCrs(self.TARGET_CRS, "Target CRS", "EPSG:4326"))

        self.addOutput(OutputVector(self.OUTPUT, "Reprojected layer"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        crsId = self.getParameterValue(self.TARGET_CRS)
        targetCrs = QgsCoordinateReferenceSystem(crsId)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(layer.pendingFields().toList(),
                     layer.wkbType(), targetCrs)

        layerCrs = layer.crs()
        crsTransform = QgsCoordinateTransform(layerCrs, targetCrs)

        outFeat = QgsFeature()
        current = 0
        features = QGisLayers.features(layer)
        total = 100.0 / float(len(features))
        for f in features:
            geom = f.geometry()
            geom.transform(crsTransform)
            outFeat.setGeometry(geom)
            outFeat.setAttributes(f.attributes())
            writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(int(current * total))

        del writer

        self.crs = targetCrs
