"""QGIS Unit tests for QgsTiledSceneLayerElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '23/08/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsTiledSceneLayer,
    QgsTiledSceneLayerElevationProperties,
    QgsProviderRegistry,
    QgsReadWriteContext,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsTiledSceneElevationProperties(QgisTestCase):

    def testBasic(self):
        props = QgsTiledSceneLayerElevationProperties(None)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)

        props.setZOffset(0.5)
        props.setZScale(2)

        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsTiledSceneLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)

        props2 = props.clone()
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)


if __name__ == '__main__':
    unittest.main()
