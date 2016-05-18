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
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.core.parameters import ParameterFile
from processing.tools import dataobjects
from qgis.utils import iface


class SetVectorStyle(GeoAlgorithm):

    INPUT = 'INPUT'
    STYLE = 'STYLE'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        # self.allowOnlyOpenedLayers = True
        self.name, self.i18n_name = self.trAlgorithm('Set style for vector layer')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Vector layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterFile(self.STYLE,
                                        self.tr('Style file'), False, False, 'qml'))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Styled'), True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)

        style = self.getParameterValue(self.STYLE)
        layer = dataobjects.getObjectFromUri(filename, False)
        if layer is None:
            dataobjects.load(filename, os.path.basename(filename), style=style)
            self.getOutputFromName(self.OUTPUT).open = False
        else:
            layer.loadNamedStyle(style)
            iface.mapCanvas().refresh()
            iface.legendInterface().refreshLayerSymbology(layer)
            layer.triggerRepaint()
