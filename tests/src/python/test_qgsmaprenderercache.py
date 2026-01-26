"""QGIS Unit tests for QgsMapRendererCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "1/02/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from time import sleep

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QImage
from qgis.core import (
    QgsMapRendererCache,
    QgsMapToPixel,
    QgsProject,
    QgsRectangle,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMapRendererCache(QgisTestCase):

    def testSetCacheImages(self):
        cache = QgsMapRendererCache()
        # not set image
        im = cache.cacheImage("littlehands")
        self.assertTrue(im.isNull())
        self.assertFalse(cache.hasCacheImage("littlehands"))

        # set image
        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("littlehands", im)
        self.assertFalse(im.isNull())
        self.assertEqual(cache.cacheImage("littlehands"), im)
        self.assertTrue(cache.hasCacheImage("littlehands"))

        # test another not set image when cache has images
        self.assertTrue(cache.cacheImage("bad").isNull())
        self.assertFalse(cache.hasCacheImage("bad"))

        # clear cache image
        cache.clearCacheImage("not in cache")  # no crash!
        cache.clearCacheImage("littlehands")
        im = cache.cacheImage("littlehands")
        self.assertTrue(im.isNull())
        self.assertFalse(cache.hasCacheImage("littlehands"))

        # clear whole cache
        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("littlehands", im)
        self.assertFalse(im.isNull())
        self.assertTrue(cache.hasCacheImage("littlehands"))
        cache.clear()
        im = cache.cacheImage("littlehands")
        self.assertTrue(im.isNull())
        self.assertFalse(cache.hasCacheImage("littlehands"))

    def testInit(self):
        cache = QgsMapRendererCache()
        extent = QgsRectangle(1, 2, 3, 4)
        self.assertFalse(cache.init(extent, 1000))

        # add a cache image
        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("layer", im)
        self.assertFalse(cache.cacheImage("layer").isNull())
        self.assertTrue(cache.hasCacheImage("layer"))

        # re init, without changing extent or scale
        self.assertTrue(cache.init(extent, 1000))

        # image should still be in cache
        self.assertFalse(cache.cacheImage("layer").isNull())
        self.assertTrue(cache.hasCacheImage("layer"))

        # reinit with different scale
        self.assertFalse(cache.init(extent, 2000))
        # cache should be cleared
        self.assertTrue(cache.cacheImage("layer").isNull())
        self.assertFalse(cache.hasCacheImage("layer"))

        # readd image to cache
        cache.setCacheImage("layer", im)
        self.assertFalse(cache.cacheImage("layer").isNull())
        self.assertTrue(cache.hasCacheImage("layer"))

        # change extent
        self.assertFalse(cache.init(QgsRectangle(11, 12, 13, 14), 2000))
        # cache should be cleared
        self.assertTrue(cache.cacheImage("layer").isNull())
        self.assertFalse(cache.hasCacheImage("layer"))

    def testRequestRepaintSimple(self):
        """test requesting repaint with a single dependent layer"""
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        QgsProject.instance().addMapLayers([layer])
        self.assertTrue(layer.isValid())

        # add image to cache
        cache = QgsMapRendererCache()
        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("xxx", im, [layer])
        self.assertFalse(cache.cacheImage("xxx").isNull())
        self.assertTrue(cache.hasCacheImage("xxx"))

        # trigger repaint on layer
        layer.triggerRepaint()
        # cache image should be cleared
        self.assertTrue(cache.cacheImage("xxx").isNull())
        self.assertFalse(cache.hasCacheImage("xxx"))
        QgsProject.instance().removeMapLayer(layer.id())

        # test that cache is also cleared on deferred update
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        cache.setCacheImage("xxx", im, [layer])
        layer.triggerRepaint(True)
        self.assertFalse(cache.hasCacheImage("xxx"))

    def testInvalidateCacheForLayer(self):
        """test invalidating the cache for a layer"""
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        QgsProject.instance().addMapLayers([layer])
        self.assertTrue(layer.isValid())

        # add image to cache
        cache = QgsMapRendererCache()
        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("xxx", im, [layer])
        self.assertFalse(cache.cacheImage("xxx").isNull())
        self.assertTrue(cache.hasCacheImage("xxx"))

        # invalidate cache for layer
        cache.invalidateCacheForLayer(layer)
        # cache image should be cleared
        self.assertTrue(cache.cacheImage("xxx").isNull())
        self.assertFalse(cache.hasCacheImage("xxx"))
        QgsProject.instance().removeMapLayer(layer.id())

    def testRequestRepaintMultiple(self):
        """test requesting repaint with multiple dependent layers"""
        layer1 = QgsVectorLayer("Point?field=fldtxt:string", "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer2", "memory")
        QgsProject.instance().addMapLayers([layer1, layer2])
        self.assertTrue(layer1.isValid())
        self.assertTrue(layer2.isValid())

        # add image to cache - no dependent layers
        cache = QgsMapRendererCache()
        im1 = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("nolayer", im1)
        self.assertFalse(cache.cacheImage("nolayer").isNull())
        self.assertTrue(cache.hasCacheImage("nolayer"))

        # trigger repaint on layer
        layer1.triggerRepaint()
        layer1.triggerRepaint()  # do this a couple of times - we don't want errors due to multiple disconnects, etc
        layer2.triggerRepaint()
        layer2.triggerRepaint()
        # cache image should still exist - it's not dependent on layers
        self.assertFalse(cache.cacheImage("nolayer").isNull())
        self.assertTrue(cache.hasCacheImage("nolayer"))

        # image depends on 1 layer
        im_l1 = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("im1", im_l1, [layer1])

        # image depends on 2 layers
        im_l1_l2 = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("im1_im2", im_l1_l2, [layer1, layer2])

        # image depends on 2nd layer alone
        im_l2 = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("im2", im_l2, [layer2])

        self.assertFalse(cache.cacheImage("im1").isNull())
        self.assertTrue(cache.hasCacheImage("im1"))
        self.assertFalse(cache.cacheImage("im1_im2").isNull())
        self.assertTrue(cache.hasCacheImage("im1_im2"))
        self.assertFalse(cache.cacheImage("im2").isNull())
        self.assertTrue(cache.hasCacheImage("im2"))

        # trigger repaint layer 1 (check twice - don't want disconnect errors)
        for i in range(2):
            layer1.triggerRepaint()
            # should be cleared
            self.assertTrue(cache.cacheImage("im1").isNull())
            self.assertFalse(cache.hasCacheImage("im1"))
            self.assertTrue(cache.cacheImage("im1_im2").isNull())
            self.assertFalse(cache.hasCacheImage("im1_im2"))
            # should be retained
            self.assertTrue(cache.hasCacheImage("im2"))
            self.assertFalse(cache.cacheImage("im2").isNull())
            self.assertEqual(cache.cacheImage("im2"), im_l2)
            self.assertTrue(cache.hasCacheImage("nolayer"))
            self.assertFalse(cache.cacheImage("nolayer").isNull())
            self.assertEqual(cache.cacheImage("nolayer"), im1)

        # trigger repaint layer 2
        for i in range(2):
            layer2.triggerRepaint()
            # should be cleared
            self.assertFalse(cache.hasCacheImage("im1"))
            self.assertTrue(cache.cacheImage("im1").isNull())
            self.assertFalse(cache.hasCacheImage("im1_im2"))
            self.assertTrue(cache.cacheImage("im1_im2").isNull())
            self.assertFalse(cache.hasCacheImage("im2"))
            self.assertTrue(cache.cacheImage("im2").isNull())
            # should be retained
            self.assertTrue(cache.hasCacheImage("nolayer"))
            self.assertFalse(cache.cacheImage("nolayer").isNull())
            self.assertEqual(cache.cacheImage("nolayer"), im1)

    def testDependentLayers(self):
        # bad layer tests
        cache = QgsMapRendererCache()
        self.assertEqual(cache.dependentLayers("not a layer"), [])

        layer1 = QgsVectorLayer("Point?field=fldtxt:string", "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer2", "memory")

        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("no depends", im, [])
        self.assertEqual(cache.dependentLayers("no depends"), [])
        cache.setCacheImage("depends", im, [layer1, layer2])
        self.assertEqual(set(cache.dependentLayers("depends")), {layer1, layer2})

    def testLayerRemoval(self):
        """test that cached image is cleared when a dependent layer is removed"""
        cache = QgsMapRendererCache()
        layer1 = QgsVectorLayer("Point?field=fldtxt:string", "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer2", "memory")
        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("depends", im, [layer1, layer2])
        cache.setCacheImage("depends2", im, [layer1])
        cache.setCacheImage("depends3", im, [layer2])
        cache.setCacheImage("no depends", im, [])
        self.assertTrue(cache.hasCacheImage("depends"))
        self.assertTrue(cache.hasCacheImage("depends2"))
        self.assertTrue(cache.hasCacheImage("depends3"))
        self.assertTrue(cache.hasCacheImage("no depends"))

        # try deleting a layer
        layer2 = None
        self.assertFalse(cache.hasCacheImage("depends"))
        self.assertTrue(cache.hasCacheImage("depends2"))
        self.assertFalse(cache.hasCacheImage("depends3"))
        self.assertTrue(cache.hasCacheImage("no depends"))

        layer1 = None
        self.assertFalse(cache.hasCacheImage("depends"))
        self.assertFalse(cache.hasCacheImage("depends2"))
        self.assertFalse(cache.hasCacheImage("depends3"))
        self.assertTrue(cache.hasCacheImage("no depends"))

    def testClearOnLayerAutoRefresh(self):
        """test that cache is cleared when layer auto refresh is triggered"""
        cache = QgsMapRendererCache()
        layer1 = QgsVectorLayer("Point?field=fldtxt:string", "layer1", "memory")
        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("l1", im, [layer1])
        self.assertTrue(cache.hasCacheImage("l1"))

        layer1.setAutoRefreshInterval(100)
        layer1.setAutoRefreshEnabled(True)
        self.assertTrue(cache.hasCacheImage("l1"))

        # wait a second...
        sleep(1)
        for i in range(100):
            QCoreApplication.processEvents()
        # cache should be cleared
        self.assertFalse(cache.hasCacheImage("l1"))

    def testSetCacheImageDifferentParams(self):
        """
        Test setting cache image with different parameters
        """
        cache = QgsMapRendererCache()
        cache.updateParameters(QgsRectangle(1, 1, 3, 3), QgsMapToPixel(5))
        im = QImage(200, 200, QImage.Format.Format_RGB32)
        cache.setCacheImage("im1", im, [])

        self.assertEqual(cache.cacheImage("im1").width(), 200)

        # if existing cached image exists with matching parameters, we don't store a new image -- old
        # one should still be retained
        im = QImage(201, 201, QImage.Format.Format_RGB32)
        cache.setCacheImageWithParameters(
            "im1", im, QgsRectangle(1, 1, 3, 4), QgsMapToPixel(5), []
        )
        self.assertEqual(cache.cacheImage("im1").width(), 200)
        cache.setCacheImageWithParameters(
            "im1", im, QgsRectangle(1, 1, 3, 3), QgsMapToPixel(6), []
        )
        self.assertEqual(cache.cacheImage("im1").width(), 200)

        # replace with matching parameters
        cache.setCacheImageWithParameters(
            "im1", im, QgsRectangle(1, 1, 3, 3), QgsMapToPixel(5), []
        )
        self.assertEqual(cache.cacheImage("im1").width(), 201)
        im = QImage(202, 202, QImage.Format.Format_RGB32)
        cache.setCacheImage("im1", im, [])
        self.assertEqual(cache.cacheImage("im1").width(), 202)


if __name__ == "__main__":
    unittest.main()
