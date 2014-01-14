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
import qgis

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *

class QgsCompositionChecker(QgsRenderChecker):
    def __init__(self,  mTestName, mComposition ):
        self.mExpectedImageFile = ""
        self.mComposition = mComposition
        self.mTestName = mTestName
        super(QgsCompositionChecker, self).__init__()

    def testComposition(self, page=0, pixelDiff=0 ):
        if ( self.mComposition == None):
            myMessage = "Composition not valid"
            return False, myMessage

        #load expected image
        self.setControlName("expected_"+self.mTestName);
        expectedImage = QImage( self.expectedImageFile() )

         #get width/height, create image and render the composition to it
        width = expectedImage.width();
        height = expectedImage.height();
        outputImage = QImage( QSize( width, height ), QImage.Format_ARGB32 )

        self.mComposition.setPlotStyle( QgsComposition.Print )
        outputImage.setDotsPerMeterX( expectedImage.dotsPerMeterX() )
        outputImage.setDotsPerMeterY( expectedImage.dotsPerMeterX() )
        outputImage.fill( 0 )
        p = QPainter( outputImage )
        self.mComposition.renderPage( p, page )
        p.end()

        renderedFilePath = QDir.tempPath() + QDir.separator() + QFileInfo(self.mTestName).baseName() + "_rendered.png"
        outputImage.save( renderedFilePath, "PNG" )

        diffFilePath = QDir.tempPath() + QDir.separator() + QFileInfo(self.mTestName).baseName() + "_result_diff.png"
        testResult = self.compareImages( self.mTestName, pixelDiff, renderedFilePath )

        myDashMessage = (('<DartMeasurementFile name="Rendered Image '
                         '%s" type="image/png">'
                         '%s</DartMeasurementFile>'
                         '<DartMeasurementFile name="Expected Image '
                         '%s" type="image/png">'
                         '%s</DartMeasurementFile>\n'
                         '<DartMeasurementFile name="Difference Image '
                         '%s" type="image/png">'
                         '%s</DartMeasurementFile>') %
                         (self.mTestName, renderedFilePath, self.mTestName,
                          self.expectedImageFile(), self.mTestName, diffFilePath )
                         )
        qDebug( myDashMessage )
        if not testResult:
            myMessage = ('Expected: %s\nGot: %s\nDifference: %s\n' %
                         (self.expectedImageFile(), renderedFilePath, diffFilePath))
        else:
            myMessage = 'Control and test images matched.'

        return testResult, myMessage

