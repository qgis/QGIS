# -*- coding: utf-8 -*-

"""
***************************************************************************
    SpatialIndex.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Alexander Bruy
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
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import QgsVectorDataProvider

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector

from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SpatialIndex(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    #def getIcon(self):
    #    return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'basic_statistics.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create spatial index')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer'),
                                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(self.OUTPUT,
                                    self.tr('Indexed layer'), True))

    def processAlgorithm(self, progress):
        fileName = self.getParameterValue(self.INPUT)
        layer = dataobjects.getObjectFromUri(fileName)
        provider = layer.dataProvider()

        if provider.capabilities() & QgsVectorDataProvider.CreateSpatialIndex:
            if not provider.createSpatialIndex():
                progress.setInfo(self.tr('Can not create spatial index'))
        else:
            progress.setInfo(self.tr("Layer's data provider does not support "
                                     "spatial indexes"))

        self.setOutputValue(self.OUTPUT, fileName)
