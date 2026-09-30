"""QGIS Unit tests for QgsMapLayerStyleManagerWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    QgsVectorLayer,
)
from qgis.gui import QgsMapLayerStyleManagerWidget
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtWidgets import QListView
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsMapLayerStyleManagerWidget(QgisTestCase):
    def testWidget(self):
        """
        Test basic widget behavior
        """
        vl = QgsVectorLayer(
            "Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
            "test",
            "memory",
        )
        widget = QgsMapLayerStyleManagerWidget(vl, None)

        # find model
        view = widget.findChildren(QListView)[0]
        self.assertEqual(view.objectName(), "mStyleList")
        model = view.model()

        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "default")

        self.assertEqual(widget.layer(), vl)

        # change layer

        vl2 = QgsVectorLayer(
            "Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
            "test",
            "memory",
        )
        vl2.styleManager().renameStyle("default", "My Style")
        vl2.styleManager().addStyleFromLayer("Another Style")

        widget.syncToLayer(vl2)
        self.assertEqual(widget.layer(), vl2)

        model = view.model()
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "Another Style")
        self.assertEqual(model.data(model.index(1, 0)), "My Style")

    def testWidgetNoLayer(self):
        """
        Test widget behavior when constructed with no layer
        """
        vl = QgsVectorLayer(
            "Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
            "test",
            "memory",
        )
        widget = QgsMapLayerStyleManagerWidget(None, None)

        # find model
        view = widget.findChildren(QListView)[0]
        self.assertEqual(view.objectName(), "mStyleList")

        # set layer

        vl = QgsVectorLayer(
            "Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
            "test",
            "memory",
        )
        vl.styleManager().renameStyle("default", "My Style")
        vl.styleManager().addStyleFromLayer("Another Style")

        widget.syncToLayer(vl)
        self.assertEqual(widget.layer(), vl)

        model = view.model()
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "Another Style")
        self.assertEqual(model.data(model.index(1, 0)), "My Style")

        widget.syncToLayer(None)
        self.assertIsNone(widget.layer())

        model = view.model()
        self.assertIsNone(model)


if __name__ == "__main__":
    unittest.main()
