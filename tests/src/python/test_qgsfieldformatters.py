# -*- coding: utf-8 -*-
"""QGIS Unit tests for field formatters.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '05/12/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import tempfile
import os

import qgis  # NOQA
from qgis.PyQt.QtCore import (
    QCoreApplication,
    QLocale,
    QVariant,
    QDate,
    QTime,
    QDateTime,
    Qt
)
from qgis.core import (
    QgsApplication,
    QgsFeature,
    QgsProject,
    QgsRelation,
    QgsVectorLayer,
    QgsValueMapFieldFormatter,
    QgsValueRelationFieldFormatter,
    QgsRelationReferenceFieldFormatter,
    QgsRangeFieldFormatter,
    QgsCheckBoxFieldFormatter,
    QgsFallbackFieldFormatter,
    QgsDateTimeFieldFormatter,
    QgsSettings,
    QgsGeometry,
    QgsPointXY,
    QgsVectorFileWriter
)
from qgis.testing import start_app, unittest
from qgis.utils import spatialite_connect

from utilities import writeShape

start_app()


class TestQgsValueMapFieldFormatter(unittest.TestCase):
    VALUEMAP_NULL_TEXT = "{2839923C-8B7D-419E-B84B-CA2FE9B80EC7}"

    def test_representValue(self):
        QgsSettings().setValue("qgis/nullValue", "NULL")
        layer = QgsVectorLayer(
            "none?field=number1:integer&field=number2:double&field=text1:string&field=number3:integer&field=number4:double&field=text2:string",
            "layer", "memory")
        self.assertTrue(layer.isValid())
        QgsProject.instance().addMapLayer(layer)
        f = QgsFeature()
        f.setAttributes([2, 2.5, 'NULL', None, None, None])
        layer.dataProvider().addFeatures([f])
        fieldFormatter = QgsValueMapFieldFormatter()

        # Tests with different value types occurring in the value map
        # old style config (pre 3.0)
        config = {'map': {'two': '2', 'twoandhalf': '2.5', 'NULL text': 'NULL',
                          'nothing': self.VALUEMAP_NULL_TEXT}}
        self.assertEqual(fieldFormatter.representValue(layer, 0, config, None, 2), 'two')
        self.assertEqual(fieldFormatter.representValue(layer, 1, config, None, 2.5), 'twoandhalf')
        self.assertEqual(fieldFormatter.representValue(layer, 2, config, None, 'NULL'), 'NULL text')
        # Tests with null values of different types, if value map contains null
        self.assertEqual(fieldFormatter.representValue(layer, 3, config, None, None), 'nothing')
        self.assertEqual(fieldFormatter.representValue(layer, 4, config, None, None), 'nothing')
        self.assertEqual(fieldFormatter.representValue(layer, 5, config, None, None), 'nothing')

        # new style config (post 3.0)
        config = {'map': [{'two': '2'},
                          {'twoandhalf': '2.5'},
                          {'NULL text': 'NULL'},
                          {'nothing': self.VALUEMAP_NULL_TEXT}]}
        self.assertEqual(fieldFormatter.representValue(layer, 0, config, None, 2), 'two')
        self.assertEqual(fieldFormatter.representValue(layer, 1, config, None, 2.5), 'twoandhalf')
        self.assertEqual(fieldFormatter.representValue(layer, 2, config, None, 'NULL'), 'NULL text')
        # Tests with null values of different types, if value map contains null
        self.assertEqual(fieldFormatter.representValue(layer, 3, config, None, None), 'nothing')
        self.assertEqual(fieldFormatter.representValue(layer, 4, config, None, None), 'nothing')
        self.assertEqual(fieldFormatter.representValue(layer, 5, config, None, None), 'nothing')

        # Tests with fallback display for different value types
        config = {}
        self.assertEqual(fieldFormatter.representValue(layer, 0, config, None, 2), '(2)')
        self.assertEqual(fieldFormatter.representValue(layer, 1, config, None, 2.5), '(2.50000)')
        self.assertEqual(fieldFormatter.representValue(layer, 2, config, None, 'NULL'), '(NULL)')
        # Tests with fallback display for null in different types of fields
        self.assertEqual(fieldFormatter.representValue(layer, 3, config, None, None), '(NULL)')
        self.assertEqual(fieldFormatter.representValue(layer, 4, config, None, None), '(NULL)')
        self.assertEqual(fieldFormatter.representValue(layer, 5, config, None, None), '(NULL)')

        QgsProject.instance().removeAllMapLayers()


class TestQgsValueRelationFieldFormatter(unittest.TestCase):

    def test_representValue(self):
        first_layer = QgsVectorLayer("none?field=foreign_key:integer",
                                     "first_layer", "memory")
        self.assertTrue(first_layer.isValid())
        second_layer = QgsVectorLayer("none?field=pkid:integer&field=decoded:string",
                                      "second_layer", "memory")
        self.assertTrue(second_layer.isValid())
        QgsProject.instance().addMapLayer(second_layer)
        f = QgsFeature()
        f.setAttributes([123])
        first_layer.dataProvider().addFeatures([f])
        f = QgsFeature()
        f.setAttributes([123, 'decoded_val'])
        second_layer.dataProvider().addFeatures([f])

        fieldFormatter = QgsValueRelationFieldFormatter()

        # Everything valid
        config = {'Layer': second_layer.id(), 'Key': 'pkid', 'Value': 'decoded'}
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '123'), 'decoded_val')

        # Code not find match in foreign layer
        config = {'Layer': second_layer.id(), 'Key': 'pkid', 'Value': 'decoded'}
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '456'), '(456)')

        # Missing Layer
        config = {'Key': 'pkid', 'Value': 'decoded'}
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '456'), '(456)')

        # Invalid Layer
        config = {'Layer': 'invalid', 'Key': 'pkid', 'Value': 'decoded'}
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '456'), '(456)')

        # Invalid Key
        config = {'Layer': second_layer.id(), 'Key': 'invalid', 'Value': 'decoded'}
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '456'), '(456)')

        # Invalid Value
        config = {'Layer': second_layer.id(), 'Key': 'pkid', 'Value': 'invalid'}
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '456'), '(456)')

        QgsProject.instance().removeMapLayer(second_layer.id())

    def test_valueToStringList(self):
        def _test(a, b):
            self.assertEqual(QgsValueRelationFieldFormatter.valueToStringList(a), b)

        _test([1, 2, 3], ["1", "2", "3"])
        _test("{1,2,3}", ["1", "2", "3"])
        _test(['1', '2', '3'], ["1", "2", "3"])
        _test('not an array', [])
        _test('[1,2,3]', ["1", "2", "3"])
        _test('{1,2,3}', ["1", "2", "3"])
        _test('{"1","2","3"}', ["1", "2", "3"])
        _test('["1","2","3"]', ["1", "2", "3"])
        _test(r'["a string,comma","a string\"quote", "another string[]"]',
              ['a string,comma', 'a string"quote', 'another string[]'])

    def test_expressionRequiresFormScope(self):
        res = list(
            QgsValueRelationFieldFormatter.expressionFormAttributes("current_value('ONE') AND current_value('TWO')"))
        res = sorted(res)
        self.assertEqual(res, ['ONE', 'TWO'])

        res = list(QgsValueRelationFieldFormatter.expressionFormVariables("@current_geometry"))
        self.assertEqual(res, ['current_geometry'])

        self.assertFalse(QgsValueRelationFieldFormatter.expressionRequiresFormScope(""))
        self.assertTrue(QgsValueRelationFieldFormatter.expressionRequiresFormScope("current_value('TWO')"))
        self.assertTrue(QgsValueRelationFieldFormatter.expressionRequiresFormScope("current_value ( 'TWO' )"))
        self.assertTrue(QgsValueRelationFieldFormatter.expressionRequiresFormScope("@current_geometry"))

        self.assertTrue(QgsValueRelationFieldFormatter.expressionIsUsable("", QgsFeature()))
        self.assertFalse(QgsValueRelationFieldFormatter.expressionIsUsable("@current_geometry", QgsFeature()))
        self.assertFalse(QgsValueRelationFieldFormatter.expressionIsUsable("current_value ( 'TWO' )", QgsFeature()))

        layer = QgsVectorLayer("none?field=pkid:integer&field=decoded:string",
                               "layer", "memory")
        self.assertTrue(layer.isValid())
        QgsProject.instance().addMapLayer(layer)
        f = QgsFeature(layer.fields())
        f.setAttributes([1, 'value'])
        point = QgsGeometry.fromPointXY(QgsPointXY(123, 456))
        f.setGeometry(point)
        self.assertTrue(QgsValueRelationFieldFormatter.expressionIsUsable("current_geometry", f))
        self.assertFalse(QgsValueRelationFieldFormatter.expressionIsUsable("current_value ( 'TWO' )", f))
        self.assertTrue(QgsValueRelationFieldFormatter.expressionIsUsable("current_value ( 'pkid' )", f))
        self.assertTrue(
            QgsValueRelationFieldFormatter.expressionIsUsable("@current_geometry current_value ( 'pkid' )", f))

        QgsProject.instance().removeMapLayer(layer.id())

    def test_expressionRequiresParentFormScope(self):
        res = list(QgsValueRelationFieldFormatter.expressionFormAttributes(
            "current_value('ONE') AND current_parent_value('TWO')"))
        res = sorted(res)
        self.assertEqual(res, ['ONE'])

        res = list(QgsValueRelationFieldFormatter.expressionParentFormAttributes(
            "current_value('ONE') AND current_parent_value('TWO')"))
        res = sorted(res)
        self.assertEqual(res, ['TWO'])

        res = list(QgsValueRelationFieldFormatter.expressionParentFormVariables("@current_parent_geometry"))
        self.assertEqual(res, ['current_parent_geometry'])

        self.assertFalse(QgsValueRelationFieldFormatter.expressionRequiresParentFormScope(""))
        self.assertTrue(QgsValueRelationFieldFormatter.expressionRequiresParentFormScope("current_parent_value('TWO')"))
        self.assertTrue(
            QgsValueRelationFieldFormatter.expressionRequiresParentFormScope("current_parent_value ( 'TWO' )"))
        self.assertTrue(QgsValueRelationFieldFormatter.expressionRequiresParentFormScope("@current_parent_geometry"))


class TestQgsRelationReferenceFieldFormatter(unittest.TestCase):

    def test_representValue(self):
        first_layer = QgsVectorLayer("none?field=foreign_key:integer",
                                     "first_layer", "memory")
        self.assertTrue(first_layer.isValid())
        second_layer = QgsVectorLayer("none?field=pkid:integer&field=decoded:string",
                                      "second_layer", "memory")
        second_layer.setDisplayExpression('pkid')
        self.assertTrue(second_layer.isValid())
        QgsProject.instance().addMapLayers([first_layer, second_layer])
        f = QgsFeature()
        f.setAttributes([123])
        first_layer.dataProvider().addFeatures([f])
        f = QgsFeature()
        f.setAttributes([123, 'decoded_val'])
        second_layer.dataProvider().addFeatures([f])

        relMgr = QgsProject.instance().relationManager()

        fieldFormatter = QgsRelationReferenceFieldFormatter()

        rel = QgsRelation()
        rel.setId('rel1')
        rel.setName('Relation Number One')
        rel.setReferencingLayer(first_layer.id())
        rel.setReferencedLayer(second_layer.id())
        rel.addFieldPair('foreign_key', 'pkid')
        self.assertTrue(rel.isValid())

        relMgr.addRelation(rel)

        # Everything valid
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '123'), 'decoded_val')

        # Code not find match in foreign layer
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '456'), '456')

        # Invalid relation id
        config = {'Relation': 'invalid'}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '123'), '123')

        # No display expression - will default internally to the decoded string
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression(None)
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '123'), 'decoded_val')

        # Invalid display expression
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('invalid +')
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '123'), '123')

        # Missing relation
        config = {}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '123'), '123')

        # Inconsistent layer provided to representValue()
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(fieldFormatter.representValue(second_layer, 0, config, None, '123'), '123')

        # Inconsistent idx provided to representValue()
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(fieldFormatter.representValue(first_layer, 1, config, None, '123'), '123')

        # Invalid relation
        rel = QgsRelation()
        rel.setId('rel2')
        rel.setName('Relation Number Two')
        rel.setReferencingLayer(first_layer.id())
        rel.addFieldPair('foreign_key', 'pkid')
        self.assertFalse(rel.isValid())

        relMgr.addRelation(rel)

        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '123'), '123')

        QgsProject.instance().removeAllMapLayers()


class TestQgsRangeFieldFormatter(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsColorScheme.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsColorScheme")
        QgsSettings().clear()
        QLocale.setDefault(QLocale(QLocale.English))
        start_app()

    @classmethod
    def tearDownClass(cls):
        """Reset locale"""
        QLocale.setDefault(QLocale(QLocale.English))

    def test_representValue(self):
        layer = QgsVectorLayer("point?field=int:integer&field=double:double&field=long:long",
                               "layer", "memory")
        self.assertTrue(layer.isValid())
        QgsProject.instance().addMapLayers([layer])

        fieldFormatter = QgsRangeFieldFormatter()

        # Precision is ignored for integers and longlongs
        self.assertEqual(fieldFormatter.representValue(layer, 0, {'Precision': 1}, None, '123'), '123')
        self.assertEqual(fieldFormatter.representValue(layer, 0, {'Precision': 1}, None, '123000'), '123,000')
        self.assertEqual(fieldFormatter.representValue(layer, 0, {'Precision': 1}, None, '9999999'),
                         '9,999,999')  # no scientific notation for integers!
        self.assertEqual(fieldFormatter.representValue(layer, 0, {'Precision': 1}, None, None), 'NULL')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '123'), '123')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '123000'), '123,000')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '9999999'),
                         '9,999,999')  # no scientific notation for long longs!
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, None), 'NULL')

        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 1}, None, None), 'NULL')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 1}, None, '123'), '123.0')

        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, None), 'NULL')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '123000'), '123,000.00')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '0'), '0.00')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '123'), '123.00')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '0.123'), '0.12')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '0.127'), '0.13')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '0'), '0.000')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '0.127'), '0.127')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '1.27e-1'), '0.127')

        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '-123'), '-123.00')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '-0.123'), '-0.12')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '-0.127'), '-0.13')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '-0.127'), '-0.127')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '-1.27e-1'), '-0.127')

        # Check with Italian locale
        QLocale.setDefault(QLocale('it'))

        self.assertEqual(fieldFormatter.representValue(layer, 0, {'Precision': 1}, None, '9999999'),
                         '9.999.999')  # scientific notation for integers!
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '123'), '123')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '123000'), '123.000')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '9999999'),
                         '9.999.999')  # scientific notation for long longs!
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, None), 'NULL')

        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, None), 'NULL')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '123000'), '123.000,00')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '0'), '0,00')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '123'), '123,00')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '0.123'), '0,12')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '0.127'), '0,13')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '0'), '0,000')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '0.127'), '0,127')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '1.27e-1'), '0,127')

        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '-123'), '-123,00')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '-0.123'), '-0,12')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '-0.127'), '-0,13')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '-0.127'), '-0,127')
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 3}, None, '-1.27e-1'), '-0,127')

        # Check with custom locale without thousand separator

        custom = QLocale('en')
        custom.setNumberOptions(QLocale.OmitGroupSeparator)
        QLocale.setDefault(custom)

        self.assertEqual(fieldFormatter.representValue(layer, 0, {'Precision': 1}, None, '9999999'),
                         '9999999')  # scientific notation for integers!
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '123'), '123')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '123000'), '123000')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '9999999'),
                         '9999999')  # scientific notation for long longs!
        self.assertEqual(fieldFormatter.representValue(layer, 1, {'Precision': 2}, None, '123000'), '123000.00')

        QgsProject.instance().removeAllMapLayers()


class TestQgsCheckBoxFieldFormatter(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsCheckBoxFieldFormatter.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsCheckBoxFieldFormatter")
        QgsSettings().clear()
        start_app()

    def test_representValue(self):
        null_value = "NULL"
        QgsSettings().setValue("qgis/nullValue", null_value)
        layer = QgsVectorLayer("point?field=int:integer&field=str:string", "layer", "memory")
        self.assertTrue(layer.isValid())

        field_formatter = QgsCheckBoxFieldFormatter()

        # test with integer
        # normal case

        config = {'UncheckedState': 0, 'CheckedState': 1}
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 1), 'true')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 0), 'false')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 10), "(10)")

        # displaying stored values
        config['TextDisplayMethod'] = QgsCheckBoxFieldFormatter.ShowStoredValues
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 1), '1')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 0), '0')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 10), "(10)")

        # invert true/false
        config = {'UncheckedState': 1, 'CheckedState': 0}
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 0), 'true')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 1), 'false')

        # displaying stored values
        config['TextDisplayMethod'] = QgsCheckBoxFieldFormatter.ShowStoredValues
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 1), '1')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 0), '0')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 10), "(10)")

        # test with string
        config = {'UncheckedState': 'nooh', 'CheckedState': 'yeah'}
        self.assertEqual(field_formatter.representValue(layer, 1, config, None, 'yeah'), 'true')
        self.assertEqual(field_formatter.representValue(layer, 1, config, None, 'nooh'), 'false')
        self.assertEqual(field_formatter.representValue(layer, 1, config, None, 'oops'), "(oops)")

        # displaying stored values
        config['TextDisplayMethod'] = QgsCheckBoxFieldFormatter.ShowStoredValues
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 'yeah'), 'yeah')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 'nooh'), 'nooh')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None, 'oops'), "(oops)")


class TestQgsFallbackFieldFormatter(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsFieldFormatter.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsFieldFormatter")
        QgsSettings().clear()
        QLocale.setDefault(QLocale(QLocale.English))
        start_app()

    @classmethod
    def tearDownClass(cls):
        """Reset locale"""
        QLocale.setDefault(QLocale(QLocale.English))

    def test_representValue(self):

        def _test(layer, is_gpkg=False):

            # Skip fid and precision tests
            offset = 1 if is_gpkg else 0

            fieldFormatter = QgsFallbackFieldFormatter()

            QLocale.setDefault(QLocale('en'))

            # Precision is ignored for integers and longlongs
            self.assertEqual(fieldFormatter.representValue(layer, 0 + offset, {}, None, '123'), '123')
            self.assertEqual(fieldFormatter.representValue(layer, 0 + offset, {}, None, '123000'), '123,000')
            self.assertEqual(fieldFormatter.representValue(layer, 0 + offset, {}, None, '9999999'), '9,999,999')
            self.assertEqual(fieldFormatter.representValue(layer, 0 + offset, {}, None, None), 'NULL')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, '123'), '123')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, '123000'), '123,000')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, '9999999'), '9,999,999')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, None), 'NULL')

            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, None), 'NULL')

            if not is_gpkg:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123'), '123.00000')
            else:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123'), '123')

            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, None), 'NULL')

            if not is_gpkg:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123000'), '123,000.00000')
            else:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123000'), '123,000')

            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '0'), '0')
            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '0.127'), '0.127')
            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '1.27e-1'), '0.127')

            if not is_gpkg:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '-123'), '-123.00000')
            else:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '-123'), '-123')

            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '-0.127'), '-0.127')
            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '-1.27e-1'), '-0.127')

            # Check with Italian locale
            QLocale.setDefault(QLocale('it'))

            self.assertEqual(fieldFormatter.representValue(layer, 0 + offset, {}, None, '9999999'),
                             '9.999.999')  # scientific notation for integers!
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, '123'), '123')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, '123000'), '123.000')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, '9999999'), '9.999.999')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, None), 'NULL')

            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, None), 'NULL')

            if not is_gpkg:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123000'), '123.000,00000')
            else:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123000'), '123.000')

            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '0'), '0')

            if not is_gpkg:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123'), '123,00000')
            else:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123'), '123')

            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '0.127'), '0,127')
            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '1.27e-1'), '0,127')

            if not is_gpkg:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '-123'), '-123,00000')
            else:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '-123'), '-123')

            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '-0.127'), '-0,127')
            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '-1.27e-1'), '-0,127')

            # Check with custom locale without thousand separator

            custom = QLocale('en')
            custom.setNumberOptions(QLocale.OmitGroupSeparator)
            QLocale.setDefault(custom)

            self.assertEqual(fieldFormatter.representValue(layer, 0 + offset, {}, None, '9999999'),
                             '9999999')  # scientific notation for integers!
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, '123'), '123')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, '9999999'), '9999999')

            if not is_gpkg:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123000'), '123000.00000')
            else:
                self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, '123000'), '123000')

            # Check string
            self.assertEqual(fieldFormatter.representValue(layer, 3 + offset, {}, None, '123'), '123')
            self.assertEqual(fieldFormatter.representValue(layer, 3 + offset, {}, None, 'a string'), 'a string')
            self.assertEqual(fieldFormatter.representValue(layer, 3 + offset, {}, None, ''), '')
            self.assertEqual(fieldFormatter.representValue(layer, 3 + offset, {}, None, None), 'NULL')

            # Check NULLs (this is what happens in real life inside QGIS)
            self.assertEqual(fieldFormatter.representValue(layer, 0 + offset, {}, None, QVariant(QVariant.String)),
                             'NULL')
            self.assertEqual(fieldFormatter.representValue(layer, 1 + offset, {}, None, QVariant(QVariant.String)),
                             'NULL')
            self.assertEqual(fieldFormatter.representValue(layer, 2 + offset, {}, None, QVariant(QVariant.String)),
                             'NULL')
            self.assertEqual(fieldFormatter.representValue(layer, 3 + offset, {}, None, QVariant(QVariant.String)),
                             'NULL')

        memory_layer = QgsVectorLayer("point?field=int:integer&field=double:double&field=long:long&field=string:string",
                                      "layer", "memory")
        self.assertTrue(memory_layer.isValid())

        _test(memory_layer)

        # Test a shapefile
        shape_path = writeShape(memory_layer, 'test_qgsfieldformatters.shp')

        shapefile_layer = QgsVectorLayer(shape_path, 'test', 'ogr')
        self.assertTrue(shapefile_layer.isValid())

        _test(shapefile_layer)

        gpkg_path = tempfile.mktemp('.gpkg')

        # Test a geopackage
        _, _ = QgsVectorFileWriter.writeAsVectorFormat(
            memory_layer,
            gpkg_path,
            'utf-8',
            memory_layer.crs(),
            'GPKG',
            False,
            [],
            [],
            False
        )

        gpkg_layer = QgsVectorLayer(gpkg_path, 'test', 'ogr')
        self.assertTrue(gpkg_layer.isValid())

        # No precision here
        _test(gpkg_layer, True)

    def test_representValueWithDefault(self):
        """
        Check representValue behaves correctly when used on a layer which define default values
        """

        dbname = os.path.join(tempfile.mkdtemp(), 'test.sqlite')
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = """
        CREATE TABLE test_table_default_values (
            id integer primary key autoincrement,
            anumber INTEGER DEFAULT 123
        )
        """
        cur.execute(sql)
        cur.execute("COMMIT")
        con.close()

        vl = QgsVectorLayer(dbname + '|layername=test_table_default_values', 'test_table_default_values', 'ogr')
        self.assertTrue(vl.isValid())

        fieldFormatter = QgsFallbackFieldFormatter()

        QLocale.setDefault(QLocale('en'))

        self.assertEqual(fieldFormatter.representValue(vl, 1, {}, None, QVariant(QVariant.Int)),
                         'NULL')
        self.assertEqual(fieldFormatter.representValue(vl, 1, {}, None, 4),
                         '4')
        self.assertEqual(fieldFormatter.representValue(vl, 1, {}, None, "123"),
                         '123')
        # bad field index
        self.assertEqual(fieldFormatter.representValue(vl, 3, {}, None, 5), "")


class TestQgsDateTimeFieldFormatter(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsDateTimeFieldFormatter.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsDateTimeFieldFormatter")
        QgsSettings().clear()
        QLocale.setDefault(QLocale(QLocale.English))
        start_app()

    @classmethod
    def tearDownClass(cls):
        """Reset locale"""
        QgsApplication.setLocale(QLocale(QLocale.English))

    def test_representValue(self):
        layer = QgsVectorLayer("point?field=datetime:datetime&field=date:date&field=time:time",
                               "layer", "memory")
        self.assertTrue(layer.isValid())
        QgsProject.instance().addMapLayers([layer])

        field_formatter = QgsDateTimeFieldFormatter()

        # if specific display format is set then use that
        config = {"display_format": "dd/MM/yyyy HH:mm:ss"}
        self.assertEqual(field_formatter.representValue(layer, 0, config, None,
                                                        QDateTime(QDate(2020, 3, 4), QTime(12, 13, 14), Qt.UTC)),
                         '04/03/2020 12:13:14')
        self.assertEqual(field_formatter.representValue(layer, 0, config, None,
                                                        QDateTime(QDate(2020, 3, 4), QTime(12, 13, 14), Qt.OffsetFromUTC, 3600)),
                         '04/03/2020 12:13:14')

        locale_assertions = {
            QLocale(QLocale.English): {
                "date_format": 'M/d/yy',
                "time_format": 'HH:mm:ss',
                "datetime_format": 'M/d/yy HH:mm:ss',
                "datetime_utc": '3/4/20 12:13:14 (UTC)',
                "datetime_utc+1": '3/4/20 12:13:14 (UTC+01:00)'
            },
            QLocale(QLocale.Finnish): {
                "date_format": 'd.M.yyyy',
                "time_format": 'HH:mm:ss',
                "datetime_format": 'd.M.yyyy HH:mm:ss',
                "datetime_utc": '4.3.2020 12:13:14 (UTC)',
                "datetime_utc+1": '4.3.2020 12:13:14 (UTC+01:00)'
            },
        }

        for locale, assertions in locale_assertions.items():
            QgsApplication.setLocale(locale)
            field_formatter = QgsDateTimeFieldFormatter()

            self.assertEqual(field_formatter.defaultFormat(QVariant.Date), assertions["date_format"], locale.name())
            self.assertEqual(field_formatter.defaultFormat(QVariant.Time), assertions["time_format"], locale.name())
            self.assertEqual(field_formatter.defaultFormat(QVariant.DateTime), assertions["datetime_format"], locale.name())

            # default configuration should show timezone information
            config = {}
            self.assertEqual(field_formatter.representValue(layer, 0, config, None,
                                                            QDateTime(QDate(2020, 3, 4), QTime(12, 13, 14), Qt.UTC)),
                             assertions["datetime_utc"], locale.name())
            self.assertEqual(field_formatter.representValue(layer, 0, config, None,
                                                            QDateTime(QDate(2020, 3, 4), QTime(12, 13, 14), Qt.OffsetFromUTC, 3600)),
                             assertions["datetime_utc+1"], locale.name())
            self.assertEqual(field_formatter.representValue(layer, 1, config, None,
                                                            QDate(2020, 3, 4)),
                             assertions["datetime_utc"].split(" ")[0], locale.name())
            config = {"display_format": "HH:mm:s"}
            self.assertEqual(field_formatter.representValue(layer, 2, config, None,
                                                            QTime(12, 13, 14)),
                             assertions["datetime_utc"].split(" ")[1], locale.name())


if __name__ == '__main__':
    unittest.main()
