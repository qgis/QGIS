# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLegendPatchShape.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '05/04/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsLegendPatchShape,
                       QgsGeometry,
                       QgsSymbol
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath


start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLegendPatchShape(unittest.TestCase):

    def testBasic(self):
        shape = QgsLegendPatchShape(QgsSymbol.Line, QgsGeometry.fromWkt('LineString( 0 0, 1 1)'), False)
        self.assertEqual(shape.symbolType(), QgsSymbol.Line)
        self.assertEqual(shape.geometry().asWkt(), 'LineString (0 0, 1 1)')
        self.assertFalse(shape.preserveAspectRatio())

        shape.setSymbolType(QgsSymbol.Marker)
        self.assertEqual(shape.symbolType(), QgsSymbol.Marker)

        shape.setGeometry(QgsGeometry.fromWkt('Multipoint( 1 1, 2 2)'))
        self.assertEqual(shape.geometry().asWkt(), 'MultiPoint ((1 1),(2 2))')

        shape.setPreserveAspectRatio(True)
        self.assertTrue(shape.preserveAspectRatio())


if __name__ == '__main__':
    unittest.main()
