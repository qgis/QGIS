# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBUtils.py
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
from qgis.core import QgsApplication
import subprocess
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import *

class OTBUtils:

    OTB_FOLDER = "OTB_FOLDER"
    OTB_LIB_FOLDER = "OTB_LIB_FOLDER"
    OTB_SRTM_FOLDER = "OTB_SRTM_FOLDER"
    OTB_GEOID_FILE = "OTB_GEOID_FILE"

    @staticmethod
    def otbPath():
        folder = ProcessingConfig.getSetting(OTBUtils.OTB_FOLDER)
        if folder == None:
            folder = ""
            #try to configure the path automatically
            if isMac():
                testfolder = os.path.join(str(QgsApplication.prefixPath()), "bin")
                if os.path.exists(os.path.join(testfolder, "otbcli")):
                    folder = testfolder
                else:
                    testfolder = "/usr/local/bin"
                    if os.path.exists(os.path.join(testfolder, "otbcli")):
                        folder = testfolder
            elif isWindows():
                testfolder = os.path.dirname(str(QgsApplication.prefixPath()))
                testfolder = os.path.dirname(testfolder)
                testfolder = os.path.join(testfolder,  "bin")
                path = os.path.join(testfolder, "otbcli.bat")
                if os.path.exists(path):
                    folder = testfolder
            else:
                testfolder = "/usr/bin"
                if os.path.exists(os.path.join(testfolder, "otbcli")):
                    folder = testfolder
        return folder

    @staticmethod
    def otbLibPath():
        folder = ProcessingConfig.getSetting(OTBUtils.OTB_LIB_FOLDER)
        if folder == None:
            folder =""
            #try to configure the path automatically
            if isMac():
                testfolder = os.path.join(str(QgsApplication.prefixPath()), "lib/otb/applications")
                if os.path.exists(testfolder):
                    folder = testfolder
                else:
                    testfolder = "/usr/local/lib/otb/applications"
                    if os.path.exists(testfolder):
                        folder = testfolder
            elif isWindows():
                testfolder = os.path.dirname(str(QgsApplication.prefixPath()))
                testfolder = os.path.join(testfolder,  "orfeotoolbox")
                testfolder = os.path.join(testfolder,  "applications")
                if os.path.exists(testfolder):
                    folder = testfolder
            else:
                testfolder = "/usr/lib/otb/applications"
                if os.path.exists(testfolder):
                    folder = testfolder
        return folder

    @staticmethod
    def otbSRTMPath():
        folder = ProcessingConfig.getSetting(OTBUtils.OTB_SRTM_FOLDER)
        if folder == None:
            folder =""
        return folder

    @staticmethod
    def otbGeoidPath():
        filepath = ProcessingConfig.getSetting(OTBUtils.OTB_GEOID_FILE)
        if filepath == None:
            filepath =""
        return filepath

    @staticmethod
    def otbDescriptionPath():
        return os.path.join(os.path.dirname(__file__), "description")

    @staticmethod
    def executeOtb(commands, progress):
        loglines = []
        loglines.append("OTB execution console output")
        os.putenv('ITK_AUTOLOAD_PATH', OTBUtils.otbLibPath())
        fused_command = ''.join(['"%s" ' % c for c in commands])
        proc = subprocess.Popen(fused_command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=True).stdout
        for line in iter(proc.readline, ""):
            if "[*" in line:
                idx = line.find("[*")
                perc = int(line[idx-4:idx-2].strip(" "))
                if perc !=0:
                    progress.setPercentage(perc)
            else:
                loglines.append(line)
                progress.setConsoleInfo(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)



