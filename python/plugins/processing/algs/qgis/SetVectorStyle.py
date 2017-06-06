# -*- coding: utf-8 -*-

"""
***************************************************************************
    SelectByLocation.py
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
from qgis.core import (QgsApplication,
                       QgsProcessingUtils)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.core.parameters import ParameterFile
from processing.tools import dataobjects
from qgis.utils import iface


class SetVectorStyle(QgisAlgorithm):

    INPUT = 'INPUT'
    STYLE = 'STYLE'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector general tools')

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Vector layer')))
        self.addParameter(ParameterFile(self.STYLE,
                                        self.tr('Style file'), False, False, 'qml'))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Styled'), True))

    def name(self):
        return 'setstyleforvectorlayer'

    def displayName(self):
        return self.tr('Set style for vector layer')

    def processAlgorithm(self, parameters, context, feedback):
        filename = self.getParameterValue(self.INPUT)

        style = self.getParameterValue(self.STYLE)
        layer = QgsProcessingUtils.mapLayerFromString(filename, context, False)
        if layer is None:
            dataobjects.load(filename, os.path.basename(filename), style=style)
        else:
            layer.loadNamedStyle(style)
            context.addLayerToLoadOnCompletion(layer.id())
            layer.triggerRepaint()
