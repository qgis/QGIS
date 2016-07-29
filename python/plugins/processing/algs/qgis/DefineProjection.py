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
import re

from qgis.core import QgsCoordinateReferenceSystem
from qgis.utils import iface

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterCrs
from processing.core.outputs import OutputVector

from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class DefineProjection(GeoAlgorithm):

    INPUT = 'INPUT'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    #def getIcon(self):
    #    return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'basic_statistics.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Define current projection')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer'),
                                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterCrs(self.CRS, 'Output CRS'))
        self.addOutput(OutputVector(self.OUTPUT,
                                    self.tr('Layer with projection'), True))

    def processAlgorithm(self, progress):
        fileName = self.getParameterValue(self.INPUT)
        layer = dataobjects.getObjectFromUri(fileName)
        crs = QgsCoordinateReferenceSystem(self.getParameterValue(self.CRS))

        provider = layer.dataProvider()
        ds = provider.dataSourceUri()
        p = re.compile('\|.*')
        dsPath = p.sub('', ds)

        if dsPath.lower().endswith('.shp'):
            dsPath = dsPath[:-4]

        wkt = crs.toWkt()
        with open(dsPath + '.prj', 'w') as f:
            f.write(wkt)

        qpjFile = dsPath + '.qpj'
        if os.path.exists(qpjFile):
            with open(qpjFile, 'w') as f:
                f.write(wkt)

        layer.setCrs(crs)
        iface.mapCanvas().refresh()

        self.setOutputValue(self.OUTPUT, fileName)
