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
        feat.initAttributes(1)
        feat.setAttribute(0, "text")
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(123,456)))
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
        myValidValue = feat.isValid()
        myMessage = '\nExpected: %s\nGot: %s' % ("True", myValidValue)
        assert myValidValue == True, myMessage

    def test_Attributes(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        provider = myLayer.dataProvider()
        fit = provider.getFeatures()
        feat = QgsFeature()
        fit.nextFeature(feat)
        myAttributes = feat.attributes()
        myExpectedAttributes = [ QVariant("Highway"), QVariant(1) ]

        # Only for printing purposes
        myAttributeDict = [
            str(myAttributes[0].toString()),
            int(myAttributes[1].toString()) ]
        myExpectedAttributes = [ "Highway",  1 ]
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedAttributes,
            myAttributes)

        assert myAttributes == myExpectedAttributes, myMessage

    def test_DeleteAttribute(self):
        feat = QgsFeature()
        feat.initAttributes(3)
        feat[0] = QVariant("text1")
        feat[1] = QVariant("text2")
        feat[2] = QVariant("text3")
        feat.deleteAttribute(1)
        myAttrs = [ str(feat[0].toString()), str(feat[1].toString()) ]
        myExpectedAttrs = [ "text1", "text3" ]
        myMessage = '\nExpected: %s\nGot: %s' % (str(myExpectedAttrs), str(myAttrs))
        assert myAttrs == myExpectedAttrs, myMessage

    def test_SetGeometry(self):
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(123,456)))
        myGeometry = feat.geometry()
        myExpectedGeometry = "!None"
        myMessage = '\nExpected: %s\nGot: %s' % (myExpectedGeometry, myGeometry)
        assert myGeometry != None, myMessage

if __name__ == '__main__':
    unittest.main()
