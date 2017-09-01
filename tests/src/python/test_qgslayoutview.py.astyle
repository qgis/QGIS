# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutView.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '05/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import QgsProject, QgsLayout, QgsUnitTypes
from qgis.gui import QgsLayoutView
from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtGui import QTransform

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutView(unittest.TestCase):

    def testScaleSafe(self):
        """ test scaleSafe method """

        view = QgsLayoutView()
        view.fitInView(QRectF(0, 0, 10, 10))
        scale = view.transform().m11()
        view.scaleSafe(2)
        self.assertAlmostEqual(view.transform().m11(), 2)
        view.scaleSafe(4)
        self.assertAlmostEqual(view.transform().m11(), 8)

        # try to zoom in heaps
        view.scaleSafe(99999999)
        # assume we have hit the limit
        scale = view.transform().m11()
        view.scaleSafe(2)
        self.assertAlmostEqual(view.transform().m11(), scale)

        view.setTransform(QTransform.fromScale(1, 1))
        self.assertAlmostEqual(view.transform().m11(), 1)
        # test zooming out
        view.scaleSafe(0.5)
        self.assertAlmostEqual(view.transform().m11(), 0.5)
        view.scaleSafe(0.1)
        self.assertAlmostEqual(view.transform().m11(), 0.05)

        # try zooming out heaps
        view.scaleSafe(0.000000001)
        # assume we have hit the limit
        scale = view.transform().m11()
        view.scaleSafe(0.5)
        self.assertAlmostEqual(view.transform().m11(), scale)

    def testLayoutScalePixels(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.setUnits(QgsUnitTypes.LayoutPixels)
        view = QgsLayoutView()
        view.setCurrentLayout(l)
        view.setZoomLevel(1)
        # should be no transform, since 100% with pixel units should be pixel-pixel
        self.assertEqual(view.transform().m11(), 1)
        view.setZoomLevel(0.5)
        self.assertEqual(view.transform().m11(), 0.5)


if __name__ == '__main__':
    unittest.main()
