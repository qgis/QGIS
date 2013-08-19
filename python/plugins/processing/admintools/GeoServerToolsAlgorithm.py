# -*- coding: utf-8 -*-

"""
***************************************************************************
    GeoserverToolsAlgorithm.py
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
from processing.parameters.ParameterString import ParameterString
from processing.admintools.geoserver.catalog import Catalog

__author__ = 'Victor Olaya'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4 import QtGui
from processing.core.GeoAlgorithm import GeoAlgorithm

class GeoServerToolsAlgorithm(GeoAlgorithm):

    URL = "URL"
    USER = "USER"
    PASSWORD = "PASSWORD"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/geoserver.png")

    def addBaseParameters(self):
        self.addParameter(ParameterString(self.URL, "URL", "http://localhost:8080/geoserver/rest"))
        self.addParameter(ParameterString(self.USER, "User", "admin"))
        self.addParameter(ParameterString(self.PASSWORD, "Password", "geoserver"))

    def createCatalog(self):
        url = self.getParameterValue(self.URL)
        user = self.getParameterValue(self.USER)
        password = self.getParameterValue(self.PASSWORD)
        self.catalog = Catalog(url, user, password)


