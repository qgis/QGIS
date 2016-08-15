# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbollayer_createsld.py
    ---------------------
    Date                 : July 2016
    Copyright            : (C) 2016 by Andrea Aime
    Email                : andrea dot aime at geosolutions dot it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *less
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Andrea Aime'
__date__ = 'July 2016'
__copyright__ = '(C) 2012, Andrea Aime'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import pyqtWrapperType, Qt, QDir, QFile, QIODevice, QPointF
from qgis.PyQt.QtXml import (QDomDocument, QDomElement)
from qgis.PyQt.QtGui import QColor

from qgis.core import QgsSimpleMarkerSymbolLayer
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsSymbolLayerCreateSld(unittest.TestCase):

    """
     This class tests the creation of SLD from QGis layers
     """

    def testSimpleMarkerSymbolLayer(self):
        symbol = QgsSimpleMarkerSymbolLayer(
            'star', QColor(255, 0, 0), QColor(0, 255, 0), 10)
        symbol.setAngle(50)
        dom = QDomDocument()
        root = dom.createElement("FakeRoot")
        dom.appendChild(root)
        symbol.toSld(dom, root, {})
        # print "This is the dom: " + dom.toString()

        # Check the rotation element is a literal, not a
        rotation = root.elementsByTagName('se:Rotation').item(0)
        literal = rotation.firstChild()
        self.assertEquals("ogc:Literal", literal.nodeName())
        self.assertEquals('50', literal.firstChild().nodeValue())


if __name__ == '__main__':
    unittest.main()
