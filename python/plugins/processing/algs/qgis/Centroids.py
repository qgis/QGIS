# -*- coding: utf-8 -*-

"""
***************************************************************************
    Centroids.py
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

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsWkbTypes

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Centroids(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'centroids.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Centroids')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Centroids'), datatype=[dataobjects.TYPE_VECTOR_POINT]))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                layer.fields(),
                QgsWkbTypes.Point,
                layer.crs())

        features = vector.features(layer)
        total = 100.0 / len(features)
        for current, input_feature in enumerate(features):
            output_feature = input_feature
            if input_feature.geometry():
                output_geometry = input_feature.geometry().centroid()
                if not output_geometry:
                    ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                                           'Error calculating centroid for feature {}'.format(input_feature.id()))
                output_feature.setGeometry(output_geometry)

            writer.addFeature(output_feature)
            progress.setPercentage(int(current * total))

        del writer
