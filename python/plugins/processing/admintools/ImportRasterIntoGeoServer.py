# -*- coding: utf-8 -*-

"""
***************************************************************************
    ImportVectorIntoGeoServer.py
    ---------------------
    Date                 : October 2012
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
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import *
from processing.parameters.ParameterString import ParameterString
from processing.admintools.GeoServerToolsAlgorithm import GeoServerToolsAlgorithm
from processing.parameters.ParameterRaster import ParameterRaster


class ImportRasterIntoGeoServer(GeoServerToolsAlgorithm):

    INPUT = "INPUT"
    WORKSPACE = "WORKSPACE"
    NAME = "NAME"

    def exportRasterLayer(self, inputFilename):
        return inputFilename

    def processAlgorithm(self, progress):
        self.createCatalog()
        inputFilename = self.getParameterValue(self.INPUT)
        name = self.getParameterValue(self.NAME)
        workspaceName = self.getParameterValue(self.WORKSPACE)
        filename = self.exportRasterLayer(inputFilename)
        workspace = self.catalog.get_workspace(workspaceName)
        ds = self.catalog.create_coveragestore2(name, workspace)
        ds.data_url = "file:" + filename;
        self.catalog.save(ds)


    def defineCharacteristics(self):
        self.addBaseParameters()
        self.name = "Import raster into GeoServer"
        self.group = "GeoServer management tools"
        self.addParameter(ParameterRaster(self.INPUT, "Layer to import"))
        self.addParameter(ParameterString(self.WORKSPACE, "Workspace"))
        self.addParameter(ParameterString(self.NAME, "Store name"))

