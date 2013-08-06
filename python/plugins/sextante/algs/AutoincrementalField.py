# -*- coding: utf-8 -*-

"""
***************************************************************************
    AutoincrementalField.py
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

from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4.QtCore import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector

class AutoincrementalField(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")
    #===========================================================================

    def processAlgorithm(self, progress):
        output = self.getOutputFromName(self.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        vprovider = vlayer.dataProvider()
        fields = vprovider.fields()
        fields.append(QgsField("AUTO", QVariant.Int))
        writer = output.getVectorWriter(fields, vprovider.geometryType(), vlayer.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nElement = 0
        features = QGisLayers.features(vlayer)
        nFeat = len(features)
        for inFeat in features:
            progress.setPercentage(int((100 * nElement)/nFeat))
            nElement += 1
            inGeom = inFeat.geometry()
            outFeat.setGeometry( inGeom )
            attrs = inFeat.attributes()
            attrs.append(nElement)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
        del writer

    def defineCharacteristics(self):
        self.name = "Add autoincremental field"
        self.group = "Vector table tools"
        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))



