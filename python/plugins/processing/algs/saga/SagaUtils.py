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
import subprocess

from PyQt4.QtCore import QCoreApplication
from qgis.core import QgsApplication
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import isWindows, isMac, userFolder

SAGA_LOG_COMMANDS = 'SAGA_LOG_COMMANDS'
SAGA_LOG_CONSOLE = 'SAGA_LOG_CONSOLE'
SAGA_FOLDER = 'SAGA_FOLDER'
SAGA_IMPORT_EXPORT_OPTIMIZATION = 'SAGA_IMPORT_EXPORT_OPTIMIZATION'

def sagaBatchJobFilename():
    if isWindows():
        filename = 'saga_batch_job.bat'
    else:
        filename = 'saga_batch_job.sh'

    batchfile = userFolder() + os.sep + filename

    return batchfile


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

def sagaPath():
    folder = ProcessingConfig.getSetting(SAGA_FOLDER)
    if folder is None or folder == '':
        folder = findSagaFolder()
        if folder is not None:
            ProcessingConfig.setSettingValue(SAGA_FOLDER, folder)
    return folder or ''

def sagaDescriptionPath():
    return os.path.join(os.path.dirname(__file__), 'description')


def createSagaBatchJobFileFromSagaCommands(commands):

    fout = open(sagaBatchJobFilename(), 'w')
    if isWindows():
        fout.write('set SAGA=' + sagaPath() + '\n')
        fout.write('set SAGA_MLB=' + sagaPath() + os.sep
                   + 'modules' + '\n')
        fout.write('PATH=PATH;%SAGA%;%SAGA_MLB%\n')
    elif isMac():
        fout.write('export SAGA_MLB=' + sagaPath()
                   + '/../lib/saga\n')
        fout.write('export PATH=' + sagaPath() + ':$PATH\n')
    else:
        pass
    for command in commands:
        fout.write('saga_cmd ' + command.encode('utf8') + '\n')

    fout.write('exit')
    fout.close()

_installedVersion = None
_installedVersionFound = False

def getSagaInstalledVersion(runSaga=False):
    global _installedVersion
    global _installedVersionFound

    if not _installedVersionFound or runSaga:
        if isWindows():
            commands = [os.path.join(sagaPath(), "saga_cmd.exe"), "-v"]
        elif isMac():
            commands = [os.path.join(sagaPath(), "saga_cmd"), "-v"]
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
        try:
            lines = proc.readlines()
        except:
            return None
        for line in lines:
            if line.startswith("SAGA Version:"):
                _installedVersion = line[len("SAGA Version:"):].strip().split(" ")[0]
        _installedVersionFound = True
    return _installedVersion

def executeSaga(progress):
    if isWindows():
        command = ['cmd.exe', '/C ', sagaBatchJobFilename()]
    else:
        os.chmod(sagaBatchJobFilename(), stat.S_IEXEC
                 | stat.S_IREAD | stat.S_IWRITE)
        command = [sagaBatchJobFilename()]
    loglines = []
    loglines.append(QCoreApplication.translate('SagaUtils', 'SAGA execution console output'))
    proc = subprocess.Popen(
        command,
        shell=True,
        stdout=subprocess.PIPE,
        stdin=open(os.devnull),
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    ).stdout
    try:
        for line in iter(proc.readline, ''):
            if '%' in line:
                s = ''.join([x for x in line if x.isdigit()])
                try:
                    progress.setPercentage(int(s))
                except:
                    pass
            else:
                line = line.strip()
                if line != '/' and line != '-' and line != '\\' and line != '|':
                    loglines.append(line)
                    progress.setConsoleInfo(line)
    except:
        pass
    if ProcessingConfig.getSetting(SAGA_LOG_CONSOLE):
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
