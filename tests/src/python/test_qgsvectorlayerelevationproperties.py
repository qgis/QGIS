# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerElevationProperties

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
    Qgis,
    QgsVectorLayerElevationProperties,
    QgsReadWriteContext,
)

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsVectorLayerElevationProperties(unittest.TestCase):

    def testBasic(self):
        props = QgsVectorLayerElevationProperties(None)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)
        self.assertFalse(props.extrusionEnabled())
        self.assertEqual(props.extrusionHeight(), 0)
        self.assertFalse(props.hasElevation())
        self.assertEqual(props.clamping(), Qgis.AltitudeClamping.Terrain)
        self.assertEqual(props.binding(), Qgis.AltitudeBinding.Centroid)

        props.setZOffset(0.5)
        props.setZScale(2)
        props.setClamping(Qgis.AltitudeClamping.Relative)
        props.setBinding(Qgis.AltitudeBinding.Vertex)
        props.setExtrusionHeight(10)
        props.setExtrusionEnabled(True)
        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)
        self.assertEqual(props.extrusionHeight(), 10)
        self.assertTrue(props.hasElevation())
        self.assertTrue(props.extrusionEnabled())
        self.assertEqual(props.clamping(), Qgis.AltitudeClamping.Relative)
        self.assertEqual(props.binding(), Qgis.AltitudeBinding.Vertex)

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsVectorLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertEqual(props2.clamping(), Qgis.AltitudeClamping.Relative)
        self.assertEqual(props2.binding(), Qgis.AltitudeBinding.Vertex)
        self.assertEqual(props2.extrusionHeight(), 10)
        self.assertTrue(props2.extrusionEnabled())


if __name__ == '__main__':
    unittest.main()
