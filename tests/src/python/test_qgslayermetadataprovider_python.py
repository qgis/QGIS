""""Test for a python implementation of layer metadata provider

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-08-19'
__copyright__ = 'Copyright 2022, ItOpen'

import os
import shutil

from qgis.core import (
    QgsPolygon,
    QgsWkbTypes,
    QgsRectangle,
    QgsMapLayerType,
    QgsProviderRegistry,
    QgsAbstractLayerMetadataProvider,
    QgsLayerMetadataSearchResult,
    QgsLayerMetadataProviderResult,
)

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import unittest, start_app
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

temp_dir = QTemporaryDir()
temp_path = temp_dir.path()


class PythonLayerMetadataProvider(QgsAbstractLayerMetadataProvider):
    """Python implementation of a layer metadata provider
    This is mainly to test the Python bindings and API
    """

    def __init__(self):
        super().__init__()

    def type(self):
        return 'python'

    def search(self, searchString='', geographicExtent=QgsRectangle(), feedback=None):

        result = QgsLayerMetadataProviderResult()
        result.standardUri = 'http://mrcc.com/qgis.dtd'
        result.layerType = QgsMapLayerType.VectorLayer
        result.uri = os.path.join(temp_path, 'geopackage.gpkg')
        result.crs = 'EPSG:4326'
        result.dataProviderName = 'ogr'
        result.geometryType = QgsWkbTypes.GeometryType.PointGeometry

        poly = QgsPolygon()
        poly.fromWkt(QgsRectangle(0, 0, 1, 1).asWktPolygon())
        result.geographicExtent = poly

        xml_md = """
        <!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
        <qgis version="3.27.0-Master">
            <identifier>MD012345</identifier>
            <parentidentifier></parentidentifier>
            <language></language>
            <type>dataset</type>
            <title>QGIS Test Title</title>
            <abstract>QGIS Some Data</abstract>
            <links/>
            <fees/>
            <encoding></encoding>
            <crs>
                <spatialrefsys nativeFormat="Wkt">
                    <wkt>GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],USAGE[SCOPE["Horizontal component of 3D system."],AREA["World."],BBOX[-90,-180,90,180]],ID["EPSG",4326]]</wkt>
                    <proj4>+proj=longlat +datum=WGS84 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>EPSG:7030</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </crs>
            <extent>
                <spatial maxz="0" maxx="-65.31999999999999318" maxy="78.29999999999999716" minx="-71.12300000000000466" crs="" minz="0" dimensions="2" miny="66.32999999999999829"/>
            </extent>
        </qgis>
        """

        doc = QDomDocument()
        assert doc.setContent(xml_md)[0]

        assert result.metadata.readMetadataXml(doc.documentElement())

        assert result.identifier() == 'MD012345'

        results = QgsLayerMetadataSearchResult()
        results.addMetadata(result)
        results.addError('Bad news from PythonLayerMetadataProvider :(')

        return results


QGIS_APP = start_app()


class TestPythonLayerMetadataProvider(unittest.TestCase):

    def test_metadataRegistryApi(self):

        reg = QGIS_APP.layerMetadataProviderRegistry()
        self.assertIsNone(reg.layerMetadataProviderFromType('python'))
        reg.registerLayerMetadataProvider(PythonLayerMetadataProvider())
        self.assertIsNotNone(reg.layerMetadataProviderFromType('python'))

        md_provider = reg.layerMetadataProviderFromType('python')
        results = md_provider.search()

        self.assertEqual(len(results.metadata()), 1)
        self.assertEqual(len(results.errors()), 1)

        result = results.metadata()[0]

        self.assertEqual(result.abstract(), 'QGIS Some Data')
        self.assertEqual(result.identifier(), 'MD012345')
        self.assertEqual(result.title(), 'QGIS Test Title')
        self.assertEqual(result.layerType, QgsMapLayerType.VectorLayer)
        self.assertEqual(result.crs, 'EPSG:4326')
        self.assertEqual(result.geometryType, QgsWkbTypes.PointGeometry)
        self.assertEqual(result.dataProviderName, 'ogr')
        self.assertEqual(result.standardUri, 'http://mrcc.com/qgis.dtd')

        reg.unregisterLayerMetadataProvider(md_provider)
        self.assertIsNone(reg.layerMetadataProviderFromType('python'))

    def setUp(self):

        super().setUp()
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        shutil.copy(os.path.join(srcpath, 'geopackage.gpkg'), temp_path)
        self.conn = os.path.join(temp_path, 'geopackage.gpkg')
        md = QgsProviderRegistry.instance().providerMetadata('ogr')


if __name__ == '__main__':
    unittest.main()
