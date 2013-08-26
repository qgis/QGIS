# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithmProvider.py
    ---------------------
    Date                 : August 2012
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
from processing.gdal.gdaladdo import gdaladdo
from processing.gdal.ogr2ogr import Ogr2Ogr
from processing.gdal.ogrinfo import OgrInfo
from processing.gdal.ogrsql import OgrSql

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.script.WrongScriptException import WrongScriptException
from processing.core.ProcessingLog import ProcessingLog
from processing.gdal.GdalAlgorithm import GdalAlgorithm
from processing.gdal.nearblack import nearblack
from processing.gdal.information import information
from processing.gdal.GdalUtils import GdalUtils
from processing.gdal.warp import warp
from processing.gdal.rgb2pct import rgb2pct
from processing.gdal.translate import translate
from processing.gdal.pct2rgb import pct2rgb
from processing.gdal.merge import merge
from processing.gdal.polygonize import polygonize

class GdalOgrAlgorithmProvider(AlgorithmProvider):

    '''This provider incorporates GDAL-based algorithms into the processing framework.
    Algorithms have been implemented using two different mechanisms,
    which should serve as an example of different ways of extending
    the processing capabilities of QGIS:
    1)when a python script exist for a given process, it has been adapted
    as a processing python script and loaded using the ScriptAlgorithm class.
    This algorithms call GDAL using its Python bindings
    2)Other algorithms are called directly using the command line interface,
    These have been implemented individually extending the GeoAlgorithm class'''

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.createAlgsList()

    def scriptsFolder(self):
        '''The folder where script algorithms are stored'''
        return os.path.dirname(__file__) + "/scripts"

    def getDescription(self):
        return "GDAL/OGR"

    def getName(self):
        return "gdalogr"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/icons/gdalicon.png")

    def _loadAlgorithms(self):
        '''This is called each time there is a change in the set of algorithms,
        for instance, when the users adds a new model or script. Since this provider
        cannot be extended by the user, we create the list in advance and then just
        assign it to self.algs'''
        self.algs = self.preloadedAlgs

    def createAlgsList(self):
        #First we populate the list of algorithms with those created extending
        #GeoAlgorithm directly (those that execute GDAL using the console)

        self.preloadedAlgs = [nearblack(), information(), warp(), translate(),
                              rgb2pct(), pct2rgb(), merge(), polygonize(), gdaladdo(),
                              OgrInfo(), Ogr2Ogr(), OgrSql()]

        #And then we add those that are created as python scripts
        folder = self.scriptsFolder()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("py"):
                try:
                    fullpath = os.path.join(self.scriptsFolder(), descriptionFile)
                    alg = GdalAlgorithm(fullpath)
                    self.preloadedAlgs.append(alg)
                except WrongScriptException,e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,e.msg)

    def getSupportedOutputRasterLayerExtensions(self):
        return GdalUtils.getSupportedRasterExtensions()

    #===========================================================================
    # def getSupportedOutputVectorLayerExtensions(self):
    #    return ["shp", "sqlite"]
    #===========================================================================
