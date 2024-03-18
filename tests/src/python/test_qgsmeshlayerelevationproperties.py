"""QGIS Unit tests for QgsMeshLayerElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsFillSymbol,
    QgsLineSymbol,
    QgsMeshLayerElevationProperties,
    QgsReadWriteContext,
    QgsDoubleRange
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMeshLayerElevationProperties(QgisTestCase):

    def testBasic(self):
        props = QgsMeshLayerElevationProperties(None)
        self.assertEqual(props.mode(),
                         Qgis.MeshElevationMode.FromVertices)

        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)
        self.assertTrue(props.hasElevation())
        self.assertTrue(props.fixedRange().isInfinite())
        self.assertIsInstance(props.profileLineSymbol(), QgsLineSymbol)
        self.assertIsInstance(props.profileFillSymbol(), QgsFillSymbol)
        self.assertEqual(props.profileSymbology(), Qgis.ProfileSurfaceSymbology.Line)

        props.setZOffset(0.5)
        props.setZScale(2)
        props.setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillBelow)
        props.setElevationLimit(909)
        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)
        self.assertTrue(props.hasElevation())
        self.assertEqual(props.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props.elevationLimit(), 909)

        sym = QgsLineSymbol.createSimple({'outline_color': '#ff4433', 'outline_width': 0.5})
        props.setProfileLineSymbol(sym)
        self.assertEqual(props.profileLineSymbol().color().name(), '#ff4433')

        sym = QgsFillSymbol.createSimple({'color': '#ff44ff'})
        props.setProfileFillSymbol(sym)
        self.assertEqual(props.profileFillSymbol().color().name(), '#ff44ff')

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsMeshLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(),
                         Qgis.MeshElevationMode.FromVertices)
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff44ff')
        self.assertEqual(props2.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props2.elevationLimit(), 909)

        props2 = props.clone()
        self.assertEqual(props2.mode(),
                         Qgis.MeshElevationMode.FromVertices)
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff44ff')
        self.assertEqual(props2.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props2.elevationLimit(), 909)

    def test_basic_fixed_range(self):
        """
        Basic tests for the class using the FixedElevationRange mode
        """
        props = QgsMeshLayerElevationProperties(None)
        self.assertTrue(props.fixedRange().isInfinite())

        props.setMode(Qgis.MeshElevationMode.FixedElevationRange)
        props.setFixedRange(QgsDoubleRange(103.1, 106.8))
        # fixed ranges should not be affected by scale/offset
        props.setZOffset(0.5)
        props.setZScale(2)
        self.assertEqual(props.fixedRange(), QgsDoubleRange(103.1, 106.8))
        self.assertEqual(props.calculateZRange(None),
                         QgsDoubleRange(103.1, 106.8))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(3.1, 6.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(3.1, 104.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(104.8, 114.8)))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(114.8, 124.8)))

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsMeshLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(),
                         Qgis.MeshElevationMode.FixedElevationRange)
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8))

        props2 = props.clone()
        self.assertEqual(props2.mode(),
                         Qgis.MeshElevationMode.FixedElevationRange)
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8))

        # include lower, exclude upper
        props.setFixedRange(QgsDoubleRange(103.1, 106.8,
                                           includeLower=True,
                                           includeUpper=False))
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsMeshLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8,
                                                             includeLower=True,
                                                             includeUpper=False))

        # exclude lower, include upper
        props.setFixedRange(QgsDoubleRange(103.1, 106.8,
                                           includeLower=False,
                                           includeUpper=True))
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsMeshLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8,
                                                             includeLower=False,
                                                             includeUpper=True))


if __name__ == '__main__':
    unittest.main()
