# -*- coding: utf-8 -*-

"""
***************************************************************************
    LAStoolsUtils.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Martin Isenburg
    Email                : martin near rapidlasso point com
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

from PyQt4.QtCore import QCoreApplication

from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig

class LAStoolsUtils:

    LASTOOLS_FOLDER = "LASTOOLS_FOLDER"
    WINE_FOLDER = "WINE_FOLDER"

    @staticmethod
    def hasWine():
        wine_folder = ProcessingConfig.getSetting(LAStoolsUtils.WINE_FOLDER)
        return wine_folder is not None and wine_folder != ""

    @staticmethod
    def LAStoolsPath():
        lastools_folder = ProcessingConfig.getSetting(LAStoolsUtils.LASTOOLS_FOLDER)
        if lastools_folder is None:
            lastools_folder = ""
        wine_folder = ProcessingConfig.getSetting(LAStoolsUtils.WINE_FOLDER)
        if wine_folder is None or wine_folder == "":
            folder = lastools_folder
        else:
            folder = wine_folder + "/wine " + lastools_folder
        return folder

    @staticmethod
    def runLAStools(commands, progress):
        loglines = []
        commandline = " ".join(commands)
        loglines.append(QCoreApplication.translate("LAStoolsUtils", "LAStools command line"))
        loglines.append(commandline)
        loglines.append(QCoreApplication.translate("LAStoolsUtils", "LAStools console output"))
        proc = subprocess.Popen(commandline, shell=True, stdout=subprocess.PIPE, stdin=open(os.devnull),
                                stderr=subprocess.STDOUT, universal_newlines=False).stdout
        for line in iter(proc.readline, ""):
            loglines.append(line)
            progress.setConsoleInfo(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
