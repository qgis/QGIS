
# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFieldDomainWidget

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
from qgis.gui import QgsFieldDomainWidget
from qgis.PyQt.QtCore import QVariant
from qgis.testing import start_app, unittest

start_app()


class TestPyQgsFieldDomainWidget(unittest.TestCase):

    def testRangeDomainWidget(self):
        w = QgsFieldDomainWidget(Qgis.FieldDomainType.Range)

        domain = w.createFieldDomain()

        self.assertIsInstance(domain, QgsRangeFieldDomain)
        self.assertEqual(domain.fieldType(), QVariant.Double)
        self.assertEqual(domain.splitPolicy(), Qgis.FieldDomainSplitPolicy.DefaultValue)
        self.assertEqual(domain.mergePolicy(), Qgis.FieldDomainMergePolicy.DefaultValue)
        self.assertEqual(domain.minimum(), 0)
        self.assertTrue(domain.minimumIsInclusive())
        self.assertEqual(domain.maximum(), 100)
        self.assertTrue(domain.maximumIsInclusive())

        # set domain and test round trips
        domain = QgsRangeFieldDomain('name', 'desc', QVariant.Int, -10, False, -1, True)
        domain.setSplitPolicy(Qgis.FieldDomainSplitPolicy.GeometryRatio)
        domain.setMergePolicy(Qgis.FieldDomainMergePolicy.Sum)
        w.setFieldDomain(domain)

        domain2 = w.createFieldDomain()
        self.assertIsInstance(domain2, QgsRangeFieldDomain)
        self.assertEqual(domain2.fieldType(), QVariant.Int)
        self.assertEqual(domain2.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(domain2.mergePolicy(), Qgis.FieldDomainMergePolicy.Sum)
        self.assertEqual(domain2.minimum(), -10.0)
        self.assertFalse(domain2.minimumIsInclusive())
        self.assertEqual(domain2.maximum(), -1.0)
        self.assertTrue(domain2.maximumIsInclusive())

        domain = QgsRangeFieldDomain('name', 'desc', QVariant.Int, -10.1, True, -1.1, False)
        w.setFieldDomain(domain)

        domain2 = w.createFieldDomain()
        self.assertIsInstance(domain2, QgsRangeFieldDomain)
        self.assertEqual(domain2.fieldType(), QVariant.Int)
        self.assertEqual(domain2.minimum(), -10.1)
        self.assertTrue(domain2.minimumIsInclusive())
        self.assertEqual(domain2.maximum(), -1.1)
        self.assertFalse(domain2.maximumIsInclusive())

    def testGlobWidget(self):
        w = QgsFieldDomainWidget(Qgis.FieldDomainType.Glob)

        domain = w.createFieldDomain()

        self.assertIsInstance(domain, QgsGlobFieldDomain)
        self.assertEqual(domain.fieldType(), QVariant.String)
        self.assertEqual(domain.splitPolicy(), Qgis.FieldDomainSplitPolicy.DefaultValue)
        self.assertEqual(domain.mergePolicy(), Qgis.FieldDomainMergePolicy.DefaultValue)
        self.assertEqual(domain.glob(), '')

        # set domain and test round trips
        domain = QgsGlobFieldDomain('name', 'desc', QVariant.Int, '*a*')
        domain.setSplitPolicy(Qgis.FieldDomainSplitPolicy.GeometryRatio)
        domain.setMergePolicy(Qgis.FieldDomainMergePolicy.Sum)
        w.setFieldDomain(domain)

        domain2 = w.createFieldDomain()
        self.assertIsInstance(domain2, QgsGlobFieldDomain)
        self.assertEqual(domain2.fieldType(), QVariant.Int)
        self.assertEqual(domain2.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(domain2.mergePolicy(), Qgis.FieldDomainMergePolicy.Sum)
        self.assertEqual(domain2.glob(), '*a*')

    def testCodedValueWidget(self):
        w = QgsFieldDomainWidget(Qgis.FieldDomainType.Coded)

        domain = w.createFieldDomain()

        self.assertIsInstance(domain, QgsCodedFieldDomain)
        self.assertEqual(domain.fieldType(), QVariant.String)
        self.assertEqual(domain.splitPolicy(), Qgis.FieldDomainSplitPolicy.DefaultValue)
        self.assertEqual(domain.mergePolicy(), Qgis.FieldDomainMergePolicy.DefaultValue)
        self.assertFalse(domain.values())

        # set domain and test round trips
        domain = QgsCodedFieldDomain('name', 'desc', QVariant.Int, [QgsCodedValue('1', 'aa'), QgsCodedValue('2', 'bb'), QgsCodedValue('3', 'cc')])
        domain.setSplitPolicy(Qgis.FieldDomainSplitPolicy.GeometryRatio)
        domain.setMergePolicy(Qgis.FieldDomainMergePolicy.Sum)
        w.setFieldDomain(domain)

        domain2 = w.createFieldDomain()
        self.assertIsInstance(domain2, QgsCodedFieldDomain)
        self.assertEqual(domain2.fieldType(), QVariant.Int)
        self.assertEqual(domain2.splitPolicy(), Qgis.FieldDomainSplitPolicy.GeometryRatio)
        self.assertEqual(domain2.mergePolicy(), Qgis.FieldDomainMergePolicy.Sum)
        self.assertEqual(domain2.values(), [QgsCodedValue('1', 'aa'), QgsCodedValue('2', 'bb'), QgsCodedValue('3', 'cc')])


if __name__ == '__main__':
    unittest.main()
