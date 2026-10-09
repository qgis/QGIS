"""QGIS Unit tests for QgsMeshLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import tempfile
import unittest

from qgis.core import (
    Qgis,
    QgsColorRampShader,
    QgsGradientColorRamp,
    QgsMapRendererSequentialJob,
    QgsMapSettings,
    QgsMeshDataBlock,
    QgsMeshDataset,
    QgsMeshDatasetGroup,
    QgsMeshDatasetGroupMetadata,
    QgsMeshDatasetIndex,
    QgsMeshDatasetMetadata,
    QgsMeshDatasetValue,
    QgsMeshLayer,
    QgsProject,
)
from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomElement
from qgis.testing import QgisTestCase, start_app

start_app()


class MemoryValuesDataset(QgsMeshDataset):
    """Dataset backed by a mutable Python list, so tests can change its
    values in place after it has already been rendered."""

    def __init__(self, group):
        super().__init__()
        self._group = group

    def datasetValues(self, isScalar, valueIndex, count):
        block = QgsMeshDataBlock(QgsMeshDataBlock.ScalarDouble, count)
        block.setValues(self._group.values[valueIndex : valueIndex + count])
        block.setValid(True)
        return block

    def datasetValue(self, valueIndex):
        return QgsMeshDatasetValue(self._group.values[valueIndex])

    def areFacesActive(self, faceIndex, count):
        block = QgsMeshDataBlock(QgsMeshDataBlock.ActiveFlagInteger, count)
        block.setValid(True)
        return block

    def isActive(self, faceIndex):
        return True

    def metadata(self):
        return QgsMeshDatasetMetadata(
            0.0, True, min(self._group.values), max(self._group.values), 0
        )

    def valuesCount(self):
        return len(self._group.values)


class MemoryValuesDatasetGroup(QgsMeshDatasetGroup):
    """Dataset group whose values can be mutated after being rendered, to
    check that the renderer cache is only refreshed on demand."""

    def __init__(self, name, values):
        super().__init__(name, QgsMeshDatasetGroupMetadata.DataOnVertices)
        self.values = values
        self._dataset = MemoryValuesDataset(self)

    def initialize(self):
        pass

    def datasetCount(self):
        return 1

    def dataset(self, index):
        return self._dataset

    def datasetMetadata(self, datasetIndex):
        return self._dataset.metadata()

    def type(self):
        return QgsMeshDatasetGroup.Memory

    def writeXml(self, doc, context):
        return QDomElement()


class TestQgsMeshLayer(QgisTestCase):
    def test_dataset_group_metadata(self):
        """
        Test datasetGroupMetadata
        """
        layer = QgsMeshLayer(
            self.get_test_data_path("mesh/netcdf_parent_quantity.nc").as_posix(),
            "mesh",
            "mdal",
        )
        self.assertTrue(layer.isValid())

        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(0)).name(),
            "air_temperature_height:10",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(0)).parentQuantityName(),
            "air_temperature_height",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(1)).name(),
            "air_temperature_height:20",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(1)).parentQuantityName(),
            "air_temperature_height",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(2)).name(),
            "air_temperature_height:30",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(2)).parentQuantityName(),
            "air_temperature_height",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(3)).name(),
            "air_temperature_height:5",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(3)).parentQuantityName(),
            "air_temperature_height",
        )
        self.assertFalse(layer.datasetGroupMetadata(QgsMeshDatasetIndex(4)).name())
        self.assertFalse(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(4)).parentQuantityName()
        )

    def test_legend_settings(self):
        ml = QgsMeshLayer(
            self.get_test_data_path("mesh/netcdf_parent_quantity.nc").as_posix(),
            "test",
            "mdal",
        )
        self.assertTrue(ml.isValid())

        self.assertFalse(ml.legend().flags() & Qgis.MapLayerLegendFlag.ExcludeByDefault)
        ml.legend().setFlag(Qgis.MapLayerLegendFlag.ExcludeByDefault)
        self.assertTrue(ml.legend().flags() & Qgis.MapLayerLegendFlag.ExcludeByDefault)

        p = QgsProject()
        p.addMapLayer(ml)

        # test saving and restoring
        with tempfile.TemporaryDirectory() as temp:
            self.assertTrue(p.write(temp + "/test.qgs"))

            p2 = QgsProject()
            self.assertTrue(p2.read(temp + "/test.qgs"))

            ml2 = list(p2.mapLayers().values())[0]
            self.assertEqual(ml2.name(), ml.name())

            self.assertTrue(
                ml2.legend().flags() & Qgis.MapLayerLegendFlag.ExcludeByDefault
            )

    def test_invalidate_renderer_cache(self):
        """
        Test that invalidateRendererCache forces the renderer to pick up
        new values for a dataset group whose values changed in place
        """
        layer = QgsMeshLayer(
            "1.0, 2.0\n2.0, 2.0\n3.0, 2.0\n2.0, 3.0\n1.0, 3.0---0, 1, 3, 4\n1, 2, 3",
            "quad and triangle",
            "mesh_memory",
        )
        self.assertTrue(layer.isValid())

        group = MemoryValuesDatasetGroup("group", [2.0] * 5)
        self.assertTrue(layer.addDatasets(group))
        group_index = layer.datasetGroupCount() - 1

        # deterministic style over a fixed [0, 10] range
        shader = QgsColorRampShader(
            0.0, 10.0, QgsGradientColorRamp(QColor(0, 0, 255), QColor(255, 0, 0))
        )
        shader.classifyColorRamp(2)
        renderer_settings = layer.rendererSettings()
        scalar_settings = renderer_settings.scalarSettings(group_index)
        scalar_settings.setClassificationMinimumMaximum(0.0, 10.0)
        scalar_settings.setColorRampShader(shader)
        renderer_settings.setScalarSettings(group_index, scalar_settings)
        renderer_settings.setActiveScalarDatasetGroup(group_index)
        layer.setRendererSettings(renderer_settings)

        map_settings = QgsMapSettings()
        map_settings.setLayers([layer])
        map_settings.setExtent(layer.extent())
        map_settings.setOutputSize(QSize(200, 200))

        def render_pixel():
            job = QgsMapRendererSequentialJob(map_settings)
            job.start()
            job.waitForFinished()
            # (50, 100) lies inside the quad, away from element edges
            return job.renderedImage().pixelColor(50, 100)

        color_low = render_pixel()

        # mutate values in place: without invalidation the cache serves
        # stale values, so the rendered color should not change yet
        group.values = [8.0] * 5
        self.assertEqual(render_pixel(), color_low)

        # invalidation forces the renderer to fetch the new values
        layer.invalidateRendererCache()
        color_high = render_pixel()
        self.assertNotEqual(color_high, color_low)


if __name__ == "__main__":
    unittest.main()
