"""QGIS Unit tests for QgsVectorLayer labeling

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os

from qgis.PyQt.QtCore import QDir, QSize
from qgis.PyQt.QtGui import QColor, QPainter

from qgis.core import (
    edit,
    Qgis,
    QgsCategorizedSymbolRenderer,
    QgsCentroidFillSymbolLayer,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFeatureRendererGenerator,
    QgsFillSymbol,
    QgsGeometry,
    QgsGeometryGeneratorSymbolLayer,
    QgsLineSymbol,
    QgsMapClippingRegion,
    QgsMapSettings,
    QgsMarkerSymbol,
    QgsPointXY,
    QgsRectangle,
    QgsRendererCategory,
    QgsRuleBasedLabeling,
    QgsProperty,
    QgsTextFormat,
    QgsRenderContext,
    QgsPalLayerSettings,
    QgsVectorLayerSimpleLabeling,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsVectorLayerLabeling(QgisTestCase):

    def testHasNonDefaultCompositionModeSimple(self):
        settings = QgsPalLayerSettings()
        labeling = QgsVectorLayerSimpleLabeling(settings)
        self.assertFalse(labeling.hasNonDefaultCompositionMode())

        t = QgsTextFormat()
        t.setBlendMode(QPainter.CompositionMode.CompositionMode_DestinationAtop)
        settings.setFormat(t)
        labeling = QgsVectorLayerSimpleLabeling(settings)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

        t = QgsTextFormat()
        settings.setFormat(t)
        settings.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontBlendMode,
            QgsProperty.fromValue("multiply"),
        )
        labeling = QgsVectorLayerSimpleLabeling(settings)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowBlendMode,
            QgsProperty.fromValue("multiply"),
        )
        labeling = QgsVectorLayerSimpleLabeling(settings)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferBlendMode,
            QgsProperty.fromValue("multiply"),
        )
        labeling = QgsVectorLayerSimpleLabeling(settings)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeBlendMode,
            QgsProperty.fromValue("multiply"),
        )
        labeling = QgsVectorLayerSimpleLabeling(settings)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

    def testHasNonDefaultCompositionModeRuleBased(self):
        settings = QgsPalLayerSettings()
        root_rule = QgsRuleBasedLabeling.Rule(settings)
        labeling = QgsRuleBasedLabeling(root_rule)
        self.assertFalse(labeling.hasNonDefaultCompositionMode())

        t = QgsTextFormat()
        t.setBlendMode(QPainter.CompositionMode.CompositionMode_DestinationAtop)
        settings = QgsPalLayerSettings()
        settings.setFormat(t)
        root_rule = QgsRuleBasedLabeling.Rule(settings)
        labeling = QgsRuleBasedLabeling(root_rule)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontBlendMode,
            QgsProperty.fromValue("multiply"),
        )
        root_rule = QgsRuleBasedLabeling.Rule(settings)
        labeling = QgsRuleBasedLabeling(root_rule)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowBlendMode,
            QgsProperty.fromValue("multiply"),
        )
        root_rule = QgsRuleBasedLabeling.Rule(settings)
        labeling = QgsRuleBasedLabeling(root_rule)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeBlendMode,
            QgsProperty.fromValue("multiply"),
        )
        root_rule = QgsRuleBasedLabeling.Rule(settings)
        labeling = QgsRuleBasedLabeling(root_rule)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())

        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferBlendMode,
            QgsProperty.fromValue("multiply"),
        )
        root_rule = QgsRuleBasedLabeling.Rule(settings)
        labeling = QgsRuleBasedLabeling(root_rule)
        self.assertTrue(labeling.hasNonDefaultCompositionMode())


if __name__ == "__main__":
    unittest.main()
