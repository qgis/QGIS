"""QGIS Unit tests for QgsPlot

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "28/3/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

from qgis.PyQt.QtCore import QDir, QSizeF, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgs2DXyPlot,
    QgsBarChartPlot,
    QgsBasicNumericFormat,
    QgsFillSymbol,
    QgsFontUtils,
    QgsLineChartPlot,
    QgsLineSymbol,
    QgsPalLayerSettings,
    QgsPieChartPlot,
    QgsPlotData,
    QgsPlotRenderContext,
    QgsPresetSchemeColorRamp,
    QgsProperty,
    QgsMarkerSymbol,
    QgsReadWriteContext,
    QgsRenderContext,
    QgsSymbolLayer,
    QgsTextFormat,
    QgsXyPlotSeries,
    Qgis,
)
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestQgsPlot(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "plot"

    def testPlot(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({"color": "#fdbf6f", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#0000ff",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.setColor(QColor(255, 0, 0))
        plot.xAxis().setTextFormat(x_axis_format)

        x_axis_number_format = QgsBasicNumericFormat()
        x_axis_number_format.setNumberDecimalPlaces(1)
        x_axis_number_format.setShowTrailingZeros(True)
        plot.xAxis().setNumericFormat(x_axis_number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.setColor(QColor(0, 255, 0))
        plot.yAxis().setTextFormat(y_axis_format)

        y_axis_number_format = QgsBasicNumericFormat()
        y_axis_number_format.setShowPlusSign(True)
        plot.yAxis().setNumericFormat(y_axis_number_format)

        plot.setXMinimum(3)
        plot.setXMaximum(9)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        im = QImage(600, 500, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc)
        painter.end()

        assert self.image_check("plot_2d_base", "plot_2d_base", im)

        plot_rect = plot.interiorPlotArea(rc, prc)
        self.assertAlmostEqual(plot_rect.left(), 64.8, 0)
        self.assertAlmostEqual(plot_rect.right(), 592.44, 0)
        self.assertAlmostEqual(plot_rect.top(), 7.559, 0)
        self.assertAlmostEqual(plot_rect.bottom(), 465.55, delta=1)

    def testPlotSuffixAll(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({"color": "#fdbf6f", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#0000ff",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.setColor(QColor(255, 0, 0))
        plot.xAxis().setTextFormat(x_axis_format)

        x_axis_number_format = QgsBasicNumericFormat()
        x_axis_number_format.setNumberDecimalPlaces(1)
        x_axis_number_format.setShowTrailingZeros(True)
        plot.xAxis().setNumericFormat(x_axis_number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.setColor(QColor(0, 255, 0))
        plot.yAxis().setTextFormat(y_axis_format)

        y_axis_number_format = QgsBasicNumericFormat()
        y_axis_number_format.setShowPlusSign(True)
        plot.yAxis().setNumericFormat(y_axis_number_format)

        plot.setXMinimum(3)
        plot.setXMaximum(9)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        plot.xAxis().setLabelSuffix("x")
        plot.xAxis().setLabelSuffixPlacement(Qgis.PlotAxisSuffixPlacement.EveryLabel)
        plot.yAxis().setLabelSuffix("y")
        plot.yAxis().setLabelSuffixPlacement(Qgis.PlotAxisSuffixPlacement.EveryLabel)

        im = QImage(600, 500, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc)
        painter.end()

        assert self.image_check(
            "plot_2d_base_suffix_all", "plot_2d_base_suffix_all", im
        )

        plot_rect = plot.interiorPlotArea(rc, prc)
        self.assertAlmostEqual(plot_rect.left(), 80.46, 0)
        self.assertAlmostEqual(plot_rect.right(), 592.44, 0)
        self.assertAlmostEqual(plot_rect.top(), 7.559, 0)
        self.assertAlmostEqual(plot_rect.bottom(), 465.55, delta=1)

    def testPlotSuffixFirst(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({"color": "#fdbf6f", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#0000ff",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.setColor(QColor(255, 0, 0))
        plot.xAxis().setTextFormat(x_axis_format)

        x_axis_number_format = QgsBasicNumericFormat()
        x_axis_number_format.setNumberDecimalPlaces(1)
        x_axis_number_format.setShowTrailingZeros(True)
        plot.xAxis().setNumericFormat(x_axis_number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.setColor(QColor(0, 255, 0))
        plot.yAxis().setTextFormat(y_axis_format)

        y_axis_number_format = QgsBasicNumericFormat()
        y_axis_number_format.setShowPlusSign(True)
        plot.yAxis().setNumericFormat(y_axis_number_format)

        plot.setXMinimum(3)
        plot.setXMaximum(9)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        plot.xAxis().setLabelSuffix("x")
        plot.xAxis().setLabelSuffixPlacement(Qgis.PlotAxisSuffixPlacement.FirstLabel)
        plot.yAxis().setLabelSuffix("y")
        plot.yAxis().setLabelSuffixPlacement(Qgis.PlotAxisSuffixPlacement.FirstLabel)

        im = QImage(600, 500, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc)
        painter.end()

        assert self.image_check(
            "plot_2d_base_suffix_first", "plot_2d_base_suffix_first", im
        )

        plot_rect = plot.interiorPlotArea(rc, prc)
        self.assertAlmostEqual(plot_rect.left(), 64.82, 0)
        self.assertAlmostEqual(plot_rect.right(), 592.44, 0)
        self.assertAlmostEqual(plot_rect.top(), 7.559, 0)
        self.assertAlmostEqual(plot_rect.bottom(), 465.55, delta=1)

    def testPlotSuffixLast(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({"color": "#fdbf6f", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#0000ff",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.setColor(QColor(255, 0, 0))
        plot.xAxis().setTextFormat(x_axis_format)

        x_axis_number_format = QgsBasicNumericFormat()
        x_axis_number_format.setNumberDecimalPlaces(1)
        x_axis_number_format.setShowTrailingZeros(True)
        plot.xAxis().setNumericFormat(x_axis_number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.setColor(QColor(0, 255, 0))
        plot.yAxis().setTextFormat(y_axis_format)

        y_axis_number_format = QgsBasicNumericFormat()
        y_axis_number_format.setShowPlusSign(True)
        plot.yAxis().setNumericFormat(y_axis_number_format)

        plot.setXMinimum(3)
        plot.setXMaximum(9.5)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        plot.xAxis().setLabelSuffix("x")
        plot.xAxis().setLabelSuffixPlacement(Qgis.PlotAxisSuffixPlacement.LastLabel)
        plot.yAxis().setLabelSuffix("y")
        plot.yAxis().setLabelSuffixPlacement(Qgis.PlotAxisSuffixPlacement.LastLabel)

        im = QImage(600, 500, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc)
        painter.end()

        assert self.image_check(
            "plot_2d_base_suffix_last", "plot_2d_base_suffix_last", im
        )

        plot_rect = plot.interiorPlotArea(rc, prc)
        self.assertAlmostEqual(plot_rect.left(), 80.46, 0)
        self.assertAlmostEqual(plot_rect.right(), 592.44, 0)
        self.assertAlmostEqual(plot_rect.top(), 7.559, 0)
        self.assertAlmostEqual(plot_rect.bottom(), 465.55, delta=1)

    def testPlotSuffixFirstAndLast(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({"color": "#fdbf6f", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#0000ff",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.setColor(QColor(255, 0, 0))
        plot.xAxis().setTextFormat(x_axis_format)

        x_axis_number_format = QgsBasicNumericFormat()
        x_axis_number_format.setNumberDecimalPlaces(1)
        x_axis_number_format.setShowTrailingZeros(True)
        plot.xAxis().setNumericFormat(x_axis_number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.setColor(QColor(0, 255, 0))
        plot.yAxis().setTextFormat(y_axis_format)

        y_axis_number_format = QgsBasicNumericFormat()
        y_axis_number_format.setShowPlusSign(True)
        plot.yAxis().setNumericFormat(y_axis_number_format)

        plot.setXMinimum(3)
        plot.setXMaximum(9.5)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        plot.xAxis().setLabelSuffix("x")
        plot.xAxis().setLabelSuffixPlacement(
            Qgis.PlotAxisSuffixPlacement.FirstAndLastLabels
        )
        plot.yAxis().setLabelSuffix("y")
        plot.yAxis().setLabelSuffixPlacement(
            Qgis.PlotAxisSuffixPlacement.FirstAndLastLabels
        )

        im = QImage(600, 500, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc)
        painter.end()

        assert self.image_check(
            "plot_2d_base_suffix_first_and_last",
            "plot_2d_base_suffix_first_and_last",
            im,
        )

        plot_rect = plot.interiorPlotArea(rc, prc)
        self.assertAlmostEqual(plot_rect.left(), 80.46, 0)
        self.assertAlmostEqual(plot_rect.right(), 592.44, 0)
        self.assertAlmostEqual(plot_rect.top(), 7.559, 0)
        self.assertAlmostEqual(plot_rect.bottom(), 465.55, delta=1)

    def testPlotIntervals(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({"color": "#fdbf6f", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#0000ff",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        plot.xAxis().setTextFormat(x_axis_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        plot.yAxis().setTextFormat(y_axis_format)

        plot.setXMinimum(1)
        plot.setXMaximum(21)

        plot.setYMinimum(27)
        plot.setYMaximum(327)

        plot.xAxis().setGridIntervalMajor(10)
        plot.xAxis().setGridIntervalMinor(2)

        plot.yAxis().setGridIntervalMajor(100)
        plot.yAxis().setGridIntervalMinor(50)

        plot.xAxis().setLabelInterval(5)
        plot.yAxis().setLabelInterval(70)

        im = QImage(600, 500, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc)
        painter.end()

        assert self.image_check("plot_2d_intervals", "plot_2d_intervals", im)

    def testPlotDataDefinedProperties(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({"color": "#ffffff", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#000000",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1, "capstyle": "flat"}
        )
        sym3[0].setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeWidth,
            QgsProperty.fromExpression(
                "case when @plot_axis_value = 10 then 3 else 1 end"
            ),
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5, "capstyle": "flat"}
        )
        sym4[0].setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeWidth,
            QgsProperty.fromExpression(
                "case when @plot_axis_value = 6 then 3 else 0.5 end"
            ),
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1, "capstyle": "flat"}
        )
        sym3[0].setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeWidth,
            QgsProperty.fromExpression(
                "case when @plot_axis_value = 5 then 3 else 0.5 end"
            ),
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5, "capstyle": "flat"}
        )
        sym4[0].setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeWidth,
            QgsProperty.fromExpression(
                "case when @plot_axis_value = 9 then 3 else 0.5 end"
            ),
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Color,
            QgsProperty.fromExpression(
                "case when @plot_axis_value %3 = 0 then '#ff0000' else '#000000' end"
            ),
        )
        plot.xAxis().setTextFormat(x_axis_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Color,
            QgsProperty.fromExpression(
                "case when @plot_axis_value %4 = 0 then '#0000ff' else '#000000' end"
            ),
        )
        plot.yAxis().setTextFormat(y_axis_format)

        plot.setXMinimum(3)
        plot.setXMaximum(13)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        im = QImage(600, 500, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc)
        painter.end()

        assert self.image_check("plot_2d_data_defined", "plot_2d_data_defined", im)

        plot_rect = plot.interiorPlotArea(rc, prc)
        self.assertAlmostEqual(plot_rect.left(), 44.71, 0)
        self.assertAlmostEqual(plot_rect.right(), 592.44, 0)
        self.assertAlmostEqual(plot_rect.top(), 7.559, 0)
        self.assertAlmostEqual(plot_rect.bottom(), 465.55, delta=1)

    def testOptimiseIntervals(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        plot.xAxis().setTextFormat(x_axis_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        plot.yAxis().setTextFormat(y_axis_format)

        plot.setXMinimum(3)
        plot.setXMaximum(13)
        plot.setYMinimum(2)
        plot.setYMaximum(12)

        im = QImage(600, 500, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(96 / 25.4 * 1000))
        im.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        prc = QgsPlotRenderContext()
        painter.end()

        plot.calculateOptimisedIntervals(rc, prc)
        self.assertEqual(plot.xAxis().labelInterval(), 1)
        self.assertEqual(plot.yAxis().labelInterval(), 2)
        self.assertEqual(plot.xAxis().gridIntervalMinor(), 1)
        self.assertEqual(plot.yAxis().gridIntervalMinor(), 1)
        self.assertEqual(plot.xAxis().gridIntervalMajor(), 5)
        self.assertEqual(plot.yAxis().gridIntervalMajor(), 4)

        plot.setXMinimum(3)
        plot.setXMaximum(113)
        plot.setYMinimum(2)
        plot.setYMaximum(112)

        plot.calculateOptimisedIntervals(rc, prc)
        self.assertEqual(plot.xAxis().labelInterval(), 20)
        self.assertEqual(plot.yAxis().labelInterval(), 20)
        self.assertEqual(plot.xAxis().gridIntervalMinor(), 10)
        self.assertEqual(plot.yAxis().gridIntervalMinor(), 10)
        self.assertEqual(plot.xAxis().gridIntervalMajor(), 40)
        self.assertEqual(plot.yAxis().gridIntervalMajor(), 40)

        plot.setXMinimum(0.3)
        plot.setXMaximum(0.5)
        plot.setYMinimum(1.1)
        plot.setYMaximum(2)

        plot.calculateOptimisedIntervals(rc, prc)
        self.assertEqual(plot.xAxis().labelInterval(), 0.05)
        self.assertEqual(plot.yAxis().labelInterval(), 0.2)
        self.assertEqual(plot.xAxis().gridIntervalMinor(), 0.025)
        self.assertEqual(plot.yAxis().gridIntervalMinor(), 0.1)
        self.assertEqual(plot.xAxis().gridIntervalMajor(), 0.1)
        self.assertEqual(plot.yAxis().gridIntervalMajor(), 0.4)

        plot.setXMinimum(-10)
        plot.setXMaximum(0)
        plot.setYMinimum(-10000)
        plot.setYMaximum(-500)

        plot.calculateOptimisedIntervals(rc, prc)
        self.assertEqual(plot.xAxis().labelInterval(), 2)
        self.assertEqual(plot.yAxis().labelInterval(), 2000)
        self.assertEqual(plot.xAxis().gridIntervalMinor(), 1)
        self.assertEqual(plot.yAxis().gridIntervalMinor(), 1000)
        self.assertEqual(plot.xAxis().gridIntervalMajor(), 4)
        self.assertEqual(plot.yAxis().gridIntervalMajor(), 4000)

        plot.setXMinimum(100000)
        plot.setXMaximum(200000)

        plot.calculateOptimisedIntervals(rc, prc)
        self.assertEqual(plot.xAxis().labelInterval(), 100000)
        self.assertEqual(plot.xAxis().gridIntervalMinor(), 50000)
        self.assertEqual(plot.xAxis().gridIntervalMajor(), 200000)

    def test_read_write(self):
        plot = Qgs2DXyPlot()
        plot.setSize(QSizeF(600, 500))

        sym1 = QgsFillSymbol.createSimple({"color": "#fdbf6f", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#0000ff",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        x_axis_format.setColor(QColor(255, 0, 0))
        plot.xAxis().setTextFormat(x_axis_format)

        x_axis_number_format = QgsBasicNumericFormat()
        x_axis_number_format.setNumberDecimalPlaces(1)
        x_axis_number_format.setShowTrailingZeros(True)
        plot.xAxis().setNumericFormat(x_axis_number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        y_axis_format.setColor(QColor(0, 255, 0))
        plot.yAxis().setTextFormat(y_axis_format)

        y_axis_number_format = QgsBasicNumericFormat()
        y_axis_number_format.setShowPlusSign(True)
        plot.yAxis().setNumericFormat(y_axis_number_format)

        plot.setXMinimum(3)
        plot.setXMaximum(9)

        plot.setYMinimum(2)
        plot.setYMaximum(12)

        plot.xAxis().setGridIntervalMinor(0.5)
        plot.xAxis().setGridIntervalMajor(1.5)
        plot.yAxis().setGridIntervalMinor(0.3)
        plot.yAxis().setGridIntervalMajor(1.3)

        plot.xAxis().setLabelInterval(32)
        plot.yAxis().setLabelInterval(23)

        plot.xAxis().setLabelSuffix("km")
        plot.xAxis().setLabelSuffixPlacement(Qgis.PlotAxisSuffixPlacement.LastLabel)
        plot.yAxis().setLabelSuffix("m")
        plot.yAxis().setLabelSuffixPlacement(
            Qgis.PlotAxisSuffixPlacement.FirstAndLastLabels
        )

        doc = QDomDocument()
        elem = doc.createElement("test")
        plot.writeXml(elem, doc, QgsReadWriteContext())

        res = Qgs2DXyPlot()
        self.assertTrue(res.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(res.xMinimum(), 3)
        self.assertEqual(res.xMaximum(), 9)
        self.assertEqual(res.yMinimum(), 2)
        self.assertEqual(res.yMaximum(), 12)

        self.assertEqual(res.xAxis().gridIntervalMinor(), 0.5)
        self.assertEqual(res.xAxis().gridIntervalMajor(), 1.5)
        self.assertEqual(res.yAxis().gridIntervalMinor(), 0.3)
        self.assertEqual(res.yAxis().gridIntervalMajor(), 1.3)

        self.assertEqual(res.xAxis().labelInterval(), 32)
        self.assertEqual(res.yAxis().labelInterval(), 23)

        self.assertEqual(res.xAxis().numericFormat().numberDecimalPlaces(), 1)
        self.assertTrue(res.yAxis().numericFormat().showPlusSign())

        self.assertEqual(res.xAxis().textFormat().color().name(), "#ff0000")
        self.assertEqual(res.yAxis().textFormat().color().name(), "#00ff00")

        self.assertEqual(res.chartBackgroundSymbol().color().name(), "#fdbf6f")
        self.assertEqual(res.chartBorderSymbol().color().name(), "#0000ff")

        self.assertEqual(res.xAxis().gridMinorSymbol().color().name(), "#ff00ff")
        self.assertEqual(res.xAxis().gridMajorSymbol().color().name(), "#00ffff")
        self.assertEqual(res.yAxis().gridMinorSymbol().color().name(), "#ff4433")
        self.assertEqual(res.yAxis().gridMajorSymbol().color().name(), "#0066ff")

        self.assertEqual(res.xAxis().labelSuffix(), "km")
        self.assertEqual(
            res.xAxis().labelSuffixPlacement(), Qgis.PlotAxisSuffixPlacement.LastLabel
        )
        self.assertEqual(res.yAxis().labelSuffix(), "m")
        self.assertEqual(
            res.yAxis().labelSuffixPlacement(),
            Qgis.PlotAxisSuffixPlacement.FirstAndLastLabels,
        )

    def testBarChartPlotXAxisCategory(self):
        width = 600
        height = 500
        dpi = 96

        plot = QgsBarChartPlot()
        plot.setSize(QSizeF(width, height))

        sym1 = QgsFillSymbol.createSimple({"color": "#ffffff", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#000000",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1, "capstyle": "flat"}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5, "capstyle": "flat"}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1, "capstyle": "flat"}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5, "capstyle": "flat"}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        plot.xAxis().setTextFormat(x_axis_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        plot.yAxis().setTextFormat(y_axis_format)

        plot.xAxis().setType(Qgis.PlotAxisType.Categorical)
        plot.setYMinimum(-10)
        plot.setYMaximum(10)

        # set symbol for first series
        series_symbol = QgsFillSymbol.createSimple(
            {
                "color": "#00BB00",
                "outline_color": "#003300",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setFillSymbolAt(0, series_symbol)

        # set symbol for second series
        series_symbol = QgsFillSymbol.createSimple(
            {
                "color": "#BB0000",
                "outline_color": "#330000",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setFillSymbolAt(1, series_symbol)

        data = QgsPlotData()
        series = QgsXyPlotSeries()
        series.append(0, 1)
        series.append(1, 5)
        series.append(2, 5)
        series.append(3, 9)
        data.addSeries(series)
        series = QgsXyPlotSeries()
        series.append(0, -5)
        series.append(1, -2)
        series.append(2, 5)
        series.append(3, 4)
        data.addSeries(series)
        data.setCategories(["Q1", "Q2", "Q3", "Q4"])

        im = QImage(width, height, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        im.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        rc.setScaleFactor(dpi / 25.4)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc, data)
        painter.end()

        assert self.image_check(
            "bar_chart_plot_x_axis_category", "bar_chart_plot_x_axis_category", im
        )

    def testBarChartPlotXAxisValue(self):
        width = 600
        height = 500
        dpi = 96

        plot = QgsBarChartPlot()
        plot.setSize(QSizeF(width, height))

        sym1 = QgsFillSymbol.createSimple({"color": "#ffffff", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#000000",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1, "capstyle": "flat"}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5, "capstyle": "flat"}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1, "capstyle": "flat"}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5, "capstyle": "flat"}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        plot.xAxis().setTextFormat(x_axis_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        plot.yAxis().setTextFormat(y_axis_format)

        plot.xAxis().setType(Qgis.PlotAxisType.Interval)
        plot.setXMinimum(-10)
        plot.setXMaximum(10)
        plot.setYMinimum(-10)
        plot.setYMaximum(10)

        # set symbol for first series
        series_symbol = QgsFillSymbol.createSimple(
            {
                "color": "#00BB00",
                "outline_color": "#003300",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setFillSymbolAt(0, series_symbol)

        # set symbol for second series
        series_symbol = QgsFillSymbol.createSimple(
            {
                "color": "#BB0000",
                "outline_color": "#330000",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setFillSymbolAt(1, series_symbol)

        data = QgsPlotData()
        series = QgsXyPlotSeries()
        series.append(-8, 1)
        series.append(0, 5)
        series.append(4, 5)
        series.append(9, 9)
        data.addSeries(series)
        series = QgsXyPlotSeries()
        series.append(-7, -5)
        series.append(1, -2)
        series.append(4, 5)
        series.append(8, 4)
        data.addSeries(series)

        im = QImage(width, height, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        im.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        rc.setScaleFactor(dpi / 25.4)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc, data)
        painter.end()

        assert self.image_check(
            "bar_chart_plot_x_axis_value", "bar_chart_plot_x_axis_value", im
        )

    def testBarChartPlotXAxisCategory(self):
        width = 600
        height = 500
        dpi = 96

        plot = QgsLineChartPlot()
        plot.setSize(QSizeF(width, height))

        sym1 = QgsFillSymbol.createSimple({"color": "#ffffff", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#000000",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1, "capstyle": "flat"}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5, "capstyle": "flat"}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1, "capstyle": "flat"}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5, "capstyle": "flat"}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        plot.xAxis().setTextFormat(x_axis_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        plot.yAxis().setTextFormat(y_axis_format)

        plot.xAxis().setType(Qgis.PlotAxisType.Categorical)
        plot.setYMinimum(-10)
        plot.setYMaximum(10)

        # set symbol for first series
        series_symbol = QgsLineSymbol.createSimple(
            {
                "outline_color": "#00BB00",
                "outline_style": "dash",
                "outline_width": 1,
            }
        )
        plot.setLineSymbolAt(0, series_symbol)
        # remove default marker
        plot.setMarkerSymbolAt(0, None)

        # set symbols for second series
        series_symbol = QgsLineSymbol.createSimple(
            {
                "outline_color": "#BB0000",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setLineSymbolAt(1, series_symbol)
        series_symbol = QgsMarkerSymbol.createSimple(
            {
                "color": "#BB0000",
                "outline_color": "#330000",
                "outline_style": "solid",
                "outline_width": 1,
                "width": 3,
            }
        )
        plot.setMarkerSymbolAt(1, series_symbol)

        data = QgsPlotData()
        series = QgsXyPlotSeries()
        series.append(0, 1)
        series.append(1, 2)
        series.append(2, 5)
        series.append(3, 9)
        data.addSeries(series)
        series = QgsXyPlotSeries()
        series.append(0, -5)
        series.append(1, -2)
        # skip 3rd category to test disconnected lines
        series.append(3, 4)
        data.addSeries(series)
        data.setCategories(["Q1", "Q2", "Q3", "Q4"])

        im = QImage(width, height, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        im.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        rc.setScaleFactor(dpi / 25.4)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc, data)
        painter.end()

        assert self.image_check(
            "line_chart_plot_x_axis_category", "line_chart_plot_x_axis_category", im
        )

    def testLineChartPlotXAxisValue(self):
        width = 600
        height = 500
        dpi = 96

        plot = QgsLineChartPlot()
        plot.setSize(QSizeF(width, height))

        sym1 = QgsFillSymbol.createSimple({"color": "#ffffff", "outline_style": "no"})
        plot.setChartBackgroundSymbol(sym1)

        sym2 = QgsFillSymbol.createSimple(
            {
                "outline_color": "#000000",
                "style": "no",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setChartBorderSymbol(sym2)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#00ffff", "outline_width": 1, "capstyle": "flat"}
        )
        plot.xAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff00ff", "outline_width": 0.5, "capstyle": "flat"}
        )
        plot.xAxis().setGridMinorSymbol(sym4)

        sym3 = QgsLineSymbol.createSimple(
            {"outline_color": "#0066ff", "outline_width": 1, "capstyle": "flat"}
        )
        plot.yAxis().setGridMajorSymbol(sym3)

        sym4 = QgsLineSymbol.createSimple(
            {"outline_color": "#ff4433", "outline_width": 0.5, "capstyle": "flat"}
        )
        plot.yAxis().setGridMinorSymbol(sym4)

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        x_axis_format = QgsTextFormat.fromQFont(font)
        plot.xAxis().setTextFormat(x_axis_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        y_axis_format = QgsTextFormat.fromQFont(font)
        plot.yAxis().setTextFormat(y_axis_format)

        plot.xAxis().setType(Qgis.PlotAxisType.Interval)
        plot.setXMinimum(-10)
        plot.setXMaximum(10)
        plot.setYMinimum(-10)
        plot.setYMaximum(10)

        # set symbol for first series
        series_symbol = QgsLineSymbol.createSimple(
            {
                "outline_color": "#00BB00",
                "outline_style": "dash",
                "outline_width": 1,
            }
        )
        plot.setLineSymbolAt(0, series_symbol)
        # remove default marker
        plot.setMarkerSymbolAt(0, None)

        # set symbols for second series
        series_symbol = QgsLineSymbol.createSimple(
            {
                "outline_color": "#BB0000",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setLineSymbolAt(1, series_symbol)
        series_symbol = QgsMarkerSymbol.createSimple(
            {
                "color": "#BB0000",
                "outline_color": "#330000",
                "outline_style": "solid",
                "outline_width": 1,
                "width": 3,
            }
        )
        plot.setMarkerSymbolAt(1, series_symbol)

        data = QgsPlotData()
        series = QgsXyPlotSeries()
        series.append(-8, 1)
        series.append(0, 5)
        series.append(4, 5)
        series.append(9, 9)
        data.addSeries(series)
        series = QgsXyPlotSeries()
        # Test data() to insure SIP conversion works well
        series.setData([(-7.0, -5.0), (1.0, -2.0), (4.0, 5.0), (8.0, 4.0)])
        self.assertEqual(
            series.data(), [(-7.0, -5.0), (1.0, -2.0), (4.0, 5.0), (8.0, 4.0)]
        )
        data.addSeries(series)

        im = QImage(width, height, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        im.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        rc.setScaleFactor(dpi / 25.4)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc, data)
        painter.end()

        assert self.image_check(
            "line_chart_plot_x_axis_value", "line_chart_plot_x_axis_value", im
        )

    def testPieChartPlotNoLabel(self):
        width = 600
        height = 500
        dpi = 96

        plot = QgsPieChartPlot()
        plot.setSize(QSizeF(width, height))

        # set symbol for first series (also used in second series)
        series_symbol = QgsFillSymbol.createSimple(
            {
                "color": "#00BB00",
                "outline_color": "#003300",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setFillSymbolAt(0, series_symbol)

        series_color_ramp = QgsPresetSchemeColorRamp(
            [
                QColor(255, 0, 0),
                QColor(0, 255, 0),
                QColor(0, 0, 255),
                QColor(150, 150, 150),
            ]
        )
        plot.setColorRampAt(0, series_color_ramp)

        data = QgsPlotData()
        series = QgsXyPlotSeries()
        series.append(0, 1)
        series.append(1, 5)
        series.append(2, 5)
        series.append(3, 9)
        data.addSeries(series)
        series = QgsXyPlotSeries()
        series.append(0, 5)
        series.append(1, 2)
        series.append(2, 5)
        series.append(3, 4)
        data.addSeries(series)
        data.setCategories(["Q1", "Q2", "Q3", "Q4"])

        im = QImage(width, height, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        im.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        rc.setScaleFactor(dpi / 25.4)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc, data)
        painter.end()

        assert self.image_check(
            "pie_chart_plot_no_label", "pie_chart_plot_no_label", im
        )

    def testPieChartPlotCategoryLabels(self):
        width = 900
        height = 300
        dpi = 96

        plot = QgsPieChartPlot()
        plot.setSize(QSizeF(width, height))

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        text_format = QgsTextFormat.fromQFont(font)
        plot.setTextFormat(text_format)
        plot.setLabelType(QgsPieChartPlot.LabelType.CategoryLabels)

        # set symbol for first series (also used in second series)
        series_symbol = QgsFillSymbol.createSimple(
            {
                "color": "#00BB00",
                "outline_color": "#003300",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setFillSymbolAt(0, series_symbol)

        series_color_ramp = QgsPresetSchemeColorRamp(
            [
                QColor(255, 0, 0),
                QColor(0, 255, 0),
                QColor(0, 0, 255),
                QColor(150, 150, 150),
            ]
        )
        plot.setColorRampAt(0, series_color_ramp)

        data = QgsPlotData()
        series = QgsXyPlotSeries()
        series.append(0, 1)
        series.append(1, 5)
        series.append(2, 5)
        series.append(3, 9)
        data.addSeries(series)
        series = QgsXyPlotSeries()
        series.append(0, 5)
        series.append(1, 2)
        series.append(2, 5)
        series.append(3, 4)
        data.addSeries(series)
        data.setCategories(["Q1", "Q2", "Q3", "Q4"])

        im = QImage(width, height, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        im.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        rc.setScaleFactor(dpi / 25.4)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc, data)
        painter.end()

        assert self.image_check(
            "pie_chart_plot_category_labels", "pie_chart_plot_category_labels", im
        )

    def testPieChartPlotValueLabels(self):
        width = 900
        height = 300
        dpi = 96

        plot = QgsPieChartPlot()
        plot.setSize(QSizeF(width, height))

        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        text_format = QgsTextFormat.fromQFont(font)
        plot.setTextFormat(text_format)
        plot.setLabelType(QgsPieChartPlot.LabelType.ValueLabels)

        # set symbol for first series (also used in second series)
        series_symbol = QgsFillSymbol.createSimple(
            {
                "color": "#00BB00",
                "outline_color": "#003300",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setFillSymbolAt(0, series_symbol)

        series_color_ramp = QgsPresetSchemeColorRamp(
            [
                QColor(255, 0, 0),
                QColor(0, 255, 0),
                QColor(0, 0, 255),
                QColor(150, 150, 150),
            ]
        )
        plot.setColorRampAt(0, series_color_ramp)

        data = QgsPlotData()
        series = QgsXyPlotSeries()
        series.append(0, 1)
        series.append(1, 5)
        series.append(2, 5)
        series.append(3, 9)
        data.addSeries(series)
        series = QgsXyPlotSeries()
        series.append(0, 5)
        series.append(1, 2)
        series.append(2, 5)
        series.append(3, 4)
        data.addSeries(series)
        data.setCategories(["Q1", "Q2", "Q3", "Q4"])

        im = QImage(width, height, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.white)
        im.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        im.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        painter = QPainter(im)
        rc = QgsRenderContext.fromQPainter(painter)
        rc.setScaleFactor(dpi / 25.4)
        prc = QgsPlotRenderContext()
        plot.render(rc, prc, data)
        painter.end()

        assert self.image_check(
            "pie_chart_plot_value_labels", "pie_chart_plot_value_labels", im
        )


if __name__ == "__main__":
    unittest.main()
