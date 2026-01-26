"""QGIS Unit tests for QgsActionWidgetWrapper.

From build dir, run: ctest -R PyQgsActionWidgetWrapper -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alessandro Pasotti"
__date__ = "16/08/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

from qgis.PyQt.QtCore import QUuid
from qgis.PyQt.QtWidgets import QPushButton, QWidget
from qgis.core import QgsAction, QgsVectorLayer
from qgis.gui import QgsActionWidgetWrapper
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()

from qgis.testing import QGISAPP


class TestQgsActionWidgetWrapper(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=flddate:datetime",
            "test_layer",
            "memory",
        )

        cls.action_id1 = QUuid.createUuid()
        cls.action_id2 = QUuid.createUuid()
        cls.action_id3 = QUuid.createUuid()
        cls.action1 = QgsAction(
            cls.action_id1,
            QgsAction.ActionType.GenericPython,
            "Test Action 1 Desc",
            "i=1",
            "",
            False,
            "Test Action 1 Short Title",
        )
        cls.action2 = QgsAction(
            cls.action_id2,
            QgsAction.ActionType.GenericPython,
            "Test Action 2 Desc",
            "i=2",
            QGISAPP.appIconPath(),
            False,
            "Test Action 2 Short Title",
        )
        cls.action3 = QgsAction(
            cls.action_id3,
            QgsAction.ActionType.GenericPython,
            "Test Action 3 Desc",
            "i=3",
            "",
            False,
        )

    @classmethod
    def tearDownClass(cls):
        cls.layer = None
        cls.manager = None
        super().tearDownClass()

    def testWrapper(self):

        wrapper = QgsActionWidgetWrapper(self.layer, None, None)
        self.assertFalse(wrapper.valid())
        wrapper.setAction(self.action1)
        self.assertTrue(wrapper.valid())

        parent = QWidget()
        button = wrapper.createWidget(parent)
        self.assertIsInstance(button, QPushButton)

        wrapper.initWidget(button)
        self.assertEqual(button.text(), "Test Action 1 Short Title")
        self.assertEqual(button.toolTip(), "Test Action 1 Desc")
        self.assertTrue(button.icon().isNull())

        wrapper.setAction(self.action2)
        button = wrapper.createWidget(parent)
        wrapper.initWidget(button)
        self.assertTrue(wrapper.valid())
        self.assertEqual(button.text(), "")
        self.assertFalse(button.icon().isNull())
        self.assertEqual(button.toolTip(), "Test Action 2 Desc")

        wrapper.setAction(self.action3)
        button = wrapper.createWidget(parent)
        wrapper.initWidget(button)
        self.assertTrue(wrapper.valid())
        self.assertEqual(button.text(), "Test Action 3 Desc")
        self.assertTrue(button.icon().isNull())
        self.assertEqual(button.toolTip(), "")


if __name__ == "__main__":
    unittest.main()
