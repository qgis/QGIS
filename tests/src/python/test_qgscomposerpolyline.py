# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerPolyline.

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

from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtGui import QPolygonF
from qgis.PyQt.QtCore import QPointF

from qgis.core import (QgsComposerPolyline,
                       QgsComposerItem,
                       QgsComposition,
                       QgsMapSettings,
                       QgsLineSymbolV2
                       )
from qgis.testing import (start_app,
                          unittest
                          )
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerPolyline(unittest.TestCase):

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

        self.mComposerPolyline = QgsComposerPolyline(
            polygon, self.mComposition)
        self.mComposition.addComposerPolyline(self.mComposerPolyline)

        # style
        props = {}
        props["color"] = "0,0,0,255"
        props["width"] = "10.0"
        props["capstyle"] = "square"

        style = QgsLineSymbolV2.createSimple(props)
        self.mComposerPolyline.setPolylineStyleSymbol(style)

    def testDisplayName(self):
        """Test if displayName is valid"""

        self.assertEqual(self.mComposerPolyline.displayName(), "<polyline>")

    def testType(self):
        """Test if type is valid"""

        self.assertEqual(
            self.mComposerPolyline.type(), QgsComposerItem.ComposerPolyline)

    def testDefaultStyle(self):
        """Test polygon rendering with default style."""

        self.mComposerPolyline.setDisplayNodes(False)
        checker = QgsCompositionChecker(
            'composerpolyline_defaultstyle', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testDisplayNodes(self):
        """Test displayNodes method"""

        self.mComposerPolyline.setDisplayNodes(True)
        checker = QgsCompositionChecker(
            'composerpolyline_displaynodes', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

        self.mComposerPolyline.setDisplayNodes(False)
        checker = QgsCompositionChecker(
            'composerpolyline_defaultstyle', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testSelectedNode(self):
        """Test selectedNode and unselectNode methods"""

        self.mComposerPolyline.setDisplayNodes(True)

        self.mComposerPolyline.setSelectedNode(3)
        checker = QgsCompositionChecker(
            'composerpolyline_selectednode', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

        self.mComposerPolyline.unselectNode()
        self.mComposerPolyline.setDisplayNodes(False)
        checker = QgsCompositionChecker(
            'composerpolyline_defaultstyle', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testRemoveNode(self):
        """Test removeNode method"""

        rc = self.mComposerPolyline.removeNode(100)
        self.assertEqual(rc, False)

        checker = QgsCompositionChecker(
            'composerpolyline_defaultstyle', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

        self.assertEqual(self.mComposerPolyline.nodesSize(), 4)
        rc = self.mComposerPolyline.removeNode(3)
        self.assertEqual(rc, True)
        self.assertEqual(self.mComposerPolyline.nodesSize(), 3)

        checker = QgsCompositionChecker(
            'composerpolyline_removednode', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testAddNode(self):
        """Test addNode method"""

        # default searching radius is 10
        self.assertEqual(self.mComposerPolyline.nodesSize(), 4)
        rc = self.mComposerPolyline.addNode(QPointF(50.0, 10.0))
        self.assertEqual(rc, False)

        # default searching radius is 10
        self.assertEqual(self.mComposerPolyline.nodesSize(), 4)
        rc = self.mComposerPolyline.addNode(QPointF(50.0, 9.99))
        self.assertEqual(rc, True)
        self.assertEqual(self.mComposerPolyline.nodesSize(), 5)

    def testAddNodeCustomRadius(self):
        """Test addNode with custom radius"""

        # default searching radius is 10
        self.assertEqual(self.mComposerPolyline.nodesSize(), 4)
        rc = self.mComposerPolyline.addNode(QPointF(50.0, 8.1), True, 8.0)
        self.assertEqual(rc, False)
        self.assertEqual(self.mComposerPolyline.nodesSize(), 4)

        # default searching radius is 10
        rc = self.mComposerPolyline.addNode(QPointF(50.0, 7.9), True, 8.0)
        self.assertEqual(rc, True)
        self.assertEqual(self.mComposerPolyline.nodesSize(), 5)

    def testAddNodeWithoutCheckingArea(self):
        """Test addNode without checking the maximum distance allowed"""

        # default searching radius is 10
        self.assertEqual(self.mComposerPolyline.nodesSize(), 4)
        rc = self.mComposerPolyline.addNode(QPointF(50.0, 20.0))
        self.assertEqual(rc, False)
        self.assertEqual(self.mComposerPolyline.nodesSize(), 4)

        # default searching radius is 10
        self.assertEqual(self.mComposerPolyline.nodesSize(), 4)
        rc = self.mComposerPolyline.addNode(QPointF(50.0, 20.0), False)
        self.assertEqual(rc, True)
        self.assertEqual(self.mComposerPolyline.nodesSize(), 5)

        checker = QgsCompositionChecker(
            'composerpolyline_addnode', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testMoveNode(self):
        """Test moveNode method"""

        rc = self.mComposerPolyline.moveNode(30, QPointF(100.0, 300.0))
        self.assertEqual(rc, False)

        rc = self.mComposerPolyline.moveNode(3, QPointF(100.0, 150.0))
        self.assertEqual(rc, True)

        checker = QgsCompositionChecker(
            'composerpolyline_movenode', self.mComposition)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testComposition()
        assert myTestResult, myMessage

    def testNodeAtPosition(self):
        """Test nodeAtPosition method"""

        # default searching radius is 10
        rc = self.mComposerPolyline.nodeAtPosition(QPointF(100.0, 210.0))
        self.assertEqual(rc, -1)

        # default searching radius is 10
        rc = self.mComposerPolyline.nodeAtPosition(
            QPointF(100.0, 210.0), False)
        self.assertEqual(rc, 3)

        # default searching radius is 10
        rc = self.mComposerPolyline.nodeAtPosition(
            QPointF(100.0, 210.0), True, 10.1)
        self.assertEqual(rc, 3)

if __name__ == '__main__':
    unittest.main()
