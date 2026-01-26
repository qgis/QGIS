"""QGIS Unit tests for QgsPointCloudLayerElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "09/11/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsPointCloudClassifiedRenderer,
    QgsPointCloudLayer,
    QgsPointCloudLayerElevationProperties,
    QgsProviderRegistry,
    QgsReadWriteContext,
    QgsUnitTypes,
    QgsDoubleRange,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudElevationProperties(QgisTestCase):

    def testBasic(self):
        props = QgsPointCloudLayerElevationProperties(None)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)
        self.assertTrue(props.respectLayerColors())
        self.assertTrue(props.pointColor().isValid())

        props.setZOffset(0.5)
        props.setZScale(2)
        props.setMaximumScreenError(0.4)
        props.setMaximumScreenErrorUnit(QgsUnitTypes.RenderUnit.RenderInches)
        props.setPointSymbol(Qgis.PointCloudSymbol.Circle)
        props.setPointColor(QColor(255, 0, 255))
        props.setPointSize(1.2)
        props.setPointSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        props.setRespectLayerColors(False)

        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)
        self.assertEqual(props.maximumScreenError(), 0.4)
        self.assertEqual(
            props.maximumScreenErrorUnit(), QgsUnitTypes.RenderUnit.RenderInches
        )
        self.assertEqual(props.pointSymbol(), Qgis.PointCloudSymbol.Circle)
        self.assertEqual(props.pointColor().name(), "#ff00ff")
        self.assertEqual(props.pointSize(), 1.2)
        self.assertEqual(props.pointSizeUnit(), QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertFalse(props.respectLayerColors())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsPointCloudLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertEqual(props2.maximumScreenError(), 0.4)
        self.assertEqual(
            props2.maximumScreenErrorUnit(), QgsUnitTypes.RenderUnit.RenderInches
        )
        self.assertEqual(props2.pointSymbol(), Qgis.PointCloudSymbol.Circle)
        self.assertEqual(props2.pointColor().name(), "#ff00ff")
        self.assertEqual(props2.pointSize(), 1.2)
        self.assertEqual(props2.pointSizeUnit(), QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertFalse(props2.respectLayerColors())

        props2 = props.clone()
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertEqual(props2.maximumScreenError(), 0.4)
        self.assertEqual(
            props2.maximumScreenErrorUnit(), QgsUnitTypes.RenderUnit.RenderInches
        )
        self.assertEqual(props2.pointSymbol(), Qgis.PointCloudSymbol.Circle)
        self.assertEqual(props2.pointColor().name(), "#ff00ff")
        self.assertEqual(props2.pointSize(), 1.2)
        self.assertEqual(props2.pointSizeUnit(), QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertFalse(props2.respectLayerColors())

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def test_signals(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        props = layer.elevationProperties()
        spy = QSignalSpy(props.profileGenerationPropertyChanged)

        # when we are respecting layer colors, changing the 2d point cloud renderer should trigger a profile regeneration
        props.setRespectLayerColors(True)
        layer.setRenderer(QgsPointCloudClassifiedRenderer())
        self.assertEqual(len(spy), 1)

        # when we aren't respecting layer colors, no signal should be emitted
        props.setRespectLayerColors(False)
        layer.setRenderer(QgsPointCloudClassifiedRenderer())
        self.assertEqual(len(spy), 1)

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def test_layer_calculations(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/rgb/ept.json", "test", "ept"
        )
        self.assertTrue(layer.isValid())

        props = layer.elevationProperties()
        self.assertEqual(props.calculateZRange(layer), QgsDoubleRange(0.98, 1.25))
        self.assertEqual(props.significantZValues(layer), [0.98, 1.25])

        props.setZScale(2)
        props.setZOffset(0.1)
        self.assertEqual(props.significantZValues(layer), [2.06, 2.6])


if __name__ == "__main__":
    unittest.main()
