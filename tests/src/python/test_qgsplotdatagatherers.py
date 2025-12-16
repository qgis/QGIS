"""QGIS Unit tests for various QgsPlotData gatherers

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Mathieu Pellerin"
__date__ = "22/8/2025"
__copyright__ = "Copyright 2025, The QGIS Project"

from qgis.PyQt.QtCore import QEventLoop
from qgis.core import (
    QgsApplication,
    QgsExpressionContext,
    QgsExpressionContextUtils,
    QgsFeature,
    QgsFeatureRequest,
    QgsPlotData,
    QgsTaskManager,
    QgsVectorLayer,
    QgsVectorLayerXyPlotDataGatherer,
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

    def testCategorical(self):
        loop = QEventLoop()

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

        expression_context = QgsExpressionContext(
            QgsExpressionContextUtils.globalProjectLayerScopes(layer)
        )

        # test a single series
        series1_details = QgsVectorLayerXyPlotDataGatherer.XySeriesDetails(
            '"category"',
            '"value"',
        )

        iterator = layer.getFeatures()
        gatherer = QgsVectorLayerXyPlotDataGatherer(Qgis.PlotAxisType.Categorical)
        gatherer.setFeatureIterator(iterator)
        gatherer.setExpressionContext(expression_context)
        gatherer.setSeriesDetails([series1_details])
        gatherer.taskCompleted.connect(loop.quit)

        QgsApplication.taskManager().addTask(gatherer)
        loop.exec()
        data = gatherer.data()
        self.assertEqual(data.categories(), ["category_a", "category_b", "category_c"])
        self.assertEqual(len(data.series()), 1)
        self.assertEqual(data.series()[0].data(), [(0, 21.0), (1, 11.0), (2, 3.0)])

        # test two series with the second series filtered and missing one category
        series2_details = QgsVectorLayerXyPlotDataGatherer.XySeriesDetails(
            '"category"',
            '"value"',
            filterExpression='"category" != \'category_b\' AND "value" <= 10',
        )

        iterator = layer.getFeatures()
        gatherer = QgsVectorLayerXyPlotDataGatherer(Qgis.PlotAxisType.Categorical)
        gatherer.setFeatureIterator(iterator)
        gatherer.setExpressionContext(expression_context)
        gatherer.setSeriesDetails([series1_details, series2_details])
        gatherer.taskCompleted.connect(loop.quit)

        QgsApplication.taskManager().addTask(gatherer)
        loop.exec()
        data = gatherer.data()
        self.assertEqual(data.categories(), ["category_a", "category_b", "category_c"])
        self.assertEqual(len(data.series()), 2)
        self.assertEqual(data.series()[0].data(), [(0, 21.0), (1, 11.0), (2, 3.0)])
        self.assertEqual(data.series()[1].data(), [(0, 10.0), (2, 3.0)])

        # test a single series with predefined categories
        series1_details = QgsVectorLayerXyPlotDataGatherer.XySeriesDetails(
            '"category"',
            '"value"',
        )

        iterator = layer.getFeatures()
        gatherer = QgsVectorLayerXyPlotDataGatherer(Qgis.PlotAxisType.Categorical)
        gatherer.setFeatureIterator(iterator)
        gatherer.setExpressionContext(expression_context)
        gatherer.setSeriesDetails([series1_details])
        gatherer.setPredefinedCategories(["category_c", "category_a"])
        gatherer.taskCompleted.connect(loop.quit)

        QgsApplication.taskManager().addTask(gatherer)
        loop.exec()
        data = gatherer.data()
        self.assertEqual(data.categories(), ["category_c", "category_a"])
        self.assertEqual(len(data.series()), 1)
        self.assertEqual(data.series()[0].data(), [(0, 3.0), (1, 21.0)])

    def testInterval(self):
        loop = QEventLoop()

        layer = QgsVectorLayer(
            "Point?field=int:double&field=value:double", "test", "memory"
        )
        provider = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes([1, 10.0])
        f2 = QgsFeature()
        f2.setAttributes([3, 5.0])
        f3 = QgsFeature()
        f3.setAttributes([2, 3.0])
        f4 = QgsFeature()
        f4.setAttributes([5, 6.0])
        f5 = QgsFeature()
        f5.setAttributes([4, 11.0])
        assert provider.addFeatures([f, f2, f3, f4, f5])
        assert layer.featureCount() == 5

        expression_context = QgsExpressionContext(
            QgsExpressionContextUtils.globalProjectLayerScopes(layer)
        )

        # test a single series
        series1_details = QgsVectorLayerXyPlotDataGatherer.XySeriesDetails(
            '"int"', '"value"'
        )

        iterator = layer.getFeatures()
        gatherer = QgsVectorLayerXyPlotDataGatherer(Qgis.PlotAxisType.Interval)
        gatherer.setFeatureIterator(iterator)
        gatherer.setExpressionContext(expression_context)
        gatherer.setSeriesDetails([series1_details])
        gatherer.taskCompleted.connect(loop.quit)

        QgsApplication.taskManager().addTask(gatherer)
        loop.exec()
        data = gatherer.data()
        self.assertEqual(data.categories(), [])
        self.assertEqual(len(data.series()), 1)
        self.assertEqual(
            data.series()[0].data(),
            [(1.0, 10.0), (3.0, 5.0), (2.0, 3.0), (5.0, 6.0), (4.0, 11.0)],
        )

        # test a single series, ordered by expression
        series1_details = QgsVectorLayerXyPlotDataGatherer.XySeriesDetails(
            '"int"',
            '"value"',
        )

        request = QgsFeatureRequest()
        request.addOrderBy('"int"')
        iterator = layer.getFeatures(request)
        gatherer = QgsVectorLayerXyPlotDataGatherer(Qgis.PlotAxisType.Interval)
        gatherer.setFeatureIterator(iterator)
        gatherer.setExpressionContext(expression_context)
        gatherer.setSeriesDetails([series1_details])
        gatherer.taskCompleted.connect(loop.quit)

        QgsApplication.taskManager().addTask(gatherer)
        loop.exec()
        data = gatherer.data()
        self.assertEqual(data.categories(), [])
        self.assertEqual(len(data.series()), 1)
        self.assertEqual(
            data.series()[0].data(),
            [(1.0, 10.0), (2.0, 3.0), (3.0, 5.0), (4.0, 11.0), (5.0, 6.0)],
        )

        # test two series with the second series filtered and missing one category
        series2_details = QgsVectorLayerXyPlotDataGatherer.XySeriesDetails(
            '"int"', '"value"', filterExpression='"int" < 3'
        )

        iterator = layer.getFeatures(request)
        gatherer = QgsVectorLayerXyPlotDataGatherer(Qgis.PlotAxisType.Interval)
        gatherer.setFeatureIterator(iterator)
        gatherer.setExpressionContext(expression_context)
        gatherer.setSeriesDetails([series1_details, series2_details])
        gatherer.taskCompleted.connect(loop.quit)

        QgsApplication.taskManager().addTask(gatherer)
        loop.exec()
        data = gatherer.data()
        self.assertEqual(data.categories(), [])
        self.assertEqual(len(data.series()), 2)
        self.assertEqual(
            data.series()[0].data(),
            [(1.0, 10.0), (2.0, 3.0), (3.0, 5.0), (4.0, 11.0), (5.0, 6.0)],
        )
        self.assertEqual(data.series()[1].data(), [(1.0, 10.0), (2.0, 3.0)])


if __name__ == "__main__":
    unittest.main()
