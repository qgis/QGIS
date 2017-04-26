# -*- coding: utf-8 -*-

"""
***************************************************************************
    FixGeometry.py
    -----------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Alexander Bruy
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
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsWkbTypes,
                       QgsGeometry,
                       QgsApplication,
                       QgsMessageLog,
                       QgsProcessingUtils)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools import dataobjects, vector


class FixGeometry(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('repair,invalid,geometry').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'fixgeometries'

    def displayName(self):
        return self.tr('Fix geometries')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer'),
                                          [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_LINE]))
        self.addOutput(OutputVector(self.OUTPUT,
                                    self.tr('Layer with fixed geometries')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))

        writer = self.getOutputFromName(
            self.OUTPUT).getVectorWriter(
                layer.fields(),
                QgsWkbTypes.multiType(layer.wkbType()),
                layer.crs())

        features = QgsProcessingUtils.getFeatures(layer, context)
        if QgsProcessingUtils.featureCount(layer, context) == 0:
            raise GeoAlgorithmExecutionException(self.tr('There are no features in the input layer'))

        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        for current, inputFeature in enumerate(features):
            outputFeature = inputFeature
            if inputFeature.geometry():
                outputGeometry = inputFeature.geometry().makeValid()
                if not outputGeometry:
                    QgsMessageLog.logMessage('makeValid failed for feature {}'.format(inputFeature.id()), self.tr('Processing'), QgsMessageLog.WARNING)

                if outputGeometry.wkbType() == QgsWkbTypes.Unknown or QgsWkbTypes.flatType(outputGeometry.geometry().wkbType()) == QgsWkbTypes.GeometryCollection:
                    tmpGeometries = outputGeometry.asGeometryCollection()
                    for g in tmpGeometries:
                        if g.type() == inputFeature.geometry().type():
                            try:
                                g.convertToMultiType()
                                outputFeature.setGeometry(QgsGeometry(g))
                                writer.addFeature(outputFeature)
                            except:
                                pass
                    feedback.setProgress(int(current * total))
                    continue

                outputGeometry.convertToMultiType()
                outputFeature.setGeometry(outputGeometry)

            writer.addFeature(outputFeature)
            feedback.setProgress(int(current * total))

        del writer
