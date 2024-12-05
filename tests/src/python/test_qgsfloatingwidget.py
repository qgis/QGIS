"""QGIS Unit tests for QgsFloatingWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "26/04/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtWidgets import QGridLayout, QWidget
from qgis.gui import QgsFloatingWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsFloatingWidget(QgisTestCase):

    def testAnchor(self):
        """test setting anchor point for widget"""
        main_frame = QWidget()
        gl = QGridLayout()
        main_frame.setLayout(gl)
        main_frame.setMinimumSize(800, 600)

        anchor_widget = QWidget(main_frame)
        anchor_widget.setMinimumSize(300, 200)
        main_frame.layout().addWidget(anchor_widget, 1, 1)
        gl.setColumnStretch(0, 1)
        gl.setColumnStretch(1, 0)
        gl.setColumnStretch(2, 1)
        gl.setRowStretch(0, 1)
        gl.setRowStretch(1, 0)
        gl.setRowStretch(2, 1)

        # 103 = WA_DontShowOnScreen (not available in PyQt)
        main_frame.setAttribute(103)
        main_frame.show()

        fw = QgsFloatingWidget(main_frame)
        fw.setMinimumSize(100, 50)
        fw.setAnchorWidget(anchor_widget)

        tests = [
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 250,
                "y": 200,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopMiddle,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 200,
                "y": 200,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopRight,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 150,
                "y": 200,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.MiddleLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 250,
                "y": 175,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.Middle,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 200,
                "y": 175,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.MiddleRight,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 150,
                "y": 175,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.BottomLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 250,
                "y": 150,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.BottomMiddle,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 200,
                "y": 150,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.BottomRight,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "x": 150,
                "y": 150,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopMiddle,
                "x": 400,
                "y": 200,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.TopRight,
                "x": 550,
                "y": 200,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.MiddleLeft,
                "x": 250,
                "y": 300,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.Middle,
                "x": 400,
                "y": 300,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.MiddleRight,
                "x": 550,
                "y": 300,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.BottomLeft,
                "x": 250,
                "y": 400,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.BottomMiddle,
                "x": 400,
                "y": 400,
            },
            {
                "anchorPoint": QgsFloatingWidget.AnchorPoint.TopLeft,
                "widgetAnchorPoint": QgsFloatingWidget.AnchorPoint.BottomRight,
                "x": 550,
                "y": 400,
            },
        ]

        for t in tests:
            fw.setAnchorPoint(t["anchorPoint"])
            fw.setAnchorWidgetPoint(t["widgetAnchorPoint"])
            self.assertEqual(fw.pos().x(), t["x"])
            self.assertEqual(fw.pos().y(), t["y"])

    def testMovingResizingAnchorWidget(self):
        """test that moving or resizing the anchor widget updates the floating widget position"""
        main_frame = QWidget()
        gl = QGridLayout()
        main_frame.setLayout(gl)
        main_frame.setMinimumSize(800, 600)

        anchor_widget = QWidget(main_frame)
        anchor_widget.setFixedSize(300, 200)
        main_frame.layout().addWidget(anchor_widget, 1, 1)
        gl.setColumnStretch(0, 1)
        gl.setColumnStretch(1, 0)
        gl.setColumnStretch(2, 1)
        gl.setRowStretch(0, 1)
        gl.setRowStretch(1, 0)
        gl.setRowStretch(2, 1)

        # 103 = WA_DontShowOnScreen (not available in PyQt)
        main_frame.setAttribute(103)
        main_frame.show()

        fw = QgsFloatingWidget(main_frame)
        fw.setMinimumSize(100, 50)
        fw.setAnchorWidget(anchor_widget)

        fw.setAnchorPoint(QgsFloatingWidget.AnchorPoint.TopLeft)
        fw.setAnchorWidgetPoint(QgsFloatingWidget.AnchorPoint.TopLeft)

        self.assertEqual(fw.pos().x(), 250)
        self.assertEqual(fw.pos().y(), 200)

        # now resize anchor widget
        anchor_widget.setFixedSize(400, 300)
        # force layout recalculation
        main_frame.layout().invalidate()
        main_frame.layout().activate()
        self.assertEqual(fw.pos().x(), 200)
        self.assertEqual(fw.pos().y(), 150)

        # now move anchor widget
        anchor_widget.move(100, 110)
        self.assertEqual(fw.pos().x(), 100)
        self.assertEqual(fw.pos().y(), 110)

    def testResizingParentWidget(self):
        """test resizing parent widget correctly repositions floating widget"""
        main_frame = QWidget()
        gl = QGridLayout()
        main_frame.setLayout(gl)
        main_frame.setMinimumSize(800, 600)

        anchor_widget = QWidget(main_frame)
        anchor_widget.setFixedSize(300, 200)
        main_frame.layout().addWidget(anchor_widget, 1, 1)
        gl.setColumnStretch(0, 1)
        gl.setColumnStretch(1, 0)
        gl.setColumnStretch(2, 1)
        gl.setRowStretch(0, 1)
        gl.setRowStretch(1, 0)
        gl.setRowStretch(2, 1)

        # 103 = WA_DontShowOnScreen (not available in PyQt)
        main_frame.setAttribute(103)
        main_frame.show()

        fw = QgsFloatingWidget(main_frame)
        fw.setMinimumSize(100, 50)
        fw.setAnchorWidget(anchor_widget)

        fw.setAnchorPoint(QgsFloatingWidget.AnchorPoint.TopLeft)
        fw.setAnchorWidgetPoint(QgsFloatingWidget.AnchorPoint.TopLeft)

        self.assertEqual(fw.pos().x(), 250)
        self.assertEqual(fw.pos().y(), 200)

        # now resize parent widget
        main_frame.setFixedSize(1000, 800)
        # force layout recalculation
        main_frame.layout().invalidate()
        main_frame.layout().activate()
        self.assertEqual(fw.pos().x(), 350)
        self.assertEqual(fw.pos().y(), 300)

    def testPositionConstrainedToParent(self):
        """test that floating widget will be placed inside parent when possible"""
        main_frame = QWidget()
        gl = QGridLayout()
        main_frame.setLayout(gl)
        main_frame.setMinimumSize(800, 600)

        anchor_widget = QWidget(main_frame)
        anchor_widget.setFixedSize(300, 200)
        main_frame.layout().addWidget(anchor_widget, 1, 1)
        gl.setColumnStretch(0, 1)
        gl.setColumnStretch(1, 0)
        gl.setColumnStretch(2, 1)
        gl.setRowStretch(0, 1)
        gl.setRowStretch(1, 0)
        gl.setRowStretch(2, 1)

        main_frame.setAttribute(103)
        main_frame.show()

        fw = QgsFloatingWidget(main_frame)
        fw.setMinimumSize(300, 50)
        fw.setAnchorWidget(anchor_widget)

        fw.setAnchorPoint(QgsFloatingWidget.AnchorPoint.TopRight)
        fw.setAnchorWidgetPoint(QgsFloatingWidget.AnchorPoint.TopLeft)

        # x-position should be 0, not -50
        self.assertEqual(fw.pos().x(), 0)

        fw.setAnchorPoint(QgsFloatingWidget.AnchorPoint.TopLeft)
        fw.setAnchorWidgetPoint(QgsFloatingWidget.AnchorPoint.TopRight)

        # x-position should be 500, not 600
        self.assertEqual(fw.pos().x(), 500)


if __name__ == "__main__":
    unittest.main()
