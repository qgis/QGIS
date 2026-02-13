"""QGIS Unit tests for QgsLayoutItemChart.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2025 by Nyall Dawson"
__date__ = "01/09/2025"
__copyright__ = "Copyright 2025, The QGIS Project"

import os
import tempfile
import unittest

from qgis.core import (
    Qgis,
    QgsBarChartPlot,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFillSymbol,
    QgsFontUtils,
    QgsLayout,
    QgsLayoutItemChart,
    QgsLineChartPlot,
    QgsLineSymbol,
    QgsMarkerSymbol,
    QgsPrintLayout,
    QgsProject,
    QgsReadWriteContext,
    QgsTextFormat,
    QgsVectorLayer,
)
from qgis.PyQt.QtCore import QRectF, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import QgisTestCase, start_app
from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemElevationProfile(QgisTestCase, LayoutItemTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "layout_chart"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemChart

    def test_bar_chart(self):
        """
        Test rendering a bar chart with X axis set to categorical
        """

        layer = QgsVectorLayer(
            "Point?field=category:string&field=value:double", "test", "memory"
        )
        provider = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["category_a", 10.0])
        f2 = QgsFeature()
        f2.setAttributes(["category_b", 5.0])
        f3 = QgsFeature()
        f3.setAttributes(["category_c", 3.0])
        f4 = QgsFeature()
        f4.setAttributes(["category_b", 6.0])
        f5 = QgsFeature()
        f5.setAttributes(["category_a", 11.0])
        assert provider.addFeatures([f, f2, f3, f4, f5])
        assert layer.featureCount() == 5

        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        chart_item = QgsLayoutItemChart(layout)
        layout.addLayoutItem(chart_item)
        chart_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        plot = QgsBarChartPlot()

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

        plot.xAxis().setType(Qgis.PlotAxisType.Categorical)

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        plot.xAxis().setTextFormat(format)

        plot.setYMaximum(50)
        plot.yAxis().setGridIntervalMajor(10)
        plot.yAxis().setGridIntervalMinor(5)

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        plot.yAxis().setTextFormat(format)
        plot.yAxis().setLabelInterval(10)

        # set bar series symbol
        series_symbol = QgsFillSymbol.createSimple(
            {
                "color": "#00BB00",
                "outline_color": "#003300",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setFillSymbolAt(0, series_symbol)

        chart_item.setPlot(plot)

        series_details = QgsLayoutItemChart.SeriesDetails("Series 1")
        series_details.setXExpression('"category"')
        series_details.setYExpression('"value"')
        chart_item.setSeriesList([series_details])

        chart_item.setSourceLayer(layer)
        chart_item.setSortFeatures(True)
        chart_item.setSortExpression('"category"')
        chart_item.setSortAscending(False)

        self.assertTrue(self.render_layout_check("bar_chart", layout))

    def test_line_chart(self):
        """
        Test rendering a bar chart with X axis set to interval
        """

        layer = QgsVectorLayer(
            "Point?field=int:integer&field=value:double", "test", "memory"
        )
        provider = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes([-4, 9.0])
        f2 = QgsFeature()
        f2.setAttributes([-1, 5.0])
        f3 = QgsFeature()
        f3.setAttributes([2, 3.0])
        f4 = QgsFeature()
        f4.setAttributes([5, 6.0])
        f5 = QgsFeature()
        f5.setAttributes([4, 2.0])
        assert provider.addFeatures([f, f2, f3, f4, f5])
        assert layer.featureCount() == 5

        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        chart_item = QgsLayoutItemChart(layout)
        layout.addLayoutItem(chart_item)
        chart_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        plot = QgsLineChartPlot()

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

        plot.xAxis().setType(Qgis.PlotAxisType.Interval)
        plot.setXMinimum(-6)
        plot.setXMaximum(6)
        plot.yAxis().setGridIntervalMajor(2)
        plot.yAxis().setGridIntervalMinor(1)

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        plot.xAxis().setTextFormat(format)
        plot.xAxis().setLabelInterval(1)

        plot.setYMaximum(10)
        plot.yAxis().setGridIntervalMajor(2)
        plot.yAxis().setGridIntervalMinor(1)

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        plot.yAxis().setTextFormat(format)
        plot.yAxis().setLabelInterval(1)

        # set line series symbols
        series_symbol = QgsLineSymbol.createSimple(
            {
                "outline_color": "#BB0000",
                "outline_style": "solid",
                "outline_width": 1,
            }
        )
        plot.setLineSymbolAt(0, series_symbol)
        series_symbol = QgsMarkerSymbol.createSimple(
            {
                "color": "#BB0000",
                "outline_color": "#330000",
                "outline_style": "solid",
                "outline_width": 1,
                "width": 3,
            }
        )
        plot.setMarkerSymbolAt(0, series_symbol)

        chart_item.setPlot(plot)

        series_details = QgsLayoutItemChart.SeriesDetails("Series 1")
        series_details.setXExpression('"int"')
        series_details.setYExpression('"value"')
        chart_item.setSeriesList([series_details])

        chart_item.setSourceLayer(layer)
        chart_item.setSortFeatures(True)
        chart_item.setSortExpression('"int"')
        chart_item.setSortAscending(True)

        self.assertTrue(self.render_layout_check("line_chart", layout))

    def test_read_write(self):
        layer = QgsVectorLayer(
            "Point?field=int:integer&field=value:double", "test_read_write", "memory"
        )
        QgsProject.instance().addMapLayer(layer)

        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        chart_item = QgsLayoutItemChart(layout)
        layout.addLayoutItem(chart_item)

        series_details = QgsLayoutItemChart.SeriesDetails("Series 1")
        series_details.setXExpression('"int"')
        series_details.setYExpression('"value"')
        series_details.setFilterExpression('"value" > 0')
        chart_item.setSeriesList([series_details])

        chart_item.setSourceLayer(layer)
        chart_item.setSortFeatures(True)
        chart_item.setSortExpression('"int"')
        chart_item.setSortAscending(False)

        doc = QDomDocument()
        elem = doc.createElement("test")
        chart_item.writePropertiesToElement(elem, doc, QgsReadWriteContext())

        chart_item_restored = QgsLayoutItemChart(layout)
        chart_item_restored.readPropertiesFromElement(elem, doc, QgsReadWriteContext())

        self.assertTrue(
            chart_item_restored.readPropertiesFromElement(
                elem, doc, QgsReadWriteContext()
            )
        )

        self.assertEqual(len(chart_item_restored.seriesList()), 1)
        self.assertEqual(chart_item_restored.seriesList()[0].xExpression(), '"int"')
        self.assertEqual(chart_item_restored.seriesList()[0].yExpression(), '"value"')
        self.assertEqual(
            chart_item_restored.seriesList()[0].filterExpression(), '"value" > 0'
        )

        self.assertTrue(chart_item_restored.sourceLayer())
        self.assertEqual(chart_item_restored.sourceLayer().name(), "test_read_write")
        self.assertTrue(chart_item_restored.sortFeatures())
        self.assertEqual(chart_item_restored.sortExpression(), '"int"')
        self.assertFalse(chart_item_restored.sortAscending())


if __name__ == "__main__":
    unittest.main()
