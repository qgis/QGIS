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
import sys
import os
from utilities import unitTestDataPath, getQgisTestApp
from PyQt4.QtCore import QUrl, QString, qDebug
from PyQt4.QtXml import QDomDocument
from qgis.core import (QgsComposition,
                       QgsComposerHtml,
                       QgsComposerFrame,
                       QgsComposerMultiFrame)

from qgscompositionchecker import QgsCompositionChecker
# support python < 2.7 via unittest2
# needed for expected failure decorator
if sys.version_info[0:2] < (2,7):
    try:
        from unittest2 import TestCase, expectedFailure
    except ImportError:
        print "You need to install unittest2 to run the salt tests"
        sys.exit(1)
else:
    from unittest import TestCase, expectedFailure

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerHtml(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        self.mComposition = QgsComposition(None)
        self.mComposition.setPaperSize(297, 210) #A4 landscape

    def tearDown(self):
        """Run after each test."""
        print "Tear down"

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

    @expectedFailure
    def XtestTable(self):
        """Test we can render a html table in a single frame."""
        composerHtml = QgsComposerHtml(self.mComposition, False)
        htmlFrame = QgsComposerFrame(self.mComposition,
                                     composerHtml, 0, 0, 100, 200)
        htmlFrame.setFrameEnabled(True)
        composerHtml.addFrame(htmlFrame)
        composerHtml.setUrl(self.htmlUrl())
        checker = QgsCompositionChecker()
        myResult, myMessage = checker.testComposition(
            "Composer html table",
            self.mComposition,
            self.controlImagePath("composerhtml_table.png"))
        qDebug(myMessage)
        assert myResult, myMessage

    @expectedFailure
    def XtestTableMultiFrame(self):
        """Test we can render to multiframes."""
        composerHtml = QgsComposerHtml(self.mComposition, False)
        htmlFrame = QgsComposerFrame(self.mComposition, composerHtml,
                                     10, 10, 100, 50)
        composerHtml.addFrame(htmlFrame)
        composerHtml.setResizeMode(
            QgsComposerMultiFrame.RepeatUntilFinished)
        composerHtml.setUrl(self.htmlUrl())
        composerHtml.frame(0).setFrameEnabled(True)

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

    def testComposerHtmlAccessor(self):
        """Test that we can retrieve the ComposerHtml instance given an item.
        """
        myComposition = QgsComposition(CANVAS.mapRenderer())
        mySubstitutionMap = {'replace-me': 'Foo bar'}
        myFile = os.path.join(TEST_DATA_DIR, 'template.qpt')
        myTemplateFile = file(myFile, 'rt')
        myTemplateContent = myTemplateFile.read()
        myTemplateFile.close()
        myDocument = QDomDocument()
        myDocument.setContent(myTemplateContent)
        myComposition.loadFromTemplate(myDocument, mySubstitutionMap)
        myItem = myComposition.getComposerItemById('html-test')
        myComposerHtml = myComposition.getComposerHtmlByItem(myItem)
        myMessage = 'Could not retrieve the composer html given an item'
        assert myComposerHtml is not None, myMessage

if __name__ == '__main__':
    unittest.main()
