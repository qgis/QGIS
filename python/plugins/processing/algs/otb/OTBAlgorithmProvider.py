# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBAlgorithmProvider.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr
    Contributors         : Victor Olaya
                           Julien Malik  - Changing the way to load algorithms : loading from xml
                           Oscar Picas   - Changing the way to load algorithms : loading from xml
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
import PyQt4.QtGui
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from OTBUtils import OTBUtils
from OTBAlgorithm import OTBAlgorithm
from processing.core.ProcessingLog import ProcessingLog

class OTBAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = True
        self.createAlgsList()


    def getDescription(self):
        return "Orfeo Toolbox (Image analysis)"

    def getName(self):
        return "otb"

    def getIcon(self):
        return PyQt4.QtGui.QIcon(os.path.dirname(__file__) + "/../../images/otb.png")

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = OTBUtils.otbDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("xml"):
                try:
                    alg = OTBAlgorithm(os.path.join(folder, descriptionFile))

                    if alg.name.strip() != "":
                        self.preloadedAlgs.append(alg)
                    else:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, "Could not open OTB algorithm: " + descriptionFile)
                except Exception,e:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, "Could not open OTB algorithm: " + descriptionFile)


    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        if OTBUtils.findOtbPath() is None:
            ProcessingConfig.addSetting(Setting(self.getDescription(), OTBUtils.OTB_FOLDER,
                                                "OTB command line tools folder", OTBUtils.otbPath()))
        if OTBUtils.findOtbLibPath() is None:
            ProcessingConfig.addSetting(Setting(self.getDescription(), OTBUtils.OTB_LIB_FOLDER,
                                                "OTB applications folder", OTBUtils.otbLibPath()))
        ProcessingConfig.addSetting(Setting(self.getDescription(), OTBUtils.OTB_SRTM_FOLDER, "SRTM tiles folder", OTBUtils.otbSRTMPath()))
        ProcessingConfig.addSetting(Setting(self.getDescription(), OTBUtils.OTB_GEOID_FILE, "Geoid file", OTBUtils.otbGeoidPath()))

    def unload(self):
        AlgorithmProvider.unload(self)
        ProcessingConfig.removeSetting(OTBUtils.OTB_FOLDER)
        ProcessingConfig.removeSetting(OTBUtils.OTB_LIB_FOLDER)
