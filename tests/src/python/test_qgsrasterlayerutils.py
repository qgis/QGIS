"""QGIS Unit tests for QgsRasterLayerUtils

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os

from qgis.PyQt.QtCore import (
    QDate,
    QTime,
    QDateTime
)
from qgis.core import (
    Qgis,
    QgsRasterLayerUtils,
    QgsRasterLayer,
    QgsDoubleRange,
    QgsDateTimeRange
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterLayerUtils(QgisTestCase):

    def test_rendered_band_for_elevation_and_temporal_ranges(self):
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, 'landsat_4326.tif'))
        self.assertTrue(raster_layer.isValid())

        # no temporal or elevation properties
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2023, 1, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2023, 12, 31),
                          QTime(23, 59, 59))
            ),
            QgsDoubleRange(30, 50)
        )
        self.assertEqual(band, -1)
        self.assertTrue(matched)

        # only elevation properties enabled
        raster_layer.elevationProperties().setEnabled(True)
        raster_layer.elevationProperties().setMode(
            Qgis.RasterElevationMode.FixedRangePerBand
        )
        raster_layer.elevationProperties().setFixedRangePerBand(
            {1: QgsDoubleRange(1, 5),
             2: QgsDoubleRange(4, 10),
             3: QgsDoubleRange(11, 15),
             4: QgsDoubleRange(1, 5),
             5: QgsDoubleRange(4, 10),
             6: QgsDoubleRange(11, 16),
             7: QgsDoubleRange(1, 5),
             8: QgsDoubleRange(4, 10),
             9: QgsDoubleRange(11, 15),
             }
        )
        # no matching elevation
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2023, 1, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2023, 12, 31),
                          QTime(23, 59, 59))
            ),
            QgsDoubleRange(30, 50)
        )
        self.assertEqual(band, -1)
        self.assertFalse(matched)
        # matching elevation
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2023, 1, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2023, 12, 31),
                          QTime(23, 59, 59))
            ),
            QgsDoubleRange(1, 3)
        )
        self.assertEqual(band, 7)
        self.assertTrue(matched)
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2023, 1, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2023, 12, 31),
                          QTime(23, 59, 59))
            ),
            QgsDoubleRange(5, 8)
        )
        self.assertEqual(band, 8)
        self.assertTrue(matched)

        # specify infinite elevation range
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2023, 1, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2023, 12, 31),
                          QTime(23, 59, 59))
            ),
            QgsDoubleRange()
        )
        self.assertEqual(band, -1)
        self.assertTrue(matched)

        # only temporal properties enabled
        raster_layer.elevationProperties().setEnabled(False)
        raster_layer.temporalProperties().setIsActive(True)
        raster_layer.temporalProperties().setMode(
            Qgis.RasterTemporalMode.FixedRangePerBand
        )
        raster_layer.temporalProperties().setFixedRangePerBand(
            {
                1: QgsDateTimeRange(
                    QDateTime(QDate(2020, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2020, 12, 31),
                              QTime(23, 59, 59))
                ),
                2: QgsDateTimeRange(
                    QDateTime(QDate(2020, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2020, 12, 31),
                              QTime(23, 59, 59))
                ),
                3: QgsDateTimeRange(
                    QDateTime(QDate(2020, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2020, 12, 31),
                              QTime(23, 59, 59))
                ),
                4: QgsDateTimeRange(
                    QDateTime(QDate(2021, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2021, 12, 31),
                              QTime(23, 59, 59))
                ),
                5: QgsDateTimeRange(
                    QDateTime(QDate(2021, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2021, 12, 31),
                              QTime(23, 59, 59))
                ),
                6: QgsDateTimeRange(
                    QDateTime(QDate(2021, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2021, 12, 31),
                              QTime(23, 59, 59))
                ),
                7: QgsDateTimeRange(
                    QDateTime(QDate(2022, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2022, 12, 31),
                              QTime(23, 59, 59))
                ),
                8: QgsDateTimeRange(
                    QDateTime(QDate(2022, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2022, 12, 31),
                              QTime(23, 59, 59))
                ),
                9: QgsDateTimeRange(
                    QDateTime(QDate(2022, 1, 1),
                              QTime(0, 0, 0)),
                    QDateTime(QDate(2022, 12, 31),
                              QTime(23, 59, 59))
                )
            }
        )

        # no matching time range
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2023, 1, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2023, 12, 31),
                          QTime(23, 59, 59))
            ),
            QgsDoubleRange(30, 50)
        )
        self.assertEqual(band, -1)
        self.assertFalse(matched)
        # matching time range
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2020, 1, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2020, 6, 30),
                          QTime(23, 59, 59))
            ),
            QgsDoubleRange(1, 3)
        )
        self.assertEqual(band, 3)
        self.assertTrue(matched)
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2020, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2021, 6, 30),
                          QTime(23, 59, 59))
            ),
            QgsDoubleRange(5, 8)
        )
        self.assertEqual(band, 6)
        self.assertTrue(matched)

        # specify infinite temporal range
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(QDateTime(), QDateTime()),
            QgsDoubleRange(5, 8)
        )
        self.assertEqual(band, -1)
        self.assertTrue(matched)

        # with both elevation and temporal handling enabled
        raster_layer.elevationProperties().setEnabled(True)

        # specify infinite temporal range
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(QDateTime(), QDateTime()),
            QgsDoubleRange(5, 8)
        )
        self.assertEqual(band, 8)
        self.assertTrue(matched)

        # specify infinite elevation range
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2020, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2021, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange()
        )
        self.assertEqual(band, 6)
        self.assertTrue(matched)

        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2020, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2020, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(2, 3)
        )
        self.assertEqual(band, 1)
        self.assertTrue(matched)
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2020, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2020, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(3, 7)
        )
        self.assertEqual(band, 2)
        self.assertTrue(matched)
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2020, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2020, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(11,
                           13)
        )
        self.assertEqual(band, 3)
        self.assertTrue(matched)

        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2021, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2021, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(2, 3)
        )
        self.assertEqual(band, 4)
        self.assertTrue(matched)
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2021, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2021, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(3, 7)
        )
        self.assertEqual(band, 5)
        self.assertTrue(matched)
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2021, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2021, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(11,
                           13)
        )
        self.assertEqual(band, 6)
        self.assertTrue(matched)

        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2022, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2022, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(2, 3)
        )
        self.assertEqual(band, 7)
        self.assertTrue(matched)
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2022, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2022, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(3, 7)
        )
        self.assertEqual(band, 8)
        self.assertTrue(matched)
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2022, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2022, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(11,
                           13)
        )
        self.assertEqual(band, 9)
        self.assertTrue(matched)

        # outside temporal range
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2023, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2023, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(11,
                           13)
        )
        self.assertEqual(band, -1)
        self.assertFalse(matched)

        # outside elevation range
        band, matched = QgsRasterLayerUtils.renderedBandForElevationAndTemporalRange(
            raster_layer,
            QgsDateTimeRange(
                QDateTime(QDate(2022, 6, 1),
                          QTime(0, 0, 0)),
                QDateTime(QDate(2022, 6, 30),
                          QTime(23, 59, 59))),
            QgsDoubleRange(111,
                           113)
        )
        self.assertEqual(band, -1)
        self.assertFalse(matched)


if __name__ == '__main__':
    unittest.main()
