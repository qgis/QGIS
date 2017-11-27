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
                       QgsProcessingException,
                       QgsProcessing)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class ReverseLineDirection(QgisFeatureBasedAlgorithm):

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def name(self):
        return 'reverselinedirection'

    def displayName(self):
        return self.tr('Reverse line direction')

    def outputName(self):
        return self.tr('Reversed')

    def outputType(self):
        return QgsProcessing.TypeVectorLine

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorLine]

    def processFeature(self, feature, feedback):
        if feature.geometry():
            inGeom = feature.geometry()
            reversedLine = inGeom.constGet().reversed()
            if not reversedLine:
                raise QgsProcessingException(
                    self.tr('Error reversing line'))
            outGeom = QgsGeometry(reversedLine)

            feature.setGeometry(outGeom)
        return feature
