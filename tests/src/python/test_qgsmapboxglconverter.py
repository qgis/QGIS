# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapBoxGlStyleConverter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '29/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import (QSize,
                              QDir)
from qgis.PyQt.QtGui import (QImage,
                             QPainter,
                             QColor)
from qgis.core import (QgsMapBoxGlStyleConverter,
                       QgsCoordinateTransform,
                       QgsProject,
                       QgsPoint,
                       QgsCoordinateReferenceSystem,
                       QgsFillSymbol,
                       QgsRenderChecker,
                       QgsReadWriteContext,
                       QgsRenderContext,
                       QgsAnnotationPolygonItem,
                       QgsRectangle,
                       QgsLineString,
                       QgsPolygon,
                       QgsCurvePolygon,
                       QgsCircularString
                       )
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMapBoxGlStyleConverter(unittest.TestCase):

    maxDiff = 100000

    def testNoLayer(self):
        c = QgsMapBoxGlStyleConverter({'x': 'y'})
        self.assertEqual(c.errorMessage(), 'Could not find layers list in JSON')
        self.assertIsNone(c.renderer())
        self.assertIsNone(c.labeling())

    def testInterpolateExpression(self):
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1),
                         '27 + 2 * (1^(@zoom_level-5)-1)/(1^(13-5)-1)')
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1.5),
                         '27 + 2 * (1.5^(@zoom_level-5)-1)/(1.5^(13-5)-1)')

    def testColorAsHslaComponents(self):
        self.assertEqual(QgsMapBoxGlStyleConverter.colorAsHslaComponents(QColor.fromHsl(30, 50, 70)), (30, 19, 27, 255))

    def testParseInterpolateColorByZoom(self):
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({}).isActive(), False)
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({'base': 1,
                                                                                'stops': [[0, '#f1f075'],
                                                                                          [150, '#b52e3e'],
                                                                                          [250, '#e55e5e']]
                                                                                }).expressionString(), 'CASE WHEN @zoom_level >= 0 AND @zoom_level < 150 THEN color_hsla(scale_linear(@zoom_level, 0, 150, 59, 352), scale_linear(@zoom_level, 0, 150, 81, 59), scale_linear(@zoom_level, 0, 150, 70, 44), scale_linear(@zoom_level, 0, 150, 255, 255)) WHEN @zoom_level >= 150 AND @zoom_level < 250 THEN color_hsla(scale_linear(@zoom_level, 150, 250, 352, 0), scale_linear(@zoom_level, 150, 250, 59, 72), scale_linear(@zoom_level, 150, 250, 44, 63), scale_linear(@zoom_level, 150, 250, 255, 255)) WHEN @zoom_level >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({'base': 2,
                                                                                'stops': [[0, '#f1f075'],
                                                                                          [150, '#b52e3e'],
                                                                                          [250, '#e55e5e']]
                                                                                }).expressionString(), 'CASE WHEN @zoom_level >= 0 AND @zoom_level < 150 THEN color_hsla(59 + 293 * (2^(@zoom_level-0)-1)/(2^(150-0)-1), 81 + -22 * (2^(@zoom_level-0)-1)/(2^(150-0)-1), 70 + -26 * (2^(@zoom_level-0)-1)/(2^(150-0)-1), 255 + 0 * (2^(@zoom_level-0)-1)/(2^(150-0)-1)) WHEN @zoom_level >= 150 AND @zoom_level < 250 THEN color_hsla(352 + -352 * (2^(@zoom_level-150)-1)/(2^(250-150)-1), 59 + 13 * (2^(@zoom_level-150)-1)/(2^(250-150)-1), 44 + 19 * (2^(@zoom_level-150)-1)/(2^(250-150)-1), 255 + 0 * (2^(@zoom_level-150)-1)/(2^(250-150)-1)) WHEN @zoom_level >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')


if __name__ == '__main__':
    unittest.main()
