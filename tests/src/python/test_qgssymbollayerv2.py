# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbollayerv2.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Massimo Endrighi
    Email                : massimo dot endrighi at geopartner dot it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Massimo Endrighi'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Massimo Endrighi'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import pyqtWrapperType, Qt, QDir, QFile, QIODevice, QPointF
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QColor

from qgis.core import (QgsCentroidFillSymbolLayerV2,
                       QgsEllipseSymbolLayerV2,
                       QgsFillSymbolLayerV2,
                       QgsFontMarkerSymbolLayerV2,
                       QgsFilledMarkerSymbolLayer,
                       QgsGradientFillSymbolLayerV2,
                       QgsImageFillSymbolLayer,
                       QgsLinePatternFillSymbolLayer,
                       QgsLineSymbolLayerV2,
                       QgsMarkerLineSymbolLayerV2,
                       QgsMarkerSymbolLayerV2,
                       QgsPointPatternFillSymbolLayer,
                       QgsSimpleFillSymbolLayerV2,
                       QgsSimpleLineSymbolLayerV2,
                       QgsSimpleMarkerSymbolLayerV2,
                       QgsSVGFillSymbolLayer,
                       QgsSvgMarkerSymbolLayerV2,
                       QgsSymbolLayerV2,
                       QgsVectorFieldSymbolLayer,
                       QgsRasterFillSymbolLayer,
                       QgsShapeburstFillSymbolLayerV2,
                       QgsArrowSymbolLayer,
                       QgsSymbolV2,
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsSymbolLayerV2(unittest.TestCase):

    """
     This class test the sip binding for QgsSymbolLayerV2 descendants
     Every class is tested using the createFromSld implementation
     An exception is done for:
     - QgsLinePatternFillSymbolLayer where createFromSld implementation
         returns NULL
     - QgsPointPatternFillSymbolLayer where createFromSld implementation
         returns NULL
     - QgsVectorFieldSymbolLayer where createFromSld implementation
         returns NULL
     """

    def testBinding(self):
        """Test python bindings existence."""
        mType = type(QgsSymbolLayerV2)
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsFillSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsGradientFillSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsLinePatternFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsPointPatternFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsImageFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsPointPatternFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsGradientFillSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsShapeburstFillSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsSVGFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsCentroidFillSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsRasterFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsSimpleFillSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsLineSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsMarkerLineSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsArrowSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsSimpleLineSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsMarkerSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsEllipseSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsFontMarkerSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsSimpleMarkerSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsFilledMarkerSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsSvgMarkerSymbolLayerV2)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsVectorFieldSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

    def testQgsSimpleFillSymbolLayerV2(self):
        """Create a new style from a .sld file and match test.
        """
        mTestName = 'QgsSimpleFillSymbolLayerV2'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSimpleFillSymbolLayerV2.createFromSld(
            mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsSimpleFillSymbolLayerV2())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.SolidPattern
        mValue = mSymbolLayer.brushStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#ffaa7f'
        mValue = mSymbolLayer.borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.DotLine
        mValue = mSymbolLayer.borderStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 0.26
        mValue = mSymbolLayer.borderWidth()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsGradientFillSymbolLayerV2(self):
        """Test setting and getting QgsGradientFillSymbolLayerV2 properties.
        """
        mGradientLayer = QgsGradientFillSymbolLayerV2()

        mExpectedValue = type(QgsGradientFillSymbolLayerV2())
        mValue = type(mGradientLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayerV2.Radial
        mGradientLayer.setGradientType(mExpectedValue)
        mValue = mGradientLayer.gradientType()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayerV2.ColorRamp
        mGradientLayer.setGradientColorType(mExpectedValue)
        mValue = mGradientLayer.gradientColorType()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QColor('#55aaff')
        mGradientLayer.setColor2(mExpectedValue)
        mValue = mGradientLayer.color2()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayerV2.Viewport
        mGradientLayer.setCoordinateMode(mExpectedValue)
        mValue = mGradientLayer.coordinateMode()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayerV2.Reflect
        mGradientLayer.setGradientSpread(mExpectedValue)
        mValue = mGradientLayer.gradientSpread()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QPointF(0.5, 0.8)
        mGradientLayer.setReferencePoint1(mExpectedValue)
        mValue = mGradientLayer.referencePoint1()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = True
        mGradientLayer.setReferencePoint1IsCentroid(mExpectedValue)
        mValue = mGradientLayer.referencePoint1IsCentroid()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QPointF(0.2, 0.4)
        mGradientLayer.setReferencePoint2(mExpectedValue)
        mValue = mGradientLayer.referencePoint2()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = True
        mGradientLayer.setReferencePoint2IsCentroid(mExpectedValue)
        mValue = mGradientLayer.referencePoint2IsCentroid()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 90
        mGradientLayer.setAngle(mExpectedValue)
        mValue = mGradientLayer.angle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QPointF(10, 20)
        mGradientLayer.setOffset(mExpectedValue)
        mValue = mGradientLayer.offset()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsSymbolV2.MapUnit
        mGradientLayer.setOffsetUnit(mExpectedValue)
        mValue = mGradientLayer.offsetUnit()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsCentroidFillSymbolLayerV2(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsCentroidFillSymbolLayerV2'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsCentroidFillSymbolLayerV2.createFromSld(
            mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsCentroidFillSymbolLayerV2())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'star'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#55aaff'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#00ff00'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testQgsLinePatternFillSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsLinePatternFillSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsLinePatternFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsLinePatternFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#ff55ff'
        mValue = mSymbolLayer.color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 1.5
        mValue = mSymbolLayer.lineWidth()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 4
        mValue = mSymbolLayer.distance()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 57
        mValue = mSymbolLayer.lineAngle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    @unittest.expectedFailure
    def testQgsPointPatternFillSymbolLayerSld(self):
        """
        Create a new style from a .sld file and match test
        """
        # at the moment there is an empty createFromSld implementation
        # that return nulls
        mTestName = 'QgsPointPatternFillSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsPointPatternFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsPointPatternFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'triangle'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#ffaa00'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#ff007f'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 5
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).angle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 3
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).size()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsPointPatternFillSymbolLayer(self):
        """
        Test point pattern fill
        """
        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer = QgsPointPatternFillSymbolLayer.create()

        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testQgsSVGFillSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsSVGFillSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSVGFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsSVGFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 'accommodation_camping.svg'
        mValue = os.path.basename(mSymbolLayer.svgFilePath())
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 6
        mValue = mSymbolLayer.patternWidth()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsMarkerLineSymbolLayerV2(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsMarkerLineSymbolLayerV2'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsMarkerLineSymbolLayerV2.createFromSld(
            mDoc.elementsByTagName('LineSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsMarkerLineSymbolLayerV2())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsMarkerLineSymbolLayerV2.CentralPoint
        mValue = mSymbolLayer.placement()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'circle'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#000000'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#ff0000'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testQgsSimpleLineSymbolLayerV2(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsSimpleLineSymbolLayerV2'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSimpleLineSymbolLayerV2.createFromSld(
            mDoc.elementsByTagName('LineSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsSimpleLineSymbolLayerV2())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#aa007f'
        mValue = mSymbolLayer.color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 1.26
        mValue = mSymbolLayer.width()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.RoundCap
        mValue = mSymbolLayer.penCapStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.MiterJoin
        mValue = mSymbolLayer.penJoinStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = True
        mValue = mSymbolLayer.useCustomDashPattern()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = [5.0, 2.0]
        mValue = mSymbolLayer.customDashVector()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsEllipseSymbolLayerV2(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsEllipseSymbolLayerV2'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsEllipseSymbolLayerV2.createFromSld(
            mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsEllipseSymbolLayerV2())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'circle'
        mValue = mSymbolLayer.symbolName()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#ffff7f'
        mValue = mSymbolLayer.fillColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'#aaaaff'
        mValue = mSymbolLayer.outlineColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 7
        mValue = mSymbolLayer.symbolWidth()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 5
        mValue = mSymbolLayer.symbolHeight()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsFontMarkerSymbolLayerV2(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsFontMarkerSymbolLayerV2'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsFontMarkerSymbolLayerV2.createFromSld(
            mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsFontMarkerSymbolLayerV2())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'Arial'
        mValue = mSymbolLayer.fontFamily()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u"M"
        mValue = mSymbolLayer.character()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 6.23
        mValue = mSymbolLayer.size()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 3
        mValue = mSymbolLayer.angle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsSvgMarkerSymbolLayerV2(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsSvgMarkerSymbolLayerV2'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSvgMarkerSymbolLayerV2.createFromSld(mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsSvgMarkerSymbolLayerV2())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = u'skull.svg'
        mValue = os.path.basename(mSymbolLayer.path())
        print("VALUE", mSymbolLayer.path())
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 12
        mValue = mSymbolLayer.size()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 45
        mValue = mSymbolLayer.angle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsFilledMarkerSymbolLayer(self):
        """
        Test QgsFilledMarkerSymbolLayer
        """
        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer = QgsFilledMarkerSymbolLayer.create()

        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testQgsVectorFieldSymbolLayer(self):
        """
        Test QgsVectorFieldSymbolLayer
        """
        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer = QgsVectorFieldSymbolLayer.create()

        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

if __name__ == '__main__':
    unittest.main()
