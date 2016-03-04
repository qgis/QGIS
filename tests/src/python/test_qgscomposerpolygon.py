# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerPolygon.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2016 by Paul Blottiere'
__date__ = '14/03/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis

from PyQt4.QtGui import QColor
from PyQt4.QtGui import QPolygonF
from PyQt4.QtCore import QPointF

from qgis.core import (QgsComposerPolygon,
                       QgsComposerItem,
                       QgsComposition,
                       QgsMapSettings
                       )
from qgis.testing import (start_app,
                          unittest
                          )
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerPolygon(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        self.mapSettings = QgsMapSettings()

        # create composition
        self.mComposition = QgsComposition(self.mapSettings)
        self.mComposition.setPaperSize(297, 210)

        # create
        polygon = QPolygonF()
        polygon.append(QPointF(0.0, 0.0))
        polygon.append(QPointF(100.0, 0.0))
        polygon.append(QPointF(200.0, 100.0))
        polygon.append(QPointF(100.0, 200.0))

        self.mComposerPolygon = QgsComposerPolygon(polygon, self.mComposition)
        self.mComposition.addComposerPolygon(self.mComposerPolygon)

    def testDisplayName(self):
        """Test if displayName is valid"""

        self.assertEqual(self.mComposerPolygon.displayName(), "<polygon>")

    def testType(self):
        """Test if type is valid"""

        self.assertEqual(
            self.mComposerPolygon.type(), QgsComposerItem.ComposerPolygon)

    def testDefaultStyle(self):
        """Test polygon rendering with default style."""

        self.mComposerPolygon.setDisplayPoints(False)
        checker = QgsCompositionChecker(
            'composerpolygon_defaultstyle', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testDisplayPoints(self):
        """Test displayPoints method"""

        self.mComposerPolygon.setDisplayPoints(True)
        checker = QgsCompositionChecker(
            'composerpolygon_displaypoints', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

        self.mComposerPolygon.setDisplayPoints(False)
        checker = QgsCompositionChecker(
            'composerpolygon_defaultstyle', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testSelectedPoint(self):
        """Test selectedPoint and unselectPoint methods"""

        self.mComposerPolygon.setDisplayPoints(True)

        self.mComposerPolygon.setSelectedPoint(3)
        checker = QgsCompositionChecker(
            'composerpolygon_selectedpoint', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

        self.mComposerPolygon.unselectPoint()
        self.mComposerPolygon.setDisplayPoints(False)
        checker = QgsCompositionChecker(
            'composerpolygon_defaultstyle', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testRemovePoint(self):
        """Test removePoint method"""

        rc = self.mComposerPolygon.removePoint(100)
        self.assertEqual(rc, False)

        checker = QgsCompositionChecker(
            'composerpolygon_defaultstyle', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

        self.assertEqual(self.mComposerPolygon.pointsSize(), 4)
        rc = self.mComposerPolygon.removePoint(3)
        self.assertEqual(rc, True)
        self.assertEqual(self.mComposerPolygon.pointsSize(), 3)

        checker = QgsCompositionChecker(
            'composerpolygon_removedpoint', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testAddPoint(self):
        """Test addPoint method"""

        # default searching radius is 10
        self.assertEqual(self.mComposerPolygon.pointsSize(), 4)
        rc = self.mComposerPolygon.addPoint(QPointF(50.0, 10.0))
        self.assertEqual(rc, False)

        # default searching radius is 10
        self.assertEqual(self.mComposerPolygon.pointsSize(), 4)
        rc = self.mComposerPolygon.addPoint(QPointF(50.0, 9.99))
        self.assertEqual(rc, True)
        self.assertEqual(self.mComposerPolygon.pointsSize(), 5)

    def testAddPointCustomRadius(self):
        """Test addPoint with custom radius"""

        # default searching radius is 10
        self.assertEqual(self.mComposerPolygon.pointsSize(), 4)
        rc = self.mComposerPolygon.addPoint(QPointF(50.0, 8.1), True, 8.0)
        self.assertEqual(rc, False)
        self.assertEqual(self.mComposerPolygon.pointsSize(), 4)

        # default searching radius is 10
        rc = self.mComposerPolygon.addPoint(QPointF(50.0, 7.9), True, 8.0)
        self.assertEqual(rc, True)
        self.assertEqual(self.mComposerPolygon.pointsSize(), 5)

    def testAddPointWithoutCheckingArea(self):
        """Test addPoint without checking the maximum distance allowed"""

        # default searching radius is 10
        self.assertEqual(self.mComposerPolygon.pointsSize(), 4)
        rc = self.mComposerPolygon.addPoint(QPointF(50.0, 20.0))
        self.assertEqual(rc, False)
        self.assertEqual(self.mComposerPolygon.pointsSize(), 4)

        # default searching radius is 10
        self.assertEqual(self.mComposerPolygon.pointsSize(), 4)
        rc = self.mComposerPolygon.addPoint(QPointF(50.0, 20.0), False)
        self.assertEqual(rc, True)
        self.assertEqual(self.mComposerPolygon.pointsSize(), 5)

        checker = QgsCompositionChecker(
            'composerpolygon_addpoint', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testMovePoint(self):
        """Test movePoint method"""

        rc = self.mComposerPolygon.movePoint(30, QPointF(100.0, 300.0))
        self.assertEqual(rc, False)

        rc = self.mComposerPolygon.movePoint(3, QPointF(100.0, 150.0))
        self.assertEqual(rc, True)

        checker = QgsCompositionChecker(
            'composerpolygon_movepoint', self.mComposition)
        checker.setControlPathPrefix("composer_polygon")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testPointAtPosition(self):
        """Test pointAtPosition method"""

        # default searching radius is 10
        rc = self.mComposerPolygon.pointAtPosition(QPointF(100.0, 210.0))
        self.assertEqual(rc, -1)

        # default searching radius is 10
        rc = self.mComposerPolygon.pointAtPosition(
            QPointF(100.0, 210.0), False)
        self.assertEqual(rc, 3)

        # default searching radius is 10
        rc = self.mComposerPolygon.pointAtPosition(
            QPointF(100.0, 210.0), True, 10.1)
        self.assertEqual(rc, 3)

if __name__ == '__main__':
    unittest.main()
