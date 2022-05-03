# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterLayerElevationProperties

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
    QgsRasterLayerElevationProperties,
    QgsReadWriteContext,
    QgsLineSymbol,
    QgsFillSymbol,
    Qgis
)

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsRasterLayerElevationProperties(unittest.TestCase):

    def testBasic(self):
        props = QgsRasterLayerElevationProperties(None)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)
        self.assertFalse(props.isEnabled())
        self.assertFalse(props.hasElevation())
        self.assertEqual(props.bandNumber(), 1)
        self.assertIsInstance(props.profileLineSymbol(), QgsLineSymbol)
        self.assertIsInstance(props.profileFillSymbol(), QgsFillSymbol)
        self.assertEqual(props.profileSymbology(), Qgis.ProfileSurfaceSymbology.Line)

        props.setZOffset(0.5)
        props.setZScale(2)
        props.setBandNumber(2)
        props.setEnabled(True)
        props.setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)
        self.assertTrue(props.isEnabled())
        self.assertEqual(props.bandNumber(), 2)
        self.assertTrue(props.hasElevation())
        self.assertEqual(props.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)

        sym = QgsLineSymbol.createSimple({'outline_color': '#ff4433', 'outline_width': 0.5})
        props.setProfileLineSymbol(sym)
        self.assertEqual(props.profileLineSymbol().color().name(), '#ff4433')

        sym = QgsFillSymbol.createSimple({'color': '#ff44ff'})
        props.setProfileFillSymbol(sym)
        self.assertEqual(props.profileFillSymbol().color().name(), '#ff44ff')

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertTrue(props2.isEnabled())
        self.assertEqual(props2.bandNumber(), 2)
        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff44ff')
        self.assertEqual(props2.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)

        props2 = props.clone()
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertTrue(props2.isEnabled())
        self.assertEqual(props2.bandNumber(), 2)
        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff44ff')
        self.assertEqual(props2.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)


if __name__ == '__main__':
    unittest.main()
