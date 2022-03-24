# -*- coding: utf-8 -*-
'''
test_vectortile.py
--------------------------------------
               Date                 : September 2021
               Copyright            : (C) 2021 David Marteau
               email                : david dot marteau at 3liz dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
'''

import qgis  # NOQA
import tempfile
import shutil

from qgis.testing import unittest, start_app
from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QUrl
from qgis.core import (QgsVectorLayer,
                       QgsVectorTileWriter,
                       QgsDataSourceUri,
                       QgsTileXYZ,
                       QgsProviderRegistry,
                       QgsVectorTileLayer,
                       QgsProviderMetadata)

from pathlib import Path

TEST_DATA_PATH = Path(unitTestDataPath())


start_app()


class TestVectorTile(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.tempdir = Path(tempfile.mkdtemp())

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tempdir, True)

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testSingleTileEncode(self):
        """ Test vector tile encoding from python
        """
        vlPoints = QgsVectorLayer(str(TEST_DATA_PATH / "points.shp"), "points", "ogr")
        vlLines = QgsVectorLayer(str(TEST_DATA_PATH / "lines.shp"), "lines", "ogr")
        vlPolys = QgsVectorLayer(str(TEST_DATA_PATH / "polys.shp"), "polys", "ogr")

        layers = [QgsVectorTileWriter.Layer(vl) for vl in (vlPoints, vlLines, vlPolys)]

        writer = QgsVectorTileWriter()
        writer.setMaxZoom(3)
        writer.setLayers(layers)

        data = writer.writeSingleTile(QgsTileXYZ(0, 0, 0))

        ds = QgsDataSourceUri()
        ds.setParam("type", "xyz")
        ds.setParam("url", (self.tempdir / "{z}-{x}-{y}.pbf").as_uri())

        # Create pbf files
        writer.setDestinationUri(ds.encodedUri().data().decode())
        res = writer.writeTiles()
        self.assertEqual(writer.errorMessage(), "")
        self.assertTrue(res)

        # Compare encoded data to written file
        # Read data from file
        with (self.tempdir / "0-0-0.pbf").open("rb") as fp:
            output = fp.read()

        # Compare binary data
        self.assertEqual(ascii(data.data()), ascii(output))

    def testEncodeDecodeUri(self):
        """ Test encodeUri/decodeUri metadata functions """
        md = QgsProviderRegistry.instance().providerMetadata('vectortile')

        uri = 'type=mbtiles&url=/my/file.mbtiles'
        parts = md.decodeUri(uri)
        self.assertEqual(parts, {'type': 'mbtiles', 'path': '/my/file.mbtiles'})

        parts['path'] = '/my/new/file.mbtiles'
        uri = md.encodeUri(parts)
        self.assertEqual(uri, 'type=mbtiles&url=/my/new/file.mbtiles')

        uri = 'type=xyz&url=https://fake.server/%7Bx%7D/%7By%7D/%7Bz%7D.png&zmin=0&zmax=2'
        parts = md.decodeUri(uri)
        self.assertEqual(parts, {'type': 'xyz', 'url': 'https://fake.server/{x}/{y}/{z}.png', 'zmin': '0', 'zmax': '2'})

        parts['url'] = 'https://fake.new.server/{x}/{y}/{z}.png'
        uri = md.encodeUri(parts)
        self.assertEqual(uri, 'type=xyz&url=https://fake.new.server/%7Bx%7D/%7By%7D/%7Bz%7D.png&zmax=2&zmin=0')

        uri = 'type=xyz&serviceType=arcgis&url=https://fake.server/%7Bx%7D/%7By%7D/%7Bz%7D.png&zmax=2&referer=https://qgis.org/&styleUrl=https://qgis.org/'
        parts = md.decodeUri(uri)
        self.assertEqual(parts, {'type': 'xyz', 'serviceType': 'arcgis', 'url': 'https://fake.server/{x}/{y}/{z}.png', 'zmax': '2', 'referer': 'https://qgis.org/', 'styleUrl': 'https://qgis.org/'})

        parts['url'] = 'https://fake.new.server/{x}/{y}/{z}.png'
        uri = md.encodeUri(parts)
        self.assertEqual(uri, 'referer=https://qgis.org/&serviceType=arcgis&styleUrl=https://qgis.org/&type=xyz&url=https://fake.new.server/%7Bx%7D/%7By%7D/%7Bz%7D.png&zmax=2')

    def testZoomRange(self):
        """
        Test retrieval of zoom range from URI
        """
        vl = QgsVectorTileLayer('type=xyz&url=https://wxs.ign.fr/parcellaire/geoportail/tms/1.0.0/PCI/%7Bz%7D/%7Bx%7D/%7By%7D.pbf&zmax=19&zmin=5', 'test')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.sourceMinZoom(), 5)
        self.assertEqual(vl.sourceMaxZoom(), 19)


if __name__ == '__main__':
    unittest.main()
