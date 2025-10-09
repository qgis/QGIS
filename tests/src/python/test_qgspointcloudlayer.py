"""QGIS Unit tests for QgsPointCloudLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import QgsPointCloudLayer, QgsProviderRegistry, Qgis, QgsProject
import unittest
import tempfile
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsPointCloudLayer(QgisTestCase):

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def test_legend_settings(self):
        ml = QgsPointCloudLayer(
            (
                self.get_test_data_path("point_clouds")
                / "ept"
                / "sunshine-coast"
                / "ept.json"
            ).as_posix(),
            "test",
            "ept",
        )
        self.assertTrue(ml.isValid())

        self.assertFalse(ml.legend().flags() & Qgis.MapLayerLegendFlag.ExcludeByDefault)
        ml.legend().setFlag(Qgis.MapLayerLegendFlag.ExcludeByDefault)
        self.assertTrue(ml.legend().flags() & Qgis.MapLayerLegendFlag.ExcludeByDefault)

        p = QgsProject()
        p.addMapLayer(ml)

        # test saving and restoring
        with tempfile.TemporaryDirectory() as temp:
            self.assertTrue(p.write(temp + "/test.qgs"))

            p2 = QgsProject()
            self.assertTrue(p2.read(temp + "/test.qgs"))

            ml2 = list(p2.mapLayers().values())[0]
            self.assertEqual(ml2.name(), ml.name())

            self.assertTrue(
                ml2.legend().flags() & Qgis.MapLayerLegendFlag.ExcludeByDefault
            )


if __name__ == "__main__":
    unittest.main()
