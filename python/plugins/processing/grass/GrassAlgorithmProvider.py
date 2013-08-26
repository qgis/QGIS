# -*- coding: utf-8 -*-

"""
***************************************************************************
    GrassAlgorithmProvider.py
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
from processing.grass.nviz import nviz

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingLog import ProcessingLog
from processing.grass.GrassUtils import GrassUtils
from processing.grass.GrassAlgorithm import GrassAlgorithm
from processing.core.ProcessingUtils import ProcessingUtils

class GrassAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.createAlgsList() #preloading algorithms to speed up

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        if ProcessingUtils.isWindows() or ProcessingUtils.isMac():
            ProcessingConfig.addSetting(Setting(self.getDescription(), GrassUtils.GRASS_FOLDER, "GRASS folder", GrassUtils.grassPath()))
            ProcessingConfig.addSetting(Setting(self.getDescription(), GrassUtils.GRASS_WIN_SHELL, "Msys folder", GrassUtils.grassWinShell()))
        ProcessingConfig.addSetting(Setting(self.getDescription(), GrassUtils.GRASS_LOG_COMMANDS, "Log execution commands", False))
        ProcessingConfig.addSetting(Setting(self.getDescription(), GrassUtils.GRASS_LOG_CONSOLE, "Log console output", False))

    def unload(self):
        AlgorithmProvider.unload(self)
        if ProcessingUtils.isWindows() or ProcessingUtils.isMac():
            ProcessingConfig.removeSetting(GrassUtils.GRASS_FOLDER)
            ProcessingConfig.removeSetting(GrassUtils.GRASS_WIN_SHELL)
        ProcessingConfig.removeSetting(GrassUtils.GRASS_LOG_COMMANDS)
        ProcessingConfig.removeSetting(GrassUtils.GRASS_LOG_CONSOLE)

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = GrassUtils.grassDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("txt"):
                try:
                    alg = GrassAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self.preloadedAlgs.append(alg)
                    else:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, "Could not open GRASS algorithm: " + descriptionFile)
                except Exception,e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, "Could not open GRASS algorithm: " + descriptionFile)
        self.preloadedAlgs.append(nviz())

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def getDescription(self):
        return "GRASS commands"

    def getName(self):
        return "grass"

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/grass.png")

    def getSupportedOutputVectorLayerExtensions(self):
        return ["shp"]

    def getSupportedOutputRasterLayerExtensions(self):
        return ["tif"]


