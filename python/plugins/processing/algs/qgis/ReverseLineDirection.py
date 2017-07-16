# -*- coding: utf-8 -*-

"""
***************************************************************************
    ReverseLineDirection.py
    -----------------------
    Date                 : November 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'November 2015'
__copyright__ = '(C) 2015, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import (QgsGeometry,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsProcessingException,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class ReverseLineDirection(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(
            QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Reversed'), QgsProcessing.TypeVectorLine))

    def name(self):
        return 'reverselinedirection'

    def displayName(self):
        return self.tr('Reverse line direction')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break

            outFeat = inFeat
            if inFeat.geometry():
                inGeom = inFeat.geometry()
                reversedLine = inGeom.geometry().reversed()
                if not reversedLine:
                    raise QgsProcessingException(
                        self.tr('Error reversing line'))
                outGeom = QgsGeometry(reversedLine)

                outFeat.setGeometry(outGeom)
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
