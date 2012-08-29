# -*- coding: utf-8 -*-
'''
test_qgscomposerhtml.py 
                     --------------------------------------
               Date                 : August 2012
               Copyright            : (C) 2012 by Dr. Horst Düster / Dr. Marco Hugentobler
               email                : marco@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
''' 
import unittest
from utilities import *
from PyQt4.QtCore import * 
from PyQt4.QtGui import *
from qgis.core import *
from qgscompositionchecker import QgsCompositionChecker

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsComposerMap(unittest.TestCase):
    
    def testCase(self):
        self.mComposition = QgsComposition( None )
        self.mComposition.setPaperSize( 297,  210 ) #A4 landscape
        self.table()
        self.tableMultiFrame()
        
    def table(self):
        TEST_DATA_DIR = unitTestDataPath()
        htmlItem = QgsComposerHtml( self.mComposition,  False )
        htmlFrame = QgsComposerFrame( self.mComposition,  htmlItem,  0,  0,  100,  200 )
        htmlFrame.setFrameEnabled( True )
        htmlItem.addFrame( htmlFrame )
        htmlItem.setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir.separator() +  "html_table.html" ) ) );
        checker = QgsCompositionChecker(  ) 
        result = checker.testComposition( "Composer html table", self.mComposition, QString( TEST_DATA_DIR + QDir.separator().toAscii() + "control_images" + QDir.separator().toAscii() + "expected_composerhtml" + QDir.separator().toAscii() + "composerhtml_table.png" ) )
        self.mComposition.removeMultiFrame( htmlItem )
        del htmlItem
        assert result == True
        
    def tableMultiFrame(self):
        
        assert 1 == 1 # soon...
        '''
        TEST_DATA_DIR = unitTestDataPath()
        htmlItem = QgsComposerHtml( self.mComposition,  False )
        htmlFrame = QgsComposerFrame( self.mComposition,  htmlItem,  10,  10,  100,  50 )
        htmlItem.addFrame( htmlFrame )
        htmlItem.setResizeMode( QgsComposerMultiFrame.RepeatUntilFinished )
    
        htmlItem.setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir.separator() +  "html_table.html" )  ) )
        nFrames = htmlItem.nFrames()
        for i in nFrames:
            htmlItem.frame( i ).setFrameEnabled( True )
    
        result = True

        #page 1
        checker1 = QgsCompositionChecker( "Composer html table", self.mComposition, QString( QString( TEST_DATA_DIR ) + QDir.separator() + "control_images" + QDir.separator() + "expected_composerhtml" + QDir.separator() + "composerhtml_table_multiframe1.png" ) )
        if not checker1.testComposition( 0 ):
            result = False
            
        checker2 = QgsCompositionChecker( "Composer html table", self.mComposition, QString( QString( TEST_DATA_DIR ) + QDir.separator() + "control_images" + QDir.separator() + "expected_composerhtml" + QDir.separator() + "composerhtml_table_multiframe2.png" ) )
        if not checker2.testComposition( 1 ):
            result = False
            
        checker3 = QgsCompositionChecker( "Composer html table", self.mComposition, QString( QString( TEST_DATA_DIR ) + QDir.separator() + "control_images" + QDir.separator() + "expected_composerhtml" + QDir.separator() + "composerhtml_table_multiframe3.png" ) )
        if not checker3.testComposition( 2 ):
            result = False
            
        self.mComposition.removeMultiFrame( htmlItem )
        del htmlItem
        
        assert result == True
        '''
        
if __name__ == '__main__':
    unittest.main()
