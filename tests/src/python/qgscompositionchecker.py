# -*- coding: utf-8 -*-
'''
qgscompositionchecker.py - check rendering of Qgscomposition against an expected image
                     --------------------------------------
               Date                 : 31 Juli 2012
               Copyright            : (C) 2012 by Dr. Horst Düster / Dr. Marco Hugentobler
               email                : horst.duester@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
'''
from PyQt4.QtCore import * 
from PyQt4.QtGui import *
from qgis.core import *

class QgsCompositionChecker:

    def testComposition(self,  mTestName, mComposition, mExpectedImageFile,  page=0 ):
        if ( mComposition == None):
           return false


        #load expected image
        expectedImage = QImage( mExpectedImageFile )
        
         #get width/height, create image and render the composition to it
        width = expectedImage.width();
        height = expectedImage.height();
        outputImage = QImage( QSize( width, height ), QImage.Format_ARGB32 )
        
        mComposition.setPlotStyle( QgsComposition.Print )
        outputImage.setDotsPerMeterX( expectedImage.dotsPerMeterX() )
        outputImage.setDotsPerMeterY( expectedImage.dotsPerMeterX() )
        outputImage.fill( 0 )
        p = QPainter( outputImage )
        mComposition.renderPage( p, page )
        p.end()
        
        renderedFilePath = QDir.tempPath() + QDir.separator() + QFileInfo( mExpectedImageFile ).baseName() + "_rendered_python.png"
        outputImage.save( renderedFilePath, "PNG" )
        
        diffFilePath = QDir.tempPath() + QDir.separator() + QFileInfo( mExpectedImageFile ).baseName() + "_diff_python.png"
        testResult = self.compareImages( expectedImage, outputImage, diffFilePath )
        
        myDashMessage = "<DartMeasurementFile name=\"Rendered Image " + mTestName + "\"" + " type=\"image/png\">" + renderedFilePath + "</DartMeasurementFile>" + "\n" + "<DartMeasurementFile name=\"Expected Image " + mTestName + "\"" + " type=\"image/png\">" + mExpectedImageFile + "</DartMeasurementFile>" + "\n" + "<DartMeasurementFile name=\"Difference Image " + mTestName + "\"" + " type=\"image/png\">" + diffFilePath + "</DartMeasurementFile>"
        qDebug( myDashMessage )
        return testResult

    def compareImages( self,  imgExpected, imgRendered, differenceImagePath ):

          if ( imgExpected.width() != imgRendered.width() or imgExpected.height() != imgRendered.height() ):
            return false
        
          imageWidth = imgExpected.width()
          imageHeight = imgExpected.height()
          mismatchCount = 0
        
          differenceImage = QImage( imageWidth, imageHeight, QImage.Format_ARGB32_Premultiplied )
          differenceImage.fill( qRgb( 152, 219, 249 ) )
        
          pixel1 = QColor().rgb()
          pixel2 = QColor().rgb()
          for i in range( imageHeight ):
            for j in range( imageWidth ):
              pixel1 = imgExpected.pixel( j, i )
              pixel2 = imgRendered.pixel( j, i )
              if ( pixel1 != pixel2 ):
                mismatchCount = mismatchCount + 1
                differenceImage.setPixel( j, i, qRgb( 255, 0, 0 ) )
        
          if  not differenceImagePath.isEmpty():
            differenceImage.save( differenceImagePath, "PNG" )
        
          #allow pixel deviation of 1 percent
          pixelCount = imageWidth * imageHeight;
#          print "MismatchCount: "+str(mismatchCount)
#          print "PixelCount: "+str(pixelCount)
          return (float(mismatchCount) / float(pixelCount) ) < 0.01
        
