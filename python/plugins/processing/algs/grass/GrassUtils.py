# -*- coding: utf-8 -*-

"""
***************************************************************************
    GrassUtils.py
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

import stat
import shutil
import codecs
import subprocess
from qgis.core import QgsApplication
from PyQt4.QtCore import *
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.tools.system import *
from processing.tests.TestData import points


class GrassUtils:

    GRASS_REGION_XMIN = 'GRASS_REGION_XMIN'
    GRASS_REGION_YMIN = 'GRASS_REGION_YMIN'
    GRASS_REGION_XMAX = 'GRASS_REGION_XMAX'
    GRASS_REGION_YMAX = 'GRASS_REGION_YMAX'
    GRASS_REGION_CELLSIZE = 'GRASS_REGION_CELLSIZE'
    GRASS_FOLDER = 'GRASS_FOLDER'
    GRASS_WIN_SHELL = 'GRASS_WIN_SHELL'
    GRASS_LOG_COMMANDS = 'GRASS_LOG_COMMANDS'
    GRASS_LOG_CONSOLE = 'GRASS_LOG_CONSOLE'

    sessionRunning = False
    sessionLayers = {}
    projectionSet = False

    isGrassInstalled = False

    @staticmethod
    def grassBatchJobFilename():
        '''This is used in Linux. This is the batch job that we assign to
        GRASS_BATCH_JOB and then call GRASS and let it do the work
        '''
        filename = 'grass_batch_job.sh'
        batchfile = userFolder() + os.sep + filename
        return batchfile

    @staticmethod
    def grassScriptFilename():
        '''This is used in windows. We create a script that initializes
        GRASS and then uses grass commands
        '''
        filename = 'grass_script.bat'
        filename = userFolder() + os.sep + filename
        return filename

    @staticmethod
    def getGrassVersion():
        # I do not know if this should be removed or let the user enter it
        # or something like that... This is just a temporary thing
        return '6.4.0'

    @staticmethod
    def grassPath():
        if not isWindows() and not isMac():
            return ''

        folder = ProcessingConfig.getSetting(GrassUtils.GRASS_FOLDER)
        if folder is None:
            if isWindows():
                testfolder = os.path.dirname(QgsApplication.prefixPath())
                testfolder = os.path.join(testfolder, 'grass')
                if os.path.isdir(testfolder):
                    for subfolder in os.listdir(testfolder):
                        if subfolder.startswith('grass'):
                            folder = os.path.join(testfolder, subfolder)
                            break
            else:
                folder = os.path.join(QgsApplication.prefixPath(), 'grass')
                if not os.path.isdir(folder):
                    folder = '/Applications/GRASS-6.4.app/Contents/MacOS'

        return folder or ''

    @staticmethod
    def grassWinShell():
        folder = ProcessingConfig.getSetting(GrassUtils.GRASS_WIN_SHELL)
        if folder is None:
            folder = os.path.dirname(str(QgsApplication.prefixPath()))
            folder = os.path.join(folder, 'msys')
        return folder

    @staticmethod
    def grassDescriptionPath():
        return os.path.join(os.path.dirname(__file__), 'description')

    @staticmethod
    def createGrassScript(commands):
        folder = GrassUtils.grassPath()
        shell = GrassUtils.grassWinShell()

        script = GrassUtils.grassScriptFilename()
        gisrc = userFolder() + os.sep + 'processing.gisrc'

        # Temporary gisrc file
        output = codecs.open(gisrc, 'w', encoding='utf-8')
        location = 'temp_location'
        gisdbase = GrassUtils.grassDataFolder()

        output.write('GISDBASE: ' + gisdbase + '\n')
        output.write('LOCATION_NAME: ' + location + '\n')
        output.write('MAPSET: PERMANENT \n')
        output.write('GRASS_GUI: text\n')
        output.close()

        output = codecs.open(script, 'w', encoding='utf-8')
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
        output.write('set GRASS_VERSION=' + GrassUtils.getGrassVersion()
                + '\n')
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
            output.write(command + '\n')
        output.write('\n')
        output.write('exit\n')
        output.close()

    @staticmethod
    def createGrassBatchJobFileFromGrassCommands(commands):
        fout = codecs.open(GrassUtils.grassBatchJobFilename(), 'w', encoding='utf-8')
        for command in commands:
            fout.write(command + '\n')
        fout.write('exit')
        fout.close()

    @staticmethod
    def grassMapsetFolder():
        folder = os.path.join(GrassUtils.grassDataFolder(), 'temp_location')
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

        folder = GrassUtils.grassMapsetFolder()
        mkdir(os.path.join(folder, 'PERMANENT'))
        mkdir(os.path.join(folder, 'PERMANENT', '.tmp'))
        GrassUtils.writeGrassWindow(os.path.join(folder, 'PERMANENT',
                                    'DEFAULT_WIND'))
        outfile = codecs.open(os.path.join(folder, 'PERMANENT', 'MYNAME'), 'w', encoding='utf-8')
        outfile.write(
            'QGIS GRASS interface: temporary data processing location.\n')
        outfile.close()
        GrassUtils.writeGrassWindow(os.path.join(folder, 'PERMANENT', 'WIND'))
        mkdir(os.path.join(folder, 'PERMANENT', 'dbf'))
        outfile = codecs.open(os.path.join(folder, 'PERMANENT', 'VAR'), 'w', encoding='utf-8')
        outfile.write('DB_DRIVER: dbf\n')
        outfile.write('DB_DATABASE: $GISDBASE/$LOCATION_NAME/$MAPSET/dbf/\n')
        outfile.close()

    @staticmethod
    def writeGrassWindow(filename):
        out = codecs.open(filename, 'w', encoding='utf-8')
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
    def prepareGrassExecution(commands):
        if isWindows():
            GrassUtils.createGrassScript(commands)
            command = ['cmd.exe', '/C ', GrassUtils.grassScriptFilename()]
        else:
            gisrc = userFolder() + os.sep + 'processing.gisrc'
            os.putenv('GISRC', gisrc)
            os.putenv('GRASS_MESSAGE_FORMAT', 'gui')
            os.putenv('GRASS_BATCH_JOB', GrassUtils.grassBatchJobFilename())
            GrassUtils.createGrassBatchJobFileFromGrassCommands(commands)
            os.chmod(GrassUtils.grassBatchJobFilename(), stat.S_IEXEC
                     | stat.S_IREAD | stat.S_IWRITE)
            if isMac():
                command = GrassUtils.grassPath() + os.sep + 'grass.sh ' \
                    + GrassUtils.grassMapsetFolder() + '/PERMANENT'
            else:
                command = 'grass64 ' + GrassUtils.grassMapsetFolder() \
                    + '/PERMANENT'

        return command

    @staticmethod
    def executeGrass(commands, progress, outputCommands=None):
        loglines = []
        loglines.append('GRASS execution console output')
        grassOutDone = False
        command = GrassUtils.prepareGrassExecution(commands)
        proc = subprocess.Popen(
            command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=open(os.devnull),
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            ).stdout
        progress.setInfo('GRASS commands output:')
        for line in iter(proc.readline, ''):
            if 'GRASS_INFO_PERCENT' in line:
                try:
                    progress.setPercentage(int(line[len('GRASS_INFO_PERCENT')
                            + 2:]))
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
            command = GrassUtils.prepareGrassExecution(outputCommands)
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

        if ProcessingConfig.getSetting(GrassUtils.GRASS_LOG_CONSOLE):
            ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
        return loglines

    # GRASS session is used to hold the layers already exported or
    # produced in GRASS between multiple calls to GRASS algorithms.
    # This way they don't have to be loaded multiple times and
    # following algorithms can use the results of the previous ones.
    # Starting a session just involves creating the temp mapset
    # structure
    @staticmethod
    def startGrassSession():
        if not GrassUtils.sessionRunning:
            GrassUtils.createTempMapset()
            GrassUtils.sessionRunning = True

    # End session by removing the temporary GRASS mapset and all
    # the layers.
    @staticmethod
    def endGrassSession():
        shutil.rmtree(GrassUtils.grassMapsetFolder(), True)
        GrassUtils.sessionRunning = False
        GrassUtils.sessionLayers = {}
        GrassUtils.projectionSet = False

    @staticmethod
    def getSessionLayers():
        return GrassUtils.sessionLayers

    @staticmethod
    def addSessionLayers(exportedLayers):
        GrassUtils.sessionLayers = dict(GrassUtils.sessionLayers.items()
                + exportedLayers.items())

    @staticmethod
    def checkGrassIsInstalled(ignorePreviousState=False):
        if isWindows():
            path = GrassUtils.grassPath()
            if path == '':
                return GrassUtils.tr(
                    'GRASS folder is not configured.\nPlease configure '
                    'it before running GRASS algorithms.')
            cmdpath = os.path.join(path, 'bin', 'r.out.gdal.exe')
            if not os.path.exists(cmdpath):
                return GrassUtils.tr(
                    'The specified GRASS folder does not contain a valid '
                    'set of GRASS modules. Please, go to the Processing '
                    'settings dialog, and check that the GRASS folder is '
                    'correctly configured')

        if not ignorePreviousState:
            if GrassUtils.isGrassInstalled:
                return
        try:
            from processing import runalg
            result = runalg(
                'grass:v.voronoi',
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
                return GrassUtils.tr(
                    'It seems that GRASS is not correctly installed and '
                    'configured in your system.\nPlease install it before '
                    'running GRASS algorithms.')
        except:
            return GrassUtils.tr(
                'Error while checking GRASS installation. GRASS might not '
                'be correctly configured.\n')

        GrassUtils.isGrassInstalled = True

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'GrassUtils'
        return QCoreApplication.translate(context, string)
