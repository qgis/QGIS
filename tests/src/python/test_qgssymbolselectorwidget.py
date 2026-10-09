"""QGIS Unit tests for QgsSymbolSelectorWidget.

From build dir, run: ctest -R PyQgsSymbolSelectorWidget -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Denis Rouzaud"
__date__ = "14/08/2026"
__copyright__ = "Copyright 2026, The QGIS Project"

import unittest

from qgis.core import QgsFillSymbol, QgsStyle
from qgis.gui import QgsSymbolSelectorWidget, QgsSymbolWidgetContext
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsSymbolSelectorWidget(QgisTestCase):
    def test_no_symbol(self):
        """
        Test that a widget created without a symbol survives the operations
        acting on the current node of the symbol layers tree. That tree is empty
        in this case, so nothing is ever current in it and each of these must be
        a no-op rather than a crash.
        """
        widget = QgsSymbolSelectorWidget(None, QgsStyle.defaultStyle(), None, None)
        self.assertIsNone(widget.symbol())

        # reaches layerChanged(), where the crash was
        widget.setContext(QgsSymbolWidgetContext())

        widget.layerChanged()
        widget.updateLayerPreview()
        widget.addLayer()
        widget.removeLayer()
        widget.duplicateLayer()
        widget.moveLayerUp()
        widget.moveLayerDown()
        widget.lockLayer()

        self.assertIsNone(widget.symbol())

    def test_symbol(self):
        symbol = QgsFillSymbol.createSimple({"color": "#ff0000"})
        widget = QgsSymbolSelectorWidget(symbol, QgsStyle.defaultStyle(), None, None)
        self.assertEqual(widget.symbol(), symbol)

        widget.setContext(QgsSymbolWidgetContext())
        self.assertEqual(widget.symbol(), symbol)


if __name__ == "__main__":
    unittest.main()
