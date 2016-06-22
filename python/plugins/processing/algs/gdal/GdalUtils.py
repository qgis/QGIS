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

from osgeo import gdal

from qgis.PyQt.QtCore import QSettings
from qgis.core import QgsApplication, QgsVectorFileWriter
from processing.core.ProcessingLog import ProcessingLog
from processing.core.SilentProgress import SilentProgress

try:
    from osgeo import gdal
    gdalAvailable = True
except:
    gdalAvailable = False


class GdalUtils:

    supportedRasters = None

    @staticmethod
    def runGdal(commands, progress=None):
        if progress is None:
            progress = SilentProgress()
        envval = os.getenv('PATH')
        # We need to give some extra hints to get things picked up on OS X
        isDarwin = False
        try:
            isDarwin = platform.system() == 'Darwin'
        except IOError: # https://travis-ci.org/m-kuhn/QGIS#L1493-L1526
            pass
        if isDarwin and os.path.isfile(os.path.join(QgsApplication.prefixPath(), "bin", "gdalinfo")):
            # Looks like there's a bundled gdal. Let's use it.
            os.environ['PATH'] = "{}{}{}".format(os.path.join(QgsApplication.prefixPath(), "bin"), os.pathsep, envval)
            os.environ['DYLD_LIBRARY_PATH'] = os.path.join(QgsApplication.prefixPath(), "lib")
        else:
            # Other platforms should use default gdal finder codepath
            settings = QSettings()
            path = settings.value('/GdalTools/gdalPath', '')
            if not path.lower() in envval.lower().split(os.pathsep):
                envval += '{}{}'.format(os.pathsep, path)
                os.putenv('PATH', envval)

        fused_command = ' '.join([unicode(c) for c in commands])
        progress.setInfo('GDAL command:')
        progress.setCommand(fused_command)
        progress.setInfo('GDAL command output:')
        success = False
        retry_count = 0
        while success == False:
            loglines = []
            loglines.append('GDAL execution console output')
            try:
                proc = subprocess.Popen(
                    fused_command,
                    shell=True,
                    stdout=subprocess.PIPE,
                    stdin=open(os.devnull),
                    stderr=subprocess.STDOUT,
                    universal_newlines=True,
                ).stdout
                for line in proc:
                    progress.setConsoleInfo(line)
                    loglines.append(line)
                success = True
            except IOError as e:
                if retry_count < 5:
                    retry_count += 1
                else:
                    raise IOError(e.message + u'\nTried 5 times without success. Last iteration stopped after reading {} line(s).\nLast line(s):\n{}'.format(len(loglines), u'\n'.join(loglines[-10:])))

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
    def getVectorDriverFromFileName(filename):
        ext = os.path.splitext(filename)[1]
        if ext == '':
            return 'ESRI Shapefile'

        formats = QgsVectorFileWriter.supportedFiltersAndFormats()
        for k, v in formats.iteritems():
            if ext in k:
                return v
        return 'ESRI Shapefile'

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

    @staticmethod
    def version():
        return int(gdal.VersionInfo('VERSION_NUM'))
