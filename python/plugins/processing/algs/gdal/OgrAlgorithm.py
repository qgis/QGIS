# -*- coding: utf-8 -*-

"""
***************************************************************************
    OgrAlgorithm.py
    ---------------------
    Date                 : November 2012
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
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import re
import os
import psycopg2

from qgis.core import QgsDataSourceURI, QgsCredentials

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools import dataobjects


class OgrAlgorithm(GdalAlgorithm):

    def ogrConnectionString(self, uri):
        ogrstr = None

        layer = dataobjects.getObjectFromUri(uri, False)
        if layer is None:
            return uri
        provider = layer.dataProvider().name()
        if provider == 'spatialite':
            # dbname='/geodata/osm_ch.sqlite' table="places" (Geometry) sql=
            regex = re.compile("dbname='(.+)'")
            r = regex.search(unicode(layer.source()))
            ogrstr = r.groups()[0]
        elif provider == 'postgres':
            # dbname='ktryjh_iuuqef' host=spacialdb.com port=9999
            # user='ktryjh_iuuqef' password='xyqwer' sslmode=disable
            # key='gid' estimatedmetadata=true srid=4326 type=MULTIPOLYGON
            # table="t4" (geom) sql=
            dsUri = QgsDataSourceURI(layer.dataProvider().dataSourceUri())
            conninfo = dsUri.connectionInfo()
            conn = None
            ok = False
            while not conn:
                try:
                    conn = psycopg2.connect(dsUri.connectionInfo())
                except psycopg2.OperationalError as e:
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
        elif provider == "oracle":
            # OCI:user/password@host:port/service:table
            dsUri = QgsDataSourceURI(layer.dataProvider().dataSourceUri())
            ogrstr = "OCI:"
            if dsUri.username() != "":
                ogrstr += dsUri.username()
                if dsUri.password() != "":
                    ogrstr += "/" + dsUri.password()
                delim = "@"

            if dsUri.host() != "":
                ogrstr += delim + dsUri.host()
                delim = ""
                if dsUri.port() != "" and dsUri.port() != '1521':
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
        else:
            ogrstr = unicode(layer.source()).split("|")[0]

        return '"' + ogrstr + '"'

    def ogrLayerName(self, uri):
        if 'host' in uri:
            regex = re.compile('(table=")(.+?)(\.)(.+?)"')
            r = regex.search(uri)
            return '"' + r.groups()[1] + '.' + r.groups()[3] + '"'
        elif 'dbname' in uri:
            regex = re.compile('(table=")(.+?)"')
            r = regex.search(uri)
            return r.groups()[1]
        elif 'layername' in uri:
            regex = re.compile('(layername=)(.*)')
            r = regex.search(uri)
            return r.groups()[1]
        else:
            return os.path.basename(os.path.splitext(uri)[0])
