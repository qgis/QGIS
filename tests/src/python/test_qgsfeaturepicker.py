"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Denis Rouzaud"
__date__ = "24/04/2020"
__copyright__ = "Copyright 2015, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy, QTest
from qgis.PyQt.QtWidgets import QComboBox
from qgis.core import (
    QgsApplication,
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsVectorLayer,
)
from qgis.gui import QgsFeaturePickerWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


def createLayer(manyFeatures: bool = False):
    layer = QgsVectorLayer(
        "Point?field=fldtxt:string&field=fldint:integer", "test layer", "memory"
    )
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test1", 123])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    f2 = QgsFeature()
    f2.setAttributes(["test2", 457])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
    f3 = QgsFeature()
    f3.setAttributes(["test2", 888])
    f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
    flist = [f, f2, f3]
    if manyFeatures:
        for i in range(4, 100):
            f = QgsFeature()
            f.setAttributes([f"test{i}", i])
            flist.append(f)

    assert pr.addFeatures(flist)
    assert layer.featureCount() == 99 if manyFeatures else 3
    return layer


class TestQgsRelationEditWidget(QgisTestCase):

    def testSetFeature(self):
        layer = createLayer()
        w = QgsFeaturePickerWidget()
        w.setLayer(layer)
        w.setFeature(2)
        spy = QSignalSpy(w.currentFeatureChanged)
        spy.wait()
        self.assertEqual(w.findChild(QComboBox).lineEdit().text(), "test2")
        self.assertTrue(
            w.feature().geometry().equals(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
        )

    def testSetAllowNull(self):
        layer = createLayer()
        w = QgsFeaturePickerWidget()
        w.setLayer(layer)
        w.setAllowNull(True)
        spy = QSignalSpy(w.featureChanged)
        spy.wait()
        self.assertEqual(
            w.findChild(QComboBox).lineEdit().text(),
            QgsApplication.nullRepresentation(),
        )
        w.setAllowNull(False)
        spy.wait()
        self.assertEqual(w.findChild(QComboBox).lineEdit().text(), "test1")

    def testFetchLimit(self):
        layer = createLayer()
        w = QgsFeaturePickerWidget()
        w.setAllowNull(False)
        w.setFetchLimit(20)
        w.setLayer(layer)
        spy = QSignalSpy(w.featureChanged)
        spy.wait()
        self.assertEqual(w.findChild(QComboBox).model().rowCount(), 20)

    def testLineEdit(self):
        layer = createLayer(True)
        w = QgsFeaturePickerWidget()
        w.setAllowNull(False)
        w.setFetchLimit(20)
        w.setLayer(layer)
        spy = QSignalSpy(w.featureChanged)
        spy.wait()
        w.findChild(QComboBox).lineEdit().clear()
        QTest.keyClicks(w.findChild(QComboBox).lineEdit(), "test99")
        spy.wait()
        self.assertEqual(w.feature().id(), 99)


if __name__ == "__main__":
    unittest.main()
