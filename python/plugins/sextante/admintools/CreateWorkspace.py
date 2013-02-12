# -*- coding: utf-8 -*-

"""
***************************************************************************
    CreateWorkspace.py
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
from sextante.parameters.ParameterString import ParameterString
from sextante.admintools.GeoServerToolsAlgorithm import GeoServerToolsAlgorithm
from sextante.outputs.OutputString import OutputString

class CreateWorkspace(GeoServerToolsAlgorithm):

    WORKSPACE = "WORKSPACE"
    WORKSPACEURI = "WORKSPACEURI"

    def processAlgorithm(self, progress):
        self.createCatalog()
        workspaceName = self.getParameterValue(self.WORKSPACE)
        workspaceUri = self.getParameterValue(self.WORKSPACEURI)
        self.catalog.create_workspace(workspaceName, workspaceUri)


    def defineCharacteristics(self):
        self.addBaseParameters()
        self.name = "Create workspace"
        self.group = "GeoServer management tools"
        self.addParameter(ParameterString(self.WORKSPACE, "Workspace"))
        self.addParameter(ParameterString(self.WORKSPACEURI, "Workspace URI"))
        self.addOutput(OutputString(self.WORKSPACE, "Workspace"))


