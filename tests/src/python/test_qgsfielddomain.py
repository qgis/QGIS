
# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFieldDomain

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2022-01-25'
__copyright__ = 'Copyright 2022, The QGIS Project'

from qgis.core import (Qgis,
                       QgsCodedValue,
                       QgsCodedFieldDomain,
                       QgsRangeFieldDomain,
                       QgsGlobFieldDomain)
from qgis.PyQt.QtCore import QVariant
from qgis.testing import unittest
from utilities import unitTestDataPath


class TestPyQgsFieldDomain(unittest.TestCase):

    def testCodedValue(self):
        c = QgsCodedValue(5, 'a')
        self.assertEqual(c.code(), 5)
        self.assertEqual(c.value(), 'a')

        self.assertEqual(str(c), '<QgsCodedValue: 5 (a)>')

        self.assertEqual(c, QgsCodedValue(5, 'a'))
        self.assertNotEqual(c, QgsCodedValue(5, 'aa'))
        self.assertNotEqual(c, QgsCodedValue(55, 'a'))

    def testCodedFieldDomain(self):
        domain = QgsCodedFieldDomain('name',
                                     'desc',
                                     QVariant.Int,
                                     [
                                         QgsCodedValue(5, 'a'),
                                         QgsCodedValue(6, 'b')
                                     ])

        self.assertEqual(str(domain), '<QgsCodedFieldDomain: name>')

        self.assertEqual(domain.type(), Qgis.FieldDomainType.Coded)
        self.assertEqual(domain.name(), 'name')
        domain.setName('n')
        self.assertEqual(domain.name(), 'n')

        self.assertEqual(domain.description(), 'desc')
        domain.setDescription('desc 2')
        self.assertEqual(domain.description(), 'desc 2')

        self.assertEqual(domain.fieldType(), QVariant.Int)
        domain.setFieldType(QVariant.Double)
        self.assertEqual(domain.fieldType(), QVariant.Double)

        self.assertEqual(domain.values(), [
            QgsCodedValue(5, 'a'),
            QgsCodedValue(6, 'b')
        ])
        domain.setValues([
            QgsCodedValue(51, 'aa'),
            QgsCodedValue(61, 'bb')
        ])
        self.assertEqual(domain.values(), [
            QgsCodedValue(51, 'aa'),
            QgsCodedValue(61, 'bb')
        ])

        domain.setSplitPolicy(Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(domain.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)

        domain.setMergePolicy(Qgis.FieldDomainMergePolicy.GeometryWeighted)
        self.assertEqual(domain.mergePolicy(), Qgis.FieldDomainMergePolicy.GeometryWeighted)

        d2 = domain.clone()
        self.assertEqual(d2.name(), 'n')
        self.assertEqual(d2.description(), 'desc 2')
        self.assertEqual(d2.fieldType(), QVariant.Double)
        self.assertEqual(d2.values(), [
            QgsCodedValue(51, 'aa'),
            QgsCodedValue(61, 'bb')
        ])

        self.assertEqual(d2.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(d2.mergePolicy(), Qgis.FieldDomainMergePolicy.GeometryWeighted)

    def testRangeFieldDomain(self):
        domain = QgsRangeFieldDomain('name',
                                     'desc',
                                     QVariant.Int,
                                     1, True, 5, True)

        self.assertEqual(str(domain), '<QgsRangeFieldDomain: name [1, 5]>')

        self.assertEqual(domain.type(), Qgis.FieldDomainType.Range)
        self.assertEqual(domain.name(), 'name')
        domain.setName('n')
        self.assertEqual(domain.name(), 'n')

        self.assertEqual(domain.description(), 'desc')
        domain.setDescription('desc 2')
        self.assertEqual(domain.description(), 'desc 2')

        self.assertEqual(domain.fieldType(), QVariant.Int)
        domain.setFieldType(QVariant.Double)
        self.assertEqual(domain.fieldType(), QVariant.Double)

        self.assertEqual(domain.minimum(), 1)
        domain.setMinimum(-1)
        self.assertEqual(domain.minimum(), -1)

        self.assertEqual(domain.maximum(), 5)
        domain.setMaximum(55)
        self.assertEqual(domain.maximum(), 55)

        self.assertTrue(domain.minimumIsInclusive())
        domain.setMinimumIsInclusive(False)
        self.assertFalse(domain.minimumIsInclusive())

        self.assertEqual(str(domain), '<QgsRangeFieldDomain: n (-1, 55]>')

        self.assertTrue(domain.maximumIsInclusive())
        domain.setMaximumIsInclusive(False)
        self.assertFalse(domain.maximumIsInclusive())

        self.assertEqual(str(domain), '<QgsRangeFieldDomain: n (-1, 55)>')

        domain.setSplitPolicy(Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(domain.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)

        domain.setMergePolicy(Qgis.FieldDomainMergePolicy.GeometryWeighted)
        self.assertEqual(domain.mergePolicy(), Qgis.FieldDomainMergePolicy.GeometryWeighted)

        d2 = domain.clone()
        self.assertEqual(d2.name(), 'n')
        self.assertEqual(d2.description(), 'desc 2')
        self.assertEqual(d2.fieldType(), QVariant.Double)
        self.assertEqual(d2.minimum(), -1)
        self.assertEqual(d2.maximum(), 55)
        self.assertFalse(d2.minimumIsInclusive())
        self.assertFalse(d2.maximumIsInclusive())
        self.assertEqual(d2.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(d2.mergePolicy(), Qgis.FieldDomainMergePolicy.GeometryWeighted)

    def testGlobFieldDomain(self):
        domain = QgsGlobFieldDomain('name',
                                    'desc',
                                    QVariant.String,
                                    '*a*')

        self.assertEqual(str(domain), "<QgsGlobFieldDomain: name '*a*'>")

        self.assertEqual(domain.type(), Qgis.FieldDomainType.Glob)
        self.assertEqual(domain.name(), 'name')
        domain.setName('n')
        self.assertEqual(domain.name(), 'n')

        self.assertEqual(domain.description(), 'desc')
        domain.setDescription('desc 2')
        self.assertEqual(domain.description(), 'desc 2')

        self.assertEqual(domain.fieldType(), QVariant.String)
        domain.setFieldType(QVariant.Double)
        self.assertEqual(domain.fieldType(), QVariant.Double)

        self.assertEqual(domain.glob(), '*a*')
        domain.setGlob('*b*')
        self.assertEqual(domain.glob(), '*b*')

        domain.setSplitPolicy(Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(domain.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)

        domain.setMergePolicy(Qgis.FieldDomainMergePolicy.GeometryWeighted)
        self.assertEqual(domain.mergePolicy(), Qgis.FieldDomainMergePolicy.GeometryWeighted)

        d2 = domain.clone()
        self.assertEqual(d2.name(), 'n')
        self.assertEqual(d2.description(), 'desc 2')
        self.assertEqual(d2.fieldType(), QVariant.Double)
        self.assertEqual(d2.glob(), '*b*')
        self.assertEqual(d2.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(d2.mergePolicy(), Qgis.FieldDomainMergePolicy.GeometryWeighted)


if __name__ == '__main__':
    unittest.main()
