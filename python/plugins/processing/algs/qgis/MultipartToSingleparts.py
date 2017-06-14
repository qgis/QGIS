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

from qgis.core import (QgsWkbTypes,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class MultipartToSingleparts(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'multi_to_single.png'))

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Single parts')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Single parts')))

    def name(self):
        return 'multiparttosingleparts'

    def displayName(self):
        return self.tr('Multipart to singleparts')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        geomType = QgsWkbTypes.singleType(source.wkbType())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), geomType, source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount()

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            input_geometry = f.geometry()
            if input_geometry:
                if input_geometry.isMultipart():
                    for g in input_geometry.asGeometryCollection():
                        output_feature = f
                        output_feature.setGeometry(g)
                        sink.addFeature(output_feature)
                else:
                    sink.addFeature(f)
            else:
                #input feature with null geometry
                sink.addFeature(f)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
