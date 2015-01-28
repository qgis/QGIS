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

try:
    from osgeo import ogr
    ogrAvailable = True
except:
    ogrAvailable = False

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

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
            connInfo = dsUri.connectionInfo()
            (success, user, passwd ) = QgsCredentials.instance().get(connInfo, None, None)
            if success:
                QgsCredentials.instance().put(connInfo, user, passwd)
            ogrstr = ("PG:dbname='%s' host='%s' port='%s' user='%s' password='%s'"
                        % (dsUri.database(), dsUri.host(), dsUri.port(), user, passwd))
        else:
            ogrstr = unicode(layer.source()).split("|")[0]
        return '"' + ogrstr + '"'

    def ogrLayerName(self, uri):
        if 'host' in uri:
            regex = re.compile('(table=")(.+?)(\.)(.+?)"')
            r = regex.search(uri)
            return '"' + r.groups()[1] + '.' + r.groups()[3] +'"'
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

