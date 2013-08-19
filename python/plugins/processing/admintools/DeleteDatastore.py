# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteDatastore.py
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

class DeleteDatastore(GeoServerToolsAlgorithm):

    DATASTORE = "DATASTORE"
    WORKSPACE = "WORKSPACE"

    def processAlgorithm(self, progress):
        self.createCatalog()
        datastoreName = self.getParameterValue(self.DATASTORE)
        workspaceName = self.getParameterValue(self.WORKSPACE)
        ds = self.catalog.get_store(datastoreName, workspaceName)
        self.catalog.delete(ds, recurse=True)


    def defineCharacteristics(self):
        self.addBaseParameters()
        self.name = "Delete datastore"
        self.group = "GeoServer management tools"
        self.addParameter(ParameterString(self.DATASTORE, "Datastore name"))
        self.addParameter(ParameterString(self.WORKSPACE, "Workspace"))



