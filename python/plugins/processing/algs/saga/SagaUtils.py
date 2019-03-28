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
import platform
import stat
import subprocess
import time

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessingUtils,
                       QgsMessageLog)
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import isWindows, isMac, userFolder

SAGA_LOG_COMMANDS = 'SAGA_LOG_COMMANDS'
SAGA_LOG_CONSOLE = 'SAGA_LOG_CONSOLE'
SAGA_IMPORT_EXPORT_OPTIMIZATION = 'SAGA_IMPORT_EXPORT_OPTIMIZATION'

_installedVersion = None
_installedVersionFound = False


def sagaBatchJobFilename():
    if isWindows():
        filename = 'saga_batch_job.bat'
    else:
        filename = 'saga_batch_job.sh'

    batchfile = os.path.join(userFolder(), filename)

    return batchfile


def findSagaFolder():
    folder = None
    if isMac() or platform.system() == 'FreeBSD':
        testfolder = os.path.join(QgsApplication.prefixPath(), 'bin')
        if os.path.exists(os.path.join(testfolder, 'saga_cmd')):
            folder = testfolder
        else:
            testfolder = '/usr/local/bin'
            if os.path.exists(os.path.join(testfolder, 'saga_cmd')):
                folder = testfolder
    elif isWindows():
        folders = []
        folders.append(os.path.join(os.path.dirname(QgsApplication.prefixPath()), 'saga-ltr'))
        folders.append(os.path.join(os.path.dirname(QgsApplication.prefixPath()), 'saga'))
        if "OSGEO4W_ROOT" in os.environ:
            folders.append(os.path.join(str(os.environ['OSGEO4W_ROOT']), "apps", "saga-ltr"))
            folders.append(os.path.join(str(os.environ['OSGEO4W_ROOT']), "apps", "saga"))

        for testfolder in folders:
            if os.path.exists(os.path.join(testfolder, 'saga_cmd.exe')):
                folder = testfolder
                break

    return folder


def sagaPath():
    if not isWindows() and not isMac() and not platform.system() == 'FreeBSD':
        return ''

    folder = findSagaFolder()
    return folder or ''


def sagaDescriptionPath():
    return os.path.join(os.path.dirname(__file__), 'description')


def createSagaBatchJobFileFromSagaCommands(commands):

    with open(sagaBatchJobFilename(), 'w', encoding="utf8") as fout:
        if isWindows():
            fout.write('set SAGA=' + sagaPath() + '\n')
            fout.write('set SAGA_MLB=' + os.path.join(sagaPath(), 'modules') + '\n')
            fout.write('PATH=%PATH%;%SAGA%;%SAGA_MLB%\n')
        elif isMac() or platform.system() == 'FreeBSD':
            fout.write('export SAGA_MLB=' + os.path.join(sagaPath(), '../lib/saga') + '\n')
            fout.write('export PATH=' + sagaPath() + ':$PATH\n')
        else:
            pass
        for command in commands:
            if isWindows():
                fout.write('call saga_cmd ' + command + '\n')
            else:
                fout.write('saga_cmd ' + command + '\n')

        fout.write('exit')


def getInstalledVersion(runSaga=False):
    global _installedVersion
    global _installedVersionFound

    maxRetries = 5
    retries = 0
    if _installedVersionFound and not runSaga:
        return _installedVersion

    if isWindows():
        commands = [os.path.join(sagaPath(), "saga_cmd.exe"), "-v"]
    elif isMac() or platform.system() == 'FreeBSD':
        commands = [os.path.join(sagaPath(), "saga_cmd -v")]
    else:
        # for Linux use just one string instead of separated parameters as the list
        # does not work well together with shell=True option
        # (python docs advices to use subprocess32 instead of python2.7's subprocess)
        commands = ["saga_cmd -v"]
    while retries < maxRetries:
        with subprocess.Popen(
            commands,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
        ) as proc:
            if isMac() or platform.system() == 'FreeBSD':  # This trick avoids having an uninterrupted system call exception if SAGA is not installed
                time.sleep(1)
            try:
                lines = proc.stdout.readlines()
                for line in lines:
                    if line.startswith("SAGA Version:"):
                        _installedVersion = line[len("SAGA Version:"):].strip().split(" ")[0]
                        _installedVersionFound = True
                        return _installedVersion
                return None
            except IOError:
                retries += 1
            except:
                return None

    return _installedVersion


def executeSaga(feedback):
    if isWindows():
        command = ['cmd.exe', '/C ', sagaBatchJobFilename()]
    else:
        os.chmod(sagaBatchJobFilename(), stat.S_IEXEC |
                 stat.S_IREAD | stat.S_IWRITE)
        command = ["'" + sagaBatchJobFilename() + "'"]
    loglines = []
    loglines.append(QCoreApplication.translate('SagaUtils', 'SAGA execution console output'))
    with subprocess.Popen(
        command,
        shell=True,
        stdout=subprocess.PIPE,
        stdin=subprocess.DEVNULL,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    ) as proc:
        try:
            for line in iter(proc.stdout.readline, ''):
                if '%' in line:
                    s = ''.join([x for x in line if x.isdigit()])
                    try:
                        feedback.setProgress(int(s))
                    except:
                        pass
                else:
                    line = line.strip()
                    if line != '/' and line != '-' and line != '\\' and line != '|':
                        loglines.append(line)
                        feedback.pushConsoleInfo(line)
        except:
            pass

    if ProcessingConfig.getSetting(SAGA_LOG_CONSOLE):
        QgsMessageLog.logMessage('\n'.join(loglines), 'Processing', Qgis.Info)
