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

import os
import re
import shutil
import stat
import subprocess
import sys
from dataclasses import (
    dataclass,
    field
)
from pathlib import Path
from typing import (
    Optional,
    List,
    Dict
)

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsProcessingUtils,
    QgsMessageLog,
    QgsCoordinateReferenceSystem,
    QgsProcessingContext,
)

from processing.algs.gdal.GdalUtils import GdalUtils
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import userFolder, isWindows, isMac, mkdir


class GrassUtils:
    GRASS_REGION_XMIN = 'GRASS7_REGION_XMIN'
    GRASS_REGION_YMIN = 'GRASS7_REGION_YMIN'
    GRASS_REGION_XMAX = 'GRASS7_REGION_XMAX'
    GRASS_REGION_YMAX = 'GRASS7_REGION_YMAX'
    GRASS_REGION_CELLSIZE = 'GRASS7_REGION_CELLSIZE'
    GRASS_LOG_COMMANDS = 'GRASS7_LOG_COMMANDS'
    GRASS_LOG_CONSOLE = 'GRASS7_LOG_CONSOLE'
    GRASS_HELP_URL = 'GRASS_HELP_URL'
    GRASS_USE_REXTERNAL = 'GRASS_USE_REXTERNAL'
    GRASS_USE_VEXTERNAL = 'GRASS_USE_VEXTERNAL'

    # TODO Review all default options formats
    GRASS_RASTER_FORMATS_CREATEOPTS = {
        'GTiff': 'TFW=YES,COMPRESS=LZW',
        'PNG': 'ZLEVEL=9',
        'WEBP': 'QUALITY=85'
    }

    sessionRunning = False
    sessionLayers = {}
    projectionSet = False

    isGrassInstalled = False

    version = None
    path = None
    command = None

    @staticmethod
    def grassBatchJobFilename():
        """
        The Batch file is executed by GRASS binary.
        On GNU/Linux and MacOSX it will be executed by a shell.
        On MS-Windows, it will be executed by cmd.exe.
        """
        gisdbase = GrassUtils.grassDataFolder()
        if isWindows():
            batchFile = os.path.join(gisdbase, 'grass_batch_job.cmd')
        else:
            batchFile = os.path.join(gisdbase, 'grass_batch_job.sh')
        return batchFile

    @staticmethod
    def exportCrsWktToFile(crs, context: QgsProcessingContext):
        """
        Exports a crs as a WKT definition to a text file, and returns the path
        to this file
        """
        wkt = crs.toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED)
        wkt_file = QgsProcessingUtils.generateTempFilename('crs.prj', context)
        with open(wkt_file, 'w', encoding='utf-8') as f:
            f.write(wkt)
        return wkt_file

    @staticmethod
    def installedVersion(run=False):
        """
        Returns the installed version of GRASS by
        launching the GRASS command with -v parameter.
        """
        if GrassUtils.isGrassInstalled and not run:
            return GrassUtils.version

        if GrassUtils.grassBin() is None:
            return None

        # Launch GRASS command with -v parameter
        # For MS-Windows, hide the console
        if isWindows():
            si = subprocess.STARTUPINFO()
            si.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            si.wShowWindow = subprocess.SW_HIDE
        with subprocess.Popen(
            [GrassUtils.command, '-v'],
            shell=False,
            stdout=subprocess.PIPE,
            stdin=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            startupinfo=si if isWindows() else None
        ) as proc:
            try:
                lines = proc.stdout.readlines()
                for line in lines:
                    if "GRASS GIS " in line:
                        line = line.split(" ")[-1].strip()
                        if line.startswith("7.") or line.startswith("8."):
                            GrassUtils.version = line
                            return GrassUtils.version
            except Exception:
                pass

        return None

    @staticmethod
    def grassBin():
        """
        Find GRASS binary path on the operating system.
        Sets global variable GrassUtils.command
        """

        def searchFolder(folder):
            """
            Inline function to search for grass binaries into a folder
            with os.walk
            """
            if os.path.exists(folder):
                for root, dirs, files in os.walk(folder):
                    for cmd in cmdList:
                        if cmd in files:
                            return os.path.join(root, cmd)
            return None

        if GrassUtils.command:
            return GrassUtils.command

        path = GrassUtils.grassPath()
        command = None

        vn = os.path.join(path, "etc", "VERSIONNUMBER")
        if os.path.isfile(vn):
            with open(vn) as f:
                major, minor, patch = f.readlines()[0].split(' ')[0].split('.')
                if patch != 'svn':
                    patch = ''
                cmdList = [
                    "grass{}{}{}".format(major, minor, patch),
                    "grass",
                    "grass{}{}{}.{}".format(major, minor, patch,
                                            "bat" if isWindows() else "sh"),
                    "grass.{}".format("bat" if isWindows() else "sh")
                ]
        else:
            cmdList = ["grass80", "grass78", "grass76", "grass74", "grass72",
                       "grass70", "grass"]
            cmdList.extend(
                ["{}.{}".format(b, "bat" if isWindows() else "sh") for b in
                 cmdList])

        # For MS-Windows there is a difference between GRASS Path and GRASS binary
        if isWindows():
            # If nothing found, use OSGEO4W or QgsPrefix:
            if "OSGEO4W_ROOT" in os.environ:
                testFolder = str(os.environ['OSGEO4W_ROOT'])
            else:
                testFolder = str(QgsApplication.prefixPath())
            testFolder = os.path.join(testFolder, 'bin')
            command = searchFolder(testFolder)
        elif isMac():
            # Search in grassPath
            command = searchFolder(path)

        # If everything has failed, use shutil (but not for Windows as it'd include .)
        if not command and not isWindows():
            for cmd in cmdList:
                testBin = shutil.which(cmd)
                if testBin:
                    command = os.path.abspath(testBin)
                    break

        if command:
            GrassUtils.command = command
            if path == '':
                GrassUtils.path = os.path.dirname(command)

        return command

    @staticmethod
    def grassPath():
        """
        Find GRASS path on the operating system.
        Sets global variable GrassUtils.path
        """
        if GrassUtils.path is not None:
            return GrassUtils.path

        if not isWindows() and not isMac():
            return ''

        folder = None
        # Under MS-Windows, we use GISBASE or QGIS Path for folder
        if isWindows():
            if "GISBASE" in os.environ:
                folder = os.environ["GISBASE"]
            else:
                testfolder = os.path.join(
                    os.path.dirname(QgsApplication.prefixPath()), 'grass')
                if os.path.isdir(testfolder):
                    grassfolders = sorted([f for f in os.listdir(testfolder) if
                                           f.startswith(
                                               "grass-7.") and os.path.isdir(
                                               os.path.join(testfolder, f))],
                                          reverse=True,
                                          key=lambda x: [int(v) for v in x[
                                              len("grass-"):].split(
                                              '.') if v != 'svn'])
                    if grassfolders:
                        folder = os.path.join(testfolder, grassfolders[0])
        elif isMac():
            # For MacOSX, first check environment
            if "GISBASE" in os.environ:
                folder = os.environ["GISBASE"]
            else:
                # Find grass folder if it exists inside QGIS bundle
                for version in ['', '8', '7', '80', '78', '76', '74', '72',
                                '71', '70']:
                    testfolder = os.path.join(str(QgsApplication.prefixPath()),
                                              'grass{}'.format(version))
                    if os.path.isdir(testfolder):
                        folder = testfolder
                        break
                    # If nothing found, try standalone GRASS installation
                    if folder is None:
                        for version in ['8', '6', '4', '2', '1', '0']:
                            testfolder = '/Applications/GRASS-7.{}.app/Contents/MacOS'.format(
                                version)
                            if os.path.isdir(testfolder):
                                folder = testfolder
                                break

        if folder is not None:
            GrassUtils.path = folder

        return folder or ''

    @staticmethod
    def userDescriptionFolder():
        """
        Creates and returns a directory for users to create additional algorithm descriptions.
        Or modified versions of stock algorithm descriptions shipped with QGIS.
        """
        folder = Path(userFolder(), 'grassaddons', 'description')
        folder.mkdir(parents=True, exist_ok=True)
        return folder

    @staticmethod
    def grassDescriptionFolders():
        """
        Returns the directories to search for algorithm descriptions.
        Note that the provider will load from these in sequence, so we put the userDescriptionFolder first
        to allow users to create modified versions of stock algorithms shipped with QGIS.
        """
        return [GrassUtils.userDescriptionFolder(),
                Path(__file__).parent.joinpath('description')]

    @staticmethod
    def getWindowsCodePage():
        """
        Determines MS-Windows CMD.exe shell codepage.
        Used into GRASS exec script under MS-Windows.
        """
        from ctypes import cdll
        return str(cdll.kernel32.GetACP())

    @staticmethod
    def createGrassBatchJobFileFromGrassCommands(commands):
        with open(GrassUtils.grassBatchJobFilename(), 'w') as fout:
            if not isWindows():
                fout.write('#!/bin/sh\n')
            else:
                fout.write(
                    'chcp {}>NUL\n'.format(GrassUtils.getWindowsCodePage()))
            for command in commands:
                GrassUtils.writeCommand(fout, command)
            fout.write('exit')

    @staticmethod
    def grassMapsetFolder():
        """
        Creates and returns the GRASS temporary DB LOCATION directory.
        """
        folder = os.path.join(GrassUtils.grassDataFolder(), 'temp_location')
        mkdir(folder)
        return folder

    @staticmethod
    def grassDataFolder():
        """
        Creates and returns the GRASS temporary DB directory.
        """
        tempfolder = os.path.normpath(
            os.path.join(QgsProcessingUtils.tempFolder(), 'grassdata'))
        mkdir(tempfolder)
        return tempfolder

    @staticmethod
    def createTempMapset():
        """
        Creates a temporary location and mapset(s) for GRASS data
        processing. A minimal set of folders and files is created in the
        system's default temporary directory. The settings files are
        written with sane defaults, so GRASS can do its work. The mapset
        projection will be set later, based on the projection of the first
        input image or vector
        """
        folder = GrassUtils.grassMapsetFolder()
        mkdir(os.path.join(folder, 'PERMANENT'))
        mkdir(os.path.join(folder, 'PERMANENT', '.tmp'))
        GrassUtils.writeGrassWindow(
            os.path.join(folder, 'PERMANENT', 'DEFAULT_WIND'))
        with open(os.path.join(folder, 'PERMANENT', 'MYNAME'), 'w') as outfile:
            outfile.write(
                'QGIS GRASS GIS interface: temporary data processing location.\n')

        GrassUtils.writeGrassWindow(os.path.join(folder, 'PERMANENT', 'WIND'))
        mkdir(os.path.join(folder, 'PERMANENT', 'sqlite'))
        with open(os.path.join(folder, 'PERMANENT', 'VAR'), 'w') as outfile:
            outfile.write('DB_DRIVER: sqlite\n')
            outfile.write(
                'DB_DATABASE: $GISDBASE/$LOCATION_NAME/$MAPSET/sqlite/sqlite.db\n')

    @staticmethod
    def writeGrassWindow(filename):
        """
        Creates the GRASS Window file
        """
        with open(filename, 'w') as out:
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

    @staticmethod
    def prepareGrassExecution(commands):
        """
        Prepare GRASS batch job in a script and
        returns it as a command ready for subprocess.
        """
        if GrassUtils.command is None:
            GrassUtils.grassBin()

        env = os.environ.copy()
        env['GRASS_MESSAGE_FORMAT'] = 'plain'
        if 'GISBASE' in env:
            del env['GISBASE']
        GrassUtils.createGrassBatchJobFileFromGrassCommands(commands)
        os.chmod(GrassUtils.grassBatchJobFilename(),
                 stat.S_IEXEC | stat.S_IREAD | stat.S_IWRITE)
        command = [GrassUtils.command,
                   os.path.join(GrassUtils.grassMapsetFolder(), 'PERMANENT'),
                   '--exec', GrassUtils.grassBatchJobFilename()]

        return command, env

    @staticmethod
    def executeGrass(commands, feedback, outputCommands=None):
        loglines = [GrassUtils.tr('GRASS GIS execution console output')]
        grassOutDone = False
        command, grassenv = GrassUtils.prepareGrassExecution(commands)
        # QgsMessageLog.logMessage('exec: {}'.format(command), 'DEBUG', Qgis.Info)

        # For MS-Windows, we need to hide the console window.
        kw = {}
        if isWindows():
            si = subprocess.STARTUPINFO()
            si.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            si.wShowWindow = subprocess.SW_HIDE
            kw['startupinfo'] = si
            if sys.version_info >= (3, 6):
                kw['encoding'] = "cp{}".format(
                    GrassUtils.getWindowsCodePage())

        def readline_with_recover(stdout):
            """A method wrapping stdout.readline() with try-except recovering.
            detailed in https://github.com/qgis/QGIS/pull/49226
            Args:
                stdout: io.TextIOWrapper - proc.stdout

            Returns:
                str: read line or replaced text when recovered
            """
            try:
                return stdout.readline()
            except UnicodeDecodeError:
                return ''  # replaced-text

        with subprocess.Popen(
            command,
            shell=False,
            stdout=subprocess.PIPE,
            stdin=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            env=grassenv,
            **kw
        ) as proc:
            for line in iter(lambda: readline_with_recover(proc.stdout), ''):
                if 'GRASS_INFO_PERCENT' in line:
                    try:
                        feedback.setProgress(
                            int(line[len('GRASS_INFO_PERCENT') + 2:]))
                    except Exception:
                        pass
                else:
                    if 'r.out' in line or 'v.out' in line:
                        grassOutDone = True
                    loglines.append(line)
                    if any([l in line for l in ['WARNING', 'ERROR']]):
                        feedback.reportError(line.strip())
                    elif 'Segmentation fault' in line:
                        feedback.reportError(line.strip())
                        feedback.reportError('\n' + GrassUtils.tr(
                            'GRASS command crashed :( Try a different set of input parameters and consult the GRASS algorithm manual for more information.') + '\n')
                        if ProcessingConfig.getSetting(
                                GrassUtils.GRASS_USE_REXTERNAL):
                            feedback.reportError(GrassUtils.tr(
                                'Suggest disabling the experimental "use r.external" option from the Processing GRASS Provider options.') + '\n')
                        if ProcessingConfig.getSetting(
                                GrassUtils.GRASS_USE_VEXTERNAL):
                            feedback.reportError(GrassUtils.tr(
                                'Suggest disabling the experimental "use v.external" option from the Processing GRASS Provider options.') + '\n')
                    elif line.strip():
                        feedback.pushConsoleInfo(line.strip())

        # Some GRASS scripts, like r.mapcalculator or r.fillnulls, call
        # other GRASS scripts during execution. This may override any
        # commands that are still to be executed by the subprocess, which
        # are usually the output ones. If that is the case runs the output
        # commands again.
        if not grassOutDone and outputCommands:
            command, grassenv = GrassUtils.prepareGrassExecution(
                outputCommands)
            # For MS-Windows, we need to hide the console window.
            kw = {}
            if isWindows():
                si = subprocess.STARTUPINFO()
                si.dwFlags |= subprocess.STARTF_USESHOWWINDOW
                si.wShowWindow = subprocess.SW_HIDE
                kw['startupinfo'] = si
                if sys.version_info >= (3, 6):
                    kw['encoding'] = "cp{}".format(
                        GrassUtils.getWindowsCodePage())
            with subprocess.Popen(
                command,
                shell=False,
                stdout=subprocess.PIPE,
                stdin=subprocess.DEVNULL,
                stderr=subprocess.STDOUT,
                universal_newlines=True,
                env=grassenv,
                **kw
            ) as proc:
                for line in iter(lambda: readline_with_recover(proc.stdout),
                                 ''):
                    if 'GRASS_INFO_PERCENT' in line:
                        try:
                            feedback.setProgress(int(
                                line[len('GRASS_INFO_PERCENT') + 2:]))
                        except Exception:
                            pass
                    if any(l in line for l in ['WARNING', 'ERROR']):
                        loglines.append(line.strip())
                        feedback.reportError(line.strip())
                    elif line.strip():
                        loglines.append(line.strip())
                        feedback.pushConsoleInfo(line.strip())

        if ProcessingConfig.getSetting(GrassUtils.GRASS_LOG_CONSOLE):
            QgsMessageLog.logMessage('\n'.join(loglines), 'Processing',
                                     Qgis.MessageLevel.Info)

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
        # shutil.rmtree(GrassUtils.grassMapsetFolder(), True)
        GrassUtils.sessionRunning = False
        GrassUtils.sessionLayers = {}
        GrassUtils.projectionSet = False

    @staticmethod
    def getSessionLayers():
        return GrassUtils.sessionLayers

    @staticmethod
    def addSessionLayers(exportedLayers):
        GrassUtils.sessionLayers = dict(
            list(GrassUtils.sessionLayers.items()) +
            list(exportedLayers.items()))

    @staticmethod
    def checkGrassIsInstalled(ignorePreviousState=False):
        if not ignorePreviousState:
            if GrassUtils.isGrassInstalled:
                return

        # We check the version of Grass
        if GrassUtils.installedVersion() is not None:
            # For Ms-Windows, we check GRASS binaries
            if isWindows():
                cmdpath = os.path.join(GrassUtils.path, 'bin',
                                       'r.out.gdal.exe')
                if not os.path.exists(cmdpath):
                    return GrassUtils.tr(
                        'The GRASS GIS folder "{}" does not contain a valid set '
                        'of GRASS modules.\nPlease, check that GRASS is correctly '
                        'installed and available on your system.'.format(
                            os.path.join(GrassUtils.path, 'bin')))
            GrassUtils.isGrassInstalled = True
            return
        # Return error messages
        else:
            # MS-Windows or MacOSX
            if isWindows() or isMac():
                if GrassUtils.path is None:
                    return GrassUtils.tr(
                        'Could not locate GRASS GIS folder. Please make '
                        'sure that GRASS GIS is correctly installed before '
                        'running GRASS algorithms.')
                if GrassUtils.command is None:
                    return GrassUtils.tr(
                        'GRASS GIS binary {} can\'t be found on this system from a shell. '
                        'Please install it or configure your PATH {} environment variable.'.format(
                            '(grass.bat)' if isWindows() else '(grass.sh)',
                            'or OSGEO4W_ROOT' if isWindows() else ''))
            # GNU/Linux
            else:
                return GrassUtils.tr(
                    'GRASS can\'t be found on this system from a shell. '
                    'Please install it or configure your PATH environment variable.')

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'Grass7Utils'
        return QCoreApplication.translate(context, string)

    @staticmethod
    def writeCommand(output, command):
        try:
            # Python 2
            output.write(command.encode('utf8') + '\n')
        except TypeError:
            # Python 3
            output.write(command + '\n')

    @staticmethod
    def grassHelpPath():
        helpPath = ProcessingConfig.getSetting(GrassUtils.GRASS_HELP_URL)

        if not helpPath:
            if isWindows() or isMac():
                if GrassUtils.path is not None:
                    localPath = os.path.join(GrassUtils.path, 'docs/html')
                    if os.path.exists(localPath):
                        helpPath = os.path.abspath(localPath)
            else:
                searchPaths = ['/usr/share/doc/grass-doc/html',
                               '/opt/grass/docs/html',
                               '/usr/share/doc/grass/docs/html']
                for path in searchPaths:
                    if os.path.exists(path):
                        helpPath = os.path.abspath(path)
                        break

        if helpPath:
            return helpPath
        elif GrassUtils.version:
            version = GrassUtils.version.replace('.', '')[:2]
            return 'https://grass.osgeo.org/grass{}/manuals/'.format(version)
        else:
            # GRASS not available!
            return 'https://grass.osgeo.org/grass-stable/manuals/'

    @staticmethod
    def getSupportedOutputRasterExtensions():
        # We use the same extensions than GDAL because:
        # - GRASS is also using GDAL for raster imports.
        # - Chances that GRASS is compiled with another version of
        # GDAL than QGIS are very limited!
        return GdalUtils.getSupportedOutputRasterExtensions()

    @staticmethod
    def getRasterFormatFromFilename(filename):
        """
        Returns Raster format name from a raster filename.
        :param filename: The name with extension of the raster.
        :return: The Gdal short format name for extension.
        """
        ext = os.path.splitext(filename)[1].lower()
        ext = ext.lstrip('.')
        if ext:
            supported = GdalUtils.getSupportedRasters()
            for name in list(supported.keys()):
                exts = supported[name]
                if ext in exts:
                    return name
        return 'GTiff'
