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

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QUrl, qDebug
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (QgsComposition,
                       QgsComposerHtml,
                       QgsComposerFrame,
                       QgsComposerMultiFrame,
                       QgsMapSettings
                       )

from qgscompositionchecker import QgsCompositionChecker

from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerHtml(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        self.iface = get_iface()
        self.mapSettings = QgsMapSettings()
        self.mComposition = QgsComposition(self.mapSettings)
        self.mComposition.setPaperSize(297, 210)  # A4 landscape

    def tearDown(self):
        """Run after each test."""
        print("Tear down")

    def htmlUrl(self):
        """Helper to get the url of the html doc."""
        myPath = os.path.join(TEST_DATA_DIR, "test_html.html")
        myUrl = QUrl("file:///" + myPath)
        return myUrl

    def testTable(self):
        """Test we can render a html table in a single frame."""
        composerHtml = QgsComposerHtml(self.mComposition, False)
        htmlFrame = QgsComposerFrame(self.mComposition,
                                     composerHtml, 0, 0, 100, 200)
        htmlFrame.setFrameEnabled(True)
        composerHtml.addFrame(htmlFrame)
        composerHtml.setUrl(self.htmlUrl())

        checker = QgsCompositionChecker('composerhtml_table', self.mComposition)
        checker.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker.testComposition()

        qDebug(myMessage)
        self.mComposition.removeMultiFrame(composerHtml)
        composerHtml = None

        assert myTestResult, myMessage

    def testTableMultiFrame(self):
        """Test we can render to multiframes."""
        composerHtml = QgsComposerHtml(self.mComposition, False)
        htmlFrame = QgsComposerFrame(self.mComposition, composerHtml,
                                     10, 10, 100, 50)
        composerHtml.addFrame(htmlFrame)
        composerHtml.setResizeMode(
            QgsComposerMultiFrame.RepeatUntilFinished)
        composerHtml.setUseSmartBreaks(False)
        composerHtml.setUrl(self.htmlUrl())
        composerHtml.frame(0).setFrameEnabled(True)

        print("Checking page 1")
        myPage = 0
        checker1 = QgsCompositionChecker('composerhtml_multiframe1', self.mComposition)
        checker1.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker1.testComposition(myPage)
        assert myTestResult, myMessage

        print("Checking page 2")
        myPage = 1
        checker2 = QgsCompositionChecker('composerhtml_multiframe2', self.mComposition)
        checker2.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker2.testComposition(myPage)
        assert myTestResult, myMessage

        self.mComposition.removeMultiFrame(composerHtml)
        composerHtml = None

        assert myTestResult, myMessage

    def testHtmlSmartBreaks(self):
        """Test rendering to multiframes with smart breaks."""
        composerHtml = QgsComposerHtml(self.mComposition, False)
        htmlFrame = QgsComposerFrame(self.mComposition, composerHtml,
                                     10, 10, 100, 52)
        composerHtml.addFrame(htmlFrame)
        composerHtml.setResizeMode(
            QgsComposerMultiFrame.RepeatUntilFinished)
        composerHtml.setUseSmartBreaks(True)
        composerHtml.setUrl(self.htmlUrl())
        composerHtml.frame(0).setFrameEnabled(True)

        print("Checking page 1")
        myPage = 0
        checker1 = QgsCompositionChecker('composerhtml_smartbreaks1', self.mComposition)
        checker1.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker1.testComposition(myPage, 200)
        assert myTestResult, myMessage

        print("Checking page 2")
        myPage = 1
        checker2 = QgsCompositionChecker('composerhtml_smartbreaks2', self.mComposition)
        checker2.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker2.testComposition(myPage, 200)
        assert myTestResult, myMessage

        self.mComposition.removeMultiFrame(composerHtml)
        composerHtml = None

        assert myTestResult, myMessage

    def testComposerHtmlAccessor(self):
        """Test that we can retrieve the ComposerHtml instance given an item.
        """
        myComposition = QgsComposition(self.iface.mapCanvas().mapRenderer())
        mySubstitutionMap = {'replace-me': 'Foo bar'}
        myFile = os.path.join(TEST_DATA_DIR, 'template.qpt')
        with open(myFile, 'rt') as myTemplateFile:
            myTemplateContent = myTemplateFile.read()
        myDocument = QDomDocument()
        myDocument.setContent(myTemplateContent)
        myComposition.loadFromTemplate(myDocument, mySubstitutionMap)
        myItem = myComposition.getComposerItemById('html-test')
        myComposerHtml = myComposition.getComposerHtmlByItem(myItem)
        myMessage = 'Could not retrieve the composer html given an item'
        self.assertIsNotNone(myComposerHtml, myMessage)

if __name__ == '__main__':
    unittest.main()
