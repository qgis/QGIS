"""QGIS Unit tests for QgsLayoutItemMapAtlasClippingSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 Nyall Dawson"
__date__ = "03/07/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsLayout,
    QgsLayoutItemMap,
    QgsLayoutItemMapAtlasClippingSettings,
    QgsMapClippingRegion,
    QgsProject,
    QgsReadWriteContext,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemMapAtlasClippingSettings(QgisTestCase):

    def testSettings(self):
        p = QgsProject()
        l = QgsLayout(p)
        map = QgsLayoutItemMap(l)

        settings = QgsLayoutItemMapAtlasClippingSettings(map)
        spy = QSignalSpy(settings.changed)

        self.assertFalse(settings.enabled())
        settings.setEnabled(True)
        self.assertTrue(settings.enabled())
        self.assertEqual(len(spy), 1)
        settings.setEnabled(True)
        self.assertEqual(len(spy), 1)

        settings.setFeatureClippingType(
            QgsMapClippingRegion.FeatureClippingType.NoClipping
        )
        self.assertEqual(
            settings.featureClippingType(),
            QgsMapClippingRegion.FeatureClippingType.NoClipping,
        )
        self.assertEqual(len(spy), 2)
        settings.setFeatureClippingType(
            QgsMapClippingRegion.FeatureClippingType.NoClipping
        )
        self.assertEqual(len(spy), 2)

        self.assertFalse(settings.forceLabelsInsideFeature())
        settings.setForceLabelsInsideFeature(True)
        self.assertTrue(settings.forceLabelsInsideFeature())
        self.assertEqual(len(spy), 3)
        settings.setForceLabelsInsideFeature(True)
        self.assertEqual(len(spy), 3)

        l1 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        l2 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        p.addMapLayers([l1, l2])
        self.assertFalse(settings.layersToClip())
        settings.setLayersToClip([l1, l2])
        self.assertCountEqual(settings.layersToClip(), [l1, l2])
        self.assertEqual(len(spy), 4)

        p.removeMapLayer(l1.id())
        self.assertCountEqual(settings.layersToClip(), [l2])

        settings.setRestrictToLayers(False)
        self.assertFalse(settings.restrictToLayers())
        self.assertEqual(len(spy), 4)

        settings.setRestrictToLayers(True)
        self.assertTrue(settings.restrictToLayers())
        self.assertEqual(len(spy), 5)

    def testSaveRestore(self):
        p = QgsProject()
        l1 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        l2 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        p.addMapLayers([l1, l2])

        l = QgsLayout(p)
        map = QgsLayoutItemMap(l)

        settings = map.atlasClippingSettings()
        settings.setEnabled(True)
        settings.setFeatureClippingType(
            QgsMapClippingRegion.FeatureClippingType.NoClipping
        )
        settings.setForceLabelsInsideFeature(True)
        settings.setRestrictToLayers(True)
        settings.setLayersToClip([l2])

        # save map to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(map.writeXml(elem, doc, QgsReadWriteContext()))

        layout2 = QgsLayout(p)
        map2 = QgsLayoutItemMap(layout2)
        self.assertFalse(map2.atlasClippingSettings().enabled())

        # restore from xml
        self.assertTrue(
            map2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )

        self.assertTrue(map2.atlasClippingSettings().enabled())
        self.assertEqual(
            map2.atlasClippingSettings().featureClippingType(),
            QgsMapClippingRegion.FeatureClippingType.NoClipping,
        )
        self.assertTrue(map2.atlasClippingSettings().forceLabelsInsideFeature())
        self.assertEqual(map2.atlasClippingSettings().layersToClip(), [l2])
        self.assertTrue(map2.atlasClippingSettings().restrictToLayers())


if __name__ == "__main__":
    unittest.main()
