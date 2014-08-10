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
from PyQt4.QtCore import *
from PyQt4.QtXml import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterRaster
from processing.core.outputs import OutputRaster
from processing.tools import dataobjects
from qgis.utils import iface

class SetRasterStyle(GeoAlgorithm):

    INPUT = 'INPUT'
    STYLE = 'STYLE'
    OUTPUT = 'OUTPUT'


    def defineCharacteristics(self):
        self.name = 'Set style for raster layer'
        self.group = 'Raster general tools'
        self.addParameter(ParameterRaster(self.INPUT, 'Raster layer'))
        self.addParameter(ParameterFile(self.STYLE,
                          'Style file', False, False, 'qml'))
        self.addOutput(OutputRaster(self.OUTPUT, 'Styled layer', True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)
        layer = dataobjects.getObjectFromUri(filename)

        style = self.getParameterValue(self.STYLE)
        if layer is None:
            dataobjects.load(filename, os.path.basename(filename), style=style)
            self.getOutputFromName(self.OUTPUT).open = False
        else:
            with open(style) as f:
                xml = "".join(f.readlines())
            d = QDomDocument()
            d.setContent(xml)
            n = d.firstChild()
            layer.readSymbology(n, '')
            self.setOutputValue(self.OUTPUT, filename)
            iface.mapCanvas().refresh()
            iface.legendInterface().refreshLayerSymbology(layer)
