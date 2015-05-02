# -*- coding: utf-8 -*-

"""
***************************************************************************
    GrassUtils.py
    ---------------------
    Date                 : February 2015
    Copyright            : (C) 2014-2015 by Victor Olaya
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
__date__ = 'February 2015'
__copyright__ = '(C) 2014-2015, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import stat
import shutil
import subprocess
import os
from qgis.core import QgsApplication
from PyQt4.QtCore import QCoreApplication
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import userFolder, isWindows, isMac, tempFolder, mkdir
from processing.tests.TestData import points


class Grass7Utils:

    GRASS_REGION_XMIN = 'GRASS7_REGION_XMIN'
    GRASS_REGION_YMIN = 'GRASS7_REGION_YMIN'
    GRASS_REGION_XMAX = 'GRASS7_REGION_XMAX'
    GRASS_REGION_YMAX = 'GRASS7_REGION_YMAX'
    GRASS_REGION_CELLSIZE = 'GRASS7_REGION_CELLSIZE'
    GRASS_FOLDER = 'GRASS7_FOLDER'
    GRASS_WIN_SHELL = 'GRASS7_WIN_SHELL'
    GRASS_LOG_COMMANDS = 'GRASS7_LOG_COMMANDS'
    GRASS_LOG_CONSOLE = 'GRASS7_LOG_CONSOLE'

    sessionRunning = False
    sessionLayers = {}
    projectionSet = False

    isGrass7Installed = False

    @staticmethod
    def grassBatchJobFilename():
        '''This is used in Linux. This is the batch job that we assign to
        GRASS_BATCH_JOB and then call GRASS and let it do the work
        '''
        filename = 'grass7_batch_job.sh'
        batchfile = userFolder() + os.sep + filename
        return batchfile

    @staticmethod
    def grassScriptFilename():
        '''This is used in windows. We create a script that initializes
        GRASS and then uses grass commands
        '''
        filename = 'grass7_script.bat'
        filename = userFolder() + os.sep + filename
        return filename

    @staticmethod
    def getGrassVersion():
        # FIXME: I do not know if this should be removed or let the user enter it
        # or something like that... This is just a temporary thing
        return '7.0.0'

    @staticmethod
    def grassPath():
        if not isWindows() and not isMac():
            return ''

        folder = ProcessingConfig.getSetting(Grass7Utils.GRASS_FOLDER)
        if folder is None:
            if isWindows():
                testfolder = os.path.dirname(str(QgsApplication.prefixPath()))
                testfolder = os.path.join(testfolder, 'grass7')
                if os.path.isdir(testfolder):
                    for subfolder in os.listdir(testfolder):
                        if subfolder.startswith('grass7'):
                            folder = os.path.join(testfolder, subfolder)
                            break
            else:
                folder = os.path.join(str(QgsApplication.prefixPath()), 'grass7'
                                      )
                if not os.path.isdir(folder):
                    folder = '/Applications/GRASS-7.0.app/Contents/MacOS'

        return folder

    @staticmethod
    def grassWinShell():
        folder = ProcessingConfig.getSetting(Grass7Utils.GRASS_WIN_SHELL)
        if folder is None:
            folder = os.path.dirname(str(QgsApplication.prefixPath()))
            folder = os.path.join(folder, 'msys')
        return folder

    @staticmethod
    def grassDescriptionPath():
        return os.path.join(os.path.dirname(__file__), 'description')

    @staticmethod
    def createGrass7Script(commands):
        folder = Grass7Utils.grassPath()
        shell = Grass7Utils.grassWinShell()

        script = Grass7Utils.grassScriptFilename()
        gisrc = userFolder() + os.sep + 'processing.gisrc7'  # FIXME: use temporary file

        # Temporary gisrc file
        output = open(gisrc, 'w')
        location = 'temp_location'
        gisdbase = Grass7Utils.grassDataFolder()

        output.write('GISDBASE: ' + gisdbase + '\n')
        output.write('LOCATION_NAME: ' + location + '\n')
        output.write('MAPSET: PERMANENT \n')
        output.write('GRASS_GUI: text\n')
        output.close()

        output = open(script, 'w')
        output.write('set HOME=' + os.path.expanduser('~') + '\n')
        output.write('set GISRC=' + gisrc + '\n')
        output.write('set GRASS_SH=' + shell + '\\bin\\sh.exe\n')
        output.write('set PATH=' + shell + os.sep + 'bin;' + shell + os.sep
                     + 'lib;' + '%PATH%\n')
        output.write('set WINGISBASE=' + folder + '\n')
        output.write('set GISBASE=' + folder + '\n')
        output.write('set GRASS_PROJSHARE=' + folder + os.sep + 'share'
                     + os.sep + 'proj' + '\n')
        output.write('set GRASS_MESSAGE_FORMAT=gui\n')

        # Replacement code for etc/Init.bat
        output.write('if "%GRASS_ADDON_PATH%"=="" set PATH=%WINGISBASE%\\bin;%WINGISBASE%\\lib;%PATH%\n')
        output.write('if not "%GRASS_ADDON_PATH%"=="" set PATH=%WINGISBASE%\\bin;%WINGISBASE%\\lib;%GRASS_ADDON_PATH%;%PATH%\n')
        output.write('\n')
        output.write('set GRASS_VERSION=' + Grass7Utils.getGrassVersion() + '\n')
        output.write('if not "%LANG%"=="" goto langset\n')
        output.write('FOR /F "usebackq delims==" %%i IN (`"%WINGISBASE%\\etc\\winlocale"`) DO @set LANG=%%i\n')
        output.write(':langset\n')
        output.write('\n')
        output.write('set PATHEXT=%PATHEXT%;.PY\n')
        output.write('set PYTHONPATH=%PYTHONPATH%;%WINGISBASE%\\etc\\python;%WINGISBASE%\\etc\\wxpython\\n')
        output.write('\n')
        output.write('g.gisenv.exe set="MAPSET=PERMANENT"\n')
        output.write('g.gisenv.exe set="LOCATION=' + location + '"\n')
        output.write('g.gisenv.exe set="LOCATION_NAME=' + location + '"\n')
        output.write('g.gisenv.exe set="GISDBASE=' + gisdbase + '"\n')
        output.write('g.gisenv.exe set="GRASS_GUI=text"\n')
        for command in commands:
            output.write(command.encode('utf8') + '\n')
        output.write('\n')
        output.write('exit\n')
        output.close()

    @staticmethod
    def createGrass7BatchJobFileFromGrass7Commands(commands):
        fout = open(Grass7Utils.grassBatchJobFilename(), 'w')
        for command in commands:
            fout.write(command.encode('utf8') + '\n')
        fout.write('exit')
        fout.close()

    @staticmethod
    def grassMapsetFolder():
        folder = os.path.join(Grass7Utils.grassDataFolder(), 'temp_location')
        mkdir(folder)
        return folder

    @staticmethod
    def grassDataFolder():
        tempfolder = os.path.join(tempFolder(), 'grassdata')
        mkdir(tempfolder)
        return tempfolder

    @staticmethod
    def createTempMapset():
        '''Creates a temporary location and mapset(s) for GRASS data
        processing. A minimal set of folders and files is created in the
        system's default temporary directory. The settings files are
        written with sane defaults, so GRASS can do its work. The mapset
        projection will be set later, based on the projection of the first
        input image or vector
        '''

        folder = Grass7Utils.grassMapsetFolder()
        mkdir(os.path.join(folder, 'PERMANENT'))
        mkdir(os.path.join(folder, 'PERMANENT', '.tmp'))
        Grass7Utils.writeGrass7Window(os.path.join(folder, 'PERMANENT', 'DEFAULT_WIND'))
        outfile = open(os.path.join(folder, 'PERMANENT', 'MYNAME'), 'w')
        outfile.write(
            'QGIS GRASS GIS 7 interface: temporary data processing location.\n')
        outfile.close()

        Grass7Utils.writeGrass7Window(os.path.join(folder, 'PERMANENT', 'WIND'))
        mkdir(os.path.join(folder, 'PERMANENT', 'sqlite'))
        outfile = open(os.path.join(folder, 'PERMANENT', 'VAR'), 'w')
        outfile.write('DB_DRIVER: sqlite\n')
        outfile.write('DB_DATABASE: $GISDBASE/$LOCATION_NAME/$MAPSET/sqlite/sqlite.db\n')
        outfile.close()

    @staticmethod
    def writeGrass7Window(filename):
        out = open(filename, 'w')
        out.write('proj:       0\n')
        out.write('zone:       0\n')
        out.write('north:      1\n')
        out.write('south:      0\n')
        out.write('east:       1\n')
        out.write('west:       0\n')
        out.write('cols:       1\n')
        out.write('rows:       1\n')
        out.write('e-w resol:  1\n')
        out.write('n-s resol:  1\n')
        out.write('top:        1\n')
        out.write('bottom:     0\n')
        out.write('cols3:      1\n')
        out.write('rows3:      1\n')
        out.write('depths:     1\n')
        out.write('e-w resol3: 1\n')
        out.write('n-s resol3: 1\n')
        out.write('t-b resol:  1\n')

        out.close()

    @staticmethod
    def prepareGrass7Execution(commands):
        if isWindows():
            Grass7Utils.createGrass7Script(commands)
            command = ['cmd.exe', '/C ', Grass7Utils.grassScriptFilename()]
        else:
            gisrc = userFolder() + os.sep + 'processing.gisrc7'
            os.putenv('GISRC', gisrc)
            os.putenv('GRASS_MESSAGE_FORMAT', 'gui')
            os.putenv('GRASS_BATCH_JOB', Grass7Utils.grassBatchJobFilename())
            Grass7Utils.createGrass7BatchJobFileFromGrass7Commands(commands)
            os.chmod(Grass7Utils.grassBatchJobFilename(), stat.S_IEXEC
                     | stat.S_IREAD | stat.S_IWRITE)
            if isMac() and os.path.exists(Grass7Utils.grassPath() + os.sep + 'grass70.sh'):
                command = Grass7Utils.grassPath() + os.sep + 'grass70.sh ' \
                    + Grass7Utils.grassMapsetFolder() + '/PERMANENT'
            else:
                command = 'grass70 ' + Grass7Utils.grassMapsetFolder() \
                    + '/PERMANENT'

        return command

    @staticmethod
    def executeGrass7(commands, progress, outputCommands=None):
        loglines = []
        loglines.append(Grass7Utils.tr('GRASS GIS 7 execution console output'))
        grassOutDone = False
        command = Grass7Utils.prepareGrass7Execution(commands)
        proc = subprocess.Popen(
            command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=open(os.devnull),
            stderr=subprocess.STDOUT,
            universal_newlines=True,
        ).stdout
        for line in iter(proc.readline, ''):
            if 'GRASS_INFO_PERCENT' in line:
                try:
                    progress.setPercentage(int(line[len('GRASS_INFO_PERCENT') + 2:]))
                except:
                    pass
            else:
                if 'r.out' in line or 'v.out' in line:
                    grassOutDone = True
                loglines.append(line)
                progress.setConsoleInfo(line)

        # Some GRASS scripts, like r.mapcalculator or r.fillnulls, call
        # other GRASS scripts during execution. This may override any
        # commands that are still to be executed by the subprocess, which
        # are usually the output ones. If that is the case runs the output
        # commands again.

        if not grassOutDone and outputCommands:
            command = Grass7Utils.prepareGrass7Execution(outputCommands)
            proc = subprocess.Popen(
                command,
                shell=True,
                stdout=subprocess.PIPE,
                stdin=open(os.devnull),
                stderr=subprocess.STDOUT,
                universal_newlines=True,
            ).stdout
            for line in iter(proc.readline, ''):
                if 'GRASS_INFO_PERCENT' in line:
                    try:
                        progress.setPercentage(int(
                            line[len('GRASS_INFO_PERCENT') + 2:]))
                    except:
                        pass
                else:
                    loglines.append(line)
                    progress.setConsoleInfo(line)

        if ProcessingConfig.getSetting(Grass7Utils.GRASS_LOG_CONSOLE):
            ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
        return loglines

    # GRASS session is used to hold the layers already exported or
    # produced in GRASS between multiple calls to GRASS algorithms.
    # This way they don't have to be loaded multiple times and
    # following algorithms can use the results of the previous ones.
    # Starting a session just involves creating the temp mapset
    # structure
    @staticmethod
    def startGrass7Session():
        if not Grass7Utils.sessionRunning:
            Grass7Utils.createTempMapset()
            Grass7Utils.sessionRunning = True

    # End session by removing the temporary GRASS mapset and all
    # the layers.
    @staticmethod
    def endGrass7Session():
        shutil.rmtree(Grass7Utils.grassMapsetFolder(), True)
        Grass7Utils.sessionRunning = False
        Grass7Utils.sessionLayers = {}
        Grass7Utils.projectionSet = False

    @staticmethod
    def getSessionLayers():
        return Grass7Utils.sessionLayers

    @staticmethod
    def addSessionLayers(exportedLayers):
        Grass7Utils.sessionLayers = dict(
            Grass7Utils.sessionLayers.items()
            + exportedLayers.items())

    @staticmethod
    def checkGrass7IsInstalled(ignorePreviousState=False):
        if isWindows():
            path = Grass7Utils.grassPath()
            if path == '':
                return Grass7Utils.tr(
                    'GRASS GIS 7 folder is not configured. Please configure '
                    'it before running GRASS GIS 7 algorithms.')
            cmdpath = os.path.join(path, 'bin', 'r.out.gdal.exe')
            if not os.path.exists(cmdpath):
                return Grass7Utils.tr(
                    'The specified GRASS GIS 7 folder does not contain a valid '
                    'set of GRASS GIS 7 modules.\nPlease, go to the Processing '
                    'settings dialog, and check that the GRASS GIS 7\n'
                    'folder is correctly configured')

        if not ignorePreviousState:
            if Grass7Utils.isGrass7Installed:
                return
        try:
            from processing import runalg
            result = runalg(
                'grass7:v.voronoi',
                points(),
                False,
                False,
                '270778.60198,270855.745301,4458921.97814,4458983.8488',
                -1,
                0.0001,
                0,
                None,
            )
            if not os.path.exists(result['output']):
                return Grass7Utils.tr(
                    'It seems that GRASS GIS 7 is not correctly installed and '
                    'configured in your system.\nPlease install it before '
                    'running GRASS GIS 7 algorithms.')
        except:
            return Grass7Utils.tr(
                'Error while checking GRASS GIS 7 installation. GRASS GIS 7 '
                'might not be correctly configured.\n')

        Grass7Utils.isGrass7Installed = True

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'Grass7Utils'
        return QCoreApplication.translate(context, string)
