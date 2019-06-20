# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsScaleWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '13/03/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import math
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtTest import QSignalSpy

from qgis.gui import QgsScaleWidget
from qgis.testing import start_app, unittest

start_app()


class TestQgsScaleWidget(unittest.TestCase):

    def testBasic(self):
        w = QgsScaleWidget()
        spy = QSignalSpy(w.scaleChanged)
        w.setScaleString('1:2345')
        self.assertEqual(w.scaleString(), '1:2,345')
        self.assertEqual(w.scale(), 2345)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], 2345)

        w.setScaleString('0.02')
        self.assertEqual(w.scaleString(), '1:50')
        self.assertEqual(w.scale(), 50)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], 50)

        w.setScaleString('1:4,000')
        self.assertEqual(w.scaleString(), '1:4,000')
        self.assertEqual(w.scale(), 4000)
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0], 4000)

    def testNull(self):
        w = QgsScaleWidget()

        w.setScale(50)
        self.assertFalse(w.allowNull())
        w.setNull() # no effect
        self.assertEqual(w.scale(), 50.0)
        self.assertFalse(w.isNull())

        spy = QSignalSpy(w.scaleChanged)
        w.setAllowNull(True)
        self.assertTrue(w.allowNull())

        w.setScaleString('')
        self.assertEqual(len(spy), 1)
        self.assertTrue(math.isnan(w.scale()))
        self.assertTrue(math.isnan(spy[-1][0]))
        self.assertTrue(w.isNull())
        w.setScaleString("    ")
        self.assertTrue(math.isnan(w.scale()))
        self.assertTrue(w.isNull())

        w.setScaleString('0.02')
        self.assertEqual(w.scale(), 50.0)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], 50.0)
        self.assertFalse(w.isNull())

        w.setScaleString('')
        self.assertTrue(math.isnan(w.scale()))
        self.assertEqual(len(spy), 3)
        self.assertTrue(math.isnan(spy[-1][0]))
        self.assertTrue(w.isNull())

        w.setScaleString('0.02')
        self.assertEqual(w.scale(), 50.0)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0], 50.0)
        self.assertFalse(w.isNull())
        w.setNull()
        self.assertTrue(math.isnan(w.scale()))
        self.assertEqual(len(spy), 5)
        self.assertTrue(math.isnan(spy[-1][0]))
        self.assertTrue(w.isNull())

        w.setAllowNull(False)
        self.assertFalse(w.allowNull())


if __name__ == '__main__':
    unittest.main()
