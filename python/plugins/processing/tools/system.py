# -*- coding: utf-8 -*-

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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import time
import sys
import uuid
import math

from qgis.PyQt.QtCore import QDir
from qgis.core import (QgsApplication,
                       QgsProcessingUtils)

numExported = 1


def userFolder():
    userDir = os.path.join(QgsApplication.qgisSettingsDirPath(), 'processing')
    if not QDir(userDir).exists():
        QDir().mkpath(userDir)

    return str(QDir.toNativeSeparators(userDir))


def defaultOutputFolder():
    folder = os.path.join(userFolder(), 'outputs')
    if not QDir(folder).exists():
        QDir().mkpath(folder)

    return str(QDir.toNativeSeparators(folder))


def isWindows():
    return os.name == 'nt'


def isMac():
    return sys.platform == 'darwin'


def getTempFilename(ext=None):
    tmpPath = QgsProcessingUtils.tempFolder()
    t = time.time()
    m = math.floor(t)
    uid = '{:8x}{:05x}'.format(m, int((t - m) * 1000000))
    if ext is None:
        filename = os.path.join(tmpPath, '{}{}'.format(uid, getNumExportedLayers()))
    else:
        filename = os.path.join(tmpPath, '{}{}.{}'.format(uid, getNumExportedLayers(), ext))
    return filename


def getNumExportedLayers():
    global numExported
    numExported += 1
    return numExported


def mkdir(newdir):
    os.makedirs(newdir.strip('\n\r '), exist_ok=True)


def tempHelpFolder():
    tmp = os.path.join(str(QDir.tempPath()), 'processing_help')
    if not QDir(tmp).exists():
        QDir().mkpath(tmp)

    return str(os.path.abspath(tmp))


def escapeAndJoin(strList):
    """
    .. deprecated:: 3.0
    Do not use, will be removed in QGIS 4.0
    """

    from warnings import warn
    warn("processing.escapeAndJoin is deprecated and will be removed in QGIS 4.0", DeprecationWarning)

    joined = ''
    for s in strList:
        if s[0] != '-' and ' ' in s:
            escaped = '"' + s.replace('\\', '\\\\').replace('"', '\\"') \
                + '"'
        else:
            escaped = s
        joined += escaped + ' '
    return joined.strip()
