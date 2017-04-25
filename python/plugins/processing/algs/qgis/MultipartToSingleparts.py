# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipartToSingleparts.py
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

from qgis.core import QgsWkbTypes, QgsProcessingUtils

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class MultipartToSingleparts(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'multi_to_single.png'))

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'multiparttosingleparts'

    def displayName(self):
        return self.tr('Multipart to singleparts')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer')))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Single parts')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(self.getParameterValue(self.INPUT))
        geomType = QgsWkbTypes.singleType(layer.wkbType())

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields().toList(), geomType, layer.crs())

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        for current, f in enumerate(features):
            input_geometry = f.geometry()
            if input_geometry:
                if input_geometry.isMultipart():
                    for g in input_geometry.asGeometryCollection():
                        output_feature = f
                        output_feature.setGeometry(g)
                        writer.addFeature(output_feature)
                else:
                    writer.addFeature(f)
            else:
                #input feature with null geometry
                writer.addFeature(f)

            feedback.setProgress(int(current * total))

        del writer
