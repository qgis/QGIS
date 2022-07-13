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

import shutil
import tempfile
from pathlib import Path

import qgis  # NOQA
from qgis.PyQt.QtCore import QUrl
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (QgsVectorLayer,
                       QgsVectorTileWriter,
                       QgsDataSourceUri,
                       QgsTileXYZ,
                       QgsProviderRegistry,
                       QgsVectorTileLayer,
                       QgsProviderMetadata,
                       QgsGeometry,
                       QgsSelectionContext,
                       Qgis
                       )
from qgis.testing import unittest, start_app
from utilities import unitTestDataPath

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

        uri = 'type=xyz&serviceType=arcgis&url=https://fake.server/%7Bx%7D/%7By%7D/%7Bz%7D.png&zmax=2&http-header:referer=https://qgis.org/&styleUrl=https://qgis.org/'
        parts = md.decodeUri(uri)
        self.assertEqual(parts, {'type': 'xyz', 'serviceType': 'arcgis', 'url': 'https://fake.server/{x}/{y}/{z}.png',
                                 'zmax': '2', 'http-header:referer': 'https://qgis.org/',
                                 'referer': 'https://qgis.org/', 'styleUrl': 'https://qgis.org/'})

        parts['url'] = 'https://fake.new.server/{x}/{y}/{z}.png'
        uri = md.encodeUri(parts)
        self.assertEqual(uri,
                         'serviceType=arcgis&styleUrl=https://qgis.org/&type=xyz&url=https://fake.new.server/%7Bx%7D/%7By%7D/%7Bz%7D.png&zmax=2&http-header:referer=https://qgis.org/')

    def testZoomRange(self):
        """
        Test retrieval of zoom range from URI
        """
        vl = QgsVectorTileLayer(
            'type=xyz&url=https://wxs.ign.fr/parcellaire/geoportail/tms/1.0.0/PCI/%7Bz%7D/%7Bx%7D/%7By%7D.pbf&zmax=19&zmin=5',
            'test')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.sourceMinZoom(), 5)
        self.assertEqual(vl.sourceMaxZoom(), 19)

    def testSelection(self):
        layer = QgsVectorTileLayer('type=vtpk&url={}'.format(unitTestDataPath() + '/testvtpk.vtpk'), 'tiles')
        self.assertTrue(layer.isValid())

        self.assertFalse(layer.selectedFeatures())
        spy = QSignalSpy(layer.selectionChanged)

        # select by polygon
        selection_geometry = QgsGeometry.fromWkt(
            'Polygon ((-12225020.2316580843180418 6030602.60334861185401678, -13521860.30317855626344681 5526975.39110779482871294, -12976264.15658413991332054 4821897.29397048708051443, -12019372.45332629978656769 4884850.69550053123384714, -11650045.83101624809205532 4754746.99900492746382952, -11469579.41329662501811981 5535369.17797833122313023, -11792740.20781794004142284 6110343.57862004917114973, -12225020.2316580843180418 6030602.60334861185401678))')
        context = QgsSelectionContext()
        context.setScale(17991708)
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.SetSelection,
                               Qgis.SelectGeometryRelationship.Intersect)

        self.assertEqual(layer.selectedFeatureCount(), 8)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()), {3320043274240,
                                                                              3320026497024,
                                                                              2220498092032,
                                                                              2224826613760,
                                                                              3320043274243,
                                                                              2220514869248,
                                                                              3324338241541,
                                                                              3324338241536})
        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()), {
            'Polygon ((-13222000 4970000, -13203000 5004000, -13189000 5073000, -13164000 5127000, -13145000 5205000, -13106000 5239000, -13047000 5274000, -12964000 5269000, -12910000 5195000, -12885000 5185000, -12846000 5185000, -12802000 5151000, -12758000 5093000, -12685000 5093000, -12611000 5107000, -12484000 5103000, -12484000 4970000, -13222000 4970000))',
            'MultiPolygon (((-12157000 5694000, -12132000 5738000, -12093000 5782000, -12039000 5817000, -11956000 5836000, -11853000 5836000, -11795000 5792000, -11785000 5763000, -11760000 5738000, -11736000 5621000, -11716000 5587000, -11653000 5528000, -11643000 5503000, -11643000 5366000, -11653000 5337000, -11692000 5278000, -11731000 5249000, -11741000 5239000, -11839000 5205000, -11863000 5220000, -11927000 5288000, -11941000 5440000, -11951000 5450000, -12005000 5450000, -12020000 5450000, -12029000 5435000, -12054000 5435000, -12093000 5450000, -12132000 5503000, -12157000 5518000, -12166000 5543000, -12166000 5650000, -12157000 5694000),(-11995000 5680000, -12015000 5626000, -12005000 5582000, -11990000 5562000, -11927000 5547000, -11907000 5528000, -11883000 5479000, -11863000 5469000, -11829000 5469000, -11809000 5489000, -11814000 5577000, -11863000 5626000, -11912000 5704000, -11966000 5704000, -11995000 5680000)),((-11521000 5773000, -11496000 5851000, -11413000 5900000, -11389000 5890000, -11325000 5787000, -11320000 5724000, -11364000 5645000, -11418000 5640000, -11462000 5655000, -11491000 5699000, -11506000 5709000, -11521000 5773000)))',
            'Polygon ((-12563000 5105000, -12470000 5102000, -12411000 4990000, -12411000 4970000, -12563000 4970000, -12563000 5105000))',
            'Polygon ((-11868000 4750000, -11809000 4882000, -11702000 4956000, -11638000 4956000, -11609000 4916000, -11574000 4892000, -11452000 4882000, -11354000 4833000, -11325000 4794000, -11291000 4765000, -11212000 4638000, -11217000 4579000, -11261000 4486000, -11266000 4432000, -11281000 4393000, -11296000 4383000, -11310000 4339000, -11398000 4300000, -11501000 4305000, -11530000 4320000, -11579000 4359000, -11633000 4427000, -11687000 4466000, -11790000 4515000, -11834000 4569000, -11873000 4652000, -11868000 4750000))',
            'Polygon ((-13228000 4960000, -13203000 5004000, -13194000 5049000, -12484000 5049000, -12484000 4869000, -12587000 4814000, -12631000 4765000, -12641000 4716000, -12680000 4652000, -12714000 4618000, -12837000 4589000, -12900000 4589000, -12944000 4608000, -12949000 4618000, -13027000 4623000, -13062000 4647000, -13135000 4662000, -13189000 4691000, -13228000 4779000, -13238000 4819000, -13238000 4907000, -13228000 4960000))',
            'MultiLineString ((-11662000 5797000, -11633000 5704000, -11589000 5640000, -11398000 5518000, -11325000 5420000, -11276000 5127000, -11247000 5053000, -11208000 4990000, -11207000 4970000),(-11305000 5337000, -11261000 5381000, -11227000 5459000, -11178000 5518000, -11105000 5562000, -11051000 5670000, -10997000 5724000, -10953000 5812000))',
            'Polygon ((-12442000 5049000, -12411000 4990000, -12411000 4956000, -12426000 4916000, -12450000 4887000, -12563000 4827000, -12563000 5049000, -12442000 5049000))',
            'Point (-12714000 5220000)'})
        self.assertEqual(len(spy), 1)
        self.assertNotEqual(layer.selectedFeatures()[0].fields().lookupField('tile_zoom'), -1)
        self.assertNotEqual(layer.selectedFeatures()[0].fields().lookupField('tile_layer'), -1)
        self.assertEqual(layer.selectedFeatures()[0]['tile_zoom'], 4)
        self.assertCountEqual([f['tile_layer'] for f in layer.selectedFeatures()], ['polys', 'polys', 'polys', 'lines', 'points', 'polys', 'polys', 'polys'])

        # select same again, should be no new signal
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.SetSelection,
                               Qgis.SelectGeometryRelationship.Intersect)
        self.assertEqual(len(spy), 1)

        # select within
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.SetSelection,
                               Qgis.SelectGeometryRelationship.Within)
        self.assertEqual(len(spy), 2)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()), {2220498092032, 3320043274243})
        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()),
                              {'Point (-12714000 5220000)',
                               'Polygon ((-12563000 5105000, -12470000 5102000, -12411000 4990000, -12411000 4970000, -12563000 4970000, -12563000 5105000))'})

        # add to selection
        selection_geometry = QgsGeometry.fromWkt(
            'Polygon ((-11104449.68442200869321823 6041094.83693683333694935, -11461185.62642602622509003 5822856.37829908169806004, -11054086.96319791302084923 5419954.60850630886852741, -10793879.57020674645900726 5835447.05860510468482971, -11104449.68442200869321823 6041094.83693683333694935))')
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.AddToSelection,
                               Qgis.SelectGeometryRelationship.Intersect)
        self.assertEqual(len(spy), 3)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()),
                              {2220498092032, 3320043274243, 3320043274240, 3320026497024})
        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()),
                              {
                                  'Polygon ((-12563000 5105000, -12470000 5102000, -12411000 4990000, -12411000 4970000, -12563000 4970000, -12563000 5105000))',
                                  'MultiLineString ((-11662000 5797000, -11633000 5704000, -11589000 5640000, -11398000 5518000, -11325000 5420000, -11276000 5127000, -11247000 5053000, -11208000 4990000, -11207000 4970000),(-11305000 5337000, -11261000 5381000, -11227000 5459000, -11178000 5518000, -11105000 5562000, -11051000 5670000, -10997000 5724000, -10953000 5812000))',
                                  'Point (-12714000 5220000)',
                                  'MultiPolygon (((-12157000 5694000, -12132000 5738000, -12093000 5782000, -12039000 5817000, -11956000 5836000, -11853000 5836000, -11795000 5792000, -11785000 5763000, -11760000 5738000, -11736000 5621000, -11716000 5587000, -11653000 5528000, -11643000 5503000, -11643000 5366000, -11653000 5337000, -11692000 5278000, -11731000 5249000, -11741000 5239000, -11839000 5205000, -11863000 5220000, -11927000 5288000, -11941000 5440000, -11951000 5450000, -12005000 5450000, -12020000 5450000, -12029000 5435000, -12054000 5435000, -12093000 5450000, -12132000 5503000, -12157000 5518000, -12166000 5543000, -12166000 5650000, -12157000 5694000),(-11995000 5680000, -12015000 5626000, -12005000 5582000, -11990000 5562000, -11927000 5547000, -11907000 5528000, -11883000 5479000, -11863000 5469000, -11829000 5469000, -11809000 5489000, -11814000 5577000, -11863000 5626000, -11912000 5704000, -11966000 5704000, -11995000 5680000)),((-11521000 5773000, -11496000 5851000, -11413000 5900000, -11389000 5890000, -11325000 5787000, -11320000 5724000, -11364000 5645000, -11418000 5640000, -11462000 5655000, -11491000 5699000, -11506000 5709000, -11521000 5773000)))'})

        # remove from selection
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.RemoveFromSelection,
                               Qgis.SelectGeometryRelationship.Intersect)
        self.assertEqual(len(spy), 4)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()), {2220498092032, 3320043274243})
        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()),
                              {'Point (-12714000 5220000)',
                               'Polygon ((-12563000 5105000, -12470000 5102000, -12411000 4990000, -12411000 4970000, -12563000 4970000, -12563000 5105000))'})

        # intersect selection
        selection_geometry = QgsGeometry.fromWkt(
            'Polygon ((-12632118.89488627389073372 5457726.64942438062280416, -12862948.03383005037903786 5310835.37918743211776018, -12850357.35352402552962303 5046431.09276092518121004, -12716056.76359310187399387 4987674.58466614596545696, -12434864.90342522785067558 5113581.38772638700902462, -12632118.89488627389073372 5457726.64942438062280416))')

        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.IntersectSelection,
                               Qgis.SelectGeometryRelationship.Within)
        self.assertEqual(len(spy), 5)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()), {2220498092032})
        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()),
                              {'Point (-12714000 5220000)'})

        layer.removeSelection()
        self.assertFalse(layer.selectedFeatures())
        self.assertEqual(len(spy), 6)

        layer.removeSelection()
        self.assertFalse(layer.selectedFeatures())
        self.assertEqual(len(spy), 6)

        # selection should depend on tile scale
        selection_geometry = QgsGeometry.fromWkt(
            'Polygon ((-12225020.2316580843180418 6030602.60334861185401678, -13521860.30317855626344681 5526975.39110779482871294, -12976264.15658413991332054 4821897.29397048708051443, -12019372.45332629978656769 4884850.69550053123384714, -11650045.83101624809205532 4754746.99900492746382952, -11469579.41329662501811981 5535369.17797833122313023, -11792740.20781794004142284 6110343.57862004917114973, -12225020.2316580843180418 6030602.60334861185401678))')
        context = QgsSelectionContext()
        context.setScale(137882080)
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.SetSelection,
                               Qgis.SelectGeometryRelationship.Intersect)

        self.assertEqual(len(layer.selectedFeatures()), 5)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()),
                              {33554432, 16777216, 0, 33554433, 33554438})
        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()), {
            'MultiPolygon (((-12152000 5694000, -12132000 5733000, -12093000 5792000, -12034000 5812000, -11956000 5831000, -11858000 5831000, -11799000 5792000, -11780000 5773000, -11760000 5733000, -11741000 5616000, -11721000 5596000, -11662000 5538000, -11643000 5499000, -11643000 5362000, -11662000 5342000, -11682000 5283000, -11721000 5244000, -11741000 5244000, -11839000 5205000, -11858000 5225000, -11917000 5283000, -11936000 5440000, -11956000 5459000, -11995000 5459000, -12015000 5459000, -12034000 5440000, -12054000 5440000, -12093000 5459000, -12132000 5499000, -12152000 5518000, -12171000 5538000, -12171000 5655000, -12152000 5694000),(-11995000 5675000, -12015000 5636000, -11995000 5577000, -11995000 5557000, -11917000 5557000, -11917000 5538000, -11878000 5479000, -11858000 5479000, -11839000 5479000, -11799000 5499000, -11819000 5577000, -11858000 5636000, -11917000 5714000, -11956000 5714000, -11995000 5675000)),((-11525000 5773000, -11486000 5851000, -11408000 5909000, -11389000 5890000, -11330000 5792000, -11310000 5733000, -11369000 5655000, -11408000 5636000, -11467000 5655000, -11486000 5694000, -11506000 5714000, -11525000 5773000)))',
            'MultiPoint ((-13052000 4471000),(-9275000 4016000),(-12201000 4261000),(-10885000 4143000),(-9828000 3992000),(-9534000 4711000),(-10371000 4965000),(-11403000 5049000),(-12499000 4775000),(-11501000 2607000),(-11452000 3703000),(-11085000 4951000),(-10493000 5919000),(-12714000 5220000),(-12607000 4290000),(-12249000 3703000),(-11687000 3331000))',
            'MultiLineString ((-13091000 4188000, -13032000 4207000, -12974000 4285000, -12915000 4364000, -12778000 4442000, -12621000 4501000, -12445000 4481000, -12367000 4461000, -12328000 4461000, -12191000 4403000, -12113000 4344000, -11995000 4285000, -11858000 4285000, -11760000 4246000, -11467000 4227000, -11291000 4227000, -11173000 4285000, -11134000 4305000, -11017000 4305000, -10821000 4246000, -10645000 4246000, -10508000 4285000, -10430000 4344000, -10351000 4422000, -10195000 4520000, -10058000 4559000, -10058000 4579000, -9960000 4559000, -9921000 4540000, -9843000 4520000, -9745000 4461000, -9706000 4442000, -9549000 4422000, -9353000 4442000, -9236000 4520000, -9197000 4520000),(-12347000 4461000, -12367000 4403000, -12406000 4325000, -12426000 4305000, -12445000 4266000, -12465000 4207000, -12504000 4148000, -12621000 4109000, -12758000 4109000, -12797000 4090000, -12817000 4051000, -12856000 3992000, -12856000 3914000, -12856000 3816000, -12837000 3796000, -12797000 3777000),(-11662000 5792000, -11623000 5714000, -11584000 5636000, -11408000 5518000, -11330000 5420000, -11271000 5127000, -11252000 5049000, -11212000 4990000, -11193000 4872000, -11193000 4833000, -11193000 4814000, -11154000 4775000, -11056000 4696000, -11036000 4638000, -11075000 4559000, -11115000 4520000, -11134000 4442000, -11134000 4422000, -11056000 4227000, -11075000 4168000, -11095000 4148000, -11134000 4090000, -11134000 4031000, -11173000 3992000, -11212000 3914000, -11212000 3816000, -11193000 3757000, -11115000 3659000, -11075000 3542000, -11017000 3503000, -10978000 3444000, -10899000 3366000, -10860000 3327000, -10684000 3190000, -10664000 3150000, -10645000 3131000, -10645000 3053000, -10704000 2974000, -10723000 2955000, -10723000 2896000, -10704000 2818000, -10606000 2700000, -10528000 2661000),(-11310000 5342000, -11252000 5381000, -11232000 5459000, -11173000 5518000, -11115000 5557000, -11056000 5675000, -10997000 5733000, -10958000 5812000),(-10273000 4461000, -10254000 4364000, -10234000 4364000, -10214000 4344000, -10175000 4305000, -10097000 4305000, -10058000 4285000, -10038000 4227000, -10019000 4129000, -10019000 4109000, -9999000 4070000, -9921000 3953000, -9901000 3953000, -9804000 3933000, -9745000 3914000, -9706000 3835000, -9686000 3816000, -9627000 3796000, -9549000 3777000, -9530000 3757000, -9432000 3698000, -9353000 3679000, -9314000 3600000, -9256000 3561000),(-9843000 4520000, -9843000 4579000, -9823000 4598000, -9725000 4716000, -9725000 4735000, -9706000 4814000, -9647000 4912000, -9549000 5029000, -9530000 5146000, -9393000 5440000, -9314000 5479000, -9158000 5499000))',
            'Polygon ((-11858000 4755000, -11799000 4892000, -11702000 4951000, -11643000 4951000, -11604000 4912000, -11565000 4892000, -11447000 4892000, -11349000 4833000, -11330000 4794000, -11291000 4775000, -11212000 4638000, -11212000 4579000, -11252000 4481000, -11271000 4442000, -11271000 4403000, -11291000 4383000, -11310000 4344000, -11408000 4305000, -11506000 4305000, -11525000 4325000, -11584000 4364000, -11623000 4422000, -11682000 4461000, -11780000 4520000, -11839000 4579000, -11878000 4657000, -11858000 4755000))',
            'Polygon ((-13228000 4970000, -13208000 5009000, -13189000 5068000, -13169000 5127000, -13150000 5205000, -13110000 5244000, -13052000 5283000, -12954000 5264000, -12915000 5205000, -12876000 5185000, -12856000 5185000, -12797000 5146000, -12758000 5088000, -12680000 5088000, -12602000 5107000, -12465000 5107000, -12406000 4990000, -12406000 4951000, -12426000 4912000, -12445000 4892000, -12582000 4814000, -12621000 4775000, -12641000 4716000, -12680000 4657000, -12719000 4618000, -12837000 4598000, -12895000 4598000, -12934000 4618000, -12954000 4618000, -13032000 4618000, -13052000 4657000, -13130000 4657000, -13189000 4696000, -13228000 4775000, -13228000 4814000, -13228000 4912000, -13228000 4970000))'})
        self.assertEqual(len(spy), 7)

        # removing features should NOT depend on the scale
        context.setScale(237882080)
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.RemoveFromSelection,
                               Qgis.SelectGeometryRelationship.Intersect)

        self.assertFalse(layer.selectedFeatures())
        self.assertEqual(len(spy), 8)

        # single feature selection
        selection_geometry = QgsGeometry.fromWkt('Polygon ((-10486370.9139375202357769 3807467.1023839432746172, -10528246.57568679377436638 3799853.34570225700736046, -10490177.79227836430072784 3735136.41390792420133948, -10486370.9139375202357769 3807467.1023839432746172))')
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.SetSelection,
                               Qgis.SelectGeometryRelationship.Intersect, Qgis.SelectionFlags(Qgis.SelectionFlag.SingleFeatureSelection))
        self.assertEqual(len(layer.selectedFeatures()), 1)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()),
                              {33554436})

        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()), {'Polygon ((-10958000 3835000, -10958000 3796000, -10919000 3835000, -10841000 3874000, -10684000 3992000, -10567000 4031000, -10410000 4031000, -10332000 3992000, -10254000 3914000, -10136000 3914000, -10058000 3874000, -10019000 3796000, -10019000 3757000, -10058000 3718000, -10097000 3718000, -10254000 3796000, -10332000 3796000, -10371000 3757000, -10371000 3718000, -10371000 3679000, -10332000 3640000, -10332000 3561000, -10410000 3483000, -10449000 3405000, -10488000 3366000, -10645000 3327000, -10723000 3366000, -10762000 3405000, -10801000 3444000, -10762000 3483000, -10801000 3522000, -10841000 3561000, -10919000 3561000, -10958000 3600000, -10958000 3640000, -10958000 3679000, -10958000 3718000, -10997000 3796000, -10958000 3835000))'})
        self.assertEqual(len(spy), 9)

        # select again
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.SetSelection,
                               Qgis.SelectGeometryRelationship.Intersect, Qgis.SelectionFlags(Qgis.SelectionFlag.SingleFeatureSelection))
        self.assertEqual(len(layer.selectedFeatures()), 1)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()),
                              {33554436})

        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()), {'Polygon ((-10958000 3835000, -10958000 3796000, -10919000 3835000, -10841000 3874000, -10684000 3992000, -10567000 4031000, -10410000 4031000, -10332000 3992000, -10254000 3914000, -10136000 3914000, -10058000 3874000, -10019000 3796000, -10019000 3757000, -10058000 3718000, -10097000 3718000, -10254000 3796000, -10332000 3796000, -10371000 3757000, -10371000 3718000, -10371000 3679000, -10332000 3640000, -10332000 3561000, -10410000 3483000, -10449000 3405000, -10488000 3366000, -10645000 3327000, -10723000 3366000, -10762000 3405000, -10801000 3444000, -10762000 3483000, -10801000 3522000, -10841000 3561000, -10919000 3561000, -10958000 3600000, -10958000 3640000, -10958000 3679000, -10958000 3718000, -10997000 3796000, -10958000 3835000))'})
        self.assertEqual(len(spy), 9)

        # with toggle mode
        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.SetSelection,
                               Qgis.SelectGeometryRelationship.Intersect, Qgis.SelectionFlags(Qgis.SelectionFlag.SingleFeatureSelection | Qgis.SelectionFlag.ToggleSelection))
        self.assertFalse(layer.selectedFeatures())
        self.assertEqual(len(spy), 10)

        layer.selectByGeometry(selection_geometry, context, Qgis.SelectBehavior.SetSelection,
                               Qgis.SelectGeometryRelationship.Intersect, Qgis.SelectionFlags(Qgis.SelectionFlag.SingleFeatureSelection | Qgis.SelectionFlag.ToggleSelection))
        self.assertEqual(len(layer.selectedFeatures()), 1)
        self.assertCountEqual(set(f.id() for f in layer.selectedFeatures()),
                              {33554436})

        self.assertCountEqual(set(f.geometry().asWkt(-3) for f in layer.selectedFeatures()), {'Polygon ((-10958000 3835000, -10958000 3796000, -10919000 3835000, -10841000 3874000, -10684000 3992000, -10567000 4031000, -10410000 4031000, -10332000 3992000, -10254000 3914000, -10136000 3914000, -10058000 3874000, -10019000 3796000, -10019000 3757000, -10058000 3718000, -10097000 3718000, -10254000 3796000, -10332000 3796000, -10371000 3757000, -10371000 3718000, -10371000 3679000, -10332000 3640000, -10332000 3561000, -10410000 3483000, -10449000 3405000, -10488000 3366000, -10645000 3327000, -10723000 3366000, -10762000 3405000, -10801000 3444000, -10762000 3483000, -10801000 3522000, -10841000 3561000, -10919000 3561000, -10958000 3600000, -10958000 3640000, -10958000 3679000, -10958000 3718000, -10997000 3796000, -10958000 3835000))'})
        self.assertEqual(len(spy), 11)


if __name__ == '__main__':
    unittest.main()
