# -*- coding: utf-8 -*-

"""
***************************************************************************
    SnapGeometries.py
    -----------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Nyall Dawson
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
from builtins import str

__author__ = 'Nyall Dawson'
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.analysis import QgsGeometrySnapper
from qgis.core import QgsFeature

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector, ParameterNumber, ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SnapGeometriesToLayer(GeoAlgorithm):

    INPUT = 'INPUT'
    REFERENCE_LAYER = 'REFERENCE_LAYER'
    TOLERANCE = 'TOLERANCE'
    OUTPUT = 'OUTPUT'
    BEHAVIOR = 'BEHAVIOR'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Snap geometries to layer')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer')))
        self.addParameter(ParameterVector(self.REFERENCE_LAYER, self.tr('Reference layer')))
        self.addParameter(ParameterNumber(self.TOLERANCE, self.tr('Tolerance (layer units)'), 0.00000001, 9999999999, default=10.0))

        self.modes = [self.tr('Prefer aligning nodes'),
                      self.tr('Prefer closest point')]
        self.addParameter(ParameterSelection(
            self.BEHAVIOR,
            self.tr('Behavior'),
            self.modes, default=0))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Snapped geometries')))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        reference_layer = dataobjects.getObjectFromUri(self.getParameterValue(self.REFERENCE_LAYER))
        tolerance = self.getParameterValue(self.TOLERANCE)
        mode = self.getParameterValue(self.BEHAVIOR)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields(), layer.wkbType(), layer.crs())

        features = vector.features(layer)

        self.processed = 0
        self.feedback = feedback
        self.total = 100.0 / len(features)

        snapper = QgsGeometrySnapper(reference_layer)
        snapper.featureSnapped.connect(self.featureSnapped)
        snapped_features = snapper.snapFeatures(features, tolerance, mode)
        for f in snapped_features:
            writer.addFeature(QgsFeature(f))

        del writer

    def featureSnapped(self):
        self.processed += 1
        self.feedback.setProgress(int(self.processed * self.total))
