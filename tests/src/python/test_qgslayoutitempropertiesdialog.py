"""QGIS Unit tests for QgsLayoutItemPropertiesDialog

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "18/07/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import unittest

from qgis.core import (
    QgsLayout,
    QgsLayoutItem,
    QgsLayoutPoint,
    QgsLayoutSize,
    QgsProject,
    QgsUnitTypes,
)
from qgis.gui import QgsLayoutItemPropertiesDialog
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLayoutItemPropertiesDialog(QgisTestCase):

    def testGettersSetters(self):
        """test dialog getters/setters"""
        dlg = QgsLayoutItemPropertiesDialog()

        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()
        dlg.setLayout(l)

        dlg.setItemPosition(QgsLayoutPoint(5, 6, QgsUnitTypes.LayoutUnit.LayoutPixels))
        self.assertEqual(dlg.itemPosition().x(), 5.0)
        self.assertEqual(dlg.itemPosition().y(), 6.0)
        self.assertEqual(
            dlg.itemPosition().units(), QgsUnitTypes.LayoutUnit.LayoutPixels
        )

        dlg.setItemSize(QgsLayoutSize(15, 16, QgsUnitTypes.LayoutUnit.LayoutInches))
        self.assertEqual(dlg.itemSize().width(), 15.0)
        self.assertEqual(dlg.itemSize().height(), 16.0)
        self.assertEqual(dlg.itemSize().units(), QgsUnitTypes.LayoutUnit.LayoutInches)

        for p in [
            QgsLayoutItem.ReferencePoint.UpperLeft,
            QgsLayoutItem.ReferencePoint.UpperMiddle,
            QgsLayoutItem.ReferencePoint.UpperRight,
            QgsLayoutItem.ReferencePoint.MiddleLeft,
            QgsLayoutItem.ReferencePoint.Middle,
            QgsLayoutItem.ReferencePoint.MiddleRight,
            QgsLayoutItem.ReferencePoint.LowerLeft,
            QgsLayoutItem.ReferencePoint.LowerMiddle,
            QgsLayoutItem.ReferencePoint.LowerRight,
        ]:
            dlg.setReferencePoint(p)
            self.assertEqual(dlg.referencePoint(), p)


if __name__ == "__main__":
    unittest.main()
