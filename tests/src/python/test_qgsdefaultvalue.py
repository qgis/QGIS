# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDefaultValue.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Matthias Kuhn'
__date__ = '26.9.2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA


from qgis.core import (QgsDefaultValue)
from qgis.testing import unittest


class TestQgsRasterColorRampShader(unittest.TestCase):

    def testValid(self):
        self.assertFalse(QgsDefaultValue())
        self.assertTrue(QgsDefaultValue('test'))
        self.assertTrue(QgsDefaultValue('abc', True))
        self.assertTrue(QgsDefaultValue('abc', False))

    def setGetExpression(self):
        value = QgsDefaultValue('abc', False)
        self.assertEqual(value.expression(), 'abc')
        value.setExpression('def')
        self.assertEqual(value.expression(), 'def')

    def setGetApplyOnUpdate(self):
        value = QgsDefaultValue('abc', False)
        self.assertEqual(value.applyOnUpdate(), False)
        value.setApplyOnUpdate(True)
        self.assertEqual(value.applyOnUpdate(), True)


if __name__ == '__main__':
    unittest.main()
