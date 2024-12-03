"""QGIS Unit tests for QgsRenderedItemDetails

From build dir, run: ctest -R QgsRenderedItemDetails -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2021 by Nyall Dawson"
__date__ = "03/09/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import math

from qgis.core import (
    QgsRectangle,
    QgsRenderContext,
    QgsRenderedAnnotationItemDetails,
    QgsRenderedLayerStatistics,
    QgsRenderedItemDetails,
    QgsRenderedItemResults,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRenderedItemResults(QgisTestCase):

    def test_basic(self):
        details = QgsRenderedItemDetails("layer_id")
        self.assertEqual(details.layerId(), "layer_id")
        details.setBoundingBox(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(details.boundingBox(), QgsRectangle(1, 2, 3, 4))

    def test_rendered_annotation_item_details(self):
        details = QgsRenderedAnnotationItemDetails("layer_id", "item_id")
        self.assertEqual(details.layerId(), "layer_id")
        self.assertEqual(details.itemId(), "item_id")
        self.assertEqual(
            str(details), "<QgsRenderedAnnotationItemDetails: layer_id - item_id>"
        )

    def test_rendered_calculated_results(self):
        results = QgsRenderedLayerStatistics("layer_id")
        self.assertEqual(results.layerId(), "layer_id")
        self.assertEqual(len(results.minimum()), 1)
        self.assertTrue(math.isnan(results.minimum(0)))
        self.assertEqual(len(results.maximum()), 1)
        self.assertTrue(math.isnan(results.maximum(0)))

        results.setBoundingBox(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(results.boundingBox(), QgsRectangle(1, 2, 3, 4))

        results.setMinimum(0, -10.0)
        results.setMaximum(0, 10.0)
        self.assertEqual(len(results.minimum()), 1)
        self.assertEqual(results.minimum(), [-10.0])
        self.assertEqual(len(results.maximum()), 1)
        self.assertEqual(results.maximum(), [10.0])

        results.setMinimum([-10.0, -3.4])
        results.setMaximum([10.0, 5.2])
        self.assertEqual(len(results.minimum()), 2)
        self.assertEqual(results.minimum(), [-10.0, -3.4])
        self.assertEqual(len(results.maximum()), 2)
        self.assertEqual(results.maximum(), [10.0, 5.2])

        results2 = QgsRenderedLayerStatistics("layer_id", [-5.2, 3.0], [5.0, 9.3])
        self.assertEqual(results2.layerId(), "layer_id")
        self.assertEqual(len(results.minimum()), 2)
        self.assertEqual(results2.minimum(), [-5.2, 3.0])
        self.assertEqual(results2.minimum(1), 3.0)
        self.assertEqual(len(results.maximum()), 2)
        self.assertEqual(results2.maximum(1), 9.3)
        self.assertEqual(
            str(results2),
            "<QgsRenderedLayerStatistics: layer_id (min: -5.2,3 - max: 5,9.3)>",
        )

    def test_results(self):
        results = QgsRenderedItemResults()
        self.assertFalse(
            results.renderedAnnotationItemsInBounds(
                QgsRectangle(-100000000, -100000000, 100000000, 100000000)
            )
        )

        rc = QgsRenderContext()
        details1 = QgsRenderedAnnotationItemDetails("layer_id", "item_id_1")
        details1.setBoundingBox(QgsRectangle(1, 1, 10, 10))

        details2 = QgsRenderedAnnotationItemDetails("layer_id", "item_id_2")
        details2.setBoundingBox(QgsRectangle(1, 1, 5, 5))

        results.appendResults([details1, details2], rc)

        # query annotation items
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results.renderedAnnotationItemsInBounds(
                    QgsRectangle(-100000000, -100000000, 100000000, 100000000)
                )
            ],
            [("layer_id", "item_id_1"), ("layer_id", "item_id_2")],
        )
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results.renderedAnnotationItemsInBounds(
                    QgsRectangle(0, 0, 10, 10)
                )
            ],
            [("layer_id", "item_id_1"), ("layer_id", "item_id_2")],
        )
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results.renderedAnnotationItemsInBounds(
                    QgsRectangle(6, 6, 10, 10)
                )
            ],
            [("layer_id", "item_id_1")],
        )

        # add another item
        details3 = QgsRenderedAnnotationItemDetails("layer_id2", "item_id_3")
        details3.setBoundingBox(QgsRectangle(4, 4, 7, 7))
        results.appendResults([details3], rc)
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results.renderedAnnotationItemsInBounds(
                    QgsRectangle(0, 0, 10, 10)
                )
            ],
            [
                ("layer_id", "item_id_1"),
                ("layer_id", "item_id_2"),
                ("layer_id2", "item_id_3"),
            ],
        )
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results.renderedAnnotationItemsInBounds(
                    QgsRectangle(6, 6, 10, 10)
                )
            ],
            [("layer_id", "item_id_1"), ("layer_id2", "item_id_3")],
        )

    def test_transfer_results(self):
        results = QgsRenderedItemResults()
        self.assertFalse(
            results.renderedAnnotationItemsInBounds(
                QgsRectangle(-100000000, -100000000, 100000000, 100000000)
            )
        )

        rc = QgsRenderContext()
        details1 = QgsRenderedAnnotationItemDetails("layer_id", "item_id_1")
        details1.setBoundingBox(QgsRectangle(1, 1, 10, 10))

        details2 = QgsRenderedAnnotationItemDetails("layer_id2", "item_id_2")
        details2.setBoundingBox(QgsRectangle(1, 1, 5, 5))

        details3 = QgsRenderedAnnotationItemDetails("layer_id3", "item_id_3")
        details3.setBoundingBox(QgsRectangle(4, 4, 7, 7))

        results.appendResults([details1, details2, details3], rc)

        results2 = QgsRenderedItemResults()
        # transfer some results to results2
        # first no layers specified => nothing should be transferred
        results2.transferResults(results, [])
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results.renderedAnnotationItemsInBounds(
                    QgsRectangle(0, 0, 10, 10)
                )
            ],
            [
                ("layer_id", "item_id_1"),
                ("layer_id2", "item_id_2"),
                ("layer_id3", "item_id_3"),
            ],
        )
        self.assertFalse(
            results2.renderedAnnotationItemsInBounds(QgsRectangle(0, 0, 10, 10))
        )

        results2.transferResults(results, ["layer_id2", "layer_id3"])
        self.assertEqual(
            [(i.layerId(), i.itemId()) for i in results.renderedItems()],
            [("layer_id", "item_id_1")],
        )
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results2.renderedAnnotationItemsInBounds(
                    QgsRectangle(0, 0, 10, 10)
                )
            ],
            [("layer_id2", "item_id_2"), ("layer_id3", "item_id_3")],
        )
        # same layers, nothing should happen
        results2.transferResults(results, ["layer_id2", "layer_id3"])
        self.assertEqual(
            [(i.layerId(), i.itemId()) for i in results.renderedItems()],
            [("layer_id", "item_id_1")],
        )
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results2.renderedAnnotationItemsInBounds(
                    QgsRectangle(0, 0, 10, 10)
                )
            ],
            [("layer_id2", "item_id_2"), ("layer_id3", "item_id_3")],
        )
        # remaining layer
        results2.transferResults(results, ["layer_id"])
        self.assertFalse([(i.layerId(), i.itemId()) for i in results.renderedItems()])
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results2.renderedAnnotationItemsInBounds(
                    QgsRectangle(0, 0, 10, 10)
                )
            ],
            [
                ("layer_id", "item_id_1"),
                ("layer_id2", "item_id_2"),
                ("layer_id3", "item_id_3"),
            ],
        )

        # transfer all results
        results3 = QgsRenderedItemResults()
        # nothing should happen -- no results in results3 to transfer
        results2.transferResults(results3)
        self.assertFalse(
            results3.renderedAnnotationItemsInBounds(QgsRectangle(0, 0, 10, 10))
        )
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results2.renderedAnnotationItemsInBounds(
                    QgsRectangle(0, 0, 10, 10)
                )
            ],
            [
                ("layer_id", "item_id_1"),
                ("layer_id2", "item_id_2"),
                ("layer_id3", "item_id_3"),
            ],
        )

        # transfer all results from results2 to results3
        results3.transferResults(results2)
        self.assertFalse(results2.renderedItems())
        self.assertCountEqual(
            [
                (i.layerId(), i.itemId())
                for i in results3.renderedAnnotationItemsInBounds(
                    QgsRectangle(0, 0, 10, 10)
                )
            ],
            [
                ("layer_id", "item_id_1"),
                ("layer_id2", "item_id_2"),
                ("layer_id3", "item_id_3"),
            ],
        )

    def test_erase_results(self):
        results = QgsRenderedItemResults()

        rc = QgsRenderContext()
        details1 = QgsRenderedAnnotationItemDetails("layer_id", "item_id_1")
        details1.setBoundingBox(QgsRectangle(1, 1, 10, 10))

        details2 = QgsRenderedAnnotationItemDetails("layer_id2", "item_id_2")
        details2.setBoundingBox(QgsRectangle(1, 1, 5, 5))

        details3 = QgsRenderedAnnotationItemDetails("layer_id3", "item_id_3")
        details3.setBoundingBox(QgsRectangle(4, 4, 7, 7))

        results.appendResults([details1, details2, details3], rc)

        self.assertCountEqual(
            [(i.layerId(), i.itemId()) for i in results.renderedItems()],
            [
                ("layer_id", "item_id_1"),
                ("layer_id2", "item_id_2"),
                ("layer_id3", "item_id_3"),
            ],
        )

        results.eraseResultsFromLayers([])
        self.assertCountEqual(
            [(i.layerId(), i.itemId()) for i in results.renderedItems()],
            [
                ("layer_id", "item_id_1"),
                ("layer_id2", "item_id_2"),
                ("layer_id3", "item_id_3"),
            ],
        )

        results.eraseResultsFromLayers(["layer_id2", "layer_id3"])
        self.assertCountEqual(
            [(i.layerId(), i.itemId()) for i in results.renderedItems()],
            [("layer_id", "item_id_1")],
        )
        results.eraseResultsFromLayers(["layer_id2", "layer_id"])
        self.assertFalse(results.renderedItems())


if __name__ == "__main__":
    unittest.main()
