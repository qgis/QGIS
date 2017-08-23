# -*- coding: utf-8 -*-

"""
***************************************************************************
    SaveSelectedFeatures.py
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

from qgis.core import (QgsFeatureSink,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class SaveSelectedFeatures(QgisAlgorithm):

    OUTPUT = 'OUTPUT'
    INPUT = 'INPUT'

    def group(self):
        return self.tr('Vector general')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Selection')))

    def name(self):
        return 'saveselectedfeatures'

    def displayName(self):
        return self.tr('Save selected features')

    def processAlgorithm(self, parameters, context, feedback):
        vectorLayer = self.parameterAsVectorLayer(parameters, self.INPUT, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               vectorLayer.fields(), vectorLayer.wkbType(), vectorLayer.sourceCrs())

        features = vectorLayer.getSelectedFeatures()
        count = int(vectorLayer.selectedFeatureCount())

        total = 100.0 / count if count else 1
        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            sink.addFeature(feat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
