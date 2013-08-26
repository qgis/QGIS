# -*- coding: utf-8 -*-

"""
***************************************************************************
    RUtils.py
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
import re

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from processing.core.ProcessingConfig import ProcessingConfig
import os
from processing.core.ProcessingUtils import mkdir, ProcessingUtils
import subprocess
from processing.core.ProcessingLog import ProcessingLog
import stat

class RUtils:

    RSCRIPTS_FOLDER = "R_SCRIPTS_FOLDER"
    R_FOLDER = "R_FOLDER"
    R_USE64 = "R_USE64"

    @staticmethod
    def RFolder():
        folder = ProcessingConfig.getSetting(RUtils.R_FOLDER)
        if folder == None:
            folder = ""

        return os.path.abspath(unicode(folder))

    @staticmethod
    def RScriptsFolder():
        folder = ProcessingConfig.getSetting(RUtils.RSCRIPTS_FOLDER)
        if folder == None:
            folder = unicode(os.path.join(ProcessingUtils.userFolder(), "rscripts"))
        mkdir(folder)

        return os.path.abspath(folder)

    @staticmethod
    def createRScriptFromRCommands(commands):
        scriptfile = open(RUtils.getRScriptFilename(), "w")
        for command in commands:
            scriptfile.write(command + "\n")
        scriptfile.close()

    @staticmethod
    def getRScriptFilename():
        return ProcessingUtils.userFolder() + os.sep + "processing_script.r"

    @staticmethod
    def getConsoleOutputFilename():
        return RUtils.getRScriptFilename()+".Rout"

    @staticmethod
    def executeRAlgorithm(alg, progress):
        RUtils.verboseCommands = alg.getVerboseCommands();
        RUtils.createRScriptFromRCommands(alg.getFullSetOfRCommands())
        if ProcessingUtils.isWindows():
            if ProcessingConfig.getSetting(RUtils.R_USE64):
                execDir = "x64"
            else:
                execDir = "i386"
            command = [RUtils.RFolder() + os.sep + "bin" + os.sep + execDir + os.sep + "R.exe", "CMD", "BATCH", "--vanilla",
                       RUtils.getRScriptFilename(), RUtils.getConsoleOutputFilename()]
        else:
            os.chmod(RUtils.getRScriptFilename(), stat.S_IEXEC | stat.S_IREAD | stat.S_IWRITE)
            command = "R CMD BATCH --vanilla " + RUtils.getRScriptFilename() + " "+ RUtils.getConsoleOutputFilename()

        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT,
                                universal_newlines=True)
        proc.wait()
        RUtils.createConsoleOutput()
        loglines = []
        loglines.append("R execution console output")
        loglines += RUtils.allConsoleResults
        for line in loglines:
            progress.setConsoleInfo(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)

    @staticmethod
    def createConsoleOutput():
        RUtils.consoleResults = []
        RUtils.allConsoleResults = []
        add = False
        if os.path.exists(RUtils.getConsoleOutputFilename()):
            lines = open(RUtils.getConsoleOutputFilename())
            for line in lines:
                line = line.strip("\n").strip(" ")
                if line.startswith(">"):
                    line = line[1:].strip(" ")
                    if line in RUtils.verboseCommands:
                        add = True
                    else:
                        add = False
                elif add:
                    RUtils.consoleResults.append("<p>" + line + "</p>\n");
                RUtils.allConsoleResults.append(line);

    @staticmethod
    def getConsoleOutput():
        s = "<font face=\"courier\">\n"
        s+="<h2> R Output</h2>\n"
        for line in RUtils.consoleResults:
            s+=line
        s+="</font>\n"

        return s

    @staticmethod
    def checkRIsInstalled(ignoreRegistrySettings=False):
        if ProcessingUtils.isWindows():
            path = RUtils.RFolder()
            if path == "":
                return "R folder is not configured.\nPlease configure it before running R scripts."

        R_INSTALLED = "R_INSTALLED"
        settings = QSettings()
        if not ignoreRegistrySettings:
            if settings.contains(R_INSTALLED):
                return
        if ProcessingUtils.isWindows():
            if ProcessingConfig.getSetting(RUtils.R_USE64):
                execDir = "x64"
            else:
                execDir = "i386"
            command = [RUtils.RFolder() + os.sep + "bin" + os.sep + execDir + os.sep + "R.exe", "--version"]
        else:
            command = ["R --version"]
        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=True).stdout

        for line in iter(proc.readline, ""):
            if "R version" in line:
                settings.setValue(R_INSTALLED, True)
                return
        html = ("<p>This algorithm requires R to be run."
            "Unfortunately, it seems that R is not installed in your system, or it is not correctly configured to be used from QGIS</p>"
            '<p><a href= "http://docs.qgis.org/2.0/html/en/docs/user_manual/processing/3rdParty.html">Click here</a>'
             'to know more about how to install and configure R to be used with QGIS</p>')
        return html

    @staticmethod
    def getRequiredPackages(code):
        regex = re.compile('library\("?(.*?)"?\)')
        return regex.findall(code)
