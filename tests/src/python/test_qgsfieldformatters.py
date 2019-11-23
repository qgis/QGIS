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

import qgis  # NOQA

from qgis.core import (QgsFeature, QgsProject, QgsRelation, QgsVectorLayer,
                       QgsValueMapFieldFormatter, QgsValueRelationFieldFormatter,
                       QgsRelationReferenceFieldFormatter, QgsRangeFieldFormatter,
                       QgsCheckBoxFieldFormatter, QgsSettings, QgsGeometry, QgsPointXY)

from qgis.PyQt.QtCore import QCoreApplication, QLocale
from qgis.testing import start_app, unittest

start_app()


class TestQgsValueMapFieldFormatter(unittest.TestCase):

    VALUEMAP_NULL_TEXT = "{2839923C-8B7D-419E-B84B-CA2FE9B80EC7}"

    def test_representValue(self):
        QgsSettings().setValue("qgis/nullValue", "NULL")
        layer = QgsVectorLayer("none?field=number1:integer&field=number2:double&field=text1:string&field=number3:integer&field=number4:double&field=text2:string",
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
        _test(r'["a string,comma","a string\"quote", "another string[]"]', ['a string,comma', 'a string"quote', 'another string[]'])

    def test_expressionRequiresFormScope(self):

        res = list(QgsValueRelationFieldFormatter.expressionFormAttributes("current_value('ONE') AND current_value('TWO')"))
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
        self.assertTrue(QgsValueRelationFieldFormatter.expressionIsUsable("@current_geometry current_value ( 'pkid' )", f))

        QgsProject.instance().removeMapLayer(layer.id())


class TestQgsRelationReferenceFieldFormatter(unittest.TestCase):

    def test_representValue(self):

        first_layer = QgsVectorLayer("none?field=foreign_key:integer",
                                     "first_layer", "memory")
        self.assertTrue(first_layer.isValid())
        second_layer = QgsVectorLayer("none?field=pkid:integer&field=decoded:string",
                                      "second_layer", "memory")
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

        # No display expression
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression(None)
        self.assertEqual(fieldFormatter.representValue(first_layer, 0, config, None, '123'), '123')

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
        self.assertEqual(fieldFormatter.representValue(layer, 0, {'Precision': 1}, None, '9999999'), '9,999,999')  # no scientific notation for integers!
        self.assertEqual(fieldFormatter.representValue(layer, 0, {'Precision': 1}, None, None), 'NULL')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '123'), '123')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '123000'), '123,000')
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '9999999'), '9,999,999')  # no scientific notation for long longs!
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
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '9999999'), '9.999.999')  # scientific notation for long longs!
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
        self.assertEqual(fieldFormatter.representValue(layer, 2, {'Precision': 1}, None, '9999999'), '9999999')  # scientific notation for long longs!
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
        self.assertEqual(field_formatter.representValue(layer, 0, {'UncheckedState': 0, 'CheckedState': 1}, None, 1), 'true')
        self.assertEqual(field_formatter.representValue(layer, 0, {'UncheckedState': 0, 'CheckedState': 1}, None, 0), 'false')
        self.assertEqual(field_formatter.representValue(layer, 0, {'UncheckedState': 0, 'CheckedState': 1}, None, 10), "(10)")
        # invert true/false
        self.assertEqual(field_formatter.representValue(layer, 0, {'UncheckedState': 1, 'CheckedState': 0}, None, 0), 'true')
        self.assertEqual(field_formatter.representValue(layer, 0, {'UncheckedState': 1, 'CheckedState': 0}, None, 1), 'false')

        # test with string
        self.assertEqual(field_formatter.representValue(layer, 1, {'UncheckedState': 'nooh', 'CheckedState': 'yeah'}, None, 'yeah'), 'true')
        self.assertEqual(field_formatter.representValue(layer, 1, {'UncheckedState': 'nooh', 'CheckedState': 'yeah'}, None, 'nooh'), 'false')
        self.assertEqual(field_formatter.representValue(layer, 1, {'UncheckedState': 'nooh', 'CheckedState': 'yeah'}, None, 'oops'), "(oops)")


if __name__ == '__main__':
    unittest.main()
