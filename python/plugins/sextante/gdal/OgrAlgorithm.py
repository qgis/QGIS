from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import string
import re
import ogr

class OgrAlgorithm(GeoAlgorithm):

    DB = "DB"

    def ogrConnectionString(self, uri):
        ogrstr = None

        layer = QGisLayers.getObjectFromUri(uri, False)
        if layer == None:
            return uri;
        provider = layer.dataProvider().name()
        qDebug("inputLayer provider '%s'" % provider)
        #qDebug("inputLayer layer '%s'" % layer.providerKey())
        qDebug("inputLayer.source '%s'" % layer.source())
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
        for iDriver in range(ogr.GetDriverCount()):
            list.append("%s" % ogr.GetDriver(iDriver).GetName())
        return list

    def failure(self, pszDataSource):
        out = ( "FAILURE:\n"
                "Unable to open datasource `%s' with the following drivers.\n" % pszDataSource )
        out = out + string.join(map(lambda d: "->"+d, self.drivers()), '\n')
        return out
