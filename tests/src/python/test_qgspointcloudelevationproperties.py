# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPointCloudLayerElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    QgsPointCloudLayerElevationProperties,
    QgsReadWriteContext,
    QgsUnitTypes,
    Qgis
)
from qgis.PyQt.QtGui import QColor

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsPointCloudElevationProperties(unittest.TestCase):

    def testBasic(self):
        props = QgsPointCloudLayerElevationProperties(None)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)
        self.assertTrue(props.respectLayerColors())
        self.assertTrue(props.pointColor().isValid())

        props.setZOffset(0.5)
        props.setZScale(2)
        props.setMaximumScreenError(0.4)
        props.setMaximumScreenErrorUnit(QgsUnitTypes.RenderInches)
        props.setPointSymbol(Qgis.PointCloudSymbol.Circle)
        props.setPointColor(QColor(255, 0, 255))
        props.setPointSize(1.2)
        props.setPointSizeUnit(QgsUnitTypes.RenderPoints)
        props.setRespectLayerColors(False)

        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)
        self.assertEqual(props.maximumScreenError(), 0.4)
        self.assertEqual(props.maximumScreenErrorUnit(), QgsUnitTypes.RenderInches)
        self.assertEqual(props.pointSymbol(), Qgis.PointCloudSymbol.Circle)
        self.assertEqual(props.pointColor().name(), '#ff00ff')
        self.assertEqual(props.pointSize(), 1.2)
        self.assertEqual(props.pointSizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertFalse(props.respectLayerColors())

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsPointCloudLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertEqual(props2.maximumScreenError(), 0.4)
        self.assertEqual(props2.maximumScreenErrorUnit(), QgsUnitTypes.RenderInches)
        self.assertEqual(props2.pointSymbol(), Qgis.PointCloudSymbol.Circle)
        self.assertEqual(props2.pointColor().name(), '#ff00ff')
        self.assertEqual(props2.pointSize(), 1.2)
        self.assertEqual(props2.pointSizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertFalse(props2.respectLayerColors())

        props2 = props.clone()
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertEqual(props2.maximumScreenError(), 0.4)
        self.assertEqual(props2.maximumScreenErrorUnit(), QgsUnitTypes.RenderInches)
        self.assertEqual(props2.pointSymbol(), Qgis.PointCloudSymbol.Circle)
        self.assertEqual(props2.pointColor().name(), '#ff00ff')
        self.assertEqual(props2.pointSize(), 1.2)
        self.assertEqual(props2.pointSizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertFalse(props2.respectLayerColors())


if __name__ == '__main__':
    unittest.main()
