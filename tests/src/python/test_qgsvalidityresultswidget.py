"""QGIS Unit tests for validity results widget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "03/12/2018"
__copyright__ = "Copyright 2018, The QGIS Project"


from qgis.core import QgsValidityCheckResult
from qgis.gui import QgsValidityCheckResultsModel
from qgis.PyQt.QtCore import QModelIndex, Qt
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestQgsValidityResultsWidget(QgisTestCase):

    def testModel(self):
        res1 = QgsValidityCheckResult()
        res1.type = QgsValidityCheckResult.Type.Warning
        res1.title = "test"
        res1.detailedDescription = "blah blah"

        res2 = QgsValidityCheckResult()
        res2.type = QgsValidityCheckResult.Type.Critical
        res2.title = "test2"
        res2.detailedDescription = "blah blah2"

        res3 = QgsValidityCheckResult()
        res3.type = QgsValidityCheckResult.Type.Warning
        res3.title = "test3"
        res3.detailedDescription = "blah blah3"
        res4 = QgsValidityCheckResult()
        res4.type = QgsValidityCheckResult.Type.Warning
        res4.title = "test4"
        res4.detailedDescription = "blah blah4"

        model = QgsValidityCheckResultsModel([])
        self.assertEqual(model.rowCount(), 0)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertFalse(
            model.data(model.index(-1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertFalse(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )

        model = QgsValidityCheckResultsModel([res1, res2, res3, res4])
        self.assertEqual(model.rowCount(), 4)
        self.assertFalse(
            model.data(model.index(-1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test2",
        )
        self.assertEqual(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test3",
        )
        self.assertEqual(
            model.data(model.index(3, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test4",
        )
        self.assertFalse(
            model.data(model.index(4, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsValidityCheckResultsModel.Roles.DescriptionRole,
            ),
            "blah blah",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsValidityCheckResultsModel.Roles.DescriptionRole,
            ),
            "blah blah2",
        )
        self.assertEqual(
            model.data(
                model.index(2, 0, QModelIndex()),
                QgsValidityCheckResultsModel.Roles.DescriptionRole,
            ),
            "blah blah3",
        )
        self.assertEqual(
            model.data(
                model.index(3, 0, QModelIndex()),
                QgsValidityCheckResultsModel.Roles.DescriptionRole,
            ),
            "blah blah4",
        )


if __name__ == "__main__":
    unittest.main()
