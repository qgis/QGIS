# -*- coding: utf-8 -*-

"""
***************************************************************************
    SetRasterStyle.py
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

from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterFile,
                       QgsProcessingOutputRasterLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class SetRasterStyle(QgisAlgorithm):

    INPUT = 'INPUT'
    STYLE = 'STYLE'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Raster tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Raster layer')))
        self.addParameter(QgsProcessingParameterFile(self.STYLE,
                                                     self.tr('Style file'), extension='qml'))
        self.addOutput(QgsProcessingOutputRasterLayer(self.INPUT, self.tr('Styled')))

    def name(self):
        return 'setstyleforrasterlayer'

    def displayName(self):
        return self.tr('Set style for raster layer')

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        style = self.parameterAsFile(parameters, self.STYLE, context)
        with open(style) as f:
            xml = "".join(f.readlines())
        d = QDomDocument()
        d.setContent(xml)
        layer.importNamedStyle(d)
        layer.triggerRepaint()
        return {self.INPUT: layer}
