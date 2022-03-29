# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPlot

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '28/3/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import (
    QDir,
    Qt,
    QSizeF
)
from qgis.PyQt.QtGui import (
    QImage,
    QPainter,
    QColor
)

from qgis.core import (
    Qgs2DPlot,
    QgsRenderContext,
    QgsRenderChecker,
    QgsFontUtils,
    QgsTextFormat,
    QgsBasicNumericFormat,
    QgsFillSymbol,
    QgsLineSymbol,
    QgsReadWriteContext
)

from qgis.PyQt.QtXml import QDomDocument, QDomElement

from qgis.testing import start_app, unittest

app = start_app()


class TestQgsPlot(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsPlot Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testPlot(self):
        plot = Qgs2DPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_style': 'no'})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {'outline_color': '#0000ff', 'style': 'no', 'outline_style': 'solid', 'outline_width': 1})
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple({'outline_color': '#00ffff', 'outline_width': 1})
        plot.setXGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple({'outline_color': '#ff00ff', 'outline_width': 0.5})
        plot.setXGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple({'outline_color': '#0066ff', 'outline_width': 1})
        plot.setYGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple({'outline_color': '#ff4433', 'outline_width': 0.5})
        plot.setYGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont('Bold', 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.setColor(QColor(255, 0, 0))
        plot.setXAxisTextFormat(x_axis_format)

        x_axis_number_format = QgsBasicNumericFormat()
        x_axis_number_format.setNumberDecimalPlaces(1)
        x_axis_number_format.setShowTrailingZeros(True)
        plot.setXAxisNumericFormat(x_axis_number_format)

        font = QgsFontUtils.getStandardTestFont('Bold', 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.setColor(QColor(0, 255, 0))
        plot.setYAxisTextFormat(y_axis_format)

        y_axis_number_format = QgsBasicNumericFormat()
        y_axis_number_format.setShowPlusSign(True)
        plot.setYAxisNumericFormat(y_axis_number_format)

        plot.setXMinimum(3)
        plot.setXMaximum(9)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        im = QImage(600, 500, QImage.Format_ARGB32)
        im.fill(Qt.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        plot.render(rc)
        painter.end()

        assert self.imageCheck('plot_2d_base', 'plot_2d_base', im)

    def test_read_write(self):
        plot = Qgs2DPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_style': 'no'})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {'outline_color': '#0000ff', 'style': 'no', 'outline_style': 'solid', 'outline_width': 1})
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple({'outline_color': '#00ffff', 'outline_width': 1})
        plot.setXGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple({'outline_color': '#ff00ff', 'outline_width': 0.5})
        plot.setXGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple({'outline_color': '#0066ff', 'outline_width': 1})
        plot.setYGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple({'outline_color': '#ff4433', 'outline_width': 0.5})
        plot.setYGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont('Bold', 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.setColor(QColor(255, 0, 0))
        plot.setXAxisTextFormat(x_axis_format)

        x_axis_number_format = QgsBasicNumericFormat()
        x_axis_number_format.setNumberDecimalPlaces(1)
        x_axis_number_format.setShowTrailingZeros(True)
        plot.setXAxisNumericFormat(x_axis_number_format)

        font = QgsFontUtils.getStandardTestFont('Bold', 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.setColor(QColor(0, 255, 0))
        plot.setYAxisTextFormat(y_axis_format)

        y_axis_number_format = QgsBasicNumericFormat()
        y_axis_number_format.setShowPlusSign(True)
        plot.setYAxisNumericFormat(y_axis_number_format)

        plot.setXMinimum(3)
        plot.setXMaximum(9)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        plot.setGridIntervalMinorX(0.5)
        plot.setGridIntervalMajorX(1.5)
        plot.setGridIntervalMinorY(0.3)
        plot.setGridIntervalMajorY(1.3)

        doc = QDomDocument()
        elem = doc.createElement('test')
        plot.writeXml(elem, doc, QgsReadWriteContext())

        res = Qgs2DPlot()
        self.assertTrue(res.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(res.xMinimum(), 3)
        self.assertEqual(res.xMaximum(), 9)
        self.assertEqual(res.yMinimum(), 2)
        self.assertEqual(res.yMaximum(), 12)

        self.assertEqual(res.gridIntervalMinorX(), 0.5)
        self.assertEqual(res.gridIntervalMajorX(), 1.5)
        self.assertEqual(res.gridIntervalMinorY(), 0.3)
        self.assertEqual(res.gridIntervalMajorY(), 1.3)

        self.assertEqual(res.xAxisNumericFormat().numberDecimalPlaces(), 1)
        self.assertTrue(res.yAxisNumericFormat().showPlusSign())

        self.assertEqual(res.xAxisTextFormat().color().name(), '#ff0000')
        self.assertEqual(res.yAxisTextFormat().color().name(), '#00ff00')

        self.assertEqual(res.chartBackgroundSymbol().color().name(), '#fdbf6f')
        self.assertEqual(res.chartBorderSymbol().color().name(), '#0000ff')

        self.assertEqual(res.xGridMinorSymbol().color().name(), '#ff00ff')
        self.assertEqual(res.xGridMajorSymbol().color().name(), '#00ffff')
        self.assertEqual(res.yGridMinorSymbol().color().name(), '#ff4433')
        self.assertEqual(res.yGridMajorSymbol().color().name(), '#0066ff')

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'plot_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("plot")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
