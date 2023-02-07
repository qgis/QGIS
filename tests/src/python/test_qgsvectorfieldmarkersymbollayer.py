"""
***************************************************************************
    test_qgsvectorfieldmarkersymbollayer.py
    ---------------------
    Date                 : January 2021
    Copyright            : (C) 2021 by Nyall Dawson
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
__date__ = 'November 2021'
__copyright__ = '(C) 2021, Nyall Dawson'

import qgis  # NOQA
from qgis.PyQt.QtCore import QDir, QVariant
from qgis.PyQt.QtGui import QImage, QColor, QPainter
from qgis.core import (QgsGeometry,
                       QgsFields,
                       QgsField,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsRenderChecker,
                       QgsLineSymbol,
                       QgsMarkerSymbol,
                       QgsVectorFieldSymbolLayer
                       )
from qgis.testing import unittest, start_app

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsVectorFieldMarkerSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsVectorFieldMarkerSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testRender(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute('x')
        field_marker.setYAttribute('y')
        field_marker.setScale(4)

        field_marker.setSubSymbol(QgsLineSymbol.createSimple({'color': '#ff0000', 'width': '2'}))

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt('Point(5 4)')
        fields = QgsFields()
        fields.append(QgsField('x', QVariant.Double))
        fields.append(QgsField('y', QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([2, 3])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        assert self.imageCheck('vectorfield', 'vectorfield', rendered_image)

    def testMapRotation(self):
        # test rendering with map rotation
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute('x')
        field_marker.setYAttribute('y')
        field_marker.setScale(4)

        field_marker.setSubSymbol(QgsLineSymbol.createSimple({'color': '#ff0000', 'width': '2'}))

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt('Point(5 4)')
        fields = QgsFields()
        fields.append(QgsField('x', QVariant.Double))
        fields.append(QgsField('y', QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([2, 3])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f, map_rotation=45)
        assert self.imageCheck('rotated_map', 'rotated_map', rendered_image)

    def testHeight(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute('x')
        field_marker.setYAttribute('y')
        field_marker.setScale(4)
        field_marker.setVectorFieldType(QgsVectorFieldSymbolLayer.Height)

        field_marker.setSubSymbol(QgsLineSymbol.createSimple({'color': '#ff0000', 'width': '2'}))

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt('Point(5 4)')
        fields = QgsFields()
        fields.append(QgsField('x', QVariant.Double))
        fields.append(QgsField('y', QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([2, 3])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        assert self.imageCheck('height', 'height', rendered_image)

    def testPolar(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute('x')
        field_marker.setYAttribute('y')
        field_marker.setVectorFieldType(QgsVectorFieldSymbolLayer.Polar)
        field_marker.setScale(1)

        field_marker.setSubSymbol(QgsLineSymbol.createSimple({'color': '#ff0000', 'width': '2'}))

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt('Point(5 4)')
        fields = QgsFields()
        fields.append(QgsField('x', QVariant.Double))
        fields.append(QgsField('y', QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([6, 135])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        assert self.imageCheck('polar', 'polar', rendered_image)

    def testPolarAnticlockwise(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute('x')
        field_marker.setYAttribute('y')
        field_marker.setVectorFieldType(QgsVectorFieldSymbolLayer.Polar)
        field_marker.setAngleOrientation(QgsVectorFieldSymbolLayer.CounterclockwiseFromEast)
        field_marker.setScale(1)

        field_marker.setSubSymbol(QgsLineSymbol.createSimple({'color': '#ff0000', 'width': '2'}))

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt('Point(5 4)')
        fields = QgsFields()
        fields.append(QgsField('x', QVariant.Double))
        fields.append(QgsField('y', QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([6, 135])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        assert self.imageCheck('anticlockwise_polar', 'anticlockwise_polar', rendered_image)

    def testPolarRadians(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute('x')
        field_marker.setYAttribute('y')
        field_marker.setVectorFieldType(QgsVectorFieldSymbolLayer.Polar)
        field_marker.setScale(1)
        field_marker.setAngleUnits(QgsVectorFieldSymbolLayer.Radians)

        field_marker.setSubSymbol(QgsLineSymbol.createSimple({'color': '#ff0000', 'width': '2'}))

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt('Point(5 4)')
        fields = QgsFields()
        fields.append(QgsField('x', QVariant.Double))
        fields.append(QgsField('y', QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([6, 135])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        assert self.imageCheck('radians_polar', 'radians_polar', rendered_image)

    def renderFeature(self, symbol, f, buffer=20, map_rotation=0):
        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = f.geometry().constGet().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / buffer)
        else:
            extent = extent.buffered(buffer / 2)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        ms.setRotation(map_rotation)
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.expressionContext().setFeature(f)

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context, f.fields())
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image

    def imageCheck(self, name, reference_image, image):
        self.report += f"<h2>Render {name}</h2>\n"
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_vectorfield")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print(self.report)
        return result


if __name__ == '__main__':
    unittest.main()
