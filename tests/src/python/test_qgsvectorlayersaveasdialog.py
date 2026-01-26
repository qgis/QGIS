"""QGIS Unit tests for QgsVectorLayerSaveAsDialog

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Benjamin Jakimow"
__date__ = "2023-04"
__copyright__ = "Copyright 2023, The QGIS Project"

import unittest
from qgis.testing import start_app, TestCase
from qgis.gui import QgsVectorLayerSaveAsDialog
from qgis.core import QgsFeature, edit, QgsVectorLayer

start_app()


class TestQgsMapCanvas(TestCase):

    def testOpenDialog(self):

        layer = QgsVectorLayer(
            "Polygon?crs=epsg:4326&field=text:string", "layer", "memory"
        )

        with edit(layer):
            f = QgsFeature(layer.fields())
            f.setAttribute("text", "foo")
            layer.addFeature(f)

            f = QgsFeature(layer.fields())
            f.setAttribute("text", "bar")
            layer.addFeature(f)

        layer.select(layer.allFeatureIds()[0])
        options = QgsVectorLayerSaveAsDialog.Options(
            QgsVectorLayerSaveAsDialog.Option.SelectedOnly
            | QgsVectorLayerSaveAsDialog.Option.Metadata
        )
        d = QgsVectorLayerSaveAsDialog(layer, options=options)
        self.assertIsInstance(d, QgsVectorLayerSaveAsDialog)
        d.setOnlySelected(True)
        self.assertTrue(d.onlySelected())


if __name__ == "__main__":
    unittest.main()
