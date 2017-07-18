# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutPageCollection

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import QgsUnitTypes, QgsLayout, QgsProject, QgsLayoutPageCollection, QgsSimpleFillSymbolLayer, QgsFillSymbol
from qgis.PyQt.QtCore import Qt
from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutPageCollection(unittest.TestCase):

    def testLayout(self):
        # test that layouts have a collection
        p = QgsProject()
        l = QgsLayout(p)
        self.assertTrue(l.pageCollection())
        self.assertEqual(l.pageCollection().layout(), l)

    def testSymbol(self):
        """
        Test setting a page symbol for the collection
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()
        self.assertTrue(collection.pageStyleSymbol())

        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeColor(Qt.red)
        fill.setStrokeWidth(6)
        collection.setPageStyleSymbol(fill_symbol)
        self.assertEqual(collection.pageStyleSymbol().symbolLayer(0).color().name(), '#00ff00')
        self.assertEqual(collection.pageStyleSymbol().symbolLayer(0).strokeColor().name(), '#ff0000')


if __name__ == '__main__':
    unittest.main()
