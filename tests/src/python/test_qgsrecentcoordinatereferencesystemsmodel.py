"""QGIS Unit tests for QgsRecentCoordinateReferenceSystemsModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2022 by Nyall Dawson"
__date__ = "12/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"


from qgis.PyQt.QtCore import Qt, QModelIndex, QCoreApplication
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsSettings,
)
from qgis.gui import (
    QgsRecentCoordinateReferenceSystemsModel,
    QgsRecentCoordinateReferenceSystemsProxyModel,
    QgsCoordinateReferenceSystemProxyModel,
)

import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsRecentCoordinateReferenceSystemsModel(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        QgsSettings().clear()
        start_app()

    def test_model(self):
        model = QgsRecentCoordinateReferenceSystemsModel()
        # should initially be nothing in the model
        self.assertEqual(model.rowCount(), 0)
        self.assertFalse(model.index(-1, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(0, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(1, 0, QModelIndex()).isValid())
        self.assertIsNone(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertFalse(model.crs(model.index(0, 0, QModelIndex())).isValid())
        self.assertFalse(model.crs(model.index(-1, 0, QModelIndex())).isValid())
        self.assertFalse(model.crs(model.index(1, 0, QModelIndex())).isValid())

        # push a crs
        registry = QgsApplication.coordinateReferenceSystemRegistry()
        registry.pushRecent(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(model.rowCount(), 1)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(1, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:3111 - GDA94 / Vicgrid",
        )
        self.assertIsNone(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertIsNone(
            model.data(model.index(-1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )

        # push another crs
        registry.pushRecent(QgsCoordinateReferenceSystem("EPSG:4326"))
        self.assertEqual(model.rowCount(), 2)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertTrue(model.index(1, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(2, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:4326 - WGS 84",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:3111 - GDA94 / Vicgrid",
        )
        self.assertIsNone(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:4326"),
        )
        self.assertEqual(
            model.crs(model.index(1, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )

        # push first crs back to top
        registry.pushRecent(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(model.rowCount(), 2)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertTrue(model.index(1, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(2, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:3111 - GDA94 / Vicgrid",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:4326 - WGS 84",
        )
        self.assertIsNone(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )
        self.assertEqual(
            model.crs(model.index(1, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:4326"),
        )

        # remove crs (not in recent list!)
        registry.removeRecent(QgsCoordinateReferenceSystem("EPSG:3857"))
        self.assertEqual(model.rowCount(), 2)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertTrue(model.index(1, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(2, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:3111 - GDA94 / Vicgrid",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:4326 - WGS 84",
        )
        self.assertIsNone(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )
        self.assertEqual(
            model.crs(model.index(1, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:4326"),
        )

        # remove crs (in recent list!)
        registry.removeRecent(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(model.rowCount(), 1)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(1, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:4326 - WGS 84",
        )
        self.assertIsNone(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:4326"),
        )

        # clear list
        registry.clearRecent()
        self.assertEqual(model.rowCount(), 0)

        # model limit of 30 items should consider crs types
        for _id in range(32510, 32550):
            registry.pushRecent(QgsCoordinateReferenceSystem(f"EPSG:{_id}"))

        self.assertEqual(model.rowCount(), 30)
        for row in range(30):
            self.assertEqual(
                model.crs(model.index(row, 0, QModelIndex())).authid(),
                f"EPSG:{32549 - row}",
            )

        # these are vertical CRS, so their addition should NOT cause
        # the existing items to drop
        for _id in range(115851, 115867):
            registry.pushRecent(QgsCoordinateReferenceSystem(f"ESRI:{_id}"))
        self.assertEqual(model.rowCount(), 46)

    def test_proxy(self):
        registry = QgsApplication.coordinateReferenceSystemRegistry()
        registry.clearRecent()

        model = QgsRecentCoordinateReferenceSystemsProxyModel()
        # should initially be nothing in the model
        self.assertEqual(model.rowCount(), 0)
        self.assertFalse(model.index(-1, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(0, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(1, 0, QModelIndex()).isValid())
        self.assertIsNone(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertFalse(model.crs(model.index(0, 0, QModelIndex())).isValid())
        self.assertFalse(model.crs(model.index(-1, 0, QModelIndex())).isValid())
        self.assertFalse(model.crs(model.index(1, 0, QModelIndex())).isValid())

        # push a crs
        registry.pushRecent(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(model.rowCount(), 1)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(1, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:3111 - GDA94 / Vicgrid",
        )
        self.assertIsNone(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertIsNone(
            model.data(model.index(-1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )

        # push a vertical crs
        registry.pushRecent(QgsCoordinateReferenceSystem("ESRI:115851"))
        self.assertEqual(model.rowCount(), 2)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertTrue(model.index(1, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(2, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "ESRI:115851 - SIRGAS-CON_DGF00P01",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:3111 - GDA94 / Vicgrid",
        )
        self.assertIsNone(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("ESRI:115851"),
        )
        self.assertEqual(
            model.crs(model.index(1, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )

        # add filter
        model.setFilters(QgsCoordinateReferenceSystemProxyModel.Filter.FilterHorizontal)
        self.assertEqual(model.rowCount(), 1)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(1, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "EPSG:3111 - GDA94 / Vicgrid",
        )
        self.assertIsNone(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )

        model.setFilters(QgsCoordinateReferenceSystemProxyModel.Filter.FilterVertical)
        self.assertEqual(model.rowCount(), 1)
        self.assertTrue(model.index(0, 0, QModelIndex()).isValid())
        self.assertFalse(model.index(1, 0, QModelIndex()).isValid())
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "ESRI:115851 - SIRGAS-CON_DGF00P01",
        )
        self.assertIsNone(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.crs(model.index(0, 0, QModelIndex())),
            QgsCoordinateReferenceSystem("ESRI:115851"),
        )


if __name__ == "__main__":
    unittest.main()
