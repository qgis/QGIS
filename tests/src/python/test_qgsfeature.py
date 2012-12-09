# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFeature.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Germán Carrillo'
__date__ = '06/10/2012'
__copyright__ = 'Copyright 2012, The Quantum GIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from PyQt4.QtCore import QVariant
from qgis.core import QgsFeature, QgsGeometry, QgsPoint, QgsVectorLayer
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest,
                       #expectedFailure
                       )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsFeature(TestCase):

    def test_CreateFeature(self):
        feat = QgsFeature()
        feat.addAttribute(1, "text")
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(123,456)))
        myId = feat.id()
        myExpectedId = 0
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedId, myId)
        assert myId == myExpectedId, myMessage

    def test_ValidFeature(self):
        myPath = os.path.join(unitTestDataPath(), 'points.shp')
        myLayer = QgsVectorLayer(myPath, 'Points', 'ogr')
        provider = myLayer.dataProvider()
        allAttrs = provider.attributeIndexes()
        provider.select(allAttrs)

        feat = QgsFeature()
        provider.nextFeature(feat)
        myValidValue = feat.isValid()
        myMessage = '\nExpected: %s\nGot: %s' % ("True", myValidValue)
        assert myValidValue == True, myMessage

    def test_TypeName(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        provider = myLayer.dataProvider()
        allAttrs = provider.attributeIndexes()
        provider.select(allAttrs)

        feat = QgsFeature()
        provider.nextFeature(feat)
        myTypeName = feat.typeName()
        myExpectedTypeName = "lines"
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedTypeName, myTypeName)
        assert myTypeName == myExpectedTypeName, myMessage

    def test_AttributeMap(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        provider = myLayer.dataProvider()
        allAttrs = provider.attributeIndexes()
        provider.select(allAttrs)
        feat = QgsFeature()
        provider.nextFeature(feat)

        myAttributeMap = feat.attributeMap()
        myExpectedAttributeMap = {0: QVariant("Highway"), 1: QVariant(1)}

        # Only for printing purposes
        myAttributeDict = {
            0:str(myAttributeMap[0].toString()),
            1:int(myAttributeMap[1].toString())}
        myExpectedAttributeDict = {0: "Highway", 1: 1}
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedAttributeDict,
            myAttributeDict)

        assert myAttributeMap == myExpectedAttributeMap, myMessage

    def test_AddAttribute(self):
        feat = QgsFeature()
        feat.addAttribute(1, "text")
        myCount = len(feat.attributeMap())
        myExpectedCount = 1
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedCount, myCount)
        assert myCount == myExpectedCount, myMessage

    def test_ChangeAttribute(self):
        feat = QgsFeature()
        feat.addAttribute(1, "text")
        feat.changeAttribute(1, "changed")
        myChangedAttribute = feat.attributeMap()[1].toString()
        myExpectedAttribute = "changed"
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedAttribute,
            myChangedAttribute)
        assert myChangedAttribute == myExpectedAttribute, myMessage

    def test_DeleteAttribute(self):
        feat = QgsFeature()
        feat.addAttribute(1, "text")
        feat.deleteAttribute(1)
        myCount = len(feat.attributeMap())
        myExpectedCount = 0
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedCount, myCount)
        assert myCount == myExpectedCount, myMessage

    def test_SetGeometry(self):
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(123,456)))
        myGeometry = feat.geometry()
        myExpectedGeometry = "!None"
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedGeometry, myGeometry)
        assert myGeometry != None, myMessage

if __name__ == '__main__':
    unittest.main()

