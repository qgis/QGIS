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
from builtins import str
from builtins import object

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import re
import os
import stat
import subprocess

from qgis.PyQt.QtCore import QSettings, QCoreApplication
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import userFolder, isWindows, mkdir, getTempFilenameInTempFolder


class PerlUtils:

    PERLSCRIPTS_FOLDER = 'PERL_SCRIPTS_FOLDER'
    PERL_FOLDER = 'PERL_FOLDER'
    PERL_USE64 = 'PERL_USE64'
    PERL_LIBS_USER = 'PERL_LIBS_USER'

    perlscriptfilename = userFolder() + os.sep + 'processing_script.pl'

    @staticmethod
    def PerlFolder():
        folder = ProcessingConfig.getSetting(PerlUtils.PERL_FOLDER)
        if folder is None:
            if isWindows():
                if 'ProgramW6432' in os.environ.keys() and os.path.isdir(os.path.join(os.environ['ProgramW6432'], 'R')):
                    testfolder = os.path.join(os.environ['ProgramW6432'], 'R')
                elif 'PROGRAMFILES(x86)' in os.environ.keys() and os.path.isdir(os.path.join(os.environ['PROGRAMFILES(x86)'], 'R')):
                    testfolder = os.path.join(os.environ['PROGRAMFILES(x86)'], 'R')
                elif 'PROGRAMFILES' in os.environ.keys() and os.path.isdir(os.path.join(os.environ['PROGRAMFILES'], 'R')):
                    testfolder = os.path.join(os.environ['PROGRAMFILES'], 'R')
                else:
                    testfolder = 'C:\\Perl'

                if os.path.isdir(testfolder):
                    subfolders = os.listdir(testfolder)
                    subfolders.sort(reverse=True)
                    for subfolder in subfolders:
                        if subfolder.startswith('R-'):
                            folder = os.path.join(testfolder, subfolder)
                            break
                else:
                    folder = ''
            else:
                folder = ''

        return os.path.abspath(unicode(folder))

    @staticmethod
    def PerlLibs():
        folder = ProcessingConfig.getSetting(PerlUtils.PERL_LIBS_USER)
        if folder is None:
            folder = unicode(os.path.join(userFolder(), 'perllibs'))
        try:
            mkdir(folder)
        except:
            folder = unicode(os.path.join(userFolder(), 'perllibs'))
            mkdir(folder)
        return os.path.abspath(unicode(folder))

    @staticmethod
    def defaultPerlScriptsFolder():
        folder = unicode(os.path.join(userFolder(), 'perlscripts'))
        mkdir(folder)
        return os.path.abspath(folder)

    @staticmethod
    def PerlScriptsFolders():
        folder = ProcessingConfig.getSetting(PerlUtils.PERLSCRIPTS_FOLDER)
        if folder is not None:
            return folder.split(';')
        else:
            return [PerlUtils.defaultPerlScriptsFolder()]

    @staticmethod
    def createPerlScriptFromPerlCommands(commands):
        scriptfile = open(PerlUtils.getPerlScriptFilename(), 'w')
        for command in commands:
            scriptfile.write(command + '\n')
        scriptfile.close()

    @staticmethod
    def getPerlScriptFilename():
        return PerlUtils.perlscriptfilename

    @staticmethod
    def getConsoleOutputFilename():
        return PerlUtils.getPerlScriptFilename() + '.Perlout'

    @staticmethod
    def executePerlAlgorithm(alg, progress):
        # generate new Perl script file name in a temp folder
        PerlUtils.rscriptfilename = getTempFilenameInTempFolder('processing_script.pl')
        # run commands
        PerlUtils.verboseCommands = alg.getVerboseCommands()
        PerlUtils.createPerlScriptFromPerlCommands(alg.getFullSetOfPerlCommands())
        if isWindows():
            if ProcessingConfig.getSetting(PerlUtils.PERL_USE64):
                execDir = 'x64'
            else:
                execDir = 'i386'
            command = [
                PerlUtils.PerlFolder() + os.sep + 'bin' + os.sep + execDir + os.sep
                + 'perl.exe',
                'CMD',
                'BATCH',
                '--vanilla',
                PerlUtils.getPerlScriptFilename(),
                PerlUtils.getConsoleOutputFilename()
            ]

        else:
            os.chmod(PerlUtils.getPerlScriptFilename(), stat.S_IEXEC | stat.S_IREAD
                     | stat.S_IWRITE)
            command = 'PERL CMD BATCH --vanilla ' + PerlUtils.getPerlScriptFilename() \
                + ' ' + PerlUtils.getConsoleOutputFilename()

        proc = subprocess.Popen(
            command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=open(os.devnull),
            stderr=subprocess.STDOUT,
            universal_newlines=True,
        )
        proc.wait()
        PerlUtils.createConsoleOutput()
        loglines = []
        loglines.append(PerlUtils.tr('Perl execution console output'))
        loglines += PerlUtils.allConsoleResults
        for line in loglines:
            progress.setConsoleInfo(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)

    @staticmethod
    def createConsoleOutput():
        PerlUtils.consoleResults = []
        PerlUtils.allConsoleResults = []
        add = False
        if os.path.exists(PerlUtils.getConsoleOutputFilename()):
            lines = open(PerlUtils.getConsoleOutputFilename())
            for line in lines:
                line = line.strip().strip(' ')
                if line.startswith('>'):
                    line = line[1:].strip(' ')
                    if line in PerlUtils.verboseCommands:
                        add = True
                    else:
                        add = False
                elif add:
                    PerlUtils.consoleResults.append('<p>' + line + '</p>\n')
                PerlUtils.allConsoleResults.append(line)

    @staticmethod
    def getConsoleOutput():
        s = '<font face="courier">\n'
        s += PerlUtils.tr('<h2>R Output</h2>\n')
        for line in PerlUtils.consoleResults:
            s += line
        s += '</font>\n'

        return s

    @staticmethod
    def checkPerlIsInstalled(ignoreRegistrySettings=False):
        if isWindows():
            path = PerlUtils.PerlFolder()
            if path == '':
                return PerlUtils.tr('Perl folder is not configured.\nPlease configure '
                                 'it before running Perl scripts.')

        PERL_INSTALLED = 'PERL_INSTALLED'
        settings = QSettings()
        if not ignoreRegistrySettings:
            if settings.contains(PERL_INSTALLED):
                return
        if isWindows():
            if ProcessingConfig.getSetting(PerlUtils.PERL_USE64):
                execDir = 'x64'
            else:
                execDir = 'i386'
            command = [PerlUtils.PerlFolder() + os.sep + 'bin' + os.sep + execDir
                       + os.sep + 'perl.exe', '--version']
        else:
            command = ['perl --version']
        proc = subprocess.Popen(
            command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=open(os.devnull),
            stderr=subprocess.STDOUT,
            universal_newlines=True,
        ).stdout

        for line in iter(proc.readline, ''):
            if 'Perl version' in line:
                settings.setValue(R_INSTALLED, True)
                return
        html = PerlUtils.tr(
            '<p>This algorithm requires Perl to be run. Unfortunately, it '
            'seems that Perl is not installed in your system, or it is not '
            'correctly configured to be used from QGIS</p>'
            '<p><a href="http://docs.qgis.org/testing/en/docs/user_manual/processing/3rdParty.html">Click here</a> '
            'to know more about how to install and configure Perl to be used with QGIS</p>')

        return html

    @staticmethod
    def getRequiredPackages(code):
        regex = re.compile('[^#]library\("?(.*?)"?\)')
        return regex.findall(code)

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'PerlUtils'
        return QCoreApplication.translate(context, string)
