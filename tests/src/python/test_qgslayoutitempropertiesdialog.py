# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemPropertiesDialog

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsUnitTypes, QgsLayoutSize, QgsLayoutPoint, QgsLayoutItem, QgsProject, QgsLayout
from qgis.gui import QgsLayoutItemPropertiesDialog

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutItemPropertiesDialog(unittest.TestCase):

    def testGettersSetters(self):
        """ test dialog getters/setters """
        dlg = qgis.gui.QgsLayoutItemPropertiesDialog()

        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()
        dlg.setLayout(l)

        dlg.setItemPosition(QgsLayoutPoint(5, 6, QgsUnitTypes.LayoutPixels))
        self.assertEqual(dlg.itemPosition().x(), 5.0)
        self.assertEqual(dlg.itemPosition().y(), 6.0)
        self.assertEqual(dlg.itemPosition().units(), QgsUnitTypes.LayoutPixels)

        dlg.setItemSize(QgsLayoutSize(15, 16, QgsUnitTypes.LayoutInches))
        self.assertEqual(dlg.itemSize().width(), 15.0)
        self.assertEqual(dlg.itemSize().height(), 16.0)
        self.assertEqual(dlg.itemSize().units(), QgsUnitTypes.LayoutInches)

        for p in [QgsLayoutItem.UpperLeft, QgsLayoutItem.UpperMiddle, QgsLayoutItem.UpperRight,
                  QgsLayoutItem.MiddleLeft, QgsLayoutItem.Middle, QgsLayoutItem.MiddleRight,
                  QgsLayoutItem.LowerLeft, QgsLayoutItem.LowerMiddle, QgsLayoutItem.LowerRight]:
            dlg.setReferencePoint(p)
            self.assertEqual(dlg.referencePoint(), p)


if __name__ == '__main__':
    unittest.main()
