# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsApplication.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Dr. Horst Düster, Dr. Marco Hugentobler, Tim Sutton'
__date__ = '20/01/2011'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import unittest
import os
import qgis
from PyQt4.QtCore import QUrl, qDebug
from PyQt4.QtXml import QDomDocument
from qgis.core import (QgsComposition,
                       QgsComposerHtml,
                       QgsComposerFrame,
                       QgsComposerMultiFrame)

from qgscompositionchecker import QgsCompositionChecker

from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       expectedFailure)
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerHtml(TestCase):

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
        myUrl = QUrl("file:///%1").arg(myPath)
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
