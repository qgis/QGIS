#===============================================================================
# # -*- coding: utf-8 -*-
#
# """
# ***************************************************************************
#    CreateMosaicDatastore.py
#    ---------------------
#    Date                 : October 2012
#    Copyright            : (C) 2012 by Victor Olaya
#    Email                : volayaf at gmail dot com
# ***************************************************************************
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# ***************************************************************************
# """
# from processing.tools import dataobjects
# from processing.parameters.ParameterString import ParameterString
# from processing.servertools.GeoServerToolsAlgorithm import GeoServerToolsAlgorithm
#
# __author__ = 'Victor Olaya'
# __date__ = 'October 2012'
# __copyright__ = '(C) 2012, Victor Olaya'
# # This will get replaced with a git SHA1 when you do a git archive
# __revision__ = '$Format:%H$'
#
# from qgis.core import *
# from processing.parameters.ParameterVector import ParameterVector
# from processing.tools import dataobjects
# import os
#
# class CreateMosaicDatastore(GeoServerToolsAlgorithm):
#
#    INPUT = "INPUT"
#    WORKSPACE = "WORKSPACE"
#
#    def processAlgorithm(self, progress):
#        self.createCatalog()
#        input = self.getParameterValue(self.INPUT)
#        workspaceName = self.getParameterValue(self.WORKSPACE)
#        connection = {
#            'shp': basepathname + '.shp',
#            'shx': basepathname + '.shx',
#            'dbf': basepathname + '.dbf',
#            'prj': basepathname + '.prj'
#        }
#
#        workspace = self.catalog.get_workspace(workspaceName)
#        self.catalog.create_featurestore(basefilename, connection, workspace)
#
#
#    def defineCharacteristics(self):
#        self.addcaddBaseParameters()
#        self.name = "Import into GeoServer"
#        self.group = "GeoServer management tools"
#        self.addParameter(ParameterVector(self.INPUT, "Layer to import", [ParameterVector.VECTOR_TYPE_ANY]))
#        self.addParameter(ParameterString(self.WORKSPACE, "Workspace"))
#===============================================================================
