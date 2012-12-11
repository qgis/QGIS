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
import subprocess
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteUtils import SextanteUtils

class OTBUtils:

    OTB_FOLDER = "OTB_FOLDER"
    OTB_LIB_FOLDER = "OTB_LIB_FOLDER"
    OTB_SRTM_FOLDER = "OTB_SRTM_FOLDER"
    OTB_GEOID_FILE = "OTB_GEOID_FILE"

    @staticmethod
    def otbPath():
        folder = SextanteConfig.getSetting(OTBUtils.OTB_FOLDER)
        if folder == None:
            folder = ""

            if os.path.exists("/usr/bin/otbcli"):
                folder = "/usr/bin"
        return folder

    @staticmethod
    def otbLibPath():
        folder = SextanteConfig.getSetting(OTBUtils.OTB_LIB_FOLDER)
        if folder == None:
            folder =""

            linuxstandardpath = "/usr/lib/otb/applications"
            if os.path.exists(linuxstandardpath):
                folder = linuxstandardpath
        return folder

    @staticmethod
    def otbSRTMPath():
        folder = SextanteConfig.getSetting(OTBUtils.OTB_SRTM_FOLDER)
        if folder == None:
            folder =""
        return folder

    @staticmethod
    def otbGeoidPath():
        filepath = SextanteConfig.getSetting(OTBUtils.OTB_GEOID_FILE)
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
        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)



