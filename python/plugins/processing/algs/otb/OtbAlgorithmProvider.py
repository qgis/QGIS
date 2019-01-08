# -*- coding: utf-8 -*-

"""
/***************************************************************************
    OtbAlgorithmProvider.py
    -----------------------
    Date                 : 2018-01-30
    Copyright            : (C) 2018 by CNES
    Email                : rashad dot kanavath at c-s fr
****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

__author__ = 'Rashad Kanavath'
__date__ = '2018-01-30'
__copyright__ = '(C) 2018 by CNES'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import re
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (Qgis, QgsApplication, QgsProcessingProvider, QgsMessageLog)
from qgis import utils

from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.algs.otb import OtbUtils
from processing.algs.otb.OtbSettings import OtbSettings
from processing.algs.otb.OtbAlgorithm import OtbAlgorithm


def otb_exe_file(f):
    if os.name == 'nt':
        return f + '.exe'
    else:
        return f


class OtbAlgorithmProvider(QgsProcessingProvider):
    def __init__(self):
        super().__init__()
        self.algs = []
        #!hack for 6.6!#
        self.version = '6.6.0'
        self.descriptionFile = ''

    def load(self):
        group = self.name()
        ProcessingConfig.settingIcons[group] = self.icon()
        ProcessingConfig.addSetting(Setting(group, OtbSettings.ACTIVATE, self.tr('Activate'), True))
        ProcessingConfig.addSetting(Setting(group, OtbSettings.FOLDER,
                                            self.tr("OTB folder"),
                                            OtbUtils.otbFolder(),
                                            valuetype=Setting.FOLDER,
                                            validator=self.validateOtbFolder
                                            ))
        ProcessingConfig.addSetting(Setting(group, OtbSettings.APP_FOLDER,
                                            self.tr("OTB application folder"),
                                            OtbUtils.appFolder(),
                                            valuetype=Setting.MULTIPLE_FOLDERS,
                                            validator=self.validateAppFolders
                                            ))
        ProcessingConfig.addSetting(Setting(group, OtbSettings.SRTM_FOLDER,
                                            self.tr("SRTM tiles folder"),
                                            OtbUtils.srtmFolder(),
                                            valuetype=Setting.FOLDER
                                            ))
        ProcessingConfig.addSetting(Setting(group, OtbSettings.GEOID_FILE,
                                            self.tr("Geoid file"),
                                            OtbUtils.geoidFile(),
                                            valuetype=Setting.FOLDER
                                            ))
        ProcessingConfig.addSetting(Setting(group, OtbSettings.MAX_RAM_HINT,
                                            self.tr("Maximum RAM to use"),
                                            OtbUtils.maxRAMHint(),
                                            valuetype=Setting.STRING
                                            ))
        ProcessingConfig.addSetting(Setting(group, OtbSettings.LOGGER_LEVEL,
                                            self.tr("Logger level"),
                                            OtbUtils.loggerLevel(),
                                            valuetype=Setting.STRING,
                                            validator=self.validateLoggerLevel
                                            ))
        ProcessingConfig.readSettings()
        self.refreshAlgorithms()
        return True

    def unload(self):
        for setting in OtbSettings.keys():
            ProcessingConfig.removeSetting(setting)

    def isActive(self):
        return ProcessingConfig.getSetting(OtbSettings.ACTIVATE)

    def setActive(self, active):
        ProcessingConfig.setSettingValue(OtbSettings.ACTIVATE, active)

    def createAlgsList(self):
        algs = []
        try:
            folder = OtbUtils.otbFolder()
            alg_names = []
            algs_txt = self.algsFile(folder)
            with open(algs_txt) as lines:
                line = lines.readline().strip('\n').strip()
                if line != '' and line.startswith('#'):
                    line = lines.readline().strip('\n').strip()
                while line != '' and not line.startswith('#'):
                    data = line.split('|')
                    self.descriptionFile = self.descrFile(folder, str(data[1]) + '.txt')
                    group, name = str(data[0]), str(data[1])
                    if name not in alg_names:
                        algs.append(OtbAlgorithm(group, name, self.descriptionFile))
                        #avoid duplicate algorithms from algs.txt file (possible but rare)
                        alg_names.append(name)
                    line = lines.readline().strip('\n').strip()
        except Exception as e:
            import traceback
            errmsg = "Could not open OTB algorithm from file: \n" + self.descriptionFile + "\nError:\n" + traceback.format_exc()
            QgsMessageLog.logMessage(self.tr(errmsg), self.tr('Processing'), Qgis.Critical)
        return algs

    def loadAlgorithms(self):
        if not self.canBeActivated():
            return

        version_file = os.path.join(OtbUtils.otbFolder(), 'share', 'doc', 'otb', 'VERSION')
        if not os.path.isfile(version_file):
            version_file = os.path.join(OtbUtils.otbFolder(), 'VERSION')

        if os.path.isfile(version_file):
            with open(version_file) as vf:
                vlines = vf.readlines()
                vlines = [l.strip() for l in vlines]
                vline = vlines[0]
                if 'OTB Version:' in vline:
                    self.version = vline.split(':')[1].strip()

        QgsMessageLog.logMessage(self.tr("Loading OTB '{}'.".format(self.version)), self.tr('Processing'), Qgis.Info)
        self.algs = self.createAlgsList()
        for a in self.algs:
            self.addAlgorithm(a)
        self.algs = []

        otb_folder = self.normalize_path(OtbUtils.otbFolder())
        otb_app_path_env = os.pathsep.join(self.appDirs(OtbUtils.appFolder()))
        gdal_data_dir = None
        geotiff_csv_dir = None
        otbcli_path = OtbUtils.cliPath()
        try:
            if os.name == 'nt':
                app_vargs = " %*"
                export_cmd = 'SET '
                first_line = ':: Setup environment for OTB package. Generated by QGIS plugin'
                otb_app_launcher = os.path.join(otb_folder, 'bin', 'otbApplicationLauncherCommandLine.exe')
                gdal_data_dir = os.path.join(otb_folder, 'share', 'data')
                geotiff_csv_dir = os.path.join(otb_folder, 'share', 'epsg_csv')
            else:
                app_vargs = " \"$@\""
                export_cmd = 'export '
                first_line = '#!/bin/sh'
                otb_app_launcher = os.path.join(otb_folder, 'bin', 'otbApplicationLauncherCommandLine')
                lines = None
                env_profile = os.path.join(otb_folder, 'otbenv.profile')
                if os.path.exists(env_profile):
                    with open(env_profile) as f:
                        lines = f.readlines()
                        lines = [x.strip() for x in lines]
                        for line in lines:
                            if not line or line.startswith('#'):
                                continue
                            if 'GDAL_DATA=' in line:
                                gdal_data_dir = line.split("GDAL_DATA=")[1]
                            if 'GEOTIFF_CSV='in line:
                                geotiff_csv_dir = line.split("GEOTIFF_CSV=")[1]
            with open(otbcli_path, 'w') as otb_cli_file:
                otb_cli_file.write(first_line + os.linesep)
                otb_cli_file.write(export_cmd + "LC_NUMERIC=C" + os.linesep)
                otb_cli_file.write(export_cmd + "GDAL_DRIVER_PATH=disable" + os.linesep)
                if gdal_data_dir:
                    otb_cli_file.write(export_cmd + "GDAL_DATA=" + "\"" + gdal_data_dir + "\"" + os.linesep)
                if geotiff_csv_dir:
                    otb_cli_file.write(export_cmd + "GEOTIFF_CSV=" + "\"" + geotiff_csv_dir + "\"" + os.linesep)
                if OtbUtils.loggerLevel():
                    otb_cli_file.write(export_cmd + "OTB_LOGGER_LEVEL=" + OtbUtils.loggerLevel() + os.linesep)
                max_ram_hint = OtbUtils.maxRAMHint()
                if max_ram_hint and not int(max_ram_hint) == 128:
                    otb_cli_file.write(export_cmd + "OTB_MAX_RAM_HINT=" + max_ram_hint + os.linesep)
                otb_cli_file.write(export_cmd + "OTB_APPLICATION_PATH=" + "\"" + otb_app_path_env + "\"" + os.linesep)
                otb_cli_file.write("\"" + otb_app_launcher + "\"" + app_vargs + os.linesep)

            if not os.name == 'nt':
                os.chmod(otbcli_path, 0o744)
        except BaseException as e:
            import traceback
            os.remove(otbcli_path)
            errmsg = "Cannot write:" + otbcli_path + "\nError:\n" + traceback.format_exc()
            QgsMessageLog.logMessage(self.tr(errmsg), self.tr('Processing'), Qgis.Critical)
            raise e
        QgsMessageLog.logMessage(self.tr("Using otbcli: '{}'.".format(otbcli_path)), self.tr('Processing'), Qgis.Info)

    def canBeActivated(self):
        if not self.isActive():
            return False
        folder = OtbUtils.otbFolder()
        if folder and os.path.exists(folder):
            if os.path.isfile(self.algsFile(folder)):
                return True
            utils.iface.messageBar().pushWarning("OTB", "Cannot find '{}'. OTB provider will be disabled".format(self.algsFile(folder)))
        self.setActive(False)
        return False

    def validateLoggerLevel(self, v):
        allowed_values = ['DEBUG', 'INFO', 'WARNING', 'CRITICAL', 'FATAL']
        if v in allowed_values:
            return True
        else:
            raise ValueError(self.tr("'{}' is not valid. Possible values are '{}'".format(v, ', '.join(allowed_values))))

    def validateAppFolders(self, v):
        if not self.isActive():
            return
        if not v:
            self.setActive(False)
            raise ValueError(self.tr('Cannot activate OTB provider'))

        folder = OtbUtils.otbFolder()
        otb_app_dirs = self.appDirs(v)
        if len(otb_app_dirs) < 1:
            self.setActive(False)
            raise ValueError(self.tr("'{}' does not exist. OTB provider will be disabled".format(v)))

        #isValid is True if there is atleast one valid otb application is given path
        isValid = False
        descr_folder = self.descrFolder(folder)
        for app_dir in otb_app_dirs:
            if not os.path.exists(app_dir):
                continue
            for otb_app in os.listdir(app_dir):
                if not otb_app.startswith('otbapp_') or \
                    'TestApplication' in otb_app or \
                        'ApplicationExample' in otb_app:
                    continue
                app_name = os.path.basename(otb_app).split('.')[0][7:]
                dfile = os.path.join(descr_folder, app_name + '.txt')
                isValid = True
                if not os.path.exists(dfile):
                    cmdlist = [os.path.join(
                        folder, 'bin',
                        otb_exe_file('otbQgisDescriptor')),
                        app_name, app_dir, descr_folder + '/']
                    commands = ' '.join(cmdlist)
                    QgsMessageLog.logMessage(self.tr(commands), self.tr('Processing'), Qgis.Critical)
                    OtbUtils.executeOtb(commands, feedback=None)

        if isValid:
            # if check needed for testsing
            if utils.iface is not None:
                utils.iface.messageBar().pushInfo("OTB", "OTB provider is activated from '{}'.".format(folder))
        else:
            self.setActive(False)
            raise ValueError(self.tr("No OTB algorithms found in '{}'. OTB will be disabled".format(','.join(otb_app_dirs))))

    def normalize_path(self, p):
        # https://stackoverflow.com/a/20713238/1003090
        return os.path.normpath(os.sep.join(re.split(r'\\|/', p)))

    def validateOtbFolder(self, v):
        if not self.isActive():
            return
        if not v or not os.path.exists(v):
            self.setActive(False)
            raise ValueError(self.tr("'{}' does not exist. OTB provider will be disabled".format(v)))
        path = self.normalize_path(v)
        if not os.path.exists(os.path.join(path, 'bin', otb_exe_file('otbApplicationLauncherCommandLine'))):
            self.setActive(False)
            raise ValueError(self.tr("Cannot find '{}'. OTB will be disabled".format(os.path.join(v, 'bin', otb_exe_file('otbApplicationLauncherCommandLine')))))

    def algsFile(self, d):
        return os.path.join(self.descrFolder(d), 'algs.txt')

    def descrFolder(self, d):
        #!hack for 6.6!#
        if os.path.exists(os.path.join(d, 'description')):
            return os.path.join(d, 'description')
        else:
            return os.path.join(d, 'share', 'otb', 'description')

    def descrFile(self, d, f):
        return os.path.join(self.descrFolder(d), f)

    def appDirs(self, v):
        #!hack needed for QGIS < 3.2!#
        v = v.replace(';', os.pathsep)
        #!hack needed for QGIS < 3.2!#
        folders = v.split(os.pathsep)
        app_dirs = []
        for f in folders:
            if f is not None and os.path.exists(f):
                app_dirs.append(self.normalize_path(f))
        return app_dirs

    def name(self):
        return 'OTB'

    def longName(self):
        return 'OTB ({})'.format(self.version) if self.version is not None else 'OTB'

    def id(self):
        return 'otb'

    def supportsNonFileBasedOutput(self):
        """
        OTB Provider doesn't support non file based outputs
        """
        return False

    def icon(self):
        return QgsApplication.getThemeIcon("/providerOtb.png")

    def tr(self, string, context=''):
        if context == '':
            context = 'OtbAlgorithmProvider'
        return QCoreApplication.translate(context, string)

    def defaultVectorFileExtension(self, hasGeometry=True):
        return 'shp'

    def defaultRasterFileExtension(self):
        return 'tif'

    def supportedOutputTableExtensions(self):
        return ['dbf']
