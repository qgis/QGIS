# -*- coding: utf-8 -*-

"""
***************************************************************************
    AdminToolsAlgorithmProvider.py
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

import os
from PyQt4 import QtGui

from processing.core.AlgorithmProvider import AlgorithmProvider
from PostGISExecuteSQL import PostGISExecuteSQL
from ImportIntoPostGIS import ImportIntoPostGIS
from ImportVectorIntoGeoServer import ImportVectorIntoGeoServer
from CreateWorkspace import CreateWorkspace
from ImportRasterIntoGeoServer import ImportRasterIntoGeoServer
from DeleteWorkspace import DeleteWorkspace
from DeleteDatastore import DeleteDatastore
from CreateStyleGeoServer import CreateStyleGeoServer


class AdminToolsAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [
            ImportVectorIntoGeoServer(),
            ImportRasterIntoGeoServer(),
            CreateWorkspace(),
            DeleteWorkspace(),
            DeleteDatastore(),
            CreateStyleGeoServer(),
            ]

        try:
            self.alglist.append(ImportIntoPostGIS())
            self.alglist.append(PostGISExecuteSQL())
        except:
            pass

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)

    def unload(self):
        AlgorithmProvider.unload(self)

    def getName(self):
        return 'gspg'

    def getDescription(self):
        return 'GeoServer/PostGIS tools'

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__)
                           + '/../../images/database.png')

    def _loadAlgorithms(self):
        self.algs = self.alglist

    def supportsNonFileBasedOutput(self):
        return False
