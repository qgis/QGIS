# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalUtils.py
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
import subprocess
import platform
from PyQt4.QtCore import *
from qgis.core import QgsApplication
from processing.core.ProcessingLog import ProcessingLog

try:
    from osgeo import gdal
    gdalAvailable = True
except:
    gdalAvailable = False


class GdalUtils:

    supportedRasters = None

    @staticmethod
    def runGdal(commands, progress):
        envval = unicode(os.getenv('PATH'))
        # We need to give some extra hints to get things picked up on OS X
        if platform.system() == 'Darwin':
            if os.path.isfile(os.path.join(QgsApplication.prefixPath(), "bin", "gdalinfo")):
                # Looks like there's a bundled gdal. Let's use it.
                os.environ['PATH'] = "%s%s%s" % (os.path.join(QgsApplication.prefixPath(), "bin"), os.pathsep, envval)
                os.environ['DYLD_LIBRARY_PATH'] = os.path.join(QgsApplication.prefixPath(), "lib")
            else:
                # Nothing internal. Let's see if we can find it elsewhere.
                settings = QSettings()
                path = unicode(settings.value('/GdalTools/gdalPath', ''))
                envval += '%s%s' % (os.pathsep, path)
                os.putenv('PATH', envval)
        else:
            # Other platforms should use default gdal finder codepath
            settings = QSettings()
            path = unicode(settings.value('/GdalTools/gdalPath', ''))
            if not path.lower() in envval.lower().split(os.pathsep):
                envval += '%s%s' % (os.pathsep, path)
                os.putenv('PATH', envval)

        loglines = []
        loglines.append('GDAL execution console output')
        fused_command = ''.join(['%s ' % c for c in commands])
        proc = subprocess.Popen(
            fused_command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=open(os.devnull),
            stderr=subprocess.STDOUT,
            universal_newlines=False,
            ).stdout
        for line in iter(proc.readline, ''):
            loglines.append(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
        GdalUtils.consoleOutput = loglines

    @staticmethod
    def getConsoleOutput():
        return GdalUtils.consoleOutput

    @staticmethod
    def getSupportedRasters():
        if not gdalAvailable:
            return {}

        if GdalUtils.supportedRasters is not None:
            return GdalUtils.supportedRasters

        if gdal.GetDriverCount() == 0:
            gdal.AllRegister()

        GdalUtils.supportedRasters = {}
        GdalUtils.supportedRasters['GTiff'] = ['tif']
        for i in range(gdal.GetDriverCount()):
            driver = gdal.GetDriver(i)
            if driver is None:
                continue
            shortName = driver.ShortName
            metadata = driver.GetMetadata()
            #===================================================================
            # if gdal.DCAP_CREATE not in metadata \
            #         or metadata[gdal.DCAP_CREATE] != 'YES':
            #     continue
            #===================================================================
            if gdal.DMD_EXTENSION in metadata:
                extensions = metadata[gdal.DMD_EXTENSION].split('/')
                if extensions:
                    GdalUtils.supportedRasters[shortName] = extensions

        return GdalUtils.supportedRasters

    @staticmethod
    def getSupportedRasterExtensions():
        allexts = ['tif']
        for exts in GdalUtils.getSupportedRasters().values():
            for ext in exts:
                if ext not in allexts and ext != '':
                    allexts.append(ext)
        return allexts

    @staticmethod
    def getFormatShortNameFromFilename(filename):
        ext = filename[filename.rfind('.') + 1:]
        supported = GdalUtils.getSupportedRasters()
        for name in supported.keys():
            exts = supported[name]
            if ext in exts:
                return name
        return 'GTiff'

    @staticmethod
    def escapeAndJoin(strList):
        joined = ''
        for s in strList:
            if s[0] != '-' and ' ' in s:
                escaped = '"' + s.replace('\\', '\\\\').replace('"', '\\"') \
                    + '"'
            else:
                escaped = s
            joined += escaped + ' '
        return joined.strip()
