"""QGIS Unit tests for QgsPlotRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Mathieu Pellerin"
__date__ = "20/08/2025"
__copyright__ = "Copyright 2025, The QGIS Project"


from qgis.core import (
    Qgs2DPlot,
    QgsPlotRegistry,
    QgsPlotAbstractMetadata,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestPlotAMetadata(QgsPlotAbstractMetadata):

    def __init__(self):
        super().__init__("test_plot_a", "test plot a")

    def createPlot(self):
        return Qgs2DPlot()


class TestPlotBMetadata(QgsPlotAbstractMetadata):

    def __init__(self):
        super().__init__("test_plot_b", "test plot b")

    def createPlot(self):
        return Qgs2DPlot()


class TestQgsPlotRegistry(QgisTestCase):

    def testRegistry(self):
        registry = QgsPlotRegistry()

        registry.addPlotType(TestPlotAMetadata())
        registry.addPlotType(TestPlotBMetadata())
        self.assertEqual(
            registry.plotTypes(),
            {
                "test_plot_a": "test plot a",
                "test_plot_b": "test plot b",
            },
        )

        plot = registry.createPlot("test_plot_a")
        self.assertTrue(plot)

        plot = registry.createPlot("test_plot_b")
        self.assertTrue(plot)

        plot = registry.createPlot("invalid_plot")
        self.assertFalse(plot)

        registry.removePlotType("test_plot_a")
        self.assertEqual(registry.plotTypes(), {"test_plot_b": "test plot b"})


if __name__ == "__main__":
    unittest.main()
