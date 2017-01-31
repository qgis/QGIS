# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapRendererCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '1/02/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsMapRendererCache,
                       QgsRectangle,
                       QgsVectorLayer,
                       QgsProject)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtGui import QImage
start_app()


class TestQgsMapRendererCache(unittest.TestCase):

    def testSetCacheImages(self):
        cache = QgsMapRendererCache()
        # not set image
        im = cache.cacheImage('littlehands')
        self.assertTrue(im.isNull())

        # set image
        im = QImage(200, 200, QImage.Format_RGB32)
        cache.setCacheImage('littlehands', im)
        self.assertFalse(im.isNull())
        self.assertEqual(cache.cacheImage('littlehands'), im)

        # test another not set image when cache has images
        self.assertTrue(cache.cacheImage('bad').isNull())

        # clear cache image
        cache.clearCacheImage('not in cache') # no crash!
        cache.clearCacheImage('littlehands')
        im = cache.cacheImage('littlehands')
        self.assertTrue(im.isNull())

        # clear whole cache
        im = QImage(200, 200, QImage.Format_RGB32)
        cache.setCacheImage('littlehands', im)
        self.assertFalse(im.isNull())
        cache.clear()
        im = cache.cacheImage('littlehands')
        self.assertTrue(im.isNull())

    def testInit(self):
        cache = QgsMapRendererCache()
        extent = QgsRectangle(1, 2, 3, 4)
        cache.init(extent, 1000)

        # add a cache image
        im = QImage(200, 200, QImage.Format_RGB32)
        cache.setCacheImage('layer', im)
        self.assertFalse(cache.cacheImage('layer').isNull())

        # re init, without changing extent or scale
        cache.init(extent, 1000)

        # image should still be in cache
        self.assertFalse(cache.cacheImage('layer').isNull())

        # reinit with different scale
        cache.init(extent, 2000)
        # cache should be cleared
        self.assertTrue(cache.cacheImage('layer').isNull())

        # readd image to cache
        cache.setCacheImage('layer', im)
        self.assertFalse(cache.cacheImage('layer').isNull())

        # change extent
        cache.init(QgsRectangle(11, 12, 13, 14), 2000)
        # cache should be cleared
        self.assertTrue(cache.cacheImage('layer').isNull())

    def testRequestRepaint(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer", "memory")
        QgsProject.instance().addMapLayers([layer])
        self.assertTrue(layer.isValid())

        # add image to cache
        cache = QgsMapRendererCache()
        im = QImage(200, 200, QImage.Format_RGB32)
        cache.setCacheImage(layer.id(), im)
        self.assertFalse(cache.cacheImage(layer.id()).isNull())

        # trigger repaint on layer
        layer.triggerRepaint()
        # cache image should be cleared
        self.assertTrue(cache.cacheImage(layer.id()).isNull())


if __name__ == '__main__':
    unittest.main()
