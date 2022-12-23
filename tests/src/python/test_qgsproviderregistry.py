# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProviderRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/03/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    QgsProviderRegistry,
    QgsMapLayerType,
    QgsProviderMetadata,
    QgsProviderSublayerDetails,
    Qgis,
    QgsProviderUtils
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# to find the srs.db
start_app()


class TestProviderMetadata(QgsProviderMetadata):
    """
    Test metadata
    """

    def __init__(self, key):
        super().__init__(key, key)

    def querySublayers(self, uri: str, flags=Qgis.SublayerQueryFlags(), feedback=None):
        res = QgsProviderSublayerDetails()
        res.setProviderKey(self.key())
        res.setUri(uri)
        res.setName(QgsProviderUtils.suggestLayerNameFromFilePath(uri))
        return [res]


class TestQgsProviderRegistry(unittest.TestCase):

    def testProviderList(self):
        """
        Test provider list
        """

        providers = QgsProviderRegistry.instance().providerList()
        self.assertIn('ogr', providers)
        self.assertIn('gdal', providers)

    def testProviderMetadata(self):
        """
        Test retrieving provider metadata
        """

        providers = QgsProviderRegistry.instance().providerList()
        for p in providers:
            self.assertTrue(QgsProviderRegistry.instance().providerMetadata(p))
            # should be case-insensitive
            self.assertTrue(QgsProviderRegistry.instance().providerMetadata(p.lower()))
            self.assertTrue(QgsProviderRegistry.instance().providerMetadata(p.upper()))

        self.assertIsNone(QgsProviderRegistry.instance().providerMetadata('asdasdasdasdasd'))

    def testProvidersForLayerType(self):
        """
        Test retrieving providers for a layer type
        """
        providers = QgsProviderRegistry.instance().providersForLayerType(QgsMapLayerType.VectorLayer)
        self.assertIn('ogr', providers)
        self.assertIn('memory', providers)
        self.assertNotIn('gdal', providers)

        providers = QgsProviderRegistry.instance().providersForLayerType(QgsMapLayerType.RasterLayer)
        self.assertNotIn('ogr', providers)
        self.assertNotIn('memory', providers)
        self.assertIn('gdal', providers)

        providers = QgsProviderRegistry.instance().providersForLayerType(QgsMapLayerType.AnnotationLayer)
        self.assertFalse(providers)

    def testCreateProvider(self):
        """
        Test creating provider instance
        """
        providers = QgsProviderRegistry.instance().providerList()
        for p in providers:
            if p == 'geonode' or p == 'vectortile':
                continue

            self.assertTrue(QgsProviderRegistry.instance().createProvider(p, ''))
            # should be case-insensitive
            self.assertTrue(QgsProviderRegistry.instance().createProvider(p.lower(), ''))
            self.assertTrue(QgsProviderRegistry.instance().createProvider(p.upper(), ''))

        self.assertIsNone(QgsProviderRegistry.instance().createProvider('asdasdasdasdasd', ''))

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testShouldDeferUriForOtherProvidersEpt(self):
        self.assertTrue(QgsProviderRegistry.instance().shouldDeferUriForOtherProviders('/home/nyall/ept.json', 'ogr'))
        self.assertFalse(QgsProviderRegistry.instance().shouldDeferUriForOtherProviders('/home/nyall/ept.json', 'ept'))
        self.assertFalse(QgsProviderRegistry.instance().shouldDeferUriForOtherProviders('/home/nyall/my.json', 'ogr'))

    def testUriIsBlocklisted(self):
        self.assertFalse(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.tif'))
        self.assertFalse(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.shp'))

        # internal details only -- we should be hiding these uris!
        self.assertTrue(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.shp.xml'))
        self.assertTrue(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.aux.xml'))
        self.assertTrue(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.AUX.XML'))
        self.assertTrue(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.tif.aux.xml'))
        self.assertTrue(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.tif.AUX.XML'))
        self.assertTrue(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.png.aux.xml'))
        self.assertTrue(QgsProviderRegistry.instance().uriIsBlocklisted('/home/nyall/me.tif.xml'))

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testFilePointCloudFilters(self):
        parts = QgsProviderRegistry.instance().filePointCloudFilters().split(';;')
        self.assertTrue(parts[0].startswith('All Supported Files ('))
        all_filter = parts[0][21:-1]
        self.assertIn('ept.json', all_filter.split(' '))
        self.assertIn('EPT.JSON', all_filter.split(' '))

        self.assertEqual(parts[1], 'All Files (*.*)')
        self.assertIn('Entwine Point Clouds (ept.json EPT.JSON)', parts)

    def testUnusableUriDetails(self):
        """
        Test retrieving user-friendly details about an unusable URI
        """
        res, details = QgsProviderRegistry.instance().handleUnusableUri('')
        self.assertFalse(res)
        res, details = QgsProviderRegistry.instance().handleUnusableUri('/home/me/test.png')
        self.assertFalse(res)
        res, details = QgsProviderRegistry.instance().handleUnusableUri('/home/me/test.las')
        self.assertTrue(res)
        self.assertIn('LAS', details.warning)
        res, details = QgsProviderRegistry.instance().handleUnusableUri('/home/me/test.laz')
        self.assertTrue(res)
        self.assertIn('LAZ', details.warning)

    def testSublayerDetails(self):
        ept_provider_metadata = QgsProviderRegistry.instance().providerMetadata('ept')
        ogr_provider_metadata = QgsProviderRegistry.instance().providerMetadata('ogr')

        if ept_provider_metadata is not None:
            # test querying a uri which should be blocklisted
            self.assertFalse(QgsProviderRegistry.instance().querySublayers(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept-build.json'))

        if ept_provider_metadata is not None and ogr_provider_metadata is not None:
            # test querying a uri which is technically capable of being opened by two providers, but which one provider is preferred
            # in this case we are testing a ept.json file, which should ALWAYS be treated as a ept point cloud layer even though
            # the OGR provider CAN technically open json files

            # when we directly query ogr provider metadata it should report sublayers for the json file...
            self.assertEqual([l.providerKey() for l in ogr_provider_metadata.querySublayers(
                unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', Qgis.SublayerQueryFlags(Qgis.SublayerQueryFlag.FastScan))], ['ogr'])

            # ...and when we query ept provider metadata directly it should also report sublayers for ept.json files...
            self.assertEqual([l.providerKey() for l in ept_provider_metadata.querySublayers(
                unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', Qgis.SublayerQueryFlags(Qgis.SublayerQueryFlag.FastScan))], ['ept'])

            # ... but when we query the provider registry itself, it should ONLY report the ept provider sublayers
            self.assertEqual([l.providerKey() for l in QgsProviderRegistry.instance().querySublayers(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', Qgis.SublayerQueryFlags(Qgis.SublayerQueryFlag.FastScan))], ['ept'])

        provider1 = TestProviderMetadata('p1')
        provider2 = TestProviderMetadata('p2')

        self.assertFalse(QgsProviderRegistry.instance().querySublayers('test_uri'))

        self.assertTrue(QgsProviderRegistry.instance().registerProvider(provider1))
        self.assertTrue(QgsProviderRegistry.instance().registerProvider(provider2))

        self.assertCountEqual([p.providerKey() for p in QgsProviderRegistry.instance().querySublayers('test_uri')],
                              ['p1', 'p2'])


if __name__ == '__main__':
    unittest.main()
