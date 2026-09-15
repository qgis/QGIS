"""
***************************************************************************
    py
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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import math
import os
import time
from typing import Optional

from qgis.core import QgsApplication, QgsProcessingContext, QgsProcessingUtils
from qgis.PyQt.QtCore import QDir

numExported = 1


def userFolder():
    userDir = os.path.join(QgsApplication.qgisSettingsDirPath(), "processing")
    if not QDir(userDir).exists():
        QDir().mkpath(userDir)

    return str(QDir.toNativeSeparators(userDir))


def defaultOutputFolder():
    folder = os.path.join(QDir.homePath(), "processing")
    return str(QDir.toNativeSeparators(folder))


def getTempFilename(ext=None, context: Optional[QgsProcessingContext] = None):
    tmpPath = QgsProcessingUtils.tempFolder(context)
    t = time.time()
    m = math.floor(t)
    uid = f"{m:8x}{int((t - m) * 1000000):05x}"
    if ext is None:
        filename = os.path.join(tmpPath, f"{uid}{getNumExportedLayers()}")
    else:
        filename = os.path.join(tmpPath, f"{uid}{getNumExportedLayers()}.{ext}")
    return filename


def getNumExportedLayers():
    global numExported
    numExported += 1
    return numExported


def mkdir(newdir):
    os.makedirs(newdir.strip("\n\r "), exist_ok=True)
