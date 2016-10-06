# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbollayer.py
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

from qgis.core import (QgsCentroidFillSymbolLayer,
                       QgsEllipseSymbolLayer,
                       QgsFillSymbolLayer,
                       QgsFontMarkerSymbolLayer,
                       QgsFilledMarkerSymbolLayer,
                       QgsGradientFillSymbolLayer,
                       QgsImageFillSymbolLayer,
                       QgsLinePatternFillSymbolLayer,
                       QgsLineSymbolLayer,
                       QgsMarkerLineSymbolLayer,
                       QgsMarkerSymbolLayer,
                       QgsPointPatternFillSymbolLayer,
                       QgsSimpleFillSymbolLayer,
                       QgsSimpleLineSymbolLayer,
                       QgsSimpleMarkerSymbolLayer,
                       QgsSimpleMarkerSymbolLayerBase,
                       QgsSVGFillSymbolLayer,
                       QgsSvgMarkerSymbolLayer,
                       QgsSymbolLayer,
                       QgsVectorFieldSymbolLayer,
                       QgsRasterFillSymbolLayer,
                       QgsShapeburstFillSymbolLayer,
                       QgsArrowSymbolLayer,
                       QgsSymbol,
                       QgsUnitTypes
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsSymbolLayer(unittest.TestCase):

    """
     This class test the sip binding for QgsSymbolLayer descendants
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
        mType = type(QgsSymbolLayer)
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsGradientFillSymbolLayer)
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
            mType = type(QgsGradientFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsShapeburstFillSymbolLayer)
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
            mType = type(QgsCentroidFillSymbolLayer)
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
            mType = type(QgsSimpleFillSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsLineSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsMarkerLineSymbolLayer)
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
            mType = type(QgsSimpleLineSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsMarkerSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsEllipseSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsFontMarkerSymbolLayer)
        except:
            mType = None
        mExpectedType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpectedType, mType)
        assert mExpectedType == mType, mMessage

        try:
            mType = type(QgsSimpleMarkerSymbolLayer)
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
            mType = type(QgsSvgMarkerSymbolLayer)
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

    def testQgsSimpleFillSymbolLayer(self):
        """Create a new style from a .sld file and match test.
        """
        mTestName = 'QgsSimpleFillSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSimpleFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsSimpleFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.SolidPattern
        mValue = mSymbolLayer.brushStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#ffaa7f'
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

    def testQgsGradientFillSymbolLayer(self):
        """Test setting and getting QgsGradientFillSymbolLayer properties.
        """
        mGradientLayer = QgsGradientFillSymbolLayer()

        mExpectedValue = type(QgsGradientFillSymbolLayer())
        mValue = type(mGradientLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayer.Radial
        mGradientLayer.setGradientType(mExpectedValue)
        mValue = mGradientLayer.gradientType()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayer.ColorRamp
        mGradientLayer.setGradientColorType(mExpectedValue)
        mValue = mGradientLayer.gradientColorType()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QColor('#55aaff')
        mGradientLayer.setColor2(mExpectedValue)
        mValue = mGradientLayer.color2()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayer.Viewport
        mGradientLayer.setCoordinateMode(mExpectedValue)
        mValue = mGradientLayer.coordinateMode()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayer.Reflect
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

        mExpectedValue = QgsUnitTypes.RenderMapUnits
        mGradientLayer.setOffsetUnit(mExpectedValue)
        mValue = mGradientLayer.offsetUnit()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsCentroidFillSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsCentroidFillSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsCentroidFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsCentroidFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsSimpleMarkerSymbolLayerBase.Star
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).shape()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#55aaff'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#00ff00'
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

        mExpectedValue = '#ff55ff'
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

        mExpectedValue = QgsSimpleMarkerSymbolLayerBase.Triangle
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).shape()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#ffaa00'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#ff007f'
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

    def testQgsMarkerLineSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsMarkerLineSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsMarkerLineSymbolLayer.createFromSld(
            mDoc.elementsByTagName('LineSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsMarkerLineSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsMarkerLineSymbolLayer.CentralPoint
        mValue = mSymbolLayer.placement()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsSimpleMarkerSymbolLayerBase.Circle
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).shape()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#000000'
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#ff0000'
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

    def testQgsSimpleLineSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsSimpleLineSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSimpleLineSymbolLayer.createFromSld(
            mDoc.elementsByTagName('LineSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsSimpleLineSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#aa007f'
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

    def testQgsEllipseSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsEllipseSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsEllipseSymbolLayer.createFromSld(
            mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsEllipseSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 'circle'
        mValue = mSymbolLayer.symbolName()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#ffff7f'
        mValue = mSymbolLayer.fillColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = '#aaaaff'
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

    def testQgsFontMarkerSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsFontMarkerSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsFontMarkerSymbolLayer.createFromSld(
            mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsFontMarkerSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 'Arial'
        mValue = mSymbolLayer.fontFamily()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "M"
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

    def testQgsSvgMarkerSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = 'QgsSvgMarkerSymbolLayer'
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSvgMarkerSymbolLayer.createFromSld(mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type(QgsSvgMarkerSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue, mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 'skull.svg'
        mValue = os.path.basename(mSymbolLayer.path())
        print(("VALUE", mSymbolLayer.path()))
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
