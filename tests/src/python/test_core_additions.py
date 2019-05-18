# -*- coding: utf-8 -*-
"""QGIS Unit tests for core additions

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '15.5.2018'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA
import os

from qgis.testing import unittest, start_app
from qgis.core import metaEnumFromValue, metaEnumFromType, QgsTolerance, QgsMapLayer
from qgis.PyQt import sip

start_app()


class TestCoreAdditions(unittest.TestCase):

    def testMetaEnum(self):
        me = metaEnumFromValue(QgsTolerance.Pixels)
        self.assertIsNotNone(me)
        self.assertEqual(me.valueToKey(QgsTolerance.Pixels), 'Pixels')

        # if using same variable twice (e.g. me = me2), this seg faults
        me2 = metaEnumFromValue(QgsTolerance.Pixels, QgsTolerance)
        self.assertIsNotNone(me)
        self.assertEqual(me2.valueToKey(QgsTolerance.Pixels), 'Pixels')

        # do not raise error
        self.assertIsNone(metaEnumFromValue(1, QgsTolerance, False))

        # do not provide an int
        with self.assertRaises(TypeError):
            metaEnumFromValue(1)

        # QgsMapLayer.LayerType is not a Q_ENUM
        with self.assertRaises(ValueError):
            metaEnumFromValue(QgsMapLayer.LayerType)


if __name__ == "__main__":
    unittest.main()
