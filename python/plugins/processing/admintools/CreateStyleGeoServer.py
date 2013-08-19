# -*- coding: utf-8 -*-

"""
***************************************************************************
    CreateStyleGeoServer.py
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
from processing.parameters.ParameterFile import ParameterFile
from processing.parameters.ParameterBoolean import ParameterBoolean


class CreateStyleGeoServer(GeoServerToolsAlgorithm):

    STYLE = "STYLE"
    OVERWRITE = "OVERWRITE"
    NAME = "NAME"

    def processAlgorithm(self, progress):
        self.createCatalog()
        stylefile = self.getParameterValue(self.STYLE)
        overwrite = self.getParameterValue(self.OVERWRITE)
        name = self.getParameterValue(self.NAME)
        self.catalog.create_style(name, stylefile, overwrite)


    def defineCharacteristics(self):
        self.addBaseParameters()
        self.name = "Add style"
        self.group = "GeoServer management tools"
        self.addParameter(ParameterString(self.NAME, "Style name"))
        self.addParameter(ParameterFile(self.STYLE, "Style SLD file"))
        self.addParameter(ParameterBoolean(self.OVERWRITE, "Overwrite"))



