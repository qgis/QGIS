# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerView.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '29/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.gui import (QgsComposerView)
from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtGui import QTransform

from qgis.testing import start_app, unittest

start_app()


class TestQgsComposerView(unittest.TestCase):

    def testScaleSafe(self):
        """ test scaleSafe method """

        view = QgsComposerView()
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


if __name__ == '__main__':
    unittest.main()
