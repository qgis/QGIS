# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptUtils.py
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

from qgis.processing import alg as algfactory
import os
import inspect
import importlib.util

from qgis.PyQt.QtCore import QCoreApplication, QDir

from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessingAlgorithm,
                       QgsProcessingFeatureBasedAlgorithm,
                       QgsMessageLog
                       )

from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import mkdir, userFolder

scriptsRegistry = {}

SCRIPTS_FOLDERS = "SCRIPTS_FOLDERS"


def defaultScriptsFolder():
    folder = str(os.path.join(userFolder(), "scripts"))
    mkdir(folder)
    return os.path.abspath(folder)


def scriptsFolders():
    folder = ProcessingConfig.getSetting(SCRIPTS_FOLDERS)
    if folder is not None:
        return folder.split(";")
    else:
        return [defaultScriptsFolder()]


def loadAlgorithm(moduleName, filePath):
    global scriptsRegistry

    try:
        spec = importlib.util.spec_from_file_location(moduleName, filePath)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        try:
            alg = algfactory.instances.pop().createInstance()
            scriptsRegistry[alg.name()] = filePath
            return alg
        except IndexError:
            for x in dir(module):
                obj = getattr(module, x)
                if inspect.isclass(obj) and issubclass(obj, (QgsProcessingAlgorithm, QgsProcessingFeatureBasedAlgorithm)) and obj.__name__ not in ("QgsProcessingAlgorithm", "QgsProcessingFeatureBasedAlgorithm"):
                    o = obj()
                    scriptsRegistry[o.name()] = filePath
                    return o
    except (ImportError, AttributeError, TypeError) as e:
        QgsMessageLog.logMessage(QCoreApplication.translate("ScriptUtils", "Could not import script algorithm '{}' from '{}'\n{}").format(moduleName, filePath, str(e)),
                                 QCoreApplication.translate("ScriptUtils", "Processing"),
                                 Qgis.Critical)


def findAlgorithmSource(name):
    global scriptsRegistry
    try:
        return scriptsRegistry[name]
    except:
        return None


def resetScriptFolder(folder):
    """Check if script folder exist. If not, notify and try to check if it is absolute to another user setting.
    If so, modify folder to change user setting to the current user setting."""

    newFolder = folder
    if os.path.exists(newFolder):
        return newFolder

    QgsMessageLog.logMessage(QgsApplication .translate("loadAlgorithms", "Script folder {} does not exist").format(newFolder),
                             QgsApplication.translate("loadAlgorithms", "Processing"),
                             Qgis.Warning)

    if not os.path.isabs(newFolder):
        return None

    # try to check if folder is absolute to other QgsApplication.qgisSettingsDirPath()

    # isolate "QGIS3/profiles/"
    appIndex = -4
    profileIndex = -3
    currentSettingPath = QDir.toNativeSeparators(QgsApplication.qgisSettingsDirPath())
    paths = currentSettingPath.split(os.sep)
    commonSettingPath = os.path.join(paths[appIndex], paths[profileIndex])

    if commonSettingPath in newFolder:
        # strip not common folder part. e.g. preserve the profile path
        # stripping the heading part that come from another location
        tail = newFolder[newFolder.find(commonSettingPath):]
        # tail folder with the actual userSetting path
        header = os.path.join(os.sep, os.path.join(*paths[:appIndex]))
        newFolder = os.path.join(header, tail)

        # skip if it does not exist
        if not os.path.exists(newFolder):
            return None

        QgsMessageLog.logMessage(QgsApplication .translate("loadAlgorithms", "Script folder changed into {}").format(newFolder),
                                 QgsApplication.translate("loadAlgorithms", "Processing"),
                                 Qgis.Warning)

    return newFolder
