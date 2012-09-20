# -*- coding: utf-8 -*-
'''
test_qgscomposerhtml.py
                     --------------------------------------
               Date                 : August 2012
               Copyright            : (C) 2012 by Dr. Horst Düster /
                                                  Dr. Marco Hugentobler
                                                  Tim Sutton
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
import os
from utilities import unitTestDataPath, getQgisTestApp
from PyQt4.QtCore import QUrl, QString, qDebug
from qgis.core import (QgsComposition,
                       QgsComposerHtml,
                       QgsComposerFrame,
                       QgsComposerMultiFrame)

from qgscompositionchecker import QgsCompositionChecker

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()

class TestQgsComposerMap(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        self.mComposition = QgsComposition(None)
        self.mComposition.setPaperSize(297, 210) #A4 landscape
        self.htmlItem = QgsComposerHtml(self.mComposition, False)

    def tearDown(self):
        """Run after each test."""
        print "Tear down"
        if self.htmlItem:
            self.mComposition.removeMultiFrame(self.htmlItem)
            del self.htmlItem

    def controlImagePath(self, theImageName):
        """Helper to get the path to a control image."""
        myPath = os.path.join(TEST_DATA_DIR,
                     "control_images",
                     "expected_composerhtml",
                     theImageName)
        assert os.path.exists(myPath)
        return myPath

    def htmlUrl(self):
        """Helper to get the url of the html doc."""
        myPath = os.path.join(TEST_DATA_DIR, "html_table.html")
        myUrl = QUrl(QString("file:///%1").arg(myPath))
        return myUrl

    def testTable(self):
        """Test we can render a html table in a single frame."""
        htmlFrame = QgsComposerFrame(self.mComposition,
                                     self.htmlItem, 0, 0, 100, 200)
        htmlFrame.setFrameEnabled(True)
        self.htmlItem.addFrame(htmlFrame)
        self.htmlItem.setUrl(self.htmlUrl())
        checker = QgsCompositionChecker()
        myResult, myMessage = checker.testComposition(
            "Composer html table",
            self.mComposition,
            self.controlImagePath("composerhtml_table.png"))
        qDebug(myMessage)
        assert myResult, myMessage

    def testTableMultiFrame(self):
        """Test we can render to multiframes."""
        htmlFrame = QgsComposerFrame(self.mComposition, self.htmlItem,
                                     10, 10, 100, 50)
        self.htmlItem.addFrame(htmlFrame)
        self.htmlItem.setResizeMode(QgsComposerMultiFrame.RepeatUntilFinished)
        self.htmlItem.setUrl(self.htmlUrl())
        self.htmlItem.frame(0).setFrameEnabled(True)

        result = True

        myPage = 0
        checker1 = QgsCompositionChecker()
        myControlImage = self.controlImagePath(
            "composerhtml_table_multiframe1.png")
        print "Checking page 1"
        myResult, myMessage = checker1.testComposition("Composer html table",
                                        self.mComposition,
                                        myControlImage,
                                        myPage)
        assert myResult, myMessage

        myPage = 1
        checker2 = QgsCompositionChecker()
        myControlImage = self.controlImagePath(
            "composerhtml_table_multiframe2.png")
        print "Checking page 2"
        myResult, myMessage = checker2.testComposition("Composer html table",
                                        self.mComposition,
                                        myControlImage,
                                        myPage)
        assert myResult, myMessage

        myPage = 2
        checker3 = QgsCompositionChecker()
        myControlImage = self.controlImagePath(
            "composerhtml_table_multiframe3.png")
        myResult, myMessage = checker3.testComposition("Composer html table",
                                        self.mComposition,
                                        myControlImage,
                                        myPage)
        print "Checking page 3"
        assert myResult, myMessage

if __name__ == '__main__':
    unittest.main()
