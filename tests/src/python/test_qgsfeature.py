# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFeature.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Germán Carrillo'
__date__ = '06/10/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
from qgis.core import QgsFeature, QgsGeometry, QgsPointXY, QgsVectorLayer, NULL, QgsFields, QgsField
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsFeature(unittest.TestCase):

    def test_CreateFeature(self):
        feat = QgsFeature()
        feat.initAttributes(1)
        feat.setAttribute(0, "text")
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))
        myId = feat.id()
        myExpectedId = 0
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedId, myId)
        assert myId == myExpectedId, myMessage

    def test_ValidFeature(self):
        myPath = os.path.join(unitTestDataPath(), 'points.shp')
        myLayer = QgsVectorLayer(myPath, 'Points', 'ogr')
        provider = myLayer.dataProvider()
        fit = provider.getFeatures()
        feat = QgsFeature()
        fit.nextFeature(feat)
        fit.close()
        myValidValue = feat.isValid()
        myMessage = '\nExpected: %s\nGot: %s' % ("True", myValidValue)
        assert myValidValue, myMessage

    def test_Validity(self):
        f = QgsFeature()
        self.assertFalse(f.isValid())
        f.setGeometry(QgsGeometry())
        self.assertTrue(f.isValid())
        f.setValid(False)
        self.assertFalse(f.isValid())
        fields = QgsFields()
        field1 = QgsField('my_field')
        fields.append(field1)
        field2 = QgsField('my_field2')
        fields.append(field2)
        f.setFields(fields)
        f.setAttribute(0, 0)
        self.assertTrue(f.isValid())
        f.setValid(False)
        self.assertFalse(f.isValid())
        f.setId(27)
        self.assertTrue(f.isValid())

    def test_Attributes(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        provider = myLayer.dataProvider()
        fit = provider.getFeatures()
        feat = QgsFeature()
        fit.nextFeature(feat)
        fit.close()
        myAttributes = feat.attributes()
        myExpectedAttributes = ["Highway", 1]

        # Only for printing purposes
        myExpectedAttributes = ["Highway", 1]
        myMessage = '\nExpected: %s\nGot: %s' % (
            myExpectedAttributes,
            myAttributes
        )

        assert myAttributes == myExpectedAttributes, myMessage

    def test_SetAttributes(self):
        feat = QgsFeature()
        feat.initAttributes(1)
        feat.setAttributes([0])
        feat.setAttributes([NULL])
        assert [NULL] == feat.attributes()

    def test_setAttribute(self):
        feat = QgsFeature()
        feat.initAttributes(1)
        with self.assertRaises(KeyError):
            feat.setAttribute(-1, 5)
        with self.assertRaises(KeyError):
            feat.setAttribute(10, 5)
        self.assertTrue(feat.setAttribute(0, 5))

    def test_DeleteAttribute(self):
        feat = QgsFeature()
        feat.initAttributes(3)
        feat[0] = "text1"
        feat[1] = "text2"
        feat[2] = "text3"
        feat.deleteAttribute(1)
        myAttrs = [feat[0], feat[1]]
        myExpectedAttrs = ["text1", "text3"]
        myMessage = '\nExpected: %s\nGot: %s' % (str(myExpectedAttrs), str(myAttrs))
        assert myAttrs == myExpectedAttrs, myMessage

    def test_DeleteAttributeByName(self):
        fields = QgsFields()
        field1 = QgsField('my_field')
        fields.append(field1)
        field2 = QgsField('my_field2')
        fields.append(field2)

        feat = QgsFeature(fields)
        feat.initAttributes(2)
        feat[0] = "text1"
        feat[1] = "text2"
        with self.assertRaises(KeyError):
            feat.deleteAttribute('not present')
        self.assertTrue(feat.deleteAttribute('my_field'))
        self.assertEqual(feat.attributes(), ['text2'])

    def test_SetGeometry(self):
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))
        myGeometry = feat.geometry()
        myExpectedGeometry = "!None"
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedGeometry, myGeometry)
        assert myGeometry is not None, myMessage


if __name__ == '__main__':
    unittest.main()
