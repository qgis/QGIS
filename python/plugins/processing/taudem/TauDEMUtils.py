# -*- coding: utf-8 -*-

"""
***************************************************************************
    TauDEMUtils.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from qgis.core import QgsApplication
import subprocess

from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import *


class TauDEMUtils:

    TAUDEM_FOLDER = 'TAUDEM_FOLDER'
    MPIEXEC_FOLDER = 'MPIEXEC_FOLDER'
    MPI_PROCESSES = 'MPI_PROCESSES'

    @staticmethod
    def taudemPath():
        folder = ProcessingConfig.getSetting(TauDEMUtils.TAUDEM_FOLDER)
        if folder is None:
            folder = ''

        if isMac():
            testfolder = os.path.join(QgsApplication.prefixPath(), 'bin')
            if os.path.exists(os.path.join(testfolder, 'slopearea')):
                folder = testfolder
            else:
                testfolder = '/usr/local/bin'
                if os.path.exists(os.path.join(testfolder, 'slopearea')):
                    folder = testfolder
        return folder

    @staticmethod
    def mpiexecPath():
        folder = ProcessingConfig.getSetting(TauDEMUtils.MPIEXEC_FOLDER)
        if folder is None:
            folder = ''

        if isMac():
            testfolder = os.path.join(QgsApplication.prefixPath(), 'bin')
            if os.path.exists(os.path.join(testfolder, 'mpiexec')):
                folder = testfolder
            else:
                testfolder = '/usr/local/bin'
                if os.path.exists(os.path.join(testfolder, 'mpiexec')):
                    folder = testfolder
        return folder

    @staticmethod
    def taudemDescriptionPath():
        return os.path.normpath(os.path.join(os.path.dirname(__file__),
                                'description'))

    @staticmethod
    def executeTauDEM(command, progress):
        loglines = []
        loglines.append('TauDEM execution console output')
        fused_command = ''.join(['"%s" ' % c for c in command])
        proc = subprocess.Popen(
            fused_command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            ).stdout
        for line in iter(proc.readline, ''):
            loglines.append(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
