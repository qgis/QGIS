# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsEllipsoidUtils

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/4/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsEllipsoidUtils)
from qgis.testing import start_app, unittest

app = start_app()


class TestQgsEllipsoidUtils(unittest.TestCase):

    def testParams(self):
        """
        Test fetching ellipsoid parameters
        """

        # run each test twice, so that ellipsoid is fetched from cache on the second time

        for i in range(2):
            params = QgsEllipsoidUtils.ellipsoidParameters("WGS84")
            self.assertTrue(params.valid)
            self.assertEqual(params.semiMajor, 6378137.0)
            self.assertAlmostEqual(params.semiMinor, 6356752.314245179, 5)
            self.assertAlmostEqual(params.inverseFlattening, 298.257223563, 5)
            self.assertFalse(params.useCustomParameters)
            self.assertEqual(params.crs.authid(), 'EPSG:4030')

        for i in range(2):
            params = QgsEllipsoidUtils.ellipsoidParameters("Ganymede2000")
            self.assertTrue(params.valid)
            self.assertEqual(params.semiMajor, 2632400.0)
            self.assertEqual(params.semiMinor, 2632350.0)
            self.assertEqual(params.inverseFlattening, 52648.0)
            self.assertFalse(params.useCustomParameters)
            self.assertEqual(params.crs.authid(), '')

        # using parameters
        for i in range(2):
            params = QgsEllipsoidUtils.ellipsoidParameters("PARAMETER:2631400:2341350")
            self.assertTrue(params.valid)
            self.assertEqual(params.semiMajor, 2631400.0)
            self.assertEqual(params.semiMinor, 2341350.0)
            self.assertAlmostEqual(params.inverseFlattening, 9.07223, 4)
            self.assertTrue(params.useCustomParameters)
            self.assertEqual(params.crs.authid(), '')

        # invalid
        for i in range(2):
            params = QgsEllipsoidUtils.ellipsoidParameters("Babies first ellipsoid!")
            self.assertFalse(params.valid)

    def testAcronyms(self):
        self.assertTrue('WGS84' in QgsEllipsoidUtils.acronyms())
        self.assertTrue('Ganymede2000' in QgsEllipsoidUtils.acronyms())

    def testDefinitions(self):
        defs = QgsEllipsoidUtils.definitions()

        gany_defs = [d for d in defs if d.acronym == 'Ganymede2000'][0]
        self.assertEqual(gany_defs.acronym, 'Ganymede2000')
        self.assertEqual(gany_defs.description, 'Ganymede2000')
        self.assertTrue(gany_defs.parameters.valid)
        self.assertEqual(gany_defs.parameters.semiMajor, 2632400.0)
        self.assertEqual(gany_defs.parameters.semiMinor, 2632350.0)
        self.assertEqual(gany_defs.parameters.inverseFlattening, 52648.0)
        self.assertFalse(gany_defs.parameters.useCustomParameters)
        self.assertEqual(gany_defs.parameters.crs.authid(), '')


if __name__ == '__main__':
    unittest.main()
