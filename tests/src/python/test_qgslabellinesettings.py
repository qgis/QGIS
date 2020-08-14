# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLabelLineSettings

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2019-12-07'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsProperty,
                       QgsPropertyCollection,
                       QgsPalLayerSettings,
                       QgsLabelLineSettings,
                       QgsExpressionContext,
                       QgsExpressionContextScope,
                       QgsGeometry,
                       QgsLabeling)

from qgis.testing import unittest, start_app

start_app()


class TestQgsLabelLineSettings(unittest.TestCase):

    def test_line_settings(self):
        """
        Test line settings
        """
        settings = QgsLabelLineSettings()
        settings.setPlacementFlags(QgsLabeling.LinePlacementFlag.OnLine)
        self.assertEqual(settings.placementFlags(), QgsLabeling.LinePlacementFlag.OnLine)
        settings.setPlacementFlags(QgsLabeling.LinePlacementFlag.OnLine | QgsLabeling.LinePlacementFlag.MapOrientation)
        self.assertEqual(settings.placementFlags(), QgsLabeling.LinePlacementFlag.OnLine | QgsLabeling.LinePlacementFlag.MapOrientation)

        settings.setMergeLines(True)
        self.assertTrue(settings.mergeLines())
        settings.setMergeLines(False)
        self.assertFalse(settings.mergeLines())

        settings.setAddDirectionSymbol(True)
        self.assertTrue(settings.addDirectionSymbol())
        settings.setAddDirectionSymbol(False)
        self.assertFalse(settings.addDirectionSymbol())
        settings.setLeftDirectionSymbol('left')
        self.assertEqual(settings.leftDirectionSymbol(), 'left')
        settings.setRightDirectionSymbol('right')
        self.assertEqual(settings.rightDirectionSymbol(), 'right')
        settings.setReverseDirectionSymbol(True)
        self.assertTrue(settings.reverseDirectionSymbol())
        settings.setReverseDirectionSymbol(False)
        self.assertFalse(settings.reverseDirectionSymbol())

        # check that compatibility code works
        pal_settings = QgsPalLayerSettings()
        pal_settings.placementFlags = QgsPalLayerSettings.OnLine | QgsPalLayerSettings.MapOrientation
        self.assertEqual(pal_settings.placementFlags, 9)
        self.assertTrue(pal_settings.lineSettings().placementFlags(), QgsLabeling.LinePlacementFlag.OnLine | QgsLabeling.LinePlacementFlag.MapOrientation)

        pal_settings.mergeLines = True
        self.assertTrue(pal_settings.mergeLines)
        self.assertTrue(pal_settings.lineSettings().mergeLines())
        pal_settings.mergeLines = False
        self.assertFalse(pal_settings.mergeLines)
        self.assertFalse(pal_settings.lineSettings().mergeLines())

        pal_settings.addDirectionSymbol = True
        self.assertTrue(pal_settings.addDirectionSymbol)
        self.assertTrue(pal_settings.lineSettings().addDirectionSymbol())
        pal_settings.addDirectionSymbol = False
        self.assertFalse(pal_settings.addDirectionSymbol)
        self.assertFalse(pal_settings.lineSettings().addDirectionSymbol())

        pal_settings.leftDirectionSymbol = 'l'
        self.assertEqual(pal_settings.leftDirectionSymbol, 'l')
        self.assertEqual(pal_settings.lineSettings().leftDirectionSymbol(), 'l')
        pal_settings.rightDirectionSymbol = 'r'
        self.assertEqual(pal_settings.rightDirectionSymbol, 'r')
        self.assertEqual(pal_settings.lineSettings().rightDirectionSymbol(), 'r')

        pal_settings.reverseDirectionSymbol = True
        self.assertTrue(pal_settings.reverseDirectionSymbol)
        self.assertTrue(pal_settings.lineSettings().reverseDirectionSymbol())
        pal_settings.reverseDirectionSymbol = False
        self.assertFalse(pal_settings.reverseDirectionSymbol)
        self.assertFalse(pal_settings.lineSettings().reverseDirectionSymbol())

    def testUpdateDataDefinedProps(self):
        settings = QgsLabelLineSettings()
        settings.setPlacementFlags(QgsLabeling.LinePlacementFlag.OnLine)
        self.assertEqual(settings.placementFlags(), QgsLabeling.LinePlacementFlag.OnLine)

        props = QgsPropertyCollection()
        props.setProperty(QgsPalLayerSettings.LinePlacementOptions, QgsProperty.fromExpression('@placement'))
        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable('placement', 'AL,LO')
        context.appendScope(scope)
        settings.updateDataDefinedProperties(props, context)
        self.assertEqual(settings.placementFlags(), QgsLabeling.LinePlacementFlag.AboveLine)


if __name__ == '__main__':
    unittest.main()
