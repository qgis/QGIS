# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProviderUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '30/06/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

from qgis.core import (
    Qgis,
    QgsWkbTypes,
    QgsProviderRegistry,
    QgsProviderUtils
)

from qgis.testing import (
    unittest,
    start_app
)
from utilities import unitTestDataPath

app = start_app()


class TestQgsProviderUtils(unittest.TestCase):

    def test_sublayerDetailsAreIncomplete(self):
        """
        Test sublayerDetailsAreIncomplete
        """
        uri = unitTestDataPath() + '/mixed_types.TAB'

        # surface scan only
        sublayers = QgsProviderRegistry.instance().querySublayers(uri)
        self.assertEqual(len(sublayers), 1)
        self.assertEqual(sublayers[0].wkbType(), QgsWkbTypes.Unknown)

        # need to resolve geometry types for complete details about this uri!
        self.assertTrue(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers, False))
        self.assertTrue(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers, True))

        # retry with retrieving geometry types
        sublayers = QgsProviderRegistry.instance().querySublayers(uri, Qgis.SublayerQueryFlag.ResolveGeometryType)
        # now we have all the details
        self.assertEqual(len(sublayers), 3)
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers, False))
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers, True))

        # this geopackage file requires manually requesting feature counts
        uri = unitTestDataPath() + '/mixed_layers.gpkg'

        # surface scan only
        sublayers = QgsProviderRegistry.instance().querySublayers(uri)
        self.assertEqual(len(sublayers), 4)
        self.assertEqual(sublayers[0].name(), 'band1')
        self.assertEqual(sublayers[1].name(), 'band2')
        self.assertEqual(sublayers[2].name(), 'points')
        self.assertEqual(sublayers[2].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(sublayers[3].name(), 'lines')
        self.assertEqual(sublayers[3].featureCount(), Qgis.FeatureCountState.Uncounted)

        # need to count features for complete details about this uri!
        self.assertTrue(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers, False))
        # ...unless we are ignoring unknown feature counts, that is...
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers, True))

        # retry with retrieving feature count
        sublayers = QgsProviderRegistry.instance().querySublayers(uri, Qgis.SublayerQueryFlag.CountFeatures)
        # now we have all the details
        self.assertEqual(len(sublayers), 4)
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers, True))
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers, False))
        self.assertEqual(sublayers[0].name(), 'band1')
        self.assertEqual(sublayers[1].name(), 'band2')
        self.assertEqual(sublayers[2].name(), 'points')
        self.assertEqual(sublayers[2].featureCount(), 0)
        self.assertEqual(sublayers[3].name(), 'lines')
        self.assertEqual(sublayers[3].featureCount(), 6)

    def test_suggestLayerNameFromFilePath(self):
        """
        test suggestLayerNameFromFilePath
        """
        self.assertEqual(QgsProviderUtils.suggestLayerNameFromFilePath(''), '')
        self.assertEqual(QgsProviderUtils.suggestLayerNameFromFilePath('/home/me/data/rivers.shp'), 'rivers')
        # adf files should return parent dir name
        self.assertEqual(QgsProviderUtils.suggestLayerNameFromFilePath('/home/me/data/rivers/hdr.adf'), 'rivers')


if __name__ == '__main__':
    unittest.main()
