"""QGIS Unit tests for QgsLayoutItemMapItemClipPathSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 Nyall Dawson'
__date__ = '03/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import QCoreApplication, QEvent, QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsLayout,
    QgsLayoutItemMap,
    QgsLayoutItemMapItemClipPathSettings,
    QgsLayoutItemShape,
    QgsMapClippingRegion,
    QgsPrintLayout,
    QgsProject,
    QgsReadWriteContext,
    QgsRectangle,
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemMapItemClipPathSettings(unittest.TestCase):

    def testSettings(self):
        p = QgsProject()
        l = QgsLayout(p)
        map = QgsLayoutItemMap(l)
        shape = QgsLayoutItemShape(l)

        settings = QgsLayoutItemMapItemClipPathSettings(map)
        spy = QSignalSpy(settings.changed)

        self.assertFalse(settings.enabled())
        settings.setEnabled(True)
        self.assertTrue(settings.enabled())
        self.assertEqual(len(spy), 1)
        settings.setEnabled(True)
        self.assertEqual(len(spy), 1)

        settings.setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        self.assertEqual(settings.featureClippingType(), QgsMapClippingRegion.FeatureClippingType.NoClipping)
        self.assertEqual(len(spy), 2)
        settings.setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        self.assertEqual(len(spy), 2)

        self.assertFalse(settings.forceLabelsInsideClipPath())
        settings.setForceLabelsInsideClipPath(True)
        self.assertTrue(settings.forceLabelsInsideClipPath())
        self.assertEqual(len(spy), 3)
        settings.setForceLabelsInsideClipPath(True)
        self.assertEqual(len(spy), 3)

        settings.setSourceItem(shape)
        self.assertEqual(len(spy), 4)
        self.assertEqual(settings.sourceItem(), shape)

        settings.setSourceItem(None)
        self.assertEqual(len(spy), 5)
        self.assertIsNone(settings.sourceItem())

        settings.setSourceItem(shape)
        self.assertEqual(len(spy), 6)
        shape.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        del shape
        self.assertIsNone(settings.sourceItem())

    def testIsActive(self):
        p = QgsProject()
        l = QgsLayout(p)
        map = QgsLayoutItemMap(l)
        shape = QgsLayoutItemShape(l)

        settings = QgsLayoutItemMapItemClipPathSettings(map)
        self.assertFalse(settings.isActive())
        settings.setEnabled(True)
        self.assertFalse(settings.isActive())
        settings.setSourceItem(shape)
        self.assertTrue(settings.isActive())
        settings.setEnabled(False)
        self.assertFalse(settings.isActive())

        settings.setEnabled(True)
        self.assertTrue(settings.isActive())
        shape.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        del shape
        self.assertFalse(settings.isActive())

    def testSaveRestore(self):
        p = QgsProject()
        l = QgsPrintLayout(p)
        p.layoutManager().addLayout(l)
        map = QgsLayoutItemMap(l)
        shape = QgsLayoutItemShape(l)
        l.addLayoutItem(map)
        l.addLayoutItem(shape)

        settings = map.itemClippingSettings()
        settings.setEnabled(True)
        settings.setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        settings.setForceLabelsInsideClipPath(True)
        settings.setSourceItem(shape)

        # save map to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(map.writeXml(elem, doc, QgsReadWriteContext()))
        elem_shape = doc.createElement("shape")
        self.assertTrue(shape.writeXml(elem_shape, doc, QgsReadWriteContext()))

        layout2 = QgsPrintLayout(p)
        layout2.setName('test2')
        p.layoutManager().addLayout(layout2)
        map2 = QgsLayoutItemMap(layout2)
        layout2.addLayoutItem(map2)
        shape2 = QgsLayoutItemShape(layout2)
        layout2.addLayoutItem(shape2)

        self.assertFalse(map2.itemClippingSettings().enabled())

        # restore from xml
        self.assertTrue(map2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext()))
        self.assertTrue(shape2.readXml(elem_shape.firstChildElement(), doc, QgsReadWriteContext()))

        self.assertTrue(map2.itemClippingSettings().enabled())
        self.assertEqual(map2.itemClippingSettings().featureClippingType(), QgsMapClippingRegion.FeatureClippingType.NoClipping)
        self.assertTrue(map2.itemClippingSettings().forceLabelsInsideClipPath())
        self.assertIsNone(map2.itemClippingSettings().sourceItem())

        map2.finalizeRestoreFromXml()

        self.assertEqual(map2.itemClippingSettings().sourceItem(), shape2)

    def testClippedMapExtent(self):
        p = QgsProject()
        l = QgsPrintLayout(p)
        map = QgsLayoutItemMap(l)
        shape = QgsLayoutItemShape(l)
        l.addLayoutItem(map)
        map.attemptSetSceneRect(QRectF(10, 20, 100, 80))
        map.zoomToExtent(QgsRectangle(100, 200, 50, 40))
        l.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Triangle)
        shape.attemptSetSceneRect(QRectF(20, 30, 70, 50))

        settings = map.itemClippingSettings()
        settings.setEnabled(True)
        settings.setSourceItem(shape)

        geom = settings.clippedMapExtent()
        self.assertEqual(geom.asWkt(), 'Polygon ((-5 80, 135 80, 65 180, -5 80))')

    def testToMapClippingRegion(self):
        p = QgsProject()
        l = QgsPrintLayout(p)
        p.layoutManager().addLayout(l)
        map = QgsLayoutItemMap(l)
        shape = QgsLayoutItemShape(l)
        l.addLayoutItem(map)
        map.attemptSetSceneRect(QRectF(10, 20, 100, 80))
        map.zoomToExtent(QgsRectangle(100, 200, 50, 40))
        l.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Triangle)
        shape.attemptSetSceneRect(QRectF(20, 30, 70, 50))

        settings = map.itemClippingSettings()
        settings.setEnabled(True)
        settings.setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        settings.setSourceItem(shape)

        region = settings.toMapClippingRegion()
        self.assertEqual(region.geometry().asWkt(), 'Polygon ((-5 80, 135 80, 65 180, -5 80))')
        self.assertEqual(region.featureClip(), QgsMapClippingRegion.FeatureClippingType.NoClipping)


if __name__ == '__main__':
    unittest.main()
