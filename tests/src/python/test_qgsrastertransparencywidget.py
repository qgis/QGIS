"""QGIS Unit tests for QgsRasterRange.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "07/06/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

import pathlib

from qgis.core import QgsRasterLayer, QgsRasterRange
from qgis.gui import QgsMapCanvas, QgsRasterTransparencyWidget
from qgis.testing import TestCase, unittest
from qgis.testing.mocked import get_iface

from utilities import unitTestDataPath


class TestQgsRasterTransparencyWidget(TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()
        cls.iface = get_iface()

    @staticmethod
    def no_data_values(layer: QgsRasterLayer):
        return [n.min() for n in layer.dataProvider().userNoDataValues(1)]

    def test_transparency_widget(self):
        path = pathlib.Path(unitTestDataPath()) / "landsat_4326.tif"
        self.assertTrue(path.is_file())
        layer = QgsRasterLayer(path.as_posix())
        self.assertTrue(layer.isValid())
        canvas = QgsMapCanvas()
        canvas.setLayers([layer])

        no_data_value = -99
        nd_ref = [no_data_value]
        layer.dataProvider().setUserNoDataValue(
            1, [QgsRasterRange(no_data_value, no_data_value)]
        )
        nd0 = self.no_data_values(layer)
        self.assertListEqual(nd0, nd_ref)

        w = QgsRasterTransparencyWidget(layer, canvas)
        self.assertIsInstance(w, QgsRasterTransparencyWidget)
        nd1 = self.no_data_values(layer)
        self.assertListEqual(
            nd1,
            nd_ref,
            msg='Widget initialization should not change the "no data value"',
        )

        w.syncToLayer()
        nd2 = self.no_data_values(layer)
        self.assertListEqual(nd2, nd_ref, msg='syncToLayer changed the "no data value"')

        w.syncToLayer()
        nd3 = self.no_data_values(layer)
        self.assertListEqual(
            nd3, nd_ref, msg='repeated syncToLayer changed the "no data value"'
        )

        w.apply()
        nd4 = self.no_data_values(layer)
        self.assertListEqual(
            nd4, nd_ref, msg='apply changed the "no data value" but should not'
        )

        w.apply()
        nd5 = self.no_data_values(layer)
        self.assertListEqual(
            nd5, nd_ref, msg='repeated apply changed the "no data value" but should not'
        )


if __name__ == "__main__":
    unittest.main()
