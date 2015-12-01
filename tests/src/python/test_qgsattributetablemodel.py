# -*- coding: utf-8 -*-
"""QGIS Unit tests for the attribute table model.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '27/05/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.gui import QgsAttributeTableModel, QgsEditorWidgetRegistry
from qgis.core import QgsFeature, QgsGeometry, QgsPoint, QgsVectorLayer, QgsVectorLayerCache, NULL

from PyQt.QtCore import QEventLoop, QObject, pyqtSlot

from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsAttributeTableModel(TestCase, QObject):

    @classmethod
    def setUpClass(cls):
        QgsEditorWidgetRegistry.initEditors()

    @pyqtSlot(long)
    def wait_thread(self, dummy):
        loop = QEventLoop()
        self.am.loadFinished.connect(loop.quit)
        loop.exec_()

    def __test_rowToid_and_return(self):
        [self.assertEqual(self.am.rowToId(self.am.idToRow(f.id())), f.id()) for f in self.am.layer().getFeatures()]

    def setUp(self):
        self.layer = self.createLayer()
        self.cache = QgsVectorLayerCache(self.layer, 100)
        self.am = QgsAttributeTableModel(self.cache)
        self.am.loadStarted.connect(self.wait_thread)
        self.am.loadLayer()
        self.am.loadAttributes()

    def tearDown(self):
        del self.am
        del self.cache
        del self.layer

    def createLayer(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = layer.dataProvider()
        features = list()
        for i in range(10):
            f = QgsFeature()
            f.setAttributes(["test", i])
            f.setGeometry(QgsGeometry.fromPoint(QgsPoint(100 * i, 2 ^ i)))
            features.append(f)

        assert pr.addFeatures(features)
        return layer

    def testLoad(self):
        assert self.am.rowCount() == 10, self.am.rowCount()
        assert self.am.columnCount() == 2, self.am.columnCount()
        # Test for ids in the map
        self.__test_rowToid_and_return()

    def testRemove(self):
        self.layer.startEditing()
        self.layer.deleteFeature(5)
        self.__test_rowToid_and_return()
        assert self.am.rowCount() == 9, self.am.rowCount()
        self.layer.setSelectedFeatures([1, 3, 6, 7])
        self.layer.deleteSelectedFeatures()
        self.__test_rowToid_and_return()
        assert self.am.rowCount() == 5, self.am.rowCount()

    def testAdd(self):
        self.layer.startEditing()

        f = QgsFeature()
        f.setAttributes(["test", 8])
        f.setGeometry(QgsGeometry.fromPoint(QgsPoint(100, 200)))
        self.layer.addFeature(f)
        assert self.am.rowCount() == 11, self.am.rowCount()

    def testRemoveColumns(self):
        assert self.layer.startEditing()
        assert self.layer.deleteAttribute(1)
        assert self.am.columnCount() == 1, self.am.columnCount()

if __name__ == '__main__':
    unittest.main()
