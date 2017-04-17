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


if __name__ == '__main__':
    unittest.main()
