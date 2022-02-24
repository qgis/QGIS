# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectViewSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '30/10/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import os

from qgis.core import (QgsProject,
                       QgsProjectViewSettings,
                       QgsReadWriteContext,
                       QgsReferencedRectangle,
                       QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsRasterLayer,
                       Qgis)
from qgis.gui import QgsMapCanvas

from qgis.PyQt.QtCore import QTemporaryDir

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectViewSettings(unittest.TestCase):

    def testMapScales(self):
        p = QgsProjectViewSettings()
        self.assertFalse(p.mapScales())
        self.assertFalse(p.useProjectScales())

        spy = QSignalSpy(p.mapScalesChanged)
        p.setMapScales([])
        self.assertEqual(len(spy), 0)
        p.setUseProjectScales(False)
        self.assertEqual(len(spy), 0)

        p.setMapScales([5000, 6000, 3000, 4000])
        # scales must be sorted
        self.assertEqual(p.mapScales(), [6000.0, 5000.0, 4000.0, 3000.0])
        self.assertEqual(len(spy), 1)
        p.setMapScales([5000, 6000, 3000, 4000])
        self.assertEqual(len(spy), 1)
        self.assertEqual(p.mapScales(), [6000.0, 5000.0, 4000.0, 3000.0])
        p.setMapScales([5000, 6000, 3000, 4000, 1000])
        self.assertEqual(len(spy), 2)
        self.assertEqual(p.mapScales(), [6000.0, 5000.0, 4000.0, 3000.0, 1000.0])

        p.setUseProjectScales(True)
        self.assertEqual(len(spy), 3)
        p.setUseProjectScales(True)
        self.assertEqual(len(spy), 3)
        p.setUseProjectScales(False)
        self.assertEqual(len(spy), 4)

        p.setUseProjectScales(True)
        p.setMapScales([5000, 6000, 3000, 4000])
        self.assertEqual(len(spy), 6)

        p.reset()
        self.assertEqual(len(spy), 7)
        self.assertFalse(p.mapScales())
        self.assertFalse(p.useProjectScales())

    def testDefaultViewExtent(self):
        p = QgsProjectViewSettings()
        self.assertTrue(p.defaultViewExtent().isNull())

        p.setDefaultViewExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        self.assertEqual(p.defaultViewExtent(), QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))

        p.setDefaultViewExtent(QgsReferencedRectangle())
        self.assertTrue(p.defaultViewExtent().isNull())

        p.setDefaultViewExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        p.reset()
        self.assertTrue(p.defaultViewExtent().isNull())

    def testDefaultViewExtentWithCanvas(self):
        p = QgsProject()
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)
        canvas.setExtent(QgsRectangle(10, 30, 20, 35))
        canvas.show()

        tmpDir = QTemporaryDir()
        tmpFile = "{}/project.qgz".format(tmpDir.path())
        self.assertTrue(p.write(tmpFile))

        QgsProject.instance().read(tmpFile)

        # no default view, extent should not change
        self.assertAlmostEqual(canvas.extent().xMinimum(), 10, 3)
        self.assertAlmostEqual(canvas.extent().yMinimum(), 29.16666, 3)
        self.assertAlmostEqual(canvas.extent().xMaximum(), 20, 3)
        self.assertAlmostEqual(canvas.extent().yMaximum(), 35.833333333, 3)
        self.assertEqual(canvas.mapSettings().destinationCrs().authid(), 'EPSG:4326')

        p.viewSettings().setDefaultViewExtent(QgsReferencedRectangle(QgsRectangle(1000, 2000, 1500, 2500), QgsCoordinateReferenceSystem('EPSG:3857')))

        self.assertTrue(p.write(tmpFile))
        QgsProject.instance().read(tmpFile)

        self.assertAlmostEqual(canvas.extent().xMinimum(), 0.0078602, 3)
        self.assertAlmostEqual(canvas.extent().yMinimum(), 0.017966, 3)
        self.assertAlmostEqual(canvas.extent().xMaximum(), 0.01459762, 3)
        self.assertAlmostEqual(canvas.extent().yMaximum(), 0.02245788, 3)
        self.assertEqual(canvas.mapSettings().destinationCrs().authid(), 'EPSG:4326')

    def testPresetFullExtent(self):
        p = QgsProjectViewSettings()
        self.assertTrue(p.presetFullExtent().isNull())

        p.setPresetFullExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        self.assertEqual(p.presetFullExtent(), QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))

        p.setPresetFullExtent(QgsReferencedRectangle())
        self.assertTrue(p.presetFullExtent().isNull())

        p.setPresetFullExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        p.reset()
        self.assertTrue(p.presetFullExtent().isNull())

    def testPresetFullExtentChangedSignal(self):
        p = QgsProjectViewSettings()
        spy = QSignalSpy(p.presetFullExtentChanged)

        p.setPresetFullExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        self.assertEqual(len(spy), 1)

        p.setPresetFullExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        self.assertEqual(len(spy), 1)

        p.setPresetFullExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:4326")))
        self.assertEqual(len(spy), 2)

        p.reset()
        self.assertEqual(len(spy), 3)

        p.reset()
        self.assertEqual(len(spy), 3)

    def testFullExtent(self):
        p = QgsProject()
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertTrue(p.viewSettings().fullExtent().isNull())

        p.viewSettings().setPresetFullExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:4326")))
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMinimum(), 111319, -1)
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMaximum(), 333958, -1)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMinimum(), 222684, -1)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMaximum(), 445640, -1)
        self.assertEqual(p.viewSettings().fullExtent().crs().authid(), 'EPSG:3857')

        # add layers
        shapefile = os.path.join(TEST_DATA_DIR, 'polys.shp')
        layer = QgsVectorLayer(shapefile, 'Polys', 'ogr')
        p.addMapLayer(layer)
        # no change, because preset extent is set
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMinimum(), 111319, -1)
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMaximum(), 333958, -1)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMinimum(), 222684, -1)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMaximum(), 445640, -1)
        self.assertEqual(p.viewSettings().fullExtent().crs().authid(), 'EPSG:3857')
        # remove preset extent
        p.viewSettings().setPresetFullExtent(QgsReferencedRectangle())
        # extent should come from layers
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMinimum(), -13238432, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMaximum(), -9327461, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMinimum(), 2815417, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMaximum(), 5897492, -2)
        self.assertEqual(p.viewSettings().fullExtent().crs().authid(), 'EPSG:3857')

        # add another layer
        shapefile = os.path.join(TEST_DATA_DIR, 'lines.shp')
        layer = QgsVectorLayer(shapefile, 'Lines', 'ogr')
        p.addMapLayer(layer)
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMinimum(), -13238432, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMaximum(), -9164115, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMinimum(), 2657217, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMaximum(), 5897492, -2)
        self.assertEqual(p.viewSettings().fullExtent().crs().authid(), 'EPSG:3857')

        # add a layer with a different crs
        layer = QgsVectorLayer("Point?crs=EPSG:3857&field=fldtxt:string&field=fldint:integer",
                               "x", "memory")
        p.addMapLayer(layer)
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-8164115, 5997492)))
        layer.startEditing()
        layer.addFeatures([f])
        layer.commitChanges()

        self.assertAlmostEqual(p.viewSettings().fullExtent().xMinimum(), -13238432, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMaximum(), -8164115, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMinimum(), 2657217, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMaximum(), 5997492, -2)
        self.assertEqual(p.viewSettings().fullExtent().crs().authid(), 'EPSG:3857')

    def testFullExtentWithBasemap(self):
        """
        Test that basemap layer's extents are ignored when calculating the full extent of
        a project, UNLESS only basemap layers are present
        """
        p = QgsProject()
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertTrue(p.viewSettings().fullExtent().isNull())

        # add only a basemap layer

        xyz_layer = QgsRasterLayer("type=xyz&url=file://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0", '', "wms")
        self.assertEqual(xyz_layer.properties(), Qgis.MapLayerProperty.IsBasemapLayer)
        p.addMapLayer(xyz_layer)

        # should be global extent of xyz layer, as only basemap layers are present in project
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMinimum(), -20037508, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMaximum(), 20037508, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMinimum(), -20037508, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMaximum(), 20037508, -2)

        # add a non-basemap layer
        shapefile = os.path.join(TEST_DATA_DIR, 'lines.shp')
        layer = QgsVectorLayer(shapefile, 'Lines', 'ogr')
        p.addMapLayer(layer)
        # now project extent should ignore basemap layer extents
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMinimum(), -13093754, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().xMaximum(), -9164115, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMinimum(), 2657217, -2)
        self.assertAlmostEqual(p.viewSettings().fullExtent().yMaximum(), 5809709, -2)
        self.assertEqual(p.viewSettings().fullExtent().crs().authid(), 'EPSG:3857')

    def testReadWrite(self):
        p = QgsProjectViewSettings()
        self.assertFalse(p.mapScales())
        self.assertFalse(p.useProjectScales())
        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectViewSettings()
        spy = QSignalSpy(p2.mapScalesChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertFalse(p2.mapScales())
        self.assertFalse(p2.useProjectScales())
        self.assertEqual(len(spy), 0)
        self.assertTrue(p2.defaultViewExtent().isNull())

        p.setUseProjectScales(True)
        p.setMapScales([56, 78, 99])
        p.setDefaultViewExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        p.setPresetFullExtent(
            QgsReferencedRectangle(QgsRectangle(11, 12, 13, 14), QgsCoordinateReferenceSystem("EPSG:3111")))
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectViewSettings()
        spy = QSignalSpy(p2.mapScalesChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(p2.mapScales(), [99.0, 78.0, 56.0])
        self.assertTrue(p2.useProjectScales())
        self.assertEqual(len(spy), 1)
        self.assertEqual(p2.defaultViewExtent(), QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        self.assertEqual(p2.presetFullExtent(),
                         QgsReferencedRectangle(QgsRectangle(11, 12, 13, 14), QgsCoordinateReferenceSystem("EPSG:3111")))


if __name__ == '__main__':
    unittest.main()
