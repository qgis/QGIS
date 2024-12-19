"""QGIS Unit tests for QgsOverlayWidgetLayout

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '24/1/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import unittest

from qgis.PyQt.QtCore import Qt, QPoint
from qgis.PyQt.QtWidgets import QWidget
from qgis.gui import (
    QgsOverlayWidgetLayout
)
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestQgsOverlayWidgetLayout(QgisTestCase):

    def testLayout(self):
        parent = QWidget()
        parent.setFixedSize(600, 400)

        layout = QgsOverlayWidgetLayout()
        layout.setContentsMargins(
            5, 6, 7, 8
        )
        parent.setLayout(layout)
        parent.show()
        self.assertEqual(parent.rect().right(), 599)
        self.assertEqual(parent.rect().bottom(), 399)

        child_left_1 = QWidget()
        child_left_1.setFixedWidth(30)
        layout.addWidget(child_left_1, Qt.Edge.LeftEdge)
        child_left_1.show()
        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().topLeft()),
            QPoint(5, 6))
        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().bottomRight()),
            QPoint(34, 390))

        child_left_2 = QWidget()
        child_left_2.setFixedWidth(40)
        layout.addWidget(child_left_2, Qt.Edge.LeftEdge)
        child_left_2.show()

        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().topLeft()),
            QPoint(5, 6))
        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().bottomRight()),
            QPoint(34, 390))

        self.assertEqual(
            child_left_2.mapToParent(child_left_2.rect().topLeft()),
            QPoint(35, 6))
        self.assertEqual(
            child_left_2.mapToParent(child_left_2.rect().bottomRight()),
            QPoint(74, 390))

        layout.setHorizontalSpacing(12)
        child_right_1 = QWidget()
        child_right_1.setFixedWidth(40)
        layout.addWidget(child_right_1, Qt.Edge.RightEdge)
        child_right_1.show()
        child_right_2 = QWidget()
        child_right_2.setFixedWidth(80)
        layout.addWidget(child_right_2, Qt.Edge.RightEdge)
        child_right_2.show()

        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().topLeft()),
            QPoint(5, 6))
        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().bottomRight()),
            QPoint(34, 390))
        self.assertEqual(
            child_left_2.mapToParent(child_left_2.rect().topLeft()),
            QPoint(47, 6))
        self.assertEqual(
            child_left_2.mapToParent(child_left_2.rect().bottomRight()),
            QPoint(86, 390))
        self.assertEqual(
            child_right_1.mapToParent(child_right_1.rect().topLeft()),
            QPoint(552, 6))
        self.assertEqual(
            child_right_1.mapToParent(child_right_1.rect().bottomRight()),
            QPoint(591, 390))
        self.assertEqual(
            child_right_2.mapToParent(child_right_2.rect().topLeft()),
            QPoint(460, 6))
        self.assertEqual(
            child_right_2.mapToParent(child_right_2.rect().bottomRight()),
            QPoint(539, 390))

        layout.setVerticalSpacing(13)
        child_top_1 = QWidget()
        child_top_1.setFixedHeight(20)
        layout.addWidget(child_top_1, Qt.Edge.TopEdge)
        child_top_1.show()
        child_top_2 = QWidget()
        child_top_2.setFixedHeight(30)
        layout.addWidget(child_top_2, Qt.Edge.TopEdge)
        child_top_2.show()

        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().topLeft()),
            QPoint(5, 6))
        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().bottomRight()),
            QPoint(34, 390))
        self.assertEqual(
            child_left_2.mapToParent(child_left_2.rect().topLeft()),
            QPoint(47, 6))
        self.assertEqual(
            child_left_2.mapToParent(child_left_2.rect().bottomRight()),
            QPoint(86, 390))
        self.assertEqual(
            child_right_1.mapToParent(child_right_1.rect().topLeft()),
            QPoint(552, 6))
        self.assertEqual(
            child_right_1.mapToParent(child_right_1.rect().bottomRight()),
            QPoint(591, 390))
        self.assertEqual(
            child_right_2.mapToParent(child_right_2.rect().topLeft()),
            QPoint(460, 6))
        self.assertEqual(
            child_right_2.mapToParent(child_right_2.rect().bottomRight()),
            QPoint(539, 390))
        self.assertEqual(
            child_top_1.mapToParent(child_top_1.rect().topLeft()),
            QPoint(99, 6))
        self.assertEqual(
            child_top_1.mapToParent(child_top_1.rect().bottomRight()),
            QPoint(447, 25))
        self.assertEqual(
            child_top_2.mapToParent(child_top_2.rect().topLeft()),
            QPoint(99, 39))
        self.assertEqual(
            child_top_2.mapToParent(child_top_2.rect().bottomRight()),
            QPoint(447, 68))

        child_bottom_1 = QWidget()
        child_bottom_1.setFixedHeight(20)
        layout.addWidget(child_bottom_1, Qt.Edge.BottomEdge)
        child_bottom_1.show()
        child_bottom_2 = QWidget()
        child_bottom_2.setFixedHeight(30)
        layout.addWidget(child_bottom_2, Qt.Edge.BottomEdge)
        child_bottom_2.show()

        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().topLeft()),
            QPoint(5, 6))
        self.assertEqual(
            child_left_1.mapToParent(child_left_1.rect().bottomRight()),
            QPoint(34, 390))
        self.assertEqual(
            child_left_2.mapToParent(child_left_2.rect().topLeft()),
            QPoint(47, 6))
        self.assertEqual(
            child_left_2.mapToParent(child_left_2.rect().bottomRight()),
            QPoint(86, 390))
        self.assertEqual(
            child_right_1.mapToParent(child_right_1.rect().topLeft()),
            QPoint(552, 6))
        self.assertEqual(
            child_right_1.mapToParent(child_right_1.rect().bottomRight()),
            QPoint(591, 390))
        self.assertEqual(
            child_right_2.mapToParent(child_right_2.rect().topLeft()),
            QPoint(460, 6))
        self.assertEqual(
            child_right_2.mapToParent(child_right_2.rect().bottomRight()),
            QPoint(539, 390))
        self.assertEqual(
            child_top_1.mapToParent(child_top_1.rect().topLeft()),
            QPoint(99, 6))
        self.assertEqual(
            child_top_1.mapToParent(child_top_1.rect().bottomRight()),
            QPoint(447, 25))
        self.assertEqual(
            child_top_2.mapToParent(child_top_2.rect().topLeft()),
            QPoint(99, 39))
        self.assertEqual(
            child_top_2.mapToParent(child_top_2.rect().bottomRight()),
            QPoint(447, 68))
        self.assertEqual(
            child_bottom_1.mapToParent(child_bottom_1.rect().topLeft()),
            QPoint(99, 371))
        self.assertEqual(
            child_bottom_1.mapToParent(child_bottom_1.rect().bottomRight()),
            QPoint(447, 390))
        self.assertEqual(
            child_bottom_2.mapToParent(child_bottom_2.rect().topLeft()),
            QPoint(99, 328))
        self.assertEqual(
            child_bottom_2.mapToParent(child_bottom_2.rect().bottomRight()),
            QPoint(447, 357))


if __name__ == '__main__':
    unittest.main()
