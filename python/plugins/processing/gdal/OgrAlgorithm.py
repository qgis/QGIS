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

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.QGisLayers import QGisLayers
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import string
import re

try:
  from osgeo import ogr
  ogrAvailable = True
except:
  ogrAvailable = False

class OgrAlgorithm(GeoAlgorithm):

    DB = "DB"

    def ogrConnectionString(self, uri):
        ogrstr = None

        layer = QGisLayers.getObjectFromUri(uri, False)
        if layer == None:
            return uri;
        provider = layer.dataProvider().name()
        if provider == 'spatialite':
            #dbname='/geodata/osm_ch.sqlite' table="places" (Geometry) sql=
            regex = re.compile("dbname='(.+)'")
            r = regex.search(str(layer.source()))
            ogrstr = r.groups()[0]
        elif provider == 'postgres':
            #dbname='ktryjh_iuuqef' host=spacialdb.com port=9999 user='ktryjh_iuuqef' password='xyqwer' sslmode=disable key='gid' estimatedmetadata=true srid=4326 type=MULTIPOLYGON table="t4" (geom) sql=
            s = re.sub(''' sslmode=.+''', '', str(layer.source()))
            ogrstr = 'PG:%s' % s
        else:
            ogrstr = str(layer.source())
        return ogrstr

    def drivers(self):
        list = []
        if ogrAvailable:
            for iDriver in range(ogr.GetDriverCount()):
                list.append("%s" % ogr.GetDriver(iDriver).GetName())
        return list

    def failure(self, pszDataSource):
        out = ( "FAILURE:\n"
                "Unable to open datasource `%s' with the following drivers.\n" % pszDataSource )
        out = out + string.join(map(lambda d: "->"+d, self.drivers()), '\n')
        return out
