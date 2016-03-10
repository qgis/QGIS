# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbolv2.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Nyall Dawson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import csv

from utilities import unitTestDataPath

from PyQt.QtCore import QSize, QDir
from PyQt.QtGui import QImage, QColor, QPainter

from qgis.core import (QgsGeometry,
                       QgsMarkerSymbolV2,
                       QgsFillSymbolV2,
                       QgsLineSymbolV2,
                       QgsRenderContext,
                       QgsFeature,
                       QGis,
                       QgsMapSettings,
                       QgsRectangle,
                       QgsRenderChecker
                       )

from qgis.testing import(
    unittest,
    start_app
)


start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSymbolV2(unittest.TestCase):

    def setUp(self):
        #Create some simple symbols
        self.fill_symbol = QgsFillSymbolV2.createSimple({'color': '#ffffff'})
        self.line_symbol = QgsLineSymbolV2.createSimple({'color': '#ffffff', 'line_width': '3'})
        self.marker_symbol = QgsMarkerSymbolV2.createSimple({'color': '#ffffff', 'size': '3'})
        self.report = "<h1>Python QgsSymbolV2 Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testGeometryRendering(self):
        '''Tests rendering a bunch of different geometries, including bad/odd geometries.'''

        tests = [{'name': 'Point',
                  'wkt': 'Point (1 2)',
                  'reference_image': 'point'},
                 {'name': 'MultiPoint',
                  'wkt': 'MultiPoint ((10 30),(40 20),(30 10),(20 10))',
                  'reference_image': 'multipoint'},
                 {'name': 'LineString',
                  'wkt': 'LineString (0 0,3 4,4 3)',
                  'reference_image': 'linestring'},
                 {'name': 'MultiLineString',
                  'wkt': 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))',
                  'reference_image': 'multilinestring'},
                 {'name': 'Polygon',
                  'wkt': 'Polygon ((0 0, 10 0, 10 10, 0 10, 0 0),(5 5, 7 5, 7 7 , 5 7, 5 5))',
                  'reference_image': 'polygon'},
                 {'name': 'MultiPolygon',
                  'wkt': 'MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))',
                  'reference_image': 'multipolygon'},
                 {'name': 'CircularString',
                  'wkt': 'CIRCULARSTRING(268 415,227 505,227 406)',
                  'reference_image': 'circular_string'},
                 {'name': 'CompoundCurve',
                  'wkt': 'COMPOUNDCURVE((5 3, 5 13), CIRCULARSTRING(5 13, 7 15, 9 13), (9 13, 9 3), CIRCULARSTRING(9 3, 7 1, 5 3))',
                  'reference_image': 'compound_curve'},
                 {'name': 'CurvePolygon',
                  'wkt': 'CURVEPOLYGON(CIRCULARSTRING(1 3, 3 5, 4 7, 7 3, 1 3))',
                  'reference_image': 'curve_polygon'},
                 {'name': 'MultiCurve',
                  'wkt': 'MultiCurve((5 5,3 5,3 3,0 3),CIRCULARSTRING(0 0, 2 1,2 2))',
                  'reference_image': 'multicurve'},
                 {'name': 'CurvePolygon_no_arc',  # refs #14028
                  'wkt': 'CURVEPOLYGON(LINESTRING(1 3, 3 5, 4 7, 7 3, 1 3))',
                  'reference_image': 'curve_polygon_no_arc'}]

        for test in tests:
            geom = QgsGeometry.fromWkt(test['wkt'])
            assert geom and not geom.isEmpty(), 'Could not create geometry'
            rendered_image = self.renderGeometry(geom)
            assert self.imageCheck(test['name'], test['reference_image'], rendered_image)

            #Note - each test is repeated with the same geometry and reference image, but with added
            #z and m dimensions. This tests that presence of the dimensions does not affect rendering

            #test with Z
            geom_z = QgsGeometry.fromWkt(test['wkt'])
            geom_z.geometry().addZValue(5)
            rendered_image = self.renderGeometry(geom_z)
            assert self.imageCheck(test['name'] + 'Z', test['reference_image'], rendered_image)

            #test with ZM
            geom_z.geometry().addMValue(15)
            rendered_image = self.renderGeometry(geom_z)
            assert self.imageCheck(test['name'] + 'ZM', test['reference_image'], rendered_image)

            #test with ZM
            geom_m = QgsGeometry.fromWkt(test['wkt'])
            geom_m.geometry().addMValue(15)
            rendered_image = self.renderGeometry(geom_m)
            assert self.imageCheck(test['name'] + 'M', test['reference_image'], rendered_image)

    def renderGeometry(self, geom):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.geometry().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffer((extent.height() + extent.width()) / 20.0)
        else:
            extent = extent.buffer(10)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4) # 96 DPI

        painter.begin(image)
        image.fill(QColor(0, 0, 0))

        if geom.type() == QGis.Polygon:
            self.fill_symbol.startRender(context)
            self.fill_symbol.renderFeature(f, context)
            self.fill_symbol.stopRender(context)

        elif geom.type() == QGis.Line:
            self.line_symbol.startRender(context)
            self.line_symbol.renderFeature(f, context)
            self.line_symbol.stopRender(context)

        elif geom.type() == QGis.Point:
            self.marker_symbol.startRender(context)
            self.marker_symbol.renderFeature(f, context)
            self.marker_symbol.stopRender(context)

        painter.end()
        return image

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbolv2")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print self.report
        return result


if __name__ == '__main__':
    unittest.main()# -*- coding: utf-8 -*-
