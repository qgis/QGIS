# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSearchWidgetWrapper.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2016-05'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.gui import (QgsSearchWidgetWrapper,
                      QgsDefaultSearchWidgetWrapper,
                      QgsValueMapSearchWidgetWrapper,
                      QgsValueRelationSearchWidgetWrapper,
                      QgsCheckboxSearchWidgetWrapper,
                      QgsDateTimeSearchWidgetWrapper)
from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsMapLayerRegistry,
                       )
from qgis.PyQt.QtCore import QDateTime, QDate, QTime
from qgis.PyQt.QtWidgets import QWidget

from qgis.testing import start_app, unittest

start_app()


class PyQgsSearchWidgetWrapper(unittest.TestCase):

    def testFlagToString(self):
        # test converting QgsSearchWidgetWrapper.FilterFlag to string
        tests = [QgsSearchWidgetWrapper.EqualTo,
                 QgsSearchWidgetWrapper.NotEqualTo,
                 QgsSearchWidgetWrapper.GreaterThan,
                 QgsSearchWidgetWrapper.LessThan,
                 QgsSearchWidgetWrapper.GreaterThanOrEqualTo,
                 QgsSearchWidgetWrapper.LessThanOrEqualTo,
                 QgsSearchWidgetWrapper.Between,
                 QgsSearchWidgetWrapper.CaseInsensitive,
                 QgsSearchWidgetWrapper.Contains,
                 QgsSearchWidgetWrapper.DoesNotContain,
                 QgsSearchWidgetWrapper.IsNull,
                 QgsSearchWidgetWrapper.IsNotNull,
                 QgsSearchWidgetWrapper.IsNotBetween
                 ]
        for t in tests:
            self.assertTrue(len(QgsSearchWidgetWrapper.toString(t)) > 0)

    def testExclusiveFlags(self):
        # test flag exclusive/non exclusive
        exclusive = QgsSearchWidgetWrapper.exclusiveFilterFlags()
        non_exclusive = QgsSearchWidgetWrapper.nonExclusiveFilterFlags()
        for e in exclusive:
            self.assertFalse(e in non_exclusive)


class PyQgsDefaultSearchWidgetWrapper(unittest.TestCase):

    def testCreateExpression(self):
        """ Test creating an expression using the widget"""
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=flddate:datetime",
                               "test", "memory")

        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 0)
        w.initWidget(parent)

        line_edit = w.lineEdit()
        line_edit.setText('test')
        case_sensitive = w.caseSensitiveCheckBox()

        case_sensitive.setChecked(False)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldtxt" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldtxt" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), 'lower("fldtxt")=lower(\'test\')')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), 'lower("fldtxt")<>lower(\'test\')')
        case_sensitive.setChecked(True)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldtxt"=\'test\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldtxt"<>\'test\'')
        case_sensitive.setChecked(False)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.Contains), '"fldtxt" ILIKE \'%test%\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.DoesNotContain), 'NOT ("fldtxt" ILIKE \'%test%\')')
        case_sensitive.setChecked(True)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.Contains), '"fldtxt" LIKE \'%test%\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.DoesNotContain), 'NOT ("fldtxt" LIKE \'%test%\')')
        case_sensitive.setChecked(False)

        # numeric field
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 1)
        w.initWidget(parent)

        # may need updating if widget layout changes:
        line_edit = w.lineEdit()
        line_edit.setText('5.5')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldint"=5.5')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldint"<>5.5')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThan), '"fldint">5.5')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThan), '"fldint"<5.5')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThanOrEqualTo), '"fldint">=5.5')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThanOrEqualTo), '"fldint"<=5.5')

        # date/time/datetime
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 2)
        w.initWidget(parent)

        # may need updating if widget layout changes:
        line_edit = w.lineEdit()
        line_edit.setText('2015-06-03')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"flddate"=\'2015-06-03\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"flddate"<>\'2015-06-03\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThan), '"flddate">\'2015-06-03\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThan), '"flddate"<\'2015-06-03\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThanOrEqualTo), '"flddate">=\'2015-06-03\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThanOrEqualTo), '"flddate"<=\'2015-06-03\'')


class PyQgsValueMapSearchWidgetWrapper(unittest.TestCase):

    def testCreateExpression(self):
        """ Test creating an expression using the widget"""
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer", "test", "memory")

        w = QgsValueMapSearchWidgetWrapper(layer, 0)
        config = {"val1": 1,
                  "val2": 200}
        w.setConfig(config)
        c = w.widget()

        # first, set it to the "select value" item
        c.setCurrentIndex(0)

        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '')

        c.setCurrentIndex(1)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldtxt" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldtxt" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldtxt"=\'1\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldtxt"<>\'1\'')
        c.setCurrentIndex(2)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldtxt" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldtxt" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldtxt"=\'200\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldtxt"<>\'200\'')

        # try with numeric field
        w = QgsValueMapSearchWidgetWrapper(layer, 1)
        w.setConfig(config)
        c = w.widget()
        c.setCurrentIndex(1)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldint" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldint" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldint"=1')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldint"<>1')


class PyQgsValueRelationSearchWidgetWrapper(unittest.TestCase):

    def testCreateExpression(self):
        """ Test creating an expression using the widget"""
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer", "test", "memory")
        # setup value relation
        parent_layer = QgsVectorLayer("Point?field=stringkey:string&field=intkey:integer&field=display:string", "parent", "memory")
        f1 = QgsFeature(parent_layer.fields(), 1)
        f1.setAttributes(['a', 1, 'value a'])
        f2 = QgsFeature(parent_layer.fields(), 2)
        f2.setAttributes(['b', 2, 'value b'])
        f3 = QgsFeature(parent_layer.fields(), 3)
        f3.setAttributes(['c', 3, 'value c'])
        parent_layer.dataProvider().addFeatures([f1, f2, f3])
        QgsMapLayerRegistry.instance().addMapLayers([layer, parent_layer])

        config = {"Layer": parent_layer.id(),
                  "Key": 'stringkey',
                  "Value": 'display'}

        w = QgsValueRelationSearchWidgetWrapper(layer, 0)
        w.setConfig(config)
        c = w.widget()

        # first, set it to the "select value" item
        c.setCurrentIndex(0)

        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldtxt" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldtxt" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '')

        c.setCurrentIndex(1)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldtxt" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldtxt" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldtxt"=\'a\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldtxt"<>\'a\'')
        c.setCurrentIndex(2)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldtxt" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldtxt" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldtxt"=\'b\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldtxt"<>\'b\'')

        # try with numeric field
        w = QgsValueRelationSearchWidgetWrapper(layer, 1)
        config['Key'] = 'intkey'
        w.setConfig(config)
        c = w.widget()
        c.setCurrentIndex(c.findText('value c'))
        self.assertEqual(c.currentIndex(), 3)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldint" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldint" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldint"=3')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldint"<>3')

        # try with allow null set
        w = QgsValueRelationSearchWidgetWrapper(layer, 1)
        config['AllowNull'] = True
        w.setConfig(config)
        c = w.widget()
        c.setCurrentIndex(c.findText('value c'))
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldint" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldint" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldint"=3')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldint"<>3')

        # try with line edit
        w = QgsValueRelationSearchWidgetWrapper(layer, 1)
        config['UseCompleter'] = True
        w.setConfig(config)
        l = w.widget()
        l.setText('value b')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldint" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldint" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldint"=2')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"fldint"<>2')


class PyQgsCheckboxSearchWidgetWrapper(unittest.TestCase):

    def testCreateExpression(self):
        """ Test creating an expression using the widget"""
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer", "test", "memory")

        w = QgsCheckboxSearchWidgetWrapper(layer, 0)
        config = {"CheckedState": 5,
                  "UncheckedState": 9}
        w.setConfig(config)
        c = w.widget()

        # first check with string field type
        c.setChecked(True)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldtxt" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldtxt" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldtxt"=\'5\'')
        c.setChecked(False)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldtxt" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldtxt" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldtxt"=\'9\'')

        # try with numeric field
        w = QgsCheckboxSearchWidgetWrapper(layer, 1)
        w.setConfig(config)
        c = w.widget()
        c.setChecked(True)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldint" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldint" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldint"=5')
        c.setChecked(False)
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"fldint" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"fldint" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"fldint"=9')


class PyQgsDateTimeSearchWidgetWrapper(unittest.TestCase):

    def testCreateExpression(self):
        """ Test creating an expression using the widget"""
        layer = QgsVectorLayer("Point?field=date:date&field=time:time&field=datetime:datetime", "test", "memory")

        w = QgsDateTimeSearchWidgetWrapper(layer, 0)
        config = {"field_format": 'yyyy-MM-dd',
                  "display_format": 'yyyy-MM-dd'}
        w.setConfig(config)
        c = w.widget()

        # first check with date field type
        c.setDateTime(QDateTime(QDate(2013, 4, 5), QTime()))
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"date" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"date" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"date"=\'2013-04-05\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"date"<>\'2013-04-05\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThan), '"date">\'2013-04-05\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThan), '"date"<\'2013-04-05\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThanOrEqualTo), '"date">=\'2013-04-05\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThanOrEqualTo), '"date"<=\'2013-04-05\'')

        # time field type
        w = QgsDateTimeSearchWidgetWrapper(layer, 1)
        config = {"field_format": 'HH:mm:ss',
                  "display_format": 'HH:mm:ss'}
        w.setConfig(config)
        c = w.widget()

        c.setDateTime(QDateTime(QDate(2013, 4, 5), QTime(13, 14, 15)))
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"time" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"time" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"time"=\'13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"time"<>\'13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThan), '"time">\'13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThan), '"time"<\'13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThanOrEqualTo), '"time">=\'13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThanOrEqualTo), '"time"<=\'13:14:15\'')

        # datetime field type
        w = QgsDateTimeSearchWidgetWrapper(layer, 2)
        config = {"field_format": 'yyyy-MM-dd HH:mm:ss',
                  "display_format": 'yyyy-MM-dd HH:mm:ss'}
        w.setConfig(config)
        c = w.widget()

        c.setDateTime(QDateTime(QDate(2013, 4, 5), QTime(13, 14, 15)))
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNull), '"datetime" IS NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.IsNotNull), '"datetime" IS NOT NULL')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.EqualTo), '"datetime"=\'2013-04-05 13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.NotEqualTo), '"datetime"<>\'2013-04-05 13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThan), '"datetime">\'2013-04-05 13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThan), '"datetime"<\'2013-04-05 13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.GreaterThanOrEqualTo), '"datetime">=\'2013-04-05 13:14:15\'')
        self.assertEquals(w.createExpression(QgsSearchWidgetWrapper.LessThanOrEqualTo), '"datetime"<=\'2013-04-05 13:14:15\'')


if __name__ == '__main__':
    unittest.main()
