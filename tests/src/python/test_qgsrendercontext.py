# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRenderContext.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/01/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import QgsRenderContext
from qgis.PyQt.QtGui import QPainter, QImage
from qgis.testing import unittest


class TestQgsRenderContext(unittest.TestCase):

    def testFromQPainter(self):
        """ test QgsRenderContext.fromQPainter """

        # no painter
        c = QgsRenderContext.fromQPainter(None)
        self.assertFalse(c.painter())
        # assuming 88 dpi as fallback
        self.assertAlmostEqual(c.scaleFactor(), 88 / 25.4, 3)

        # no painter destination
        p = QPainter()
        c = QgsRenderContext.fromQPainter(p)
        self.assertEqual(c.painter(), p)
        self.assertAlmostEqual(c.scaleFactor(), 88 / 25.4, 3)

        im = QImage(1000, 600, QImage.Format_RGB32)
        dots_per_m = 300 / 25.4 * 1000 # 300 dpi to dots per m
        im.setDotsPerMeterX(dots_per_m)
        im.setDotsPerMeterY(dots_per_m)
        p = QPainter(im)
        c = QgsRenderContext.fromQPainter(p)
        self.assertEqual(c.painter(), p)
        self.assertAlmostEqual(c.scaleFactor(), dots_per_m / 1000, 3) # scaleFactor should be pixels/mm


if __name__ == '__main__':
    unittest.main()
