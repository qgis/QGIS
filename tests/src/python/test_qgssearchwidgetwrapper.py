"""QGIS Unit tests for QgsSearchWidgetWrapper.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2016-05"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, QTime
from qgis.PyQt.QtWidgets import QWidget
from qgis.core import QgsFeature, QgsProject, QgsRelation, QgsVectorLayer
from qgis.gui import (
    QgsCheckboxSearchWidgetWrapper,
    QgsDateTimeSearchWidgetWrapper,
    QgsDefaultSearchWidgetWrapper,
    QgsRelationReferenceSearchWidgetWrapper,
    QgsSearchWidgetWrapper,
    QgsValueMapSearchWidgetWrapper,
    QgsValueRelationSearchWidgetWrapper,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class PyQgsSearchWidgetWrapper(QgisTestCase):

    def testFlagToString(self):
        # test converting QgsSearchWidgetWrapper.FilterFlag to string
        tests = [
            QgsSearchWidgetWrapper.FilterFlag.EqualTo,
            QgsSearchWidgetWrapper.FilterFlag.NotEqualTo,
            QgsSearchWidgetWrapper.FilterFlag.GreaterThan,
            QgsSearchWidgetWrapper.FilterFlag.LessThan,
            QgsSearchWidgetWrapper.FilterFlag.GreaterThanOrEqualTo,
            QgsSearchWidgetWrapper.FilterFlag.LessThanOrEqualTo,
            QgsSearchWidgetWrapper.FilterFlag.Between,
            QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive,
            QgsSearchWidgetWrapper.FilterFlag.Contains,
            QgsSearchWidgetWrapper.FilterFlag.DoesNotContain,
            QgsSearchWidgetWrapper.FilterFlag.IsNull,
            QgsSearchWidgetWrapper.FilterFlag.IsNotNull,
            QgsSearchWidgetWrapper.FilterFlag.IsNotBetween,
        ]
        for t in tests:
            self.assertGreater(len(QgsSearchWidgetWrapper.toString(t)), 0)

    def testExclusiveFlags(self):
        # test flag exclusive/non exclusive
        exclusive = QgsSearchWidgetWrapper.exclusiveFilterFlags()
        non_exclusive = QgsSearchWidgetWrapper.nonExclusiveFilterFlags()
        for e in exclusive:
            self.assertNotIn(e, non_exclusive)


class PyQgsDefaultSearchWidgetWrapper(QgisTestCase):

    def testCreateExpression(self):
        """Test creating an expression using the widget"""
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=flddate:datetime",
            "test",
            "memory",
        )

        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 0)
        w.initWidget(parent)

        line_edit = w.lineEdit()
        line_edit.setText("test")
        case_sensitive = w.caseSensitiveCheckBox()

        case_sensitive.setChecked(False)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "lower(\"fldtxt\")=lower('test')",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "lower(\"fldtxt\")<>lower('test')",
        )
        case_sensitive.setChecked(True)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='test'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"fldtxt\"<>'test'",
        )
        case_sensitive.setChecked(False)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.Contains),
            "\"fldtxt\" ILIKE '%test%'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.DoesNotContain),
            "NOT (\"fldtxt\" ILIKE '%test%')",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.StartsWith),
            "\"fldtxt\" ILIKE 'test%'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EndsWith),
            "\"fldtxt\" ILIKE '%test'",
        )
        case_sensitive.setChecked(True)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.Contains),
            "\"fldtxt\" LIKE '%test%'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.DoesNotContain),
            "NOT (\"fldtxt\" LIKE '%test%')",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.StartsWith),
            "\"fldtxt\" LIKE 'test%'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EndsWith),
            "\"fldtxt\" LIKE '%test'",
        )
        case_sensitive.setChecked(False)

        # numeric field
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 1)
        w.initWidget(parent)

        # may need updating if widget layout changes:
        line_edit = w.lineEdit()
        line_edit.setText("5.5")
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            '"fldint"=5.5',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            '"fldint"<>5.5',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThan),
            '"fldint">5.5',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThan),
            '"fldint"<5.5',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThanOrEqualTo),
            '"fldint">=5.5',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThanOrEqualTo),
            '"fldint"<=5.5',
        )

        # date/time/datetime
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 2)
        w.initWidget(parent)

        # may need updating if widget layout changes:
        line_edit = w.lineEdit()
        line_edit.setText("2015-06-03")
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"flddate\"='2015-06-03'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"flddate\"<>'2015-06-03'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThan),
            "\"flddate\">'2015-06-03'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThan),
            "\"flddate\"<'2015-06-03'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThanOrEqualTo),
            "\"flddate\">='2015-06-03'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThanOrEqualTo),
            "\"flddate\"<='2015-06-03'",
        )


class PyQgsValueMapSearchWidgetWrapper(QgisTestCase):

    def testCreateExpression(self):
        """Test creating an expression using the widget"""
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "test", "memory"
        )

        w = QgsValueMapSearchWidgetWrapper(layer, 0)
        config = {"map": [{"val1": 1}, {"val2": 200}]}
        w.setConfig(config)
        c = w.widget()

        # first, set it to the "select value" item
        c.setCurrentIndex(0)

        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo), ""
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo), ""
        )

        c.setCurrentIndex(1)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='1'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"fldtxt\"<>'1'",
        )
        c.setCurrentIndex(2)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='200'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"fldtxt\"<>'200'",
        )

        # try with numeric field
        w = QgsValueMapSearchWidgetWrapper(layer, 1)
        w.setConfig(config)
        c = w.widget()
        c.setCurrentIndex(1)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldint" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldint" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo), '"fldint"=1'
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            '"fldint"<>1',
        )


class PyQgsValueRelationSearchWidgetWrapper(QgisTestCase):

    def testCreateExpression(self):
        """Test creating an expression using the widget"""
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "test", "memory"
        )
        # setup value relation
        parent_layer = QgsVectorLayer(
            "Point?field=stringkey:string&field=intkey:integer&field=display:string",
            "parent",
            "memory",
        )
        f1 = QgsFeature(parent_layer.fields(), 1)
        f1.setAttributes(["a", 1, "value a"])
        f2 = QgsFeature(parent_layer.fields(), 2)
        f2.setAttributes(["b", 2, "value b"])
        f3 = QgsFeature(parent_layer.fields(), 3)
        f3.setAttributes(["c", 3, "value c"])
        parent_layer.dataProvider().addFeatures([f1, f2, f3])
        QgsProject.instance().addMapLayers([layer, parent_layer])

        config = {"Layer": parent_layer.id(), "Key": "stringkey", "Value": "display"}

        w = QgsValueRelationSearchWidgetWrapper(layer, 0)
        w.setConfig(config)
        c = w.widget()

        # first, set it to the "select value" item
        c.setCurrentIndex(0)

        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo), ""
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo), ""
        )

        c.setCurrentIndex(1)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='a'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"fldtxt\"<>'a'",
        )
        c.setCurrentIndex(2)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='b'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"fldtxt\"<>'b'",
        )

        # try with numeric field
        w = QgsValueRelationSearchWidgetWrapper(layer, 1)
        config["Key"] = "intkey"
        w.setConfig(config)
        c = w.widget()
        c.setCurrentIndex(c.findText("value c"))
        self.assertEqual(c.currentIndex(), 3)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldint" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldint" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo), '"fldint"=3'
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            '"fldint"<>3',
        )

        # try with allow null set
        w = QgsValueRelationSearchWidgetWrapper(layer, 1)
        config["AllowNull"] = True
        w.setConfig(config)
        c = w.widget()
        c.setCurrentIndex(c.findText("value c"))
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldint" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldint" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo), '"fldint"=3'
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            '"fldint"<>3',
        )

        # try with line edit
        w = QgsValueRelationSearchWidgetWrapper(layer, 1)
        config["UseCompleter"] = True
        w.setConfig(config)
        l = w.widget()
        l.setText("value b")
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldint" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldint" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo), '"fldint"=2'
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            '"fldint"<>2',
        )


class PyQgsCheckboxSearchWidgetWrapper(QgisTestCase):

    def testCreateExpression(self):
        """Test creating an expression using the widget"""
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=fieldbool:bool",
            "test",
            "memory",
        )

        w = QgsCheckboxSearchWidgetWrapper(layer, 0)
        config = {"CheckedState": 5, "UncheckedState": 9}
        w.setConfig(config)
        c = w.widget()

        # first check with string field type
        c.setChecked(True)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='5'",
        )
        c.setChecked(False)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='9'",
        )

        # try with numeric field
        w = QgsCheckboxSearchWidgetWrapper(layer, 1)
        w.setConfig(config)
        c = w.widget()
        c.setChecked(True)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldint" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldint" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo), '"fldint"=5'
        )
        c.setChecked(False)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldint" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldint" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo), '"fldint"=9'
        )

        # Check boolean expression
        parent = QWidget()
        w = QgsCheckboxSearchWidgetWrapper(layer, 2)
        w.initWidget(parent)
        c = w.widget()
        c.setChecked(True)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            '"fieldbool"=true',
        )
        c.setChecked(False)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            '"fieldbool"=false',
        )


class PyQgsDateTimeSearchWidgetWrapper(QgisTestCase):

    def testCreateExpression(self):
        """Test creating an expression using the widget"""
        layer = QgsVectorLayer(
            "Point?field=date:date&field=time:time&field=datetime:datetime",
            "test",
            "memory",
        )

        w = QgsDateTimeSearchWidgetWrapper(layer, 0)
        config = {"field_format": "yyyy-MM-dd", "display_format": "yyyy-MM-dd"}
        w.setConfig(config)
        c = w.widget()

        # first check with date field type
        c.setDateTime(QDateTime(QDate(2013, 4, 5), QTime()))
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"date" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"date" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"date\"='2013-04-05'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"date\"<>'2013-04-05'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThan),
            "\"date\">'2013-04-05'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThan),
            "\"date\"<'2013-04-05'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThanOrEqualTo),
            "\"date\">='2013-04-05'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThanOrEqualTo),
            "\"date\"<='2013-04-05'",
        )

        # time field type
        w = QgsDateTimeSearchWidgetWrapper(layer, 1)
        config = {"field_format": "HH:mm:ss", "display_format": "HH:mm:ss"}
        w.setConfig(config)
        c = w.widget()

        c.setDateTime(QDateTime(QDate(2013, 4, 5), QTime(13, 14, 15)))
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"time" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"time" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"time\"='13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"time\"<>'13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThan),
            "\"time\">'13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThan),
            "\"time\"<'13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThanOrEqualTo),
            "\"time\">='13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThanOrEqualTo),
            "\"time\"<='13:14:15'",
        )

        # datetime field type
        w = QgsDateTimeSearchWidgetWrapper(layer, 2)
        config = {
            "field_format": "yyyy-MM-dd HH:mm:ss",
            "display_format": "yyyy-MM-dd HH:mm:ss",
        }
        w.setConfig(config)
        c = w.widget()

        c.setDateTime(QDateTime(QDate(2013, 4, 5), QTime(13, 14, 15)))
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"datetime" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"datetime" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"datetime\"='2013-04-05 13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"datetime\"<>'2013-04-05 13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThan),
            "\"datetime\">'2013-04-05 13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThan),
            "\"datetime\"<'2013-04-05 13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.GreaterThanOrEqualTo),
            "\"datetime\">='2013-04-05 13:14:15'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.LessThanOrEqualTo),
            "\"datetime\"<='2013-04-05 13:14:15'",
        )


class PyQgsRelationReferenceSearchWidgetWrapper(QgisTestCase):

    def testCreateExpression(self):
        """Test creating an expression using the widget"""
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "test", "memory"
        )
        # setup value relation
        parent_layer = QgsVectorLayer(
            "Point?field=stringkey:string&field=intkey:integer&field=display:string",
            "parent",
            "memory",
        )
        f1 = QgsFeature(parent_layer.fields(), 1)
        f1.setAttributes(["a", 1, "value a"])
        f2 = QgsFeature(parent_layer.fields(), 2)
        f2.setAttributes(["b", 2, "value b"])
        f3 = QgsFeature(parent_layer.fields(), 3)
        f3.setAttributes(["c", 3, "value c"])
        parent_layer.dataProvider().addFeatures([f1, f2, f3])
        QgsProject.instance().addMapLayers([layer, parent_layer])

        relationManager = QgsProject.instance().relationManager()
        relation = QgsRelation()
        relation.setId("relation")
        relation.setReferencingLayer(layer.id())
        relation.setReferencedLayer(parent_layer.id())
        relation.addFieldPair("fldtxt", "stringkey")
        self.assertTrue(relation.isValid())

        relationManager.addRelation(relation)

        # Everything valid
        config = {"Relation": relation.id(), "AllowNULL": True}

        w = QgsRelationReferenceSearchWidgetWrapper(layer, 0, None)
        w.setConfig(config)

        w.widget().setForeignKey("a")
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='a'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"fldtxt\"<>'a'",
        )

        w.widget().setForeignKey("b")
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='b'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"fldtxt\"<>'b'",
        )

        w.widget().setForeignKey("c")
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            "\"fldtxt\"='c'",
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            "\"fldtxt\"<>'c'",
        )

        w.widget().setForeignKey(None)
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNull),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.IsNotNull),
            '"fldtxt" IS NOT NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.EqualTo),
            '"fldtxt" IS NULL',
        )
        self.assertEqual(
            w.createExpression(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo),
            '"fldtxt" IS NOT NULL',
        )


if __name__ == "__main__":
    unittest.main()
