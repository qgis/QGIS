# -*- coding: utf-8 -*-

"""
***************************************************************************
    FusionUtils.py
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
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import userFolder


class FusionUtils:

    FUSION_FOLDER = 'FUSION_FOLDER'

    @staticmethod
    def FusionPath():
        folder = ProcessingConfig.getSetting(FusionUtils.FUSION_FOLDER)
        if folder is None:
            folder = ''

        return folder

    @staticmethod
    def tempFileListFilepath():
        filename = 'fusion_files_list.txt'
        filepath = userFolder() + os.sep + filename
        return filepath

    @staticmethod
    def createFileList(files):
        out = open(FusionUtils.tempFileListFilepath(), 'w')
        for f in files:
            out.write(f + '\n')
        out.close()

    @staticmethod
    def runFusion(commands, progress):
        loglines = []
        loglines.append('Fusion execution console output')
        proc = subprocess.Popen(
            commands,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=False,
            ).stdout
        for line in iter(proc.readline, ''):
            loglines.append(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
