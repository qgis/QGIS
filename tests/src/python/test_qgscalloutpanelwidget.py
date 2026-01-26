"""QGIS Unit tests for QgsCalloutPanelWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtWidgets import QComboBox
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsBalloonCallout, QgsSimpleLineCallout
from qgis.gui import QgsCalloutPanelWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCalloutPanelWidget(QgisTestCase):

    def testWidget(self):
        widget = QgsCalloutPanelWidget()

        changed_spy = QSignalSpy(widget.calloutChanged)

        callout = QgsSimpleLineCallout()
        callout.setEnabled(True)
        widget.setCallout(callout)
        self.assertEqual(len(changed_spy), 1)

        style_combo = widget.findChild(QComboBox, "mCalloutStyleComboBox")
        self.assertIsInstance(style_combo, QComboBox)
        style_combo.setCurrentIndex(style_combo.findData("curved"))
        self.assertEqual(len(changed_spy), 2)

        self.assertEqual(widget.callout().type(), "curved")

        style_combo.setCurrentIndex(style_combo.findData("balloon"))
        self.assertEqual(len(changed_spy), 3)

        self.assertEqual(widget.callout().type(), "balloon")
        callout = QgsBalloonCallout()
        callout.setWedgeWidth(11)

        widget.setCallout(callout)
        self.assertEqual(len(changed_spy), 4)

        self.assertEqual(widget.callout().wedgeWidth(), 11)


if __name__ == "__main__":
    unittest.main()
