# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAttributeTableConfig.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '07/06/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsAttributeTableConfig, QgsVectorLayer
from qgis.testing import start_app, unittest

start_app()


class TestQgsAttributeTableConfig(unittest.TestCase):

    def testLayerConfig(self):
        """ test retrieving attribute table config from a layer """

        # make a layer
        point_layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                     "pointlayer", "memory")

        # make sure attribute table config is initially populated
        config = point_layer.attributeTableConfig()
        self.assertFalse(config.isEmpty())
        self.assertEqual(config.columns()[0].name, 'fldtxt')
        self.assertEqual(config.columns()[1].name, 'fldint')

        # try replacing it
        config.setColumns([config.columns()[1], config.columns()[0]])
        point_layer.setAttributeTableConfig(config)

        # and make sure changes were applied
        config = point_layer.attributeTableConfig()
        self.assertFalse(config.isEmpty())
        self.assertEqual(config.columns()[0].name, 'fldint')
        self.assertEqual(config.columns()[1].name, 'fldtxt')

    def testIsEmpty(self):
        """ test isEmpty method """
        config = QgsAttributeTableConfig()
        self.assertTrue(config.isEmpty())

        c = QgsAttributeTableConfig.ColumnConfig()
        c.name = 'test'
        config.setColumns([c])
        self.assertFalse(config.isEmpty())

    def testSize(self):
        """
        Test QgsAttributeTableConfig.size and __len__
        """
        config = QgsAttributeTableConfig()
        self.assertEqual(config.size(), 0)
        self.assertEqual(len(config), 0)

        c1 = QgsAttributeTableConfig.ColumnConfig()
        c2 = QgsAttributeTableConfig.ColumnConfig()
        config.setColumns([c1, c2])

        self.assertEqual(config.size(), 2)
        self.assertEqual(len(config), 2)

    def testSetColumns(self):
        """ test setting columns """
        config = QgsAttributeTableConfig()
        self.assertEqual(config.columns(), [])

        c1 = QgsAttributeTableConfig.ColumnConfig()
        c1.name = 'test'
        c1.hidden = False
        c1.width = 9
        c2 = QgsAttributeTableConfig.ColumnConfig()
        c2.name = 'test2'
        c2.hidden = True
        c2.width = 11
        config.setColumns([c1, c2])
        result = config.columns()
        self.assertEqual(result[0].name, 'test')
        self.assertEqual(result[1].name, 'test2')
        self.assertEqual(result[0].hidden, False)
        self.assertEqual(result[1].hidden, True)
        self.assertEqual(result[0].width, 9)
        self.assertEqual(result[1].width, 11)

    def testColumnHidden(self):
        """ test hiding columns """

        config = QgsAttributeTableConfig()
        c1 = QgsAttributeTableConfig.ColumnConfig()
        c1.name = 'test'
        c1.hidden = False
        c2 = QgsAttributeTableConfig.ColumnConfig()
        c2.name = 'test2'
        c2.hidden = False
        config.setColumns([c1, c2])

        self.assertFalse(config.columnHidden(0))
        self.assertFalse(config.columnHidden(1))

        with self.assertRaises(IndexError):
            config.columnHidden(-1)
        with self.assertRaises(IndexError):
            config.columnHidden(2)

        config.setColumnHidden(1, True)
        self.assertFalse(config.columnHidden(0))
        self.assertTrue(config.columnHidden(1))
        self.assertFalse(config.columns()[0].hidden)
        self.assertTrue(config.columns()[1].hidden)

        config.setColumnHidden(0, True)
        self.assertTrue(config.columnHidden(0))
        self.assertTrue(config.columnHidden(1))
        self.assertTrue(config.columns()[0].hidden)
        self.assertTrue(config.columns()[1].hidden)

        with self.assertRaises(IndexError):
            config.setColumnHidden(-1, True)
        with self.assertRaises(IndexError):
            config.setColumnHidden(2, True)

        c2.hidden = True
        config.setColumns([c1, c2])
        self.assertFalse(config.columnHidden(0))
        self.assertTrue(config.columnHidden(1))

    def testColumnWidth(self):
        """ test setting column widths """

        config = QgsAttributeTableConfig()
        c1 = QgsAttributeTableConfig.ColumnConfig()
        c1.name = 'test'
        c1.width = -1
        c2 = QgsAttributeTableConfig.ColumnConfig()
        c2.name = 'test2'
        c2.width = 27
        config.setColumns([c1, c2])

        self.assertEqual(config.columnWidth(0), -1)
        self.assertEqual(config.columnWidth(1), 27)

        with self.assertRaises(IndexError):
            config.columnWidth(-1)
        with self.assertRaises(IndexError):
            config.columnWidth(2)

        config.setColumnWidth(1, -1)
        self.assertEqual(config.columnWidth(0), -1)
        self.assertEqual(config.columnWidth(1), -1)
        self.assertEqual(config.columns()[0].width, -1)
        self.assertEqual(config.columns()[1].width, -1)

        config.setColumnWidth(0, 34)
        self.assertEqual(config.columnWidth(0), 34)
        self.assertEqual(config.columnWidth(1), -1)
        self.assertEqual(config.columns()[0].width, 34)
        self.assertEqual(config.columns()[1].width, -1)

        with self.assertRaises(IndexError):
            config.setColumnWidth(-1, 11)
        with self.assertRaises(IndexError):
            config.setColumnWidth(2, 11)

        c2.width = 12
        config.setColumns([c1, c2])
        self.assertEqual(config.columnWidth(0), -1)
        self.assertEqual(config.columnWidth(1), 12)

    def testSameColumns(self):
        """ test hasSameColumns() check """

        config = QgsAttributeTableConfig()
        c1 = QgsAttributeTableConfig.ColumnConfig()
        c1.name = 'test'
        c1.hidden = False
        c1.width = 100
        c2 = QgsAttributeTableConfig.ColumnConfig()
        c2.name = 'test2'
        c2.hidden = False
        c2.width = 120
        config.setColumns([c1, c2])
        config2 = QgsAttributeTableConfig()

        config2.setColumns([c1, c2])
        self.assertTrue(config.hasSameColumns(config2))

        c1.width = 200
        config2.setColumns([c1, c2])
        self.assertTrue(config.hasSameColumns(config2))

        c1.hidden = True
        config2.setColumns([c1, c2])
        self.assertFalse(config.hasSameColumns(config2))

        config2.setColumns([c2, c1])
        self.assertFalse(config.hasSameColumns(config2))

        c2.name = 'test3'
        config2.setColumns([c1, c2])
        self.assertFalse(config.hasSameColumns(config2))

    def testMapVisibleColumn(self):
        pass


if __name__ == '__main__':
    unittest.main()
