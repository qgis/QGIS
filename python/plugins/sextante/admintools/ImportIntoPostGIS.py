# -*- coding: utf-8 -*-

"""
***************************************************************************
    ImportIntoPostGIS.py
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
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.GeoAlgorithm import GeoAlgorithm

__author__ = 'Victor Olaya'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.parameters.ParameterString import ParameterString
from sextante.admintools import postgis_utils
import PyQt4

class ImportIntoPostGIS(GeoAlgorithm):

    DATABASE = "DATABASE"
    TABLENAME = "TABLENAME"
    INPUT = "INPUT"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/postgis.png")

    def processAlgorithm(self, progress):
        pass

    def defineCharacteristics(self):
        self.name = "Import into PostGIS"
        self.group = "PostGIS management tools"
        self.addParameter(ParameterVector(self.INPUT, "Layer to import"))
        self.addParameter(ParameterString(self.DATABASE, "Database"))
        self.addParameter(ParameterString(self.TABLENAME, "Name for new table"))




