# -*- coding: utf-8 -*-

"""
***************************************************************************
    SplitRGBBands.py
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
from PyQt4 import QtGui
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.outputs import OutputRaster
from processing.tools.system import getTempFilename, isWindows
import SagaUtils

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class SplitRGBBands(GeoAlgorithm):

    INPUT = 'INPUT'
    R = 'R'
    G = 'G'
    B = 'B'

    def getIcon(self):
        return QtGui.QIcon(os.path.join(pluginPath, 'images', 'saga.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Split RGB bands')
        self.group, self.i18n_group = self.trAlgorithm('Grid - Tools')
        self.addParameter(ParameterRaster(SplitRGBBands.INPUT,
            self.tr('Input layer'), False))
        self.addOutput(OutputRaster(SplitRGBBands.R,
            self.tr('Output R band layer')))
        self.addOutput(OutputRaster(SplitRGBBands.G,
            self.tr('Output G band layer')))
        self.addOutput(OutputRaster(SplitRGBBands.B,
            self.tr('Output B band layer')))

    def processAlgorithm(self, progress):
        # TODO: check correct num of bands
        input = self.getParameterValue(SplitRGBBands.INPUT)
        temp = getTempFilename(None).replace('.', '')
        basename = os.path.basename(temp)
        validChars = \
            'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
        safeBasename = ''.join(c for c in basename if c in validChars)
        temp = os.path.join(os.path.dirname(temp), safeBasename)

        r = self.getOutputValue(SplitRGBBands.R)
        g = self.getOutputValue(SplitRGBBands.G)
        b = self.getOutputValue(SplitRGBBands.B)
        commands = []
        version = SagaUtils.getSagaInstalledVersion(True)
        trailing = ""
        lib = ""
        commands.append('%sio_gdal 0 -GRIDS "%s" -FILES "%s"' % (lib, temp, input)
                        )
        commands.append('%sio_gdal 1 -GRIDS "%s_%s1.sgrd" -FORMAT 1 -TYPE 0 -FILE "%s"' %(lib, temp, trailing, r)
                        )
        commands.append('%sio_gdal 1 -GRIDS "%s_%s2.sgrd" -FORMAT 1 -TYPE 0 -FILE "%s"' %(lib, temp, trailing, g)
                        )
        commands.append('%sio_gdal 1 -GRIDS "%s_%s3.sgrd" -FORMAT 1 -TYPE 0 -FILE "%s"' %(lib, temp, trailing, b)
                        )


        SagaUtils.createSagaBatchJobFileFromSagaCommands(commands)
        SagaUtils.executeSaga(progress)
