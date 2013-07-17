# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaAlgorithmProvider.py
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.saga.SagaAlgorithm import SagaAlgorithm
from sextante.saga.SplitRGBBands import SplitRGBBands
from sextante.saga.SagaUtils import SagaUtils
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteUtils import SextanteUtils

class SagaAlgorithmProvider(AlgorithmProvider):


    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = True
        self.createAlgsList() #preloading algorithms to speed up

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        if SextanteUtils.isWindows():
            SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_FOLDER, "SAGA folder", SagaUtils.sagaPath()))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_AUTO_RESAMPLING, "Use min covering grid system for resampling", True))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_LOG_COMMANDS, "Log execution commands", False))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_LOG_CONSOLE, "Log console output", False))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_XMIN, "Resampling region min x", 0))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_YMIN, "Resampling region min y", 0))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_XMAX, "Resampling region max x", 1000))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_YMAX, "Resampling region max y", 1000))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_CELLSIZE, "Resampling region cellsize", 1))

    def unload(self):
        AlgorithmProvider.unload(self)
        if SextanteUtils.isWindows():
            SextanteConfig.removeSetting(SagaUtils.SAGA_FOLDER)
        SextanteConfig.removeSetting(SagaUtils.SAGA_AUTO_RESAMPLING)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_XMIN)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_YMIN)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_XMAX)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_YMAX)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_CELLSIZE)
        SextanteConfig.removeSetting(SagaUtils.SAGA_LOG_CONSOLE)
        SextanteConfig.removeSetting(SagaUtils.SAGA_LOG_COMMANDS)

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("txt"):
                if SextanteUtils.isWindows() or SextanteUtils.isMac():
                    if descriptionFile.startswith("2.0.8"):
                        continue
                else:
                    if descriptionFile.startswith("2.1"):
                        continue
                try:
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self.preloadedAlgs.append(alg)
                        print alg.name
                    else:                        
                        SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open SAGA algorithm: " + descriptionFile)
                except Exception,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open SAGA algorithm: " + descriptionFile +"\n" + str(e))
        self.preloadedAlgs.append(SplitRGBBands())

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def getDescription(self):
        return "SAGA"

    def getName(self):
        return "saga"

    def getPostProcessingErrorMessage(self, wrongLayers):
        html = AlgorithmProvider.getPostProcessingErrorMessage(self, wrongLayers)
        msg = SagaUtils.checkSagaIsInstalled(True)
        html += ("<p>This algorithm requires SAGA to be run. A test to check if SAGA is correctly installed "
                "and configured in your system has been performed, with the following result:</p><ul><i>")
        if msg is None:
            html += "SAGA seems to be correctly installed and configured</li></ul>"
        else:
            html += msg + "</i></li></ul>"
            html += '<p><a href= "http://docs.qgis.org/2.0/html/en/docs/user_manual/sextante/3rdParty.html">Click here</a> to know more about how to install and configure SAGA to be used with SEXTANTE</p>'

        return html

    def getSupportedOutputVectorLayerExtensions(self):
        return ["shp"]

    def getSupportedOutputRasterLayerExtensions(self):
        return ["tif"]

    def getSupportedOutputTableLayerExtensions(self):
        return ["dbf"]

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/saga.png")

