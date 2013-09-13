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
from processing.tools.general import removeInvalidChars

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import time
import sys
import uuid
from PyQt4.QtCore import *
from qgis.core import *


def userFolder():
    userDir = QFileInfo(QgsApplication.qgisUserDbFilePath()).path() + "/processing"
    if not QDir(userDir).exists():
        QDir().mkpath(userDir)

    return unicode(QDir.toNativeSeparators(userDir))

def isWindows():
    return os.name =="nt"

def isMac():
    return sys.platform == "darwin"

def tempFolder():
    tempDir = os.path.join(unicode(QDir.tempPath()), "processing")
    if not QDir(tempDir).exists():
        QDir().mkpath(tempDir)

    return unicode(os.path.abspath(tempDir))

def setTempOutput(out, alg):
    ext = out.getDefaultFileExtension(alg)
    out.value = getTempFilenameInTempFolder(out.name + "." + ext)

def getTempFilename(ext):
    path = tempFolder()
    if ext is None:
        filename = path + os.sep + str(time.time()) + str(getNumExportedLayers())
    else:
        filename = path + os.sep + str(time.time()) + str(getNumExportedLayers()) + "." + ext
    return filename

def getTempFilenameInTempFolder(basename):
    '''returns a temporary filename for a given file, putting it into a temp folder but not changing its basename'''
    path = tempFolder()
    path = os.path.join(path, str(uuid.uuid4()).replace("-",""))
    mkdir(path)
    basename = removeInvalidChars(basename)
    filename =  os.path.join(path, basename)
    return filename

NUM_EXPORTED = 1

def getNumExportedLayers():
    NUM_EXPORTED += 1
    return NUM_EXPORTED

def mkdir(newdir):
    if os.path.isdir(newdir):
        pass
    else:
        head, tail = os.path.split(newdir)
        if head and not os.path.isdir(head):
            mkdir(head)
        if tail:
            os.mkdir(newdir)
