# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaAlgorithm230.py
    ---------------------
    Date                 : March 2017
    Copyright            : (C) 2017 by Victor Olaya
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
__date__ = 'March 2017'
__copyright__ = '(C) 2017, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from SagaAlgorithm214 import SagaAlgorithm214
from processing.tools import dataobjects
from processing.tools.system import getTempFilenameInTempFolder

sessionExportedLayers = {}


class SagaAlgorithm230(SagaAlgorithm214):

    def getCopy(self):
        newone = SagaAlgorithm230(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def exportRasterLayer(self, source):
        global sessionExportedLayers
        if source in sessionExportedLayers:
            exportedLayer = sessionExportedLayers[source]
            if os.path.exists(exportedLayer):
                self.exportedLayers[source] = exportedLayer
                return None
            else:
                del sessionExportedLayers[source]
        layer = dataobjects.getObjectFromUri(source, False)
        if layer:
            filename = layer.name()
        else:
            filename = os.path.basename(source)
        validChars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
        filename = ''.join(c for c in filename if c in validChars)
        if len(filename) == 0:
            filename = 'layer'
        destFilename = getTempFilenameInTempFolder(filename + '.sgrd')
        self.exportedLayers[source] = destFilename
        sessionExportedLayers[source] = destFilename
        return 'io_gdal 0 -TRANSFORM 1 -RESAMPLING 0 -GRIDS "' + destFilename + '" -FILES "' + source + '"'
