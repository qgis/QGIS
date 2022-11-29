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

import os
import subprocess
import platform
import re
import warnings

import psycopg2

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    from osgeo import ogr

from qgis.core import (Qgis,
                       QgsBlockingProcess,
                       QgsRunProcess,
                       QgsApplication,
                       QgsVectorFileWriter,
                       QgsProcessingFeedback,
                       QgsProcessingUtils,
                       QgsMessageLog,
                       QgsSettings,
                       QgsCredentials,
                       QgsDataSourceUri,
                       QgsProjUtils,
                       QgsCoordinateReferenceSystem,
                       QgsProcessingException)

from qgis.PyQt.QtCore import (
    QCoreApplication,
    QProcess
)

from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import isWindows, isMac

try:
    with warnings.catch_warnings():
        warnings.filterwarnings("ignore", category=DeprecationWarning)
        from osgeo import gdal  # NOQA

    gdalAvailable = True
except:
    gdalAvailable = False


class GdalUtils:
    GDAL_HELP_PATH = 'GDAL_HELP_PATH'

    supportedRasters = None
    supportedOutputRasters = None

    @staticmethod
    def runGdal(commands, feedback=None):
        if feedback is None:
            feedback = QgsProcessingFeedback()
        envval = os.getenv('PATH')
        # We need to give some extra hints to get things picked up on OS X
        isDarwin = False
        try:
            isDarwin = platform.system() == 'Darwin'
        except IOError:  # https://travis-ci.org/m-kuhn/QGIS#L1493-L1526
            pass
        if isDarwin and os.path.isfile(os.path.join(QgsApplication.prefixPath(), "bin", "gdalinfo")):
            # Looks like there's a bundled gdal. Let's use it.
            os.environ['PATH'] = "{}{}{}".format(os.path.join(QgsApplication.prefixPath(), "bin"), os.pathsep, envval)
            os.environ['DYLD_LIBRARY_PATH'] = os.path.join(QgsApplication.prefixPath(), "lib")
        else:
            # Other platforms should use default gdal finder codepath
            settings = QgsSettings()
            path = settings.value('/GdalTools/gdalPath', '')
            if not path.lower() in envval.lower().split(os.pathsep):
                envval += '{}{}'.format(os.pathsep, path)
                os.putenv('PATH', envval)

        fused_command = ' '.join([str(c) for c in commands])
        QgsMessageLog.logMessage(fused_command, 'Processing', Qgis.Info)
        feedback.pushInfo(GdalUtils.tr('GDAL command:'))
        feedback.pushCommandInfo(fused_command)
        feedback.pushInfo(GdalUtils.tr('GDAL command output:'))

        loglines = [GdalUtils.tr('GDAL execution console output')]
        # create string list of number from 0 to 99
        progress_string_list = [str(a) for a in range(0, 100)]

        def on_stdout(ba):
            val = ba.data().decode('UTF-8')
            # catch progress reports
            if val == '100 - done.':
                on_stdout.progress = 100
                feedback.setProgress(on_stdout.progress)
            else:
                # remove any number of trailing "." or ".." strings
                match = re.match(r'.*?(\d+)\.+\s*$', val)
                found_number = False
                if match:
                    int_val = match.group(1)
                    if int_val in progress_string_list:
                        on_stdout.progress = int(int_val)
                        feedback.setProgress(on_stdout.progress)
                        found_number = True

                if not found_number and val == '.':
                    on_stdout.progress += 2.5
                    feedback.setProgress(on_stdout.progress)

            on_stdout.buffer += val
            if on_stdout.buffer.endswith('\n') or on_stdout.buffer.endswith('\r'):
                # flush buffer
                feedback.pushConsoleInfo(on_stdout.buffer.rstrip())
                loglines.append(on_stdout.buffer.rstrip())
                on_stdout.buffer = ''

        on_stdout.progress = 0
        on_stdout.buffer = ''

        def on_stderr(ba):
            val = ba.data().decode('UTF-8')
            on_stderr.buffer += val

            if on_stderr.buffer.endswith('\n') or on_stderr.buffer.endswith('\r'):
                # flush buffer
                feedback.reportError(on_stderr.buffer.rstrip())
                loglines.append(on_stderr.buffer.rstrip())
                on_stderr.buffer = ''

        on_stderr.buffer = ''

        command, *arguments = QgsRunProcess.splitCommand(fused_command)
        proc = QgsBlockingProcess(command, arguments)
        proc.setStdOutHandler(on_stdout)
        proc.setStdErrHandler(on_stderr)

        res = proc.run(feedback)
        if feedback.isCanceled() and res != 0:
            feedback.pushInfo(GdalUtils.tr('Process was canceled and did not complete'))
        elif not feedback.isCanceled() and proc.exitStatus() == QProcess.CrashExit:
            raise QgsProcessingException(GdalUtils.tr('Process was unexpectedly terminated'))
        elif res == 0:
            feedback.pushInfo(GdalUtils.tr('Process completed successfully'))
        elif proc.processError() == QProcess.FailedToStart:
            raise QgsProcessingException(GdalUtils.tr('Process {} failed to start. Either {} is missing, or you may have insufficient permissions to run the program.').format(command, command))
        else:
            feedback.reportError(GdalUtils.tr('Process returned error code {}').format(res))

        return loglines

    @staticmethod
    def getSupportedRasters():
        if not gdalAvailable:
            return {}

        if GdalUtils.supportedRasters is not None:
            return GdalUtils.supportedRasters

        if gdal.GetDriverCount() == 0:
            gdal.AllRegister()

        GdalUtils.supportedRasters = {}
        GdalUtils.supportedOutputRasters = {}
        GdalUtils.supportedRasters['GTiff'] = ['tif', 'tiff']
        GdalUtils.supportedOutputRasters['GTiff'] = ['tif', 'tiff']

        for i in range(gdal.GetDriverCount()):
            driver = gdal.GetDriver(i)
            if driver is None:
                continue
            shortName = driver.ShortName
            metadata = driver.GetMetadata()
            if gdal.DCAP_RASTER not in metadata \
                    or metadata[gdal.DCAP_RASTER] != 'YES':
                continue

            if gdal.DMD_EXTENSIONS in metadata:
                extensions = metadata[gdal.DMD_EXTENSIONS].split(' ')
                if extensions:
                    GdalUtils.supportedRasters[shortName] = extensions
                    # Only creatable rasters can be referenced in output rasters
                    if ((gdal.DCAP_CREATE in metadata and
                         metadata[gdal.DCAP_CREATE] == 'YES') or
                        (gdal.DCAP_CREATECOPY in metadata and
                            metadata[gdal.DCAP_CREATECOPY] == 'YES')):
                        GdalUtils.supportedOutputRasters[shortName] = extensions

        return GdalUtils.supportedRasters

    @staticmethod
    def getSupportedOutputRasters():
        if not gdalAvailable:
            return {}

        if GdalUtils.supportedOutputRasters is not None:
            return GdalUtils.supportedOutputRasters
        else:
            GdalUtils.getSupportedRasters()

        return GdalUtils.supportedOutputRasters

    @staticmethod
    def getSupportedRasterExtensions():
        allexts = []
        for exts in list(GdalUtils.getSupportedRasters().values()):
            for ext in exts:
                if ext not in allexts and ext not in ['', 'tif', 'tiff']:
                    allexts.append(ext)
        allexts.sort()
        allexts[0:0] = ['tif', 'tiff']
        return allexts

    @staticmethod
    def getSupportedOutputRasterExtensions():
        allexts = []
        for exts in list(GdalUtils.getSupportedOutputRasters().values()):
            for ext in exts:
                if ext not in allexts and ext not in ['', 'tif', 'tiff']:
                    allexts.append(ext)
        allexts.sort()
        allexts[0:0] = ['tif', 'tiff']
        return allexts

    @staticmethod
    def getVectorDriverFromFileName(filename):
        ext = os.path.splitext(filename)[1]
        if ext == '':
            return 'ESRI Shapefile'

        formats = QgsVectorFileWriter.supportedFiltersAndFormats()
        for format in formats:
            if ext in format.filterString:
                return format.driverName
        return 'ESRI Shapefile'

    @staticmethod
    def getFormatShortNameFromFilename(filename):
        ext = filename[filename.rfind('.') + 1:]
        supported = GdalUtils.getSupportedRasters()
        for name in list(supported.keys()):
            exts = supported[name]
            if ext in exts:
                return name
        return 'GTiff'

    @staticmethod
    def escapeAndJoin(strList):
        escChars = [' ', '&', '(', ')', '"', ';']
        joined = ''
        for s in strList:
            if not isinstance(s, str):
                s = str(s)
            # don't escape if command starts with - and isn't a negative number, e.g. -9999
            if s and re.match(r'^([^-]|-\d)', s) and any(c in s for c in escChars):
                escaped = '"' + s.replace('\\', '\\\\').replace('"', '"""') \
                          + '"'
            else:
                escaped = s
            if escaped is not None:
                joined += escaped + ' '
        return joined.strip()

    @staticmethod
    def version():
        return int(gdal.VersionInfo('VERSION_NUM'))

    @staticmethod
    def readableVersion():
        return gdal.VersionInfo('RELEASE_NAME')

    @staticmethod
    def ogrConnectionStringFromLayer(layer):
        """Generates OGR connection string from a layer
        """
        return GdalUtils.ogrConnectionStringAndFormatFromLayer(layer)[0]

    @staticmethod
    def ogrConnectionStringAndFormat(uri, context):
        """Generates OGR connection string and format string from layer source
        Returned values are a tuple of the connection string and format string
        """
        ogrstr = None
        format = None

        layer = QgsProcessingUtils.mapLayerFromString(uri, context, False)
        if layer is None:
            path, ext = os.path.splitext(uri)
            format = QgsVectorFileWriter.driverForExtension(ext)
            return uri, '"' + format + '"'

        return GdalUtils.ogrConnectionStringAndFormatFromLayer(layer)

    @staticmethod
    def ogrConnectionStringAndFormatFromLayer(layer):
        provider = layer.dataProvider().name()
        if provider == 'spatialite':
            # dbname='/geodata/osm_ch.sqlite' table="places" (Geometry) sql=
            regex = re.compile("dbname='(.+)'")
            r = regex.search(str(layer.source()))
            ogrstr = r.groups()[0]
            format = 'SQLite'
        elif provider == 'postgres':
            # dbname='ktryjh_iuuqef' host=spacialdb.com port=9999
            # user='ktryjh_iuuqef' password='xyqwer' sslmode=disable
            # key='gid' estimatedmetadata=true srid=4326 type=MULTIPOLYGON
            # table="t4" (geom) sql=
            dsUri = QgsDataSourceUri(layer.dataProvider().dataSourceUri())
            conninfo = dsUri.connectionInfo()
            conn = None
            ok = False
            while not conn:
                try:
                    conn = psycopg2.connect(dsUri.connectionInfo())
                except psycopg2.OperationalError:
                    (ok, user, passwd) = QgsCredentials.instance().get(conninfo, dsUri.username(), dsUri.password())
                    if not ok:
                        break

                    dsUri.setUsername(user)
                    dsUri.setPassword(passwd)

            if not conn:
                raise RuntimeError('Could not connect to PostgreSQL database - check connection info')

            if ok:
                QgsCredentials.instance().put(conninfo, user, passwd)

            ogrstr = "PG:%s" % dsUri.connectionInfo()
            format = 'PostgreSQL'
        elif provider == 'mssql':
            # 'dbname=\'db_name\' host=myHost estimatedmetadata=true
            # srid=27700 type=MultiPolygon table="dbo"."my_table"
            # #(Shape) sql='
            dsUri = layer.dataProvider().uri()
            ogrstr = 'MSSQL:'
            ogrstr += 'database={0};'.format(dsUri.database())
            ogrstr += 'server={0};'.format(dsUri.host())
            if dsUri.username() != "":
                ogrstr += 'uid={0};'.format(dsUri.username())
            else:
                ogrstr += 'trusted_connection=yes;'
            if dsUri.password() != '':
                ogrstr += 'pwd={0};'.format(dsUri.password())
            ogrstr += 'tables={0}'.format(dsUri.table())
            format = 'MSSQL'
        elif provider == "oracle":
            # OCI:user/password@host:port/service:table
            dsUri = QgsDataSourceUri(layer.dataProvider().dataSourceUri())
            ogrstr = "OCI:"
            if dsUri.username() != "":
                ogrstr += dsUri.username()
                if dsUri.password() != "":
                    ogrstr += "/" + dsUri.password()
                delim = "@"

            if dsUri.host() != "":
                ogrstr += delim + dsUri.host()
                delim = ""
                if dsUri.port() not in ["", '1521']:
                    ogrstr += ":" + dsUri.port()
                ogrstr += "/"
                if dsUri.database() != "":
                    ogrstr += dsUri.database()
            elif dsUri.database() != "":
                ogrstr += delim + dsUri.database()

            if ogrstr == "OCI:":
                raise RuntimeError('Invalid oracle data source - check connection info')

            ogrstr += ":"
            if dsUri.schema() != "":
                ogrstr += dsUri.schema() + "."

            ogrstr += dsUri.table()
            format = 'OCI'
        elif provider.lower() == "wfs":
            uri = QgsDataSourceUri(layer.source())
            baseUrl = uri.param('url').split('?')[0]
            ogrstr = "WFS:{}".format(baseUrl)
            format = 'WFS'
        else:
            ogrstr = str(layer.source()).split("|")[0]
            path, ext = os.path.splitext(ogrstr)
            format = QgsVectorFileWriter.driverForExtension(ext)

        return ogrstr, '"' + format + '"'

    @staticmethod
    def ogrOutputLayerName(uri):
        uri = uri.strip('"')
        return os.path.basename(os.path.splitext(uri)[0])

    @staticmethod
    def ogrLayerName(uri):
        uri = uri.strip('"')
        if ' table=' in uri:
            # table="schema"."table"
            re_table_schema = re.compile(' table="([^"]*)"\\."([^"]*)"')
            r = re_table_schema.search(uri)
            if r:
                return r.groups()[0] + '.' + r.groups()[1]
            # table="table"
            re_table = re.compile(' table="([^"]*)"')
            r = re_table.search(uri)
            if r:
                return r.groups()[0]
        elif 'layername' in uri:
            regex = re.compile('(layername=)([^|]*)')
            r = regex.search(uri)
            return r.groups()[1]

        fields = uri.split('|')
        basePath = fields[0]
        fields = fields[1:]
        layerid = 0
        for f in fields:
            if f.startswith('layername='):
                return f.split('=')[1]
            if f.startswith('layerid='):
                layerid = int(f.split('=')[1])

        ds = ogr.Open(basePath)
        if not ds:
            return None

        ly = ds.GetLayer(layerid)
        if not ly:
            return None

        name = ly.GetName()
        ds = None
        return name

    @staticmethod
    def parseCreationOptions(value):
        parts = value.split('|')
        options = []
        for p in parts:
            options.extend(['-co', p])
        return options

    @staticmethod
    def writeLayerParameterToTextFile(filename, alg, parameters, parameter_name, context, quote=True, executing=False):
        listFile = QgsProcessingUtils.generateTempFilename(filename)

        if executing:
            layers = []
            for l in alg.parameterAsLayerList(parameters, parameter_name, context):
                if quote:
                    layers.append('"' + l.source() + '"')
                else:
                    layers.append(l.source())

            with open(listFile, 'w') as f:
                f.write('\n'.join(layers))

        return listFile

    @staticmethod
    def gdal_crs_string(crs):
        """
        Converts a QgsCoordinateReferenceSystem to a string understandable
        by GDAL
        :param crs: crs to convert
        :return: gdal friendly string
        """
        if crs.authid().upper().startswith('EPSG:') or crs.authid().upper().startswith('IGNF:') or crs.authid().upper().startswith('ESRI:'):
            return crs.authid()

        return crs.toWkt(QgsCoordinateReferenceSystem.WKT_PREFERRED_GDAL)

    @classmethod
    def tr(cls, string, context=''):
        if context == '':
            context = cls.__name__
        return QCoreApplication.translate(context, string)
