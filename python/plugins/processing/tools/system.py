# -*- coding: utf-8 -*-

"""
***************************************************************************
    system.py
    ---------
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
import time
import sys
import uuid
import io
import subprocess

from PyQt4.QtCore import QDir
from qgis.core import QgsApplication

numExported = 1


def userFolder():
    userDir = os.path.join(QgsApplication.qgisSettingsDirPath(), 'processing')
    if not QDir(userDir).exists():
        QDir().mkpath(userDir)

    return unicode(QDir.toNativeSeparators(userDir))


def defaultOutputFolder():
    folder = os.path.join(userFolder(), "outputs")
    if not QDir(folder).exists():
        QDir().mkpath(folder)

    return unicode(QDir.toNativeSeparators(folder))


def isWindows():
    return os.name == 'nt'


def isMac():
    return sys.platform == 'darwin'

_tempFolderSuffix = unicode(uuid.uuid4()).replace('-', '')


def tempFolder():
    tempDir = os.path.join(unicode(QDir.tempPath()), 'processing' + _tempFolderSuffix)
    if not QDir(tempDir).exists():
        QDir().mkpath(tempDir)

    return unicode(os.path.abspath(tempDir))


def setTempOutput(out, alg):
    if hasattr(out, 'directory'):
        out.value = getTempDirInTempFolder()
    else:
        ext = out.getDefaultFileExtension(alg)
        out.value = getTempFilenameInTempFolder(out.name + '.' + ext)


def getTempFilename(ext=None):
    path = tempFolder()
    if ext is None:
        filename = path + os.sep + unicode(time.time()) \
            + unicode(getNumExportedLayers())
    else:
        filename = path + os.sep + unicode(time.time()) \
            + unicode(getNumExportedLayers()) + '.' + ext
    return filename


def getTempFilenameInTempFolder(basename):
    """Returns a temporary filename for a given file, putting it into
    a temp folder but not changing its basename.
    """

    path = tempFolder()
    path = os.path.join(path, unicode(uuid.uuid4()).replace('-', ''))
    mkdir(path)
    basename = removeInvalidChars(basename)
    filename = os.path.join(path, basename)
    return filename


def getTempDirInTempFolder():
    """Returns a temporary directory, putting it into a temp folder.
    """

    path = tempFolder()
    path = os.path.join(path, unicode(uuid.uuid4()).replace('-', ''))
    mkdir(path)
    return path


def removeInvalidChars(string):
    validChars = \
        'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.'
    string = ''.join(c for c in string if c in validChars)
    return string


def getNumExportedLayers():
    global numExported
    numExported += 1
    return numExported


def mkdir(newdir):
    newdir = newdir.strip('\n\r ')
    if os.path.isdir(newdir):
        pass
    else:
        (head, tail) = os.path.split(newdir)
        if head and not os.path.isdir(head):
            mkdir(head)
        if tail:
            os.mkdir(newdir)


def findWindowsCmdEncoding():
    """ Find MS-Windows encoding in the shell (cmd.exe)"""
    # Creates a temp python file
    tempPython = getTempFilenameInTempFolder('cmdEncoding.py')
    with io.open(tempPython, 'w', encoding='utf-8') as f:
        command = (u'# -*- coding: utf-8 -*-\n\n'
                   u'import sys\n'
                   u'print(sys.stdin.encoding)')
        f.write(command)

    env = os.environ.copy()
    command = ['cmd.exe', '/C', 'python.exe', tempPython]

    # execute temp python file
    p = subprocess.Popen(
        command,
        shell=True,
        stdout=subprocess.PIPE,
        stdin=open(os.devnull),
        stderr=subprocess.STDOUT,
        universal_newlines=True,
        env=env)
    data = p.communicate()[0]

    # Return codepage
    if p.returncode == 0:
        return data

    # If our Python script fails, use locale
    import locale
    return locale.getpreferredencoding()
