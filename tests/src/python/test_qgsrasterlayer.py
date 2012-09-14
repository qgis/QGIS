import os
import unittest

from PyQt4.QtCore import QFileInfo, QString, QStringList
from PyQt4 import QtGui

from qgis.core import (QgsRasterLayer, 
                       QgsColorRampShader,
                       QgsContrastEnhancement,
                       QgsMapLayerRegistry,
                       QgsMapRenderer,
                       QgsPoint,
                       QgsRasterShader,
                       QgsRasterTransparency,
                       QgsRenderChecker,
                       QgsSingleBandGrayRenderer,
                       QgsSingleBandPseudoColorRenderer)

# Convenience instances in case you may need them
# not used in this test
from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsRasterLayer(unittest.TestCase):

    def testIdentify(self):
        myPath = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'testdata', 'landsat.tif'))
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        myMessage = 'Raster not loaded: %s' % myPath
        assert myRasterLayer.isValid(), myMessage
        myPoint = QgsPoint(786690, 3345803)
        #print 'Extents: %s' % myRasterLayer.extent().toString()
        myResult, myRasterValues = myRasterLayer.identify(myPoint)
        assert myResult
        # Get the name of the first band
        myBandName = myRasterValues.keys()[0] 
        myExpectedName = QString('Band 1')
        myMessage = 'Expected "%s" got "%s" for first raster band name' % (
                    myExpectedName, myBandName)
        assert myExpectedName == myBandName, myMessage

        # Convert each band value to a list of ints then to a string

        myValues = myRasterValues.values()
        myIntValues = []
        for myValue in myValues:
          myIntValues.append(int(str(myValue)))
        myValues = str(myIntValues)
        myExpectedValues = '[127, 141, 112, 72, 86, 126, 156, 211, 170]'
        myMessage = 'Expected: %s\nGot: %s' % (myValues, myExpectedValues)
        self.assertEquals(myValues, myExpectedValues, myMessage)

    def testTransparency(self):
        myPath = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'testdata', 'raster', 'band1_float32_noct_epsg4326.tif'))
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        myMessage = 'Raster not loaded: %s' % myPath
        assert myRasterLayer.isValid(), myMessage

        renderer = QgsSingleBandGrayRenderer( myRasterLayer.dataProvider(), 1 );
        myRasterLayer.setRenderer( renderer );
        myRasterLayer.setContrastEnhancementAlgorithm( QgsContrastEnhancement.StretchToMinimumMaximum, QgsRasterLayer.ContrastEnhancementMinMax );

        myContrastEnhancement = myRasterLayer.renderer().contrastEnhancement()
        #print "myContrastEnhancement.minimumValue = %.17g" % myContrastEnhancement.minimumValue()
        #print "myContrastEnhancement.maximumValue = %.17g" % myContrastEnhancement.maximumValue()

        # Unfortunately the minimum/maximum values calculated in C++ and Python
        # are slightely different (e.g. 3.3999999521443642e+38 x 3.3999999521444001e+38)
        # It is not clear where the precision is lost. We set the same values as C++.
        myContrastEnhancement.setMinimumValue( -3.3319999287625854e+38 )
        myContrastEnhancement.setMaximumValue( 3.3999999521443642e+38 )
        #myType = myRasterLayer.dataProvider().dataType( 1 );
        #myEnhancement = QgsContrastEnhancement( myType );
        


        myTransparentSingleValuePixelList = []
        rasterTransparency = QgsRasterTransparency()

        myTransparentPixel1 = QgsRasterTransparency.TransparentSingleValuePixel()
        myTransparentPixel1.min = -2.5840000772112106e+38
        myTransparentPixel1.max = -1.0879999684602689e+38
        myTransparentPixel1.percentTransparent = 50
        myTransparentSingleValuePixelList.append( myTransparentPixel1 )

        myTransparentPixel2 = QgsRasterTransparency.TransparentSingleValuePixel()
        myTransparentPixel2.min = 1.359999960575336e+37
        myTransparentPixel2.max = 9.520000231087593e+37
        myTransparentPixel2.percentTransparent = 70
        myTransparentSingleValuePixelList.append( myTransparentPixel2 )

        rasterTransparency.setTransparentSingleValuePixelList( myTransparentSingleValuePixelList )

        rasterRenderer = myRasterLayer.renderer()
        assert rasterRenderer

        rasterRenderer.setRasterTransparency( rasterTransparency )

        QgsMapLayerRegistry.instance().addMapLayers( [ myRasterLayer, ] )

        myMapRenderer = QgsMapRenderer()

        myLayers = QStringList()
        myLayers.append( myRasterLayer.id() )
        myMapRenderer.setLayerSet( myLayers )
        myMapRenderer.setExtent( myRasterLayer.extent() ) 
        
        myChecker = QgsRenderChecker()
        myChecker.setControlName( "expected_raster_transparency" )
        myChecker.setMapRenderer( myMapRenderer )

        myResultFlag = myChecker.runTest( "raster_transparency_python" );
        assert myResultFlag, "Raster transparency rendering test failed"

    def testShaderCrash(self):
        """Check if we assign a shader and then reassign it no crash occurs."""
        myPath = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'testdata', 'raster', 'band1_float32_noct_epsg4326.tif'))
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        myMessage = 'Raster not loaded: %s' % myPath
        assert myRasterLayer.isValid(), myMessage

	myRasterShader = QgsRasterShader()
	myColorRampShader = QgsColorRampShader()
	myColorRampShader.setColorRampType(QgsColorRampShader.INTERPOLATED)
	myItems = []
	myItem = QgsColorRampShader.ColorRampItem(10, QtGui.QColor('#ffff00'), 'foo')
	myItems.append(myItem)
	myItem = QgsColorRampShader.ColorRampItem(100, QtGui.QColor('#ff00ff'), 'bar')
	myItems.append(myItem)
	myItem = QgsColorRampShader.ColorRampItem(1000, QtGui.QColor('#00ff00'), 'kazam')
	myItems.append(myItem)
	myColorRampShader.setColorRampItemList(myItems)
	myRasterShader.setRasterShaderFunction(myColorRampShader)
	myPseudoRenderer = QgsSingleBandPseudoColorRenderer(myRasterLayer.dataProvider(), 1,  myRasterShader)
	myRasterLayer.setRenderer(myPseudoRenderer)

        return
	######## works first time #############

	myRasterShader = QgsRasterShader()
	myColorRampShader = QgsColorRampShader()
	myColorRampShader.setColorRampType(QgsColorRampShader.INTERPOLATED)
	myItems = []
	myItem = QgsColorRampShader.ColorRampItem(10, QtGui.QColor('#ffff00'), 'foo')
	myItems.append(myItem)
	myItem = QgsColorRampShader.ColorRampItem(100, QtGui.QColor('#ff00ff'), 'bar')
	myItems.append(myItem)
	myItem = QgsColorRampShader.ColorRampItem(1000, QtGui.QColor('#00ff00'), 'kazam')
	myItems.append(myItem)
	myColorRampShader.setColorRampItemList(myItems)
	myRasterShader.setRasterShaderFunction(myColorRampShader)
	######## crash on next line ##################
	myPseudoRenderer = QgsSingleBandPseudoColorRenderer(myRasterLayer.dataProvider(), 1,  myRasterShader)
	myRasterLayer.setRenderer(myPseudoRenderer)

if __name__ == '__main__':
    unittest.main()
