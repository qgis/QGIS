# -*- coding: utf-8 -*-

"""
***************************************************************************
    OtbUtils.py
    -----------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya
                           Julien Malik, Oscar Picas  (CS SI) - add functions to manage xml tree
                           Alexia Mondot (CS SI) - add a trick for OTBApplication SplitImages
                           Rashad Kanavath (CS SI) - re-integration of provider to QGIS
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import sys
import re
import subprocess

from processing.core.ProcessingConfig import ProcessingConfig
from qgis.core import (Qgis, QgsApplication, QgsMessageLog)
from qgis.PyQt.QtCore import QCoreApplication


class OtbUtils:
    # Path to otb installation folder (string, directory).
    FOLDER = "OTB_FOLDER"

    # Path to otb application folder. multiple paths are supported (string, directory).
    APP_FOLDER = "OTB_APP_FOLDER"

    # A string to hold current version number. Useful for bug reporting.
    VERSION = "OTB_VERSION"

    # Default directory were DEM tiles are stored. It should only contain ```.hgt`` or or georeferenced ``.tif`` files. Empty if not set (no directory set).
    SRTM_FOLDER = "OTB_SRTM_FOLDER"

    # Default path to the geoid file that will be used to retrieve height of DEM above ellipsoid. Empty if not set (no geoid set).
    GEOID_FILE = "OTB_GEOID_FILE"

    # Default maximum memory that OTB should use for processing, in MB. If not set, default value is 128 MB.
    # This is set through environment variable ``OTB_MAX_RAM_HINT``
    MAX_RAM_HINT = 'OTB_MAX_RAM_HINT'

    # ``OTB_LOGGER_LEVEL``: Default level of logging for OTB. Should be one of ``DEBUG``, ``INFO``, ``WARNING``, ``CRITICAL`` or ``FATAL``, by increasing order of priority. Only messages with a higher priority than the level of logging will be displayed. If not set, default level is ``INFO``.
    LOGGER_LEVEL = 'OTB_LOGGER_LEVEL'

    @staticmethod
    def settingNames():
        return [
            OtbUtils.FOLDER,
            OtbUtils.SRTM_FOLDER,
            OtbUtils.GEOID_FILE,
            OtbUtils.LOGGER_LEVEL,
            OtbUtils.MAX_RAM_HINT
        ]

    @staticmethod
    def version():
        return ProcessingConfig.getSetting(OtbUtils.VERSION) or '0.0.0'

    @staticmethod
    def loggerLevel():
        return ProcessingConfig.getSetting(OtbUtils.LOGGER_LEVEL) or 'INFO'

    @staticmethod
    def maxRAMHint():
        return ProcessingConfig.getSetting(OtbUtils.MAX_RAM_HINT) or ''

    @staticmethod
    def otbFolder():
        if ProcessingConfig.getSetting(OtbUtils.FOLDER):
            return os.path.normpath(os.sep.join(re.split(r'\\|/', ProcessingConfig.getSetting(OtbUtils.FOLDER))))
        else:
            return None

    @staticmethod
    def appFolder():
        app_folder = ProcessingConfig.getSetting(OtbUtils.APP_FOLDER)
        if app_folder:
            return os.pathsep.join(app_folder.split(';'))
        else:
            return None

    @staticmethod
    def srtmFolder():
        return ProcessingConfig.getSetting(OtbUtils.SRTM_FOLDER) or ''

    @staticmethod
    def geoidFile():
        return ProcessingConfig.getSetting(OtbUtils.GEOID_FILE) or ''

    @staticmethod
    def getExecutableInPath(path, exe):
        ext = '.exe' if os.name == 'nt' else ''
        return os.path.join(path, 'bin', exe + ext)

    @staticmethod
    def getAuxiliaryDataDirectories():
        gdal_data_dir = None
        gtiff_csv_dir = None
        proj_dir = None
        otb_folder = OtbUtils.otbFolder()
        if os.name == 'nt':
            gdal_data_dir = os.path.join(otb_folder, 'share', 'data')
            gtiff_csv_dir = os.path.join(otb_folder, 'share', 'epsg_csv')
            proj_dir = os.path.join(otb_folder, 'share', 'proj')
        else:
            env_profile = os.path.join(otb_folder, 'otbenv.profile')
            try:
                if os.path.exists(env_profile):
                    with open(env_profile) as f:
                        lines = f.readlines()
                        lines = [x.strip() for x in lines]
                        for line in lines:
                            if not line or line.startswith('#'):
                                continue
                            if 'GDAL_DATA=' in line:
                                gdal_data_dir = line.split("GDAL_DATA=")[1]
                            if 'GEOTIFF_CSV=' in line:
                                gtiff_csv_dir = line.split("GEOTIFF_CSV=")[1]
                            if 'PROJ_LIB=' in line:
                                proj_dir = line.split("PROJ_LIB=")[1]
            except BaseException as exc:
                errmsg = 'Cannot find gdal and geotiff data directory.' + str(exc)
                QgsMessageLog.logMessage(errmsg, OtbUtils.tr('Processing'), Qgis.Info)
        return gdal_data_dir, gtiff_csv_dir, proj_dir

    @staticmethod
    def executeOtb(commands, feedback, addToLog=True):
        otb_env = {
            'LC_NUMERIC': 'C',
            'GDAL_DRIVER_PATH': 'disable'
        }
        gdal_data_dir, gtiff_csv_dir, proj_dir = OtbUtils.getAuxiliaryDataDirectories()
        if gdal_data_dir and os.path.exists(gdal_data_dir):
            otb_env['GDAL_DATA'] = gdal_data_dir
        if gtiff_csv_dir and os.path.exists(gtiff_csv_dir):
            otb_env['GEOTIFF_CSV'] = gtiff_csv_dir
        if proj_dir and os.path.exists(proj_dir):
            otb_env['PROJ_LIB'] = proj_dir

        otb_env['OTB_LOGGER_LEVEL'] = OtbUtils.loggerLevel()
        max_ram_hint = OtbUtils.maxRAMHint()
        if max_ram_hint and int(max_ram_hint) > 256:
            otb_env['OTB_MAX_RAM_HINT'] = max_ram_hint

        kw = {'env': otb_env}
        if os.name == 'nt' and sys.version_info >= (3, 6):
            kw['encoding'] = "cp{}".format(OtbUtils.getWindowsCodePage())

        QgsMessageLog.logMessage("{}".format(kw), OtbUtils.tr('Processing'), Qgis.Info)
        QgsMessageLog.logMessage("cmd={}".format(commands), OtbUtils.tr('Processing'), Qgis.Info)
        with subprocess.Popen(
            commands,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            **kw
        ) as proc:

            for line in iter(proc.stdout.readline, ''):
                line = line.strip()
                # '* ]' and '  ]' says its some progress update
                if '% [' in line:
                    part = line.split(':')[1]
                    percent = part.split('%')[0]
                    try:
                        if int(percent) >= 100:
                            feedback.pushConsoleInfo(line)
                        feedback.setProgress(int(percent))
                    except Exception:
                        pass
                else:
                    if feedback is None:
                        QgsMessageLog.logMessage(line, OtbUtils.tr('Processing'), Qgis.Info)
                    else:
                        if any(l in line for l in ['(WARNING)', 'WARNING:']):
                            feedback.reportError(line, False)
                        elif any(l in line for l in ['(FATAL)', 'ERROR:', 'ERROR']):
                            feedback.reportError(line, True)
                        else:
                            feedback.pushConsoleInfo(line.strip())

    @staticmethod
    def getWindowsCodePage():
        """
        Determines MS-Windows CMD.exe shell codepage.
        Used into GRASS exec script under MS-Windows.
        """
        from ctypes import cdll
        return str(cdll.kernel32.GetACP())

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'OtbUtils'
        return QCoreApplication.translate(context, string)
