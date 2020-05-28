# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsColorRampShader.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Nyall Dawson'
__date__ = '17/08/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA


from qgis.PyQt.QtGui import QColor

from qgis.core import (QgsColorRampShader)
from qgis.testing import unittest


class TestQgsRasterColorRampShader(unittest.TestCase):

    def testNan(self):
        shader = QgsColorRampShader()

        item1 = QgsColorRampShader.ColorRampItem(1, QColor(0, 0, 0))
        item2 = QgsColorRampShader.ColorRampItem(2, QColor(255, 255, 255))
        shader.setColorRampItemList([item1, item2])
        self.assertFalse(shader.shade(float('NaN'))[0])
        self.assertFalse(shader.shade(float("inf"))[0])


if __name__ == '__main__':
    unittest.main()
