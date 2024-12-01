"""QGIS Unit tests for QgsLabelLineSettings

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2019-12-07"
__copyright__ = "Copyright 2019, The QGIS Project"

import os

from qgis.core import (
    QgsExpressionContext,
    QgsExpressionContextScope,
    QgsLabeling,
    QgsLabelLineSettings,
    QgsMapUnitScale,
    QgsPalLayerSettings,
    QgsProperty,
    QgsPropertyCollection,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLabelLineSettings(QgisTestCase):

    def test_line_settings(self):
        """
        Test line settings
        """
        settings = QgsLabelLineSettings()
        settings.setPlacementFlags(QgsLabeling.LinePlacementFlag.OnLine)
        self.assertEqual(
            settings.placementFlags(), QgsLabeling.LinePlacementFlag.OnLine
        )
        settings.setPlacementFlags(
            QgsLabeling.LinePlacementFlag.OnLine
            | QgsLabeling.LinePlacementFlag.MapOrientation
        )
        self.assertEqual(
            settings.placementFlags(),
            QgsLabeling.LinePlacementFlag.OnLine
            | QgsLabeling.LinePlacementFlag.MapOrientation,
        )

        settings.setMergeLines(True)
        self.assertTrue(settings.mergeLines())
        settings.setMergeLines(False)
        self.assertFalse(settings.mergeLines())

        settings.setAddDirectionSymbol(True)
        self.assertTrue(settings.addDirectionSymbol())
        settings.setAddDirectionSymbol(False)
        self.assertFalse(settings.addDirectionSymbol())
        settings.setLeftDirectionSymbol("left")
        self.assertEqual(settings.leftDirectionSymbol(), "left")
        settings.setRightDirectionSymbol("right")
        self.assertEqual(settings.rightDirectionSymbol(), "right")
        settings.setReverseDirectionSymbol(True)
        self.assertTrue(settings.reverseDirectionSymbol())
        settings.setReverseDirectionSymbol(False)
        self.assertFalse(settings.reverseDirectionSymbol())

        settings.setDirectionSymbolPlacement(
            QgsLabelLineSettings.DirectionSymbolPlacement.SymbolBelow
        )
        self.assertEqual(
            settings.directionSymbolPlacement(),
            QgsLabelLineSettings.DirectionSymbolPlacement.SymbolBelow,
        )
        settings.setDirectionSymbolPlacement(
            QgsLabelLineSettings.DirectionSymbolPlacement.SymbolAbove
        )
        self.assertEqual(
            settings.directionSymbolPlacement(),
            QgsLabelLineSettings.DirectionSymbolPlacement.SymbolAbove,
        )

        settings.setOverrunDistance(5.6)
        self.assertEqual(settings.overrunDistance(), 5.6)
        settings.setOverrunDistanceUnit(QgsUnitTypes.RenderUnit.RenderInches)
        self.assertEqual(
            settings.overrunDistanceUnit(), QgsUnitTypes.RenderUnit.RenderInches
        )
        scale = QgsMapUnitScale(1, 2)
        settings.setOverrunDistanceMapUnitScale(scale)
        self.assertEqual(settings.overrunDistanceMapUnitScale().minScale, 1)
        self.assertEqual(settings.overrunDistanceMapUnitScale().maxScale, 2)

        settings.setLineAnchorPercent(0.3)
        self.assertEqual(settings.lineAnchorPercent(), 0.3)

        # check that compatibility code works
        pal_settings = QgsPalLayerSettings()
        pal_settings.placementFlags = (
            QgsPalLayerSettings.LinePlacementFlags.OnLine
            | QgsPalLayerSettings.LinePlacementFlags.MapOrientation
        )
        self.assertEqual(
            pal_settings.placementFlags,
            QgsPalLayerSettings.LinePlacementFlags.OnLine
            | QgsPalLayerSettings.LinePlacementFlags.MapOrientation,
        )
        self.assertEqual(
            pal_settings.lineSettings().placementFlags(),
            QgsLabeling.LinePlacementFlag.OnLine
            | QgsLabeling.LinePlacementFlag.MapOrientation,
        )

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

        pal_settings.leftDirectionSymbol = "l"
        self.assertEqual(pal_settings.leftDirectionSymbol, "l")
        self.assertEqual(pal_settings.lineSettings().leftDirectionSymbol(), "l")
        pal_settings.rightDirectionSymbol = "r"
        self.assertEqual(pal_settings.rightDirectionSymbol, "r")
        self.assertEqual(pal_settings.lineSettings().rightDirectionSymbol(), "r")

        pal_settings.reverseDirectionSymbol = True
        self.assertTrue(pal_settings.reverseDirectionSymbol)
        self.assertTrue(pal_settings.lineSettings().reverseDirectionSymbol())
        pal_settings.reverseDirectionSymbol = False
        self.assertFalse(pal_settings.reverseDirectionSymbol)
        self.assertFalse(pal_settings.lineSettings().reverseDirectionSymbol())

        pal_settings.placeDirectionSymbol = (
            QgsPalLayerSettings.DirectionSymbols.SymbolAbove
        )
        self.assertEqual(pal_settings.placeDirectionSymbol, 1)
        self.assertEqual(
            pal_settings.lineSettings().directionSymbolPlacement(),
            QgsLabelLineSettings.DirectionSymbolPlacement.SymbolAbove,
        )

        pal_settings.overrunDistance = 4.2
        self.assertEqual(pal_settings.overrunDistance, 4.2)
        self.assertEqual(pal_settings.lineSettings().overrunDistance(), 4.2)

        pal_settings.overrunDistanceUnit = QgsUnitTypes.RenderUnit.RenderInches
        self.assertEqual(
            pal_settings.overrunDistanceUnit, QgsUnitTypes.RenderUnit.RenderInches
        )
        self.assertEqual(
            pal_settings.lineSettings().overrunDistanceUnit(),
            QgsUnitTypes.RenderUnit.RenderInches,
        )
        pal_settings.overrunDistanceMapUnitScale = scale
        self.assertEqual(pal_settings.overrunDistanceMapUnitScale.minScale, 1)
        self.assertEqual(pal_settings.overrunDistanceMapUnitScale.maxScale, 2)
        self.assertEqual(
            pal_settings.lineSettings().overrunDistanceMapUnitScale().minScale, 1
        )
        self.assertEqual(
            pal_settings.lineSettings().overrunDistanceMapUnitScale().maxScale, 2
        )

    def testUpdateDataDefinedProps(self):
        settings = QgsLabelLineSettings()
        settings.setPlacementFlags(QgsLabeling.LinePlacementFlag.OnLine)
        settings.setOverrunDistance(5.6)
        settings.setLineAnchorPercent(0.3)
        self.assertEqual(
            settings.placementFlags(), QgsLabeling.LinePlacementFlag.OnLine
        )
        self.assertEqual(settings.overrunDistance(), 5.6)
        self.assertEqual(settings.lineAnchorPercent(), 0.3)

        props = QgsPropertyCollection()
        props.setProperty(
            QgsPalLayerSettings.Property.LinePlacementOptions,
            QgsProperty.fromExpression("@placement"),
        )
        props.setProperty(
            QgsPalLayerSettings.Property.OverrunDistance,
            QgsProperty.fromExpression("@dist"),
        )
        props.setProperty(
            QgsPalLayerSettings.Property.LineAnchorPercent,
            QgsProperty.fromExpression("@line_anchor"),
        )
        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable("placement", "AL,LO")
        scope.setVariable("dist", "11.2")
        scope.setVariable("line_anchor", "0.6")
        context.appendScope(scope)
        settings.updateDataDefinedProperties(props, context)
        self.assertEqual(
            settings.placementFlags(), QgsLabeling.LinePlacementFlag.AboveLine
        )
        self.assertEqual(settings.overrunDistance(), 11.2)
        self.assertEqual(settings.lineAnchorPercent(), 0.6)


if __name__ == "__main__":
    unittest.main()
