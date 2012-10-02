import os
import unittest
from utilities import unitTestDataPath

from qgis.core import *
from PyQt4.QtCore import QString, QChar
from PyQt4.QtCore import pyqtWrapperType
from PyQt4.QtCore import QFile, QIODevice, QDir
from PyQt4.QtCore import Qt
from PyQt4.QtGui import QColor
from PyQt4.QtXml import QDomDocument

# Convenience instances in case you may need them
# not used in this test
from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsSymbolLayerV2(unittest.TestCase):
    '''
    This class test the sip binding for QgsSymbolLayerV2 descendants
    Every class is tested using the createFromSld implementation
    An exception is done for:
    - QgsLinePatternFillSymbolLayer where createFromSld implementation returns NULL
    - QgsPointPatternFillSymbolLayer where createFromSld implementation returns NULL
    - QgsVectorFieldSymbolLayer where createFromSld implementation returns NULL
    '''

    def testBinding(self):
        '''
        test pyhton bindings existance
        '''
        try:
            mType = type (QgsSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsFillSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsImageFillSymbolLayer)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsPointPatternFillSymbolLayer)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsSVGFillSymbolLayer)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsCentroidFillSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsSimpleFillSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsLineSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsLineDecorationSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsMarkerLineSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsSimpleLineSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsMarkerSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsEllipseSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsFontMarkerSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsSimpleMarkerSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsSvgMarkerSymbolLayerV2)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

        try:
            mType = type (QgsVectorFieldSymbolLayer)
        except:
            mType = None
        mExpecteType = pyqtWrapperType
        mMessage = 'Expected "%s" got "%s"' % (mExpecteType, mType)
        assert mExpecteType == mType, mMessage

    def testQgsSimpleFillSymbolLayerV2(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsSimpleFillSymbolLayerV2')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsSimpleFillSymbolLayerV2.createFromSld(mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsSimpleFillSymbolLayerV2())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.SolidPattern
        mValue = mSymbolLayer.brushStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#ffaa7f')
        mValue = mSymbolLayer.borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.DotLine
        mValue = mSymbolLayer.borderStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 0.26
        mValue = mSymbolLayer.borderWidth()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsCentroidFillSymbolLayerV2(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsCentroidFillSymbolLayerV2')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsCentroidFillSymbolLayerV2.createFromSld(mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsCentroidFillSymbolLayerV2())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'regular_star')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#55aaff')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#00ff00')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsLinePatternFillSymbolLayer(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsLinePatternFillSymbolLayer')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsLinePatternFillSymbolLayer.createFromSld(mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsLinePatternFillSymbolLayer())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#ff55ff')
        mValue = mSymbolLayer.color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 1.5
        mValue = mSymbolLayer.lineWidth()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 4
        mValue = mSymbolLayer.distance()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 57
        mValue = mSymbolLayer.lineAngle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsPointPatternFillSymbolLayer(self):
        '''
        Create a new style from a .sld file and match test
        '''
        # at the moment there is an empty createFromSld implementation that return null
        return
        mTestName = QString ('QgsPointPatternFillSymbolLayer')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsPointPatternFillSymbolLayer.createFromSld(mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsPointPatternFillSymbolLayer())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'triangle')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#ffaa00')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#ff007f')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 5
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).angle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 3
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).size()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsSVGFillSymbolLayer(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsSVGFillSymbolLayer')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsSVGFillSymbolLayer.createFromSld(mDoc.elementsByTagName('PolygonSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsSVGFillSymbolLayer())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 'accommodation_camping.svg'
        mValue = os.path.basename (str (mSymbolLayer.svgFilePath()))
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 6
        mValue = mSymbolLayer.patternWidth()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsMarkerLineSymbolLayerV2(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsMarkerLineSymbolLayerV2')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsMarkerLineSymbolLayerV2.createFromSld(mDoc.elementsByTagName('LineSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsMarkerLineSymbolLayerV2())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsMarkerLineSymbolLayerV2.CentralPoint
        mValue = mSymbolLayer.placement()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString (u'circle')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#000000')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).borderColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#ff0000')
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsSimpleLineSymbolLayerV2(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsSimpleLineSymbolLayerV2')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsSimpleLineSymbolLayerV2.createFromSld(mDoc.elementsByTagName('LineSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsSimpleLineSymbolLayerV2())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#aa007f')
        mValue = mSymbolLayer.color().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 1.26
        mValue = mSymbolLayer.width()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.RoundCap
        mValue = mSymbolLayer.penCapStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.MiterJoin
        mValue = mSymbolLayer.penJoinStyle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = True
        mValue = mSymbolLayer.useCustomDashPattern()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = [5.0, 2.0]
        mValue = mSymbolLayer.customDashVector()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsEllipseSymbolLayerV2(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsEllipseSymbolLayerV2')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsEllipseSymbolLayerV2.createFromSld(mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsEllipseSymbolLayerV2())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'circle')
        mValue = mSymbolLayer.symbolName()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#ffff7f')
        mValue = mSymbolLayer.fillColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'#aaaaff')
        mValue = mSymbolLayer.outlineColor().name()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 7
        mValue = mSymbolLayer.symbolWidth()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 5
        mValue = mSymbolLayer.symbolHeight()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsFontMarkerSymbolLayerV2(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsFontMarkerSymbolLayerV2')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsFontMarkerSymbolLayerV2.createFromSld(mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsFontMarkerSymbolLayerV2())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'Arial')
        mValue = mSymbolLayer.fontFamily()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QChar(0x4d)
        mValue = mSymbolLayer.character()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 6.23
        mValue = mSymbolLayer.size()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 3
        mValue = mSymbolLayer.angle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

    def testQgsSvgMarkerSymbolLayerV2(self):
        '''
        Create a new style from a .sld file and match test
        '''
        mTestName = QString ('QgsSvgMarkerSymbolLayerV2')
        mFilePath = QDir.toNativeSeparators (QString ('%1/symbol_layer/%2.sld').arg (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile (mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile,True)
        mFile.close()
        mSymbolLayer = QgsSvgMarkerSymbolLayerV2.createFromSld(mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())

        mExpectedValue = type (QgsSvgMarkerSymbolLayerV2())
        mValue = type (mSymbolLayer)
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QString(u'skull.svg')
        mValue = os.path.basename (str(mSymbolLayer.path()))
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 12
        mValue = mSymbolLayer.size()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 45
        mValue = mSymbolLayer.angle()
        mMessage = 'Expected "%s" got "%s"' % (mExpectedValue,mValue)
        assert mExpectedValue == mValue, mMessage


if __name__ == '__main__':
    unittest.main()
