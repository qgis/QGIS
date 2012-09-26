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
from qgis.core import (QgsComposition, QgsPoint)

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


class TestQgsComposition(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testSubstitutionMap(self):
        """Test that we can use degree symbols in substitutions.
        """
        # Create a point and convert it to text containing a degree symbol.
        myPoint = QgsPoint(12.3, -33.33)
        myCoordinates = myPoint.toDegreesMinutesSeconds(2)
        myTokens = myCoordinates.split(',')
        myLongitude = myTokens[0]
        myLatitude = myTokens[1]
        myText = 'Latitude: %s, Longitude: %s' % (myLatitude, myLongitude)

        # Load the composition with the substitutions
        myComposition = QgsComposition(CANVAS.mapRenderer())
        mySubstitutionMap = {'replace-me': myText }
        myFile = os.path.join(TEST_DATA_DIR, 'template.qpt')
        myTemplateFile = file(myFile, 'rt')
        myTemplateContent = myTemplateFile.read()
        myTemplateFile.close()
        myDocument = QDomDocument()
        myDocument.setContent(myTemplateContent)
        myComposition.loadFromTemplate(myDocument, mySubstitutionMap)
        

if __name__ == '__main__':
    unittest.main()
