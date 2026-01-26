"""QGIS Unit tests for QgsLabelPlacementSettings

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2024-02-02"
__copyright__ = "Copyright 2024, The QGIS Project"


from qgis.core import (
    Qgis,
    QgsPalLayerSettings,
    QgsLabelPlacementSettings,
    QgsPropertyCollection,
    QgsProperty,
    QgsExpressionContext,
    QgsExpressionContextScope,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLabelPlacementSettings(QgisTestCase):

    def test_placement_settings(self):
        """
        Test placement settings
        """

        # check that compatibility code works
        pal_settings = QgsPalLayerSettings()
        pal_settings.displayAll = True
        self.assertTrue(pal_settings.displayAll)
        self.assertEqual(
            pal_settings.placementSettings().overlapHandling(),
            Qgis.LabelOverlapHandling.AllowOverlapIfRequired,
        )
        self.assertTrue(pal_settings.placementSettings().allowDegradedPlacement())
        pal_settings.displayAll = False
        self.assertFalse(pal_settings.displayAll)
        self.assertEqual(
            pal_settings.placementSettings().overlapHandling(),
            Qgis.LabelOverlapHandling.PreventOverlap,
        )
        self.assertFalse(pal_settings.placementSettings().allowDegradedPlacement())

        pal_settings.labelPerPart = True
        self.assertTrue(pal_settings.labelPerPart)
        self.assertEqual(
            pal_settings.placementSettings().multiPartBehavior(),
            Qgis.MultiPartLabelingBehavior.LabelEveryPartWithEntireLabel,
        )
        pal_settings.labelPerPart = False
        self.assertFalse(pal_settings.labelPerPart)
        self.assertEqual(
            pal_settings.placementSettings().multiPartBehavior(),
            Qgis.MultiPartLabelingBehavior.LabelLargestPartOnly,
        )
        pal_settings.placementSettings().setMultiPartBehavior(
            Qgis.MultiPartLabelingBehavior.LabelEveryPartWithEntireLabel
        )
        self.assertTrue(pal_settings.labelPerPart)
        pal_settings.placementSettings().setMultiPartBehavior(
            Qgis.MultiPartLabelingBehavior.LabelLargestPartOnly
        )
        self.assertFalse(pal_settings.labelPerPart)
        pal_settings.placementSettings().setMultiPartBehavior(
            Qgis.MultiPartLabelingBehavior.SplitLabelTextLinesOverParts
        )
        self.assertFalse(pal_settings.labelPerPart)

        pal_settings.placementSettings().setWhitespaceCollisionHandling(
            Qgis.LabelWhitespaceCollisionHandling.IgnoreWhitespaceCollisions
        )
        self.assertEqual(
            pal_settings.placementSettings().whitespaceCollisionHandling(),
            Qgis.LabelWhitespaceCollisionHandling.IgnoreWhitespaceCollisions,
        )

    def testUpdateDataDefinedProps(self):
        settings = QgsLabelPlacementSettings()
        settings.setAllowDegradedPlacement(True)
        settings.setOverlapHandling(Qgis.LabelOverlapHandling.AllowOverlapIfRequired)
        settings.setMultiPartBehavior(
            Qgis.MultiPartLabelingBehavior.SplitLabelTextLinesOverParts
        )

        props = QgsPropertyCollection()
        props.setProperty(
            QgsPalLayerSettings.Property.AllowDegradedPlacement,
            QgsProperty.fromExpression("@allow_degraded"),
        )
        props.setProperty(
            QgsPalLayerSettings.Property.OverlapHandling,
            QgsProperty.fromExpression("@overlap_handling"),
        )
        props.setProperty(
            QgsPalLayerSettings.Property.LabelAllParts,
            QgsProperty.fromExpression("@multi_part"),
        )
        props.setProperty(
            QgsPalLayerSettings.Property.WhitespaceCollisionHandling,
            QgsProperty.fromExpression("@whitespace"),
        )
        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable("allow_degraded", "1")
        scope.setVariable("overlap_handling", "alwaysallow")
        scope.setVariable("multi_part", "largestPartOnly")
        scope.setVariable("whitespace", "ignoreWhitespaceCollisions")
        context.appendScope(scope)
        settings.updateDataDefinedProperties(props, context)
        self.assertTrue(settings.allowDegradedPlacement())
        self.assertEqual(
            settings.overlapHandling(), Qgis.LabelOverlapHandling.AllowOverlapAtNoCost
        )
        self.assertEqual(
            settings.multiPartBehavior(),
            Qgis.MultiPartLabelingBehavior.LabelLargestPartOnly,
        )
        self.assertEqual(
            settings.whitespaceCollisionHandling(),
            Qgis.LabelWhitespaceCollisionHandling.IgnoreWhitespaceCollisions,
        )

        scope.setVariable("allow_degraded", "0")
        scope.setVariable("multi_part", "LabelEveryPart")
        settings.updateDataDefinedProperties(props, context)
        self.assertFalse(settings.allowDegradedPlacement())
        self.assertEqual(
            settings.multiPartBehavior(),
            Qgis.MultiPartLabelingBehavior.LabelEveryPartWithEntireLabel,
        )

        scope.setVariable("multi_part", "SplitLabelTextLinesOverParts")
        settings.updateDataDefinedProperties(props, context)
        self.assertEqual(
            settings.multiPartBehavior(),
            Qgis.MultiPartLabelingBehavior.SplitLabelTextLinesOverParts,
        )

        # LabelAllParts should support booleans too, for compatibility with older
        # projects
        scope.setVariable("multi_part", "1")
        settings.updateDataDefinedProperties(props, context)
        self.assertEqual(
            settings.multiPartBehavior(),
            Qgis.MultiPartLabelingBehavior.LabelEveryPartWithEntireLabel,
        )
        scope.setVariable("multi_part", "0")
        settings.updateDataDefinedProperties(props, context)
        self.assertEqual(
            settings.multiPartBehavior(),
            Qgis.MultiPartLabelingBehavior.LabelLargestPartOnly,
        )


if __name__ == "__main__":
    unittest.main()
