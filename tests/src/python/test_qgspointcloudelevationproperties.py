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
)

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsPointCloudElevationProperties(unittest.TestCase):

    def testBasic(self):
        props = QgsPointCloudLayerElevationProperties(None)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)

        props.setZOffset(0.5)
        props.setZScale(2)
        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsPointCloudLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)


if __name__ == '__main__':
    unittest.main()
