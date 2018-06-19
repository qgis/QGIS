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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import inspect
import importlib

from qgis.PyQt.QtCore import QCoreApplication

from qgis.core import (Qgis,
                       QgsProcessingAlgorithm,
                       QgsProcessingFeatureBasedAlgorithm,
                       QgsMessageLog
                       )

from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import mkdir, userFolder

scriptsRegistry = dict()

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
        return [ScriptUtils.defaultScriptsFolder()]


def loadAlgorithm(moduleName, filePath):
    global scriptsRegistry

    try:
        spec = importlib.util.spec_from_file_location(moduleName, filePath)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        for x in dir(module):
            obj = getattr(module, x)
            if inspect.isclass(obj) and issubclass(obj, (QgsProcessingAlgorithm, QgsProcessingFeatureBasedAlgorithm)) and obj.__name__ not in ("QgsProcessingAlgorithm", "QgsProcessingFeatureBasedAlgorithm"):
                scriptsRegistry[x] = filePath
                return obj()
    except ImportError as e:
        QgsMessageLog.logMessage(QCoreApplication.translate("ScriptUtils", "Could not import script algorithm '{}' from '{}'\n{}").format(moduleName, filePath, str(e)),
                                 QCoreApplication.translate("ScriptUtils", "Processing"),
                                 Qgis.Critical)


def findAlgorithmSource(className):
    global scriptsRegistry
    try:
        return scriptsRegistry[className]
    except:
        return None
