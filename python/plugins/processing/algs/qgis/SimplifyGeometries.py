# -*- coding: utf-8 -*-

"""
***************************************************************************
    SimplifyGeometries.py
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

from qgis.core import Qgis, QgsFeature, QgsGeometry, QgsWkbTypes, QgsMapToPixelSimplifier

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SimplifyGeometries(GeoAlgorithm):

    INPUT = 'INPUT'
    TOLERANCE = 'TOLERANCE'
    OUTPUT = 'OUTPUT'
    METHOD = 'METHOD'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'simplify.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Simplify geometries')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'),
                                          [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_LINE]))
        self.methods = [self.tr('Distance (Douglas-Peucker)'),
                        'Snap to grid',
                        'Area (Visvalingam)']
        self.addParameter(ParameterSelection(
            self.METHOD,
            self.tr('Simplification method'),
            self.methods, default=0))
        self.addParameter(ParameterNumber(self.TOLERANCE,
                                          self.tr('Tolerance'), 0.0, 10000000.0, 1.0))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Simplified')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        tolerance = self.getParameterValue(self.TOLERANCE)
        method = self.getParameterValue(self.METHOD)

        pointsBefore = 0
        pointsAfter = 0

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields().toList(), layer.wkbType(), layer.crs())

        features = vector.features(layer)
        total = 100.0 / len(features)

        if method != 0:
            simplifier = QgsMapToPixelSimplifier(QgsMapToPixelSimplifier.SimplifyGeometry, tolerance, method)

        for current, input_feature in enumerate(features):
            out_feature = input_feature
            if input_feature.geometry():
                input_geometry = input_feature.geometry()
                pointsBefore += input_geometry.geometry().nCoordinates()

                if method == 0: # distance
                    output_geometry = input_geometry.simplify(tolerance)
                else:
                    output_geometry = simplifier.simplify(input_geometry)

                pointsAfter += output_geometry.geometry().nCoordinates()
                out_feature.setGeometry(output_geometry)
            writer.addFeature(out_feature)
            progress.setPercentage(int(current * total))

        del writer

        ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                               self.tr('Simplify: Input geometries have been simplified from %s to %s points' % (pointsBefore, pointsAfter)))
