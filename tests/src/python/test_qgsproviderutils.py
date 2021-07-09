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
        self.assertTrue(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers))

        # retry with retrieving geometry types
        sublayers = QgsProviderRegistry.instance().querySublayers(uri, Qgis.SublayerQueryFlag.ResolveGeometryType)
        # now we have all the details
        self.assertEqual(len(sublayers), 3)
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers))


if __name__ == '__main__':
    unittest.main()
