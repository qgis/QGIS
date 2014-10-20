# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaUtils.py
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
import stat
import traceback
import subprocess
from PyQt4.QtCore import *
from qgis.core import *
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import *

class SagaUtils:

    SAGA_LATEST = 'SAGA_LATEST'
    SAGA_LOG_COMMANDS = 'SAGA_LOG_COMMANDS'
    SAGA_LOG_CONSOLE = 'SAGA_LOG_CONSOLE'
    SAGA_FOLDER = 'SAGA_FOLDER'
    SAGA_IMPORT_EXPORT_OPTIMIZATION = 'SAGA_IMPORT_EXPORT_OPTIMIZATION'

    LATEST_VERSION_NUMBER = '2.1.2'
    BASE_VERSION_NUMBER = '2.0.8'

    isSagaInstalled = False

    @staticmethod
    def sagaBatchJobFilename():
        if isWindows():
            filename = 'saga_batch_job.bat'
        else:
            filename = 'saga_batch_job.sh'

        batchfile = userFolder() + os.sep + filename

        return batchfile

    @staticmethod
    def findSagaFolder():
        folder = None
        if isMac():
            testfolder = os.path.join(QgsApplication.prefixPath(), 'bin')
            if os.path.exists(os.path.join(testfolder, 'saga_cmd')):
                folder = testfolder
            else:
                testfolder = '/usr/local/bin'
                if os.path.exists(os.path.join(testfolder, 'saga_cmd')):
                    folder = testfolder
        elif isWindows():
            testfolder = os.path.join(os.path.dirname(QgsApplication.prefixPath()), 'saga')
            if os.path.exists(os.path.join(testfolder, 'saga_cmd.exe')):
                folder = testfolder
        return folder

    @staticmethod
    def sagaPath():
        folder = SagaUtils.findSagaFolder()
        if folder is None:
            folder = ProcessingConfig.getSetting(SagaUtils.SAGA_FOLDER)
        return folder or ''

    @staticmethod
    def isSaga208():
        if SagaUtils.findSagaFolder() is not None:
            return not isMac();
        else:
            return not ProcessingConfig.getSetting(SagaUtils.SAGA_LATEST)

    @staticmethod
    def sagaDescriptionPath():
        return os.path.join(os.path.dirname(__file__), 'description')

    @staticmethod
    def createSagaBatchJobFileFromSagaCommands(commands):

        fout = open(SagaUtils.sagaBatchJobFilename(), 'w')
        if isWindows():
            fout.write('set SAGA=' + SagaUtils.sagaPath() + '\n')
            fout.write('set SAGA_MLB=' + SagaUtils.sagaPath() + os.sep
                       + 'modules' + '\n')
            fout.write('PATH=PATH;%SAGA%;%SAGA_MLB%\n')
        elif isMac():
            fout.write('export SAGA_MLB=' + SagaUtils.sagaPath()
                       + '/../lib/saga\n')
            fout.write('export PATH=' + SagaUtils.sagaPath() + ':$PATH\n')
        else:
            pass
        for command in commands:
            fout.write('saga_cmd ' + command.encode('utf8') + '\n')

        fout.write('exit')
        fout.close()

    @staticmethod
    def getSagaInstalledVersion():
        if isWindows():
            commands = [os.path.join(SagaUtils.sagaPath(), "saga_cmd.exe"), "-v"]
        elif isMac():
            commands = [os.path.join(SagaUtils.sagaPath(), "saga_cmd"), "-v"]
        else:
            commands = ["saga_cmd", "-v"]
        proc = subprocess.Popen(
            commands,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=open(os.devnull),
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            ).stdout
        lines = proc.readlines()
        for line in lines:
            if line.startswith("SAGA Version:"):
                return line.split(" ")[-1].strip()
        return None


    @staticmethod
    def executeSaga(progress):
        if isWindows():
            command = ['cmd.exe', '/C ', SagaUtils.sagaBatchJobFilename()]
        else:
            os.chmod(SagaUtils.sagaBatchJobFilename(), stat.S_IEXEC
                     | stat.S_IREAD | stat.S_IWRITE)
            command = [SagaUtils.sagaBatchJobFilename()]
        loglines = []
        loglines.append('SAGA execution console output')
        proc = subprocess.Popen(
            command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=open(os.devnull),
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            ).stdout
        for line in iter(proc.readline, ''):
            if '%' in line:
                s = ''.join([x for x in line if x.isdigit()])
                try:
                    progress.setPercentage(int(s))
                except:
                    pass
            else:
                line = line.strip()
                if line != '/' and line != '-' and line != '\\' and line \
                    != '|':
                    loglines.append(line)
                    progress.setConsoleInfo(line)
        if ProcessingConfig.getSetting(SagaUtils.SAGA_LOG_CONSOLE):
            ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)

    @staticmethod
    def checkSagaIsInstalled(ignorePreviousState=False):
        if isWindows():
            path = SagaUtils.sagaPath()
            if path == '':
                return 'SAGA folder is not configured.\nPlease configure it \
                        before running SAGA algorithms.'
            cmdpath = os.path.join(path, 'saga_cmd.exe')
            if not os.path.exists(cmdpath):
                return 'The specified SAGA folder does not contain a valid \
                    SAGA executable.\n' \
                    + 'Please, go to the processing settings dialog, and \
                    check that the SAGA\n' \
                    + 'folder is correctly configured'

        if not ignorePreviousState:
            if SagaUtils.isSagaInstalled:
                return

        try:
            installedVersion = SagaUtils.getSagaInstalledVersion()
            if installedVersion is None:
                return 'It seems that SAGA is not correctly installed in \
                        your system.\nPlease install it before running SAGA \
                        algorithms.'
            if installedVersion not in [SagaUtils.LATEST_VERSION_NUMBER, SagaUtils.BASE_VERSION_NUMBER]:
                return ('The installed version (%s) is not supported\n'
                        'You must installed SAGA %s or SAGA %s'
                        % (installedVersion, SagaUtils.LATEST_VERSION_NUMBER,
                           SagaUtils.BASE_VERSION_NUMBER))
            expectedVersion = SagaUtils.BASE_VERSION_NUMBER if SagaUtils.isSaga208() else SagaUtils.LATEST_VERSION_NUMBER
            if expectedVersion != installedVersion:
                return ('The installed version (%s) does not match the version'
                        'configured in the Processing settings (%s)\n'
                        'Please, modify your configuration' % (installedVersion, expectedVersion))
        except:
            s = traceback.format_exc()
            return 'Error while checking SAGA installation. SAGA might not \
                    be correctly configured.\n' + s

        SagaUtils.isSagaInstalled = True
