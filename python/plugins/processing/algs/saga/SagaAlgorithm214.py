# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaAlgorithm213.py
    ---------------------
    Date                 : December 2014
    Copyright            : (C) 2014 by Victor Olaya
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
__date__ = 'December 2014'
__copyright__ = '(C) 2014, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from SagaAlgorithm213 import SagaAlgorithm213
from processing.tools import dataobjects
from processing.tools.system import getTempFilenameInTempFolder

sessionExportedLayers = {}

class SagaAlgorithm214(SagaAlgorithm213):


    def getCopy(self):
        newone = SagaAlgorithm214(self.descriptionFile)
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
        return 'io_gdal -TRANSFORM -INTERPOL 0 -GRIDS "' + destFilename + '" -FILES "' + source +  '"'
