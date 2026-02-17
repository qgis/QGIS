"""QGIS Unit tests for QgsCollapsibleGroupBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alexander Bruy"
__date__ = "08/10/2025"
__copyright__ = "Copyright 2025, The QGIS Project"


import unittest

from qgis.gui import QgsCollapsibleGroupBox
from qgis.PyQt.QtWidgets import (
    QComboBox,
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QToolButton,
    QVBoxLayout,
)
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsCollapsibleGroupBox(QgisTestCase):
    def testExpandCollapse(self):
        """Test expanding/collapsing"""
        w = QgsCollapsibleGroupBox()

        layout = QVBoxLayout()
        line_edit = QLineEdit()
        line_edit.setObjectName("line_edit")
        layout.addWidget(line_edit)
        frame = QFrame()
        frame.setObjectName("frame")
        frame_layout = QHBoxLayout()
        label = QLabel("Test label")
        label.setObjectName("label")
        frame_layout.addWidget(label)
        combo_box = QComboBox()
        combo_box.setObjectName("combo_box")
        frame_layout.addWidget(combo_box)
        frame.setLayout(frame_layout)
        layout.addWidget(frame)

        self.assertFalse(w.isCollapsed())
        # all child widgets should be visible and should not have CollGrpBxHide
        # property set
        for c in w.children():
            if isinstance(c, QToolButton):
                continue
            self.assertTrue(c.isVisible())
            self.assertNotIn("CollGrpBxHide", c.dynamicPropertyNames())

        w.setCollapsed(True)
        self.assertTrue(w.isCollapsed())
        # all child widgets should be hidden now and should have CollGrpBxHide
        # property set to True
        for c in w.children():
            if isinstance(c, QToolButton):
                continue
            self.assertFalse(c.isVisible())
            self.assertTrue(c.property("CollGrpBxHide"))

        w.setCollapsed(False)
        self.assertFalse(w.isCollapsed())
        # all child widgets should be shown now and should have CollGrpBxHide
        # property set to False
        for c in w.children():
            if isinstance(c, QToolButton):
                continue
            self.assertTrue(c.isVisible())
            self.assertFalse(c.property("CollGrpBxHide"))

        # hide frame and its content
        frame.hide()

        self.assertFalse(w.isCollapsed())
        # all child widgets, except frame and its content should be visible
        # all widgets should not have CollGrpBxHide property set
        for c in w.children():
            if isinstance(c, QToolButton):
                continue

            if c.objectName() and c.objectName() in ("frame", "label", "combo_box"):
                self.assertFalse(c.isVisible())
                self.assertNotIn("CollGrpBxHide", c.dynamicPropertyNames())
                continue

            self.assertTrue(c.isVisible())
            self.assertNotIn("CollGrpBxHide", c.dynamicPropertyNames())

        w.setCollapsed(True)
        self.assertTrue(w.isCollapsed())
        # all child widgets should be hidden now
        # all child widgets, except frame and its content should have CollGrpBxHide property set to True
        for c in w.children():
            if isinstance(c, QToolButton):
                continue

            if c.objectName() and c.objectName() in ("frame", "label", "combo_box"):
                self.assertFalse(c.isVisible())
                self.assertNotIn("CollGrpBxHide", c.dynamicPropertyNames())
                continue

            self.assertFalse(c.isVisible())
            self.assertTrue(c.property("CollGrpBxHide"))

        w.setCollapsed(False)
        self.assertFalse(w.isCollapsed())
        # all child widgets, except frame and its content should be visible
        # all child widgets, except frame and its content should have CollGrpBxHide property set to False
        for c in w.children():
            if isinstance(c, QToolButton):
                continue

            if c.objectName() and c.objectName() in ("frame", "label", "combo_box"):
                self.assertFalse(c.isVisible())
                self.assertNotIn("CollGrpBxHide", c.dynamicPropertyNames())
                continue

            self.assertTrue(c.isVisible())
            self.assertFalse(c.property("CollGrpBxHide"))

        # show frame and its content
        frame.show()

        self.assertFalse(w.isCollapsed())
        # all child widgets should be visible
        # all widgets, except frame and its content should have CollGrpBxHide property set to True
        for c in w.children():
            if isinstance(c, QToolButton):
                continue

            if c.objectName() and c.objectName() in ("frame", "label", "combo_box"):
                self.assertTrue(c.isVisible())
                self.assertNotIn("CollGrpBxHide", c.dynamicPropertyNames())
                continue

            self.assertTrue(c.isVisible())
            self.assertNotIn("CollGrpBxHide", c.dynamicPropertyNames())

        w.setCollapsed(True)
        self.assertTrue(w.isCollapsed())
        # all child widgets should be hidden now and should have CollGrpBxHide property set to True
        for c in w.children():
            if isinstance(c, QToolButton):
                continue

            self.assertFalse(c.isVisible())
            self.assertTrue(c.property("CollGrpBxHide"))

        w.setCollapsed(False)
        self.assertFalse(w.isCollapsed())
        # all child widgets should be visible and should have CollGrpBxHide property set to False
        for c in w.children():
            if isinstance(c, QToolButton):
                continue

            self.assertTrue(c.isVisible())
            self.assertFalse(c.property("CollGrpBxHide"))


if __name__ == "__main__":
    unittest.main()
