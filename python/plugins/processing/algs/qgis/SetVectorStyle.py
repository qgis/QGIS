# -*- coding: utf-8 -*-

"""
***************************************************************************
    SetVectorStyle.py
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

from qgis.core import (QgsProcessingAlgorithm,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingOutputVectorLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class SetVectorStyle(QgisAlgorithm):

    INPUT = 'INPUT'
    STYLE = 'STYLE'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def __init__(self):
        super().__init__()

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagNoThreading

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT,
                                                            self.tr('Vector layer')))
        self.addParameter(QgsProcessingParameterFile(self.STYLE,
                                                     self.tr('Style file'), extension='qml'))
        self.addOutput(QgsProcessingOutputVectorLayer(self.INPUT,
                                                      self.tr('Styled')))

    def name(self):
        return 'setstyleforvectorlayer'

    def displayName(self):
        return self.tr('Set style for vector layer')

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        style = self.parameterAsFile(parameters, self.STYLE, context)
        layer.loadNamedStyle(style)
        layer.triggerRepaint()
        return {self.INPUT: layer}
