# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFloatingWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '26/04/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtWidgets import QWidget, QGridLayout

from qgis.gui import QgsFloatingWidget

from qgis.testing import start_app, unittest

start_app()


class TestQgsFloatingWidget(unittest.TestCase):

    def testAnchor(self):
        """ test setting anchor point for widget """
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

        fw = qgis.gui.QgsFloatingWidget(main_frame)
        fw.setMinimumSize(100, 50)
        fw.setAnchorWidget(anchor_widget)

        tests = [{'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 250, 'y': 200},
                 {'anchorPoint': QgsFloatingWidget.TopMiddle, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 200, 'y': 200},
                 {'anchorPoint': QgsFloatingWidget.TopRight, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 150, 'y': 200},
                 {'anchorPoint': QgsFloatingWidget.MiddleLeft, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 250, 'y': 175},
                 {'anchorPoint': QgsFloatingWidget.Middle, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 200, 'y': 175},
                 {'anchorPoint': QgsFloatingWidget.MiddleRight, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 150, 'y': 175},
                 {'anchorPoint': QgsFloatingWidget.BottomLeft, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 250, 'y': 150},
                 {'anchorPoint': QgsFloatingWidget.BottomMiddle, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 200, 'y': 150},
                 {'anchorPoint': QgsFloatingWidget.BottomRight, 'widgetAnchorPoint': QgsFloatingWidget.TopLeft, 'x': 150, 'y': 150},
                 {'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.TopMiddle, 'x': 400, 'y': 200},
                 {'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.TopRight, 'x': 550, 'y': 200},
                 {'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.MiddleLeft, 'x': 250, 'y': 300},
                 {'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.Middle, 'x': 400, 'y': 300},
                 {'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.MiddleRight, 'x': 550, 'y': 300},
                 {'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.BottomLeft, 'x': 250, 'y': 400},
                 {'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.BottomMiddle, 'x': 400, 'y': 400},
                 {'anchorPoint': QgsFloatingWidget.TopLeft, 'widgetAnchorPoint': QgsFloatingWidget.BottomRight, 'x': 550, 'y': 400}]

        for t in tests:
            fw.setAnchorPoint(t['anchorPoint'])
            fw.setAnchorWidgetPoint(t['widgetAnchorPoint'])
            self.assertEqual(fw.pos().x(), t['x'])
            self.assertEqual(fw.pos().y(), t['y'])

    def testMovingResizingAnchorWidget(self):
        """ test that moving or resizing the anchor widget updates the floating widget position """
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

        fw = qgis.gui.QgsFloatingWidget(main_frame)
        fw.setMinimumSize(100, 50)
        fw.setAnchorWidget(anchor_widget)

        fw.setAnchorPoint(QgsFloatingWidget.TopLeft)
        fw.setAnchorWidgetPoint(QgsFloatingWidget.TopLeft)

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
        """ test resizing parent widget correctly repositions floating widget"""
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

        fw = qgis.gui.QgsFloatingWidget(main_frame)
        fw.setMinimumSize(100, 50)
        fw.setAnchorWidget(anchor_widget)

        fw.setAnchorPoint(QgsFloatingWidget.TopLeft)
        fw.setAnchorWidgetPoint(QgsFloatingWidget.TopLeft)

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
        """ test that floating widget will be placed inside parent when possible """
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

        fw = qgis.gui.QgsFloatingWidget(main_frame)
        fw.setMinimumSize(300, 50)
        fw.setAnchorWidget(anchor_widget)

        fw.setAnchorPoint(QgsFloatingWidget.TopRight)
        fw.setAnchorWidgetPoint(QgsFloatingWidget.TopLeft)

        # x-position should be 0, not -50
        self.assertEqual(fw.pos().x(), 0)

        fw.setAnchorPoint(QgsFloatingWidget.TopLeft)
        fw.setAnchorWidgetPoint(QgsFloatingWidget.TopRight)

        # x-position should be 500, not 600
        self.assertEqual(fw.pos().x(), 500)


if __name__ == '__main__':
    unittest.main()
