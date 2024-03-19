"""QGIS Unit tests for QgsRasterLayerTemporalProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.PyQt.QtCore import (
    QDate,
    QTime,
    QDateTime
)
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsRasterLayerTemporalProperties,
    QgsReadWriteContext,
    QgsDateTimeRange
)
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsRasterLayerTemporalProperties(QgisTestCase):

    def test_basic_fixed_range(self):
        """
        Basic tests for the class using the FixedTemporalRange mode
        """
        props = QgsRasterLayerTemporalProperties(None)
        self.assertTrue(props.fixedTemporalRange().isInfinite())

        props.setIsActive(True)
        props.setMode(Qgis.RasterTemporalMode.FixedTemporalRange)
        props.setFixedTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 6), QTime(12, 13, 14)),
            QDateTime(QDate(2023, 7, 3), QTime(1, 3, 4))))
        self.assertEqual(props.fixedTemporalRange(),
                         QgsDateTimeRange(
                             QDateTime(QDate(2023, 5, 6), QTime(12, 13, 14)),
                             QDateTime(QDate(2023, 7, 3), QTime(1, 3, 4))))
        self.assertFalse(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2022, 5, 6), QTime(12, 13, 14)),
            QDateTime(QDate(2022, 7, 3), QTime(1, 3, 4)))))
        self.assertTrue(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 1, 6), QTime(12, 13, 14)),
            QDateTime(QDate(2023, 6, 3), QTime(1, 3, 4)))))
        self.assertTrue(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 6, 6), QTime(12, 13, 14)),
            QDateTime(QDate(2023, 9, 3), QTime(1, 3, 4)))))
        self.assertFalse(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2024, 5, 6), QTime(12, 13, 14)),
            QDateTime(QDate(2024, 7, 3), QTime(1, 3, 4)))))
        self.assertEqual(props.allTemporalRanges(None),
                         [QgsDateTimeRange(
                             QDateTime(QDate(2023, 5, 6), QTime(12, 13, 14)),
                             QDateTime(QDate(2023, 7, 3), QTime(1, 3, 4)))])

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerTemporalProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.FixedElevationRange)
        self.assertEqual(props2.fixedTemporalRange(),
                         QgsDateTimeRange(
                             QDateTime(QDate(2023, 5, 6), QTime(12, 13, 14)),
                             QDateTime(QDate(2023, 7, 3), QTime(1, 3, 4))))

    def test_basic_fixed_range_per_band(self):
        """
        Basic tests for the class using the FixedRangePerBand mode
        """
        props = QgsRasterLayerTemporalProperties(None)
        props.setIsActive(True)
        self.assertFalse(props.fixedRangePerBand())

        props.setMode(Qgis.RasterTemporalMode.FixedRangePerBand)
        props.setFixedRangePerBand({
            1: QgsDateTimeRange(
                QDateTime(QDate(2023, 5, 6), QTime(12, 13, 14)),
                QDateTime(QDate(2023, 5, 8), QTime(12, 13, 14))
            ),
            2: QgsDateTimeRange(
                QDateTime(QDate(2023, 5, 7),
                          QTime(12, 13, 14)),
                QDateTime(QDate(2023, 5, 9),
                          QTime(12, 13, 14))
            ),
            3: QgsDateTimeRange(
                QDateTime(QDate(2023, 5, 9),
                          QTime(12, 13, 14)),
                QDateTime(QDate(2023, 5, 11),
                          QTime(12, 13, 14))
            )})
        self.assertEqual(props.fixedRangePerBand(),
                         {
                             1: QgsDateTimeRange(
                                 QDateTime(QDate(2023, 5, 6),
                                           QTime(12, 13, 14)),
                                 QDateTime(QDate(2023, 5, 8),
                                           QTime(12, 13, 14))
                             ),
                             2: QgsDateTimeRange(
                                 QDateTime(QDate(2023, 5, 7),
                                           QTime(12, 13, 14)),
                                 QDateTime(QDate(2023, 5, 9),
                                           QTime(12, 13, 14))
                             ),
                             3: QgsDateTimeRange(
                                 QDateTime(QDate(2023, 5, 9),
                                           QTime(12, 13, 14)),
                                 QDateTime(QDate(2023, 5, 11),
                                           QTime(12, 13, 14))
                             )})
        self.assertFalse(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 1),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 3),
                      QTime(12, 13, 14))
        )))
        self.assertTrue(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 5),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 7),
                      QTime(12, 13, 14))
        )))
        self.assertTrue(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 8),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 11),
                      QTime(12, 13, 14))
        )))
        self.assertTrue(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 10),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 13),
                      QTime(12, 13, 14))
        )))
        self.assertFalse(props.isVisibleInTemporalRange(QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 12),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 14),
                      QTime(12, 13, 14))
        )))
        self.assertEqual(props.allTemporalRanges(None),
                         [QgsDateTimeRange(
                             QDateTime(QDate(2023, 5, 6),
                                       QTime(12, 13, 14)),
                             QDateTime(QDate(2023, 5, 8),
                                       QTime(12, 13, 14))
                         ),
                             QgsDateTimeRange(
                                 QDateTime(QDate(2023, 5, 7),
                                           QTime(12, 13, 14)),
                                 QDateTime(QDate(2023, 5, 9),
                                           QTime(12, 13, 14))
                         ),
                             QgsDateTimeRange(
                                 QDateTime(QDate(2023, 5, 9),
                                           QTime(12, 13, 14)),
                                 QDateTime(QDate(2023, 5, 11),
                                           QTime(12, 13, 14))
                         )])

        self.assertEqual(props.bandForTemporalRange(None, QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 3),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 4),
                      QTime(12, 13, 14))
        )), -1)
        self.assertEqual(props.bandForTemporalRange(None, QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 3),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 6),
                      QTime(12, 14, 14))
        )), 1)
        self.assertEqual(props.bandForTemporalRange(None, QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 3),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 8),
                      QTime(12, 14, 14))
        )), 2)
        self.assertEqual(props.bandForTemporalRange(None, QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 10),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 12),
                      QTime(12, 14, 14))
        )), 3)
        self.assertEqual(props.bandForTemporalRange(None, QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 13),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 16),
                      QTime(12, 14, 14))
        )), -1)

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerTemporalProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(),
                         Qgis.RasterTemporalMode.FixedRangePerBand)
        self.assertEqual(props2.fixedRangePerBand(),
                         {
                             1: QgsDateTimeRange(
                                 QDateTime(QDate(2023, 5, 6),
                                           QTime(12, 13, 14)),
                                 QDateTime(QDate(2023, 5, 8),
                                           QTime(12, 13, 14))
                             ),
                             2: QgsDateTimeRange(
                                 QDateTime(QDate(2023, 5, 7),
                                           QTime(12, 13, 14)),
                                 QDateTime(QDate(2023, 5, 9),
                                           QTime(12, 13, 14))
                             ),
                             3: QgsDateTimeRange(
                                 QDateTime(QDate(2023, 5, 9),
                                           QTime(12, 13, 14)),
                                 QDateTime(QDate(2023, 5, 11),
                                           QTime(12, 13, 14))
                             )})

        # include lower, exclude upper
        props.setFixedRangePerBand({1: QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 9),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 11),
                      QTime(12, 13, 14)),
            includeBeginning=True,
            includeEnd=False)})
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerTemporalProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.fixedRangePerBand(),
                         {1: QgsDateTimeRange(
                             QDateTime(QDate(2023, 5, 9),
                                       QTime(12, 13, 14)),
                             QDateTime(QDate(2023, 5, 11),
                                       QTime(12, 13, 14)),
                             includeBeginning=True,
                             includeEnd=False)})

        # exclude lower, include upper
        props.setFixedRangePerBand({1: QgsDateTimeRange(
            QDateTime(QDate(2023, 5, 9),
                      QTime(12, 13, 14)),
            QDateTime(QDate(2023, 5, 11),
                      QTime(12, 13, 14)),
            includeBeginning=False,
            includeEnd=True)})
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerTemporalProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.fixedRangePerBand(),
                         {1: QgsDateTimeRange(
                             QDateTime(QDate(2023, 5, 9),
                                       QTime(12, 13, 14)),
                             QDateTime(QDate(2023, 5, 11),
                                       QTime(12, 13, 14)),
                             includeBeginning=False,
                             includeEnd=True)})


if __name__ == '__main__':
    unittest.main()
