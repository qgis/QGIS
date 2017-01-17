# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbollayer_readsld.py
    ---------------------
    Date                 : January 2017
    Copyright            : (C) 2017, Jorge Gustavo Rocha
    Email                : jgr at di dot uminho dot pt
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Jorge Gustavo Rocha'
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Jorge Gustavo Rocha'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
from qgis.testing import start_app, unittest
from qgis.core import (QgsVectorLayer,
                       QgsProject,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsSingleSymbolRenderer,
                       QgsFillSymbol,
                       QgsFeatureRequest
                       )
from qgis.testing import unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()

class TestQgsSymbolLayerReadSld(unittest.TestCase):

    """
    This class loads an SLD style and checks if the styling was properly applied
    """

    def setUp(self):
        self.iface = get_iface()
        myShpFile = os.path.join(TEST_DATA_DIR, 'streams.shp')
        self.layer = QgsVectorLayer(myShpFile, 'streams', 'ogr')
        mFilePath = os.path.join(TEST_DATA_DIR, 'symbol_layer/external_sld/simple_streams.sld')
        self.layer.loadSldStyle(mFilePath)
        self.props = self.layer.renderer().symbol().symbolLayers()[0].properties()

    def testLineColor(self):
        # stroke CSSParameter within ogc:Literal
        # expected color is #003EBA, RGB 0,62,186
        self.assertEqual(self.layer.renderer().symbol().symbolLayers()[0].color().name(), '#003eba')

    def testLineWidth(self):
        # stroke-width CSSParameter within ogc:Literal
        self.assertEqual(self.props['line_width'], '2')

    def testLineOpacity(self):
        # stroke-opacity CSSParameter NOT within ogc:Literal
        # stroke-opacity=0.1
        self.assertEqual(self.props['line_color'], '0,62,186,25')

if __name__ == '__main__':
    unittest.main()
