# -*- coding: utf-8 -*-
"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '24/04/2020'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA

import os

from qgis.core import (
    QgsFeature,
    QgsVectorLayer,
    QgsProject,
    QgsRelation,
    QgsTransaction,
    QgsFeatureRequest,
    QgsVectorLayerTools,
    QgsGeometry
)

from qgis.gui import (
    QgsGui,
    QgsRelationWidgetWrapper,
    QgsAttributeEditorContext,
    QgsMapCanvas,
    QgsAdvancedDigitizingDockWidget
)

from qgis.PyQt.QtCore import QTimer
from qgis.PyQt.QtWidgets import (
    QToolButton,
    QMessageBox,
    QDialogButtonBox,
    QTableView,
    QDialog
)
from qgis.testing import start_app, unittest

start_app()


def createLayer():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "test layer", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    f2 = QgsFeature()
    f2.setAttributes(["test2", 457])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
    f3 = QgsFeature()
    f3.setAttributes(["test2", 888])
    f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
    assert pr.addFeatures([f, f2, f3])
    assert layer.featureCount() == 3
    return layer


class TestQgsRelationEditWidget(unittest.TestCase):

    def testSetFeature(self):
        createLayer()


if __name__ == '__main__':
    unittest.main()


