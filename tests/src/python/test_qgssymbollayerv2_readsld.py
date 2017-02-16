# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbollayerv2_readsld.py
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
                       QgsFeature,
                       QgsGeometry,
                       QgsPoint
                       )
from qgis.testing import unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


def createLayerWithOneLine():
    # create a temporary layer
    # linelayer = iface.addVectorLayer("LineString?crs=epsg:4326&field=gid:int&field=name:string", "simple_line", "memory")
    linelayer = QgsVectorLayer("LineString?crs=epsg:4326&field=gid:int&field=name:string", "simple_line", "memory")
    one = QgsFeature(linelayer.dataProvider().fields(), 0)
    one.setAttributes([1, 'one'])
    one.setGeometry(QgsGeometry.fromPolyline([QgsPoint(-7, 38), QgsPoint(-8, 42)]))
    linelayer.dataProvider().addFeatures([one])
    return linelayer


class TestQgsSymbolLayerReadSld(unittest.TestCase):

    """
    This class checks if SLD styles are properly applied
    """

    def setUp(self):
        self.iface = get_iface()

    # test <CSSParameter>VALUE<CSSParameter/>
    # test <CSSParameter><ogc:Literal>VALUE<ogc:Literal/><CSSParameter/>
    def test_Literal_within_CSSParameter(self):
        layer = createLayerWithOneLine()
        mFilePath = os.path.join(TEST_DATA_DIR, 'symbol_layer/external_sld/simple_streams.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.rendererV2().symbol().symbolLayers()[0].properties()

        def testLineColor():
            # stroke CSSParameter within ogc:Literal
            # expected color is #003EBA, RGB 0,62,186
            self.assertEqual(layer.rendererV2().symbol().symbolLayers()[0].color().name(), '#003eba')

        def testLineWidth():
            # stroke-width CSSParameter within ogc:Literal
            self.assertEqual(props['line_width'], '2')

        def testLineOpacity():
            # stroke-opacity CSSParameter NOT within ogc:Literal
            # stroke-opacity=0.1
            self.assertEqual(props['line_color'], '0,62,186,25')

        testLineColor()
        testLineWidth()
        testLineOpacity()

    def testSimpleMarkerRotation(self):
        """
        Test if pointMarker property sld:Rotation value can be read if format is:
        <sld:Rotation>50.0</sld:Rotation>
        or
        <se:Rotation><ogc:Literal>50</ogc:Literal></se:Rotation>
        """
        # technically it's not necessary to use a real shape, but a empty memory
        # layer. In case these tests will upgrate to a rendering where to
        # compare also rendering not only properties
        #myShpFile = os.path.join(unitTestDataPath(), 'points.shp')
        #layer = QgsVectorLayer(myShpFile, 'points', 'ogr')
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        assert(layer.isValid())
        # test if able to read <sld:Rotation>50.0</sld:Rotation>
        mFilePath = os.path.join(unitTestDataPath(), 'symbol_layer/external_sld/testSimpleMarkerRotation-directValue.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.rendererV2().symbol().symbolLayers()[0].properties()
        self.assertEqual(props['angle'], '50')
        # test if able to read <se:Rotation><ogc:Literal>50</ogc:Literal></se:Rotation>
        mFilePath = os.path.join(unitTestDataPath(), 'symbol_layer/external_sld/testSimpleMarkerRotation-ogcLiteral.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.rendererV2().symbol().symbolLayers()[0].properties()
        self.assertEqual(props['angle'], '50')


if __name__ == '__main__':
    unittest.main()
