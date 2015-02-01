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

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SaveSelectedFeatures(GeoAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'

    def defineCharacteristics(self):
        self.name = 'Save selected features'
        self.group = 'Vector general tools'

        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY], False))

        self.addOutput(OutputVector(self.OUTPUT_LAYER,
            self.tr('Output layer with selected features')))

    def processAlgorithm(self, progress):
        inputFilename = self.getParameterValue(self.INPUT_LAYER)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        vectorLayer = dataobjects.getObjectFromUri(inputFilename)

        provider = vectorLayer.dataProvider()
        writer = output.getVectorWriter(provider.fields(),
                provider.geometryType(), vectorLayer.crs())

        features = vector.features(vectorLayer)
        total = len(features)
        for (i, feat) in enumerate(features):
            writer.addFeature(feat)
            progress.setPercentage(100 * i / float(total))
        del writer
