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

from qgis.core import (QgsColorRampShader, QgsGradientColorRamp, QgsGradientStop)
from qgis.testing import unittest


class TestQgsRasterColorRampShader(unittest.TestCase):

    def testNan(self):
        shader = QgsColorRampShader()

        item1 = QgsColorRampShader.ColorRampItem(1, QColor(0, 0, 0))
        item2 = QgsColorRampShader.ColorRampItem(2, QColor(255, 255, 255))
        shader.setColorRampItemList([item1, item2])
        self.assertFalse(shader.shade(float('NaN'))[0])
        self.assertFalse(shader.shade(float("inf"))[0])

    def testCreateColorRamp(self):
        shader = QgsColorRampShader(1, 3)

        item1 = QgsColorRampShader.ColorRampItem(1, QColor(255, 0, 0))
        item2 = QgsColorRampShader.ColorRampItem(2, QColor(255, 255, 0))
        item3 = QgsColorRampShader.ColorRampItem(3, QColor(255, 255, 255))
        shader.setColorRampItemList([item1, item2, item3])
        shaderRamp = shader.createColorRamp()

        gradientRamp = QgsGradientColorRamp(QColor(255, 0, 0), QColor(255, 255, 255), False, [QgsGradientStop(0.5, QColor(255, 255, 0))])

        self.assertEqual(shaderRamp.color1(), gradientRamp.color1())
        self.assertEqual(shaderRamp.color2(), gradientRamp.color2())
        self.assertEqual(shaderRamp.stops(), gradientRamp.stops())

    def testTwoClassesDiscrete(self):
        # test for #47759
        shader = QgsColorRampShader(0, 50, None, QgsColorRampShader.Discrete)

        item1 = QgsColorRampShader.ColorRampItem(50, QColor(0, 0, 0))
        item2 = QgsColorRampShader.ColorRampItem(float("inf"), QColor(255, 255, 255))
        shader.setColorRampItemList([item1, item2])

        color1 = shader.shade(50)
        self.assertEqual(color1[1:4], (0, 0, 0))

        color2 = shader.shade(50.00000000001)
        self.assertEqual(color2[1:4], (255, 255, 255))


if __name__ == '__main__':
    unittest.main()
