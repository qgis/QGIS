"""QGIS Unit tests for QgsAttributeFormEditorWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2016-05"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, QTime, QVariant, QTemporaryDir
from qgis.PyQt.QtWidgets import QDateTimeEdit, QWidget
from qgis.core import QgsVectorLayer, QgsField, QgsFeature
from qgis.gui import (
    QgsAttributeForm,
    QgsAttributeFormEditorWidget,
    QgsDefaultSearchWidgetWrapper,
    QgsGui,
    QgsSearchWidgetWrapper,
    QgsAttributeFormWidget,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from osgeo import gdal, ogr, osr

start_app()
QgsGui.editorWidgetRegistry().initEditors()


class PyQgsAttributeFormEditorWidget(QgisTestCase):

    def testCurrentFilterExpression(self):
        """Test creating an expression using the widget"""

        layer = QgsVectorLayer("Point?field=fldint:integer", "test", "memory")
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 0, parent)
        setup = QgsGui.editorWidgetRegistry().findBest(layer, "fldint")
        wrapper = QgsGui.editorWidgetRegistry().create(layer, 0, None, parent)
        af = QgsAttributeFormEditorWidget(wrapper, setup.type(), None)
        af.setSearchWidgetWrapper(w)
        af.setMode(QgsAttributeFormWidget.Mode.SearchMode)

        # test that filter combines both current value in search widget wrapper and flags from search tool button
        w.lineEdit().setText("5.5")
        sb = af.findChild(QWidget, "SearchWidgetToolButton")
        sb.setActiveFlags(QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertEqual(af.currentFilterExpression(), '"fldint"=5.5')
        sb.setActiveFlags(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertEqual(af.currentFilterExpression(), '"fldint"<>5.5')

    def testSetActive(self):
        """Test setting the search as active - should set active flags to match search widget wrapper's defaults"""

        layer = QgsVectorLayer(
            "Point?field=fldtext:string&field=fldint:integer", "test", "memory"
        )
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 0, parent)
        setup = QgsGui.editorWidgetRegistry().findBest(layer, "fldint")
        wrapper = QgsGui.editorWidgetRegistry().create(layer, 0, None, parent)
        af = QgsAttributeFormEditorWidget(wrapper, setup.type(), None)
        af.setSearchWidgetWrapper(w)
        af.setMode(QgsAttributeFormWidget.Mode.SearchMode)

        sb = af.findChild(QWidget, "SearchWidgetToolButton")
        # start with inactive
        sb.setActiveFlags(QgsSearchWidgetWrapper.FilterFlags())
        # set to inactive
        sb.setActive()
        # check that correct default flag was taken from search widget wrapper
        self.assertTrue(sb.activeFlags() & QgsSearchWidgetWrapper.FilterFlag.Contains)

        # try again with numeric field - default should be "EqualTo"
        w = QgsDefaultSearchWidgetWrapper(layer, 1, parent)
        af.setSearchWidgetWrapper(w)
        # start with inactive
        sb.setActiveFlags(QgsSearchWidgetWrapper.FilterFlags())
        # set to inactive
        sb.setActive()
        # check that correct default flag was taken from search widget wrapper
        self.assertTrue(sb.activeFlags() & QgsSearchWidgetWrapper.FilterFlag.EqualTo)

    def testBetweenFilter(self):
        """Test creating a between type filter"""
        layer = QgsVectorLayer(
            "Point?field=fldtext:string&field=fldint:integer", "test", "memory"
        )
        form = QgsAttributeForm(layer)
        wrapper = QgsGui.editorWidgetRegistry().create(layer, 0, None, form)
        af = QgsAttributeFormEditorWidget(wrapper, "DateTime", None)
        af.createSearchWidgetWrappers()
        af.setMode(QgsAttributeFormWidget.Mode.SearchMode)

        d1 = af.findChildren(QDateTimeEdit)[0]
        d2 = af.findChildren(QDateTimeEdit)[1]
        d1.setDateTime(QDateTime(QDate(2013, 5, 6), QTime()))
        d2.setDateTime(QDateTime(QDate(2013, 5, 16), QTime()))

        sb = af.findChild(QWidget, "SearchWidgetToolButton")
        sb.setActiveFlags(QgsSearchWidgetWrapper.FilterFlag.Between)
        self.assertEqual(
            af.currentFilterExpression(),
            "\"fldtext\">='2013-05-06' AND \"fldtext\"<='2013-05-16'",
        )
        sb.setActiveFlags(QgsSearchWidgetWrapper.FilterFlag.IsNotBetween)
        self.assertEqual(
            af.currentFilterExpression(),
            "\"fldtext\"<'2013-05-06' OR \"fldtext\">'2013-05-16'",
        )

    def verifyJSONTypeFindBest(self, field_type, layer):

        # Create a JSON field
        field = QgsField("json", field_type, "JSON", 0, 0, "comment", QVariant.String)
        self.assertTrue(layer.startEditing())
        self.assertTrue(layer.addAttribute(field))
        self.assertTrue(layer.commitChanges())

        # No records, so should default to key/value
        registry = QgsGui.editorWidgetRegistry()
        setup = registry.findBest(layer, "json")
        self.assertEqual(setup.type(), "KeyValue")

        # Add a key/value record
        layer.startEditing()
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", '{"key": "value"}')
        self.assertTrue(layer.addFeature(feature))
        setup = registry.findBest(layer, "json")
        self.assertEqual(setup.type(), "KeyValue")
        layer.rollBack()

        # Add an array record
        self.assertTrue(layer.startEditing())
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", '["value", "another_value"]')
        self.assertTrue(layer.addFeature(feature))
        setup = registry.findBest(layer, "json")
        self.assertEqual(setup.type(), "List")
        self.assertTrue(layer.rollBack())

        # Add a null record followed by a map record followed by a list record
        self.assertTrue(layer.startEditing())
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", None)
        self.assertTrue(layer.addFeature(feature))
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", '{"key": "value"}')
        self.assertTrue(layer.addFeature(feature))
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", '["value", "another_value"]')
        self.assertTrue(layer.addFeature(feature))
        setup = registry.findBest(layer, "json")
        self.assertEqual(setup.type(), "KeyValue")
        self.assertTrue(layer.rollBack())

        # Add a null record followed by A list record followed by a map record
        self.assertTrue(layer.startEditing())
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", None)
        self.assertTrue(layer.addFeature(feature))
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", '["value", "another_value"]')
        self.assertTrue(layer.addFeature(feature))
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", '{"key": "value"}')
        self.assertTrue(layer.addFeature(feature))
        setup = registry.findBest(layer, "json")
        self.assertEqual(setup.type(), "List")
        self.assertTrue(layer.rollBack())

        # Add a string record which is neither a list or a map
        self.assertTrue(layer.startEditing())
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", "not a list or map")
        self.assertTrue(layer.addFeature(feature))
        setup = registry.findBest(layer, "json")
        self.assertNotEqual(setup.type(), "List")
        self.assertNotEqual(setup.type(), "KeyValue")
        self.assertTrue(layer.rollBack())

        # Add 21 records with a JSON field, only the last is NOT NULL
        self.assertTrue(layer.startEditing())
        for i in range(20):
            feature = QgsFeature(layer.fields())
            feature.setAttribute("json", None)
            self.assertTrue(layer.addFeature(feature))
        feature = QgsFeature(layer.fields())
        feature.setAttribute("json", '["value", "another_value"]')
        self.assertTrue(layer.addFeature(feature))
        setup = registry.findBest(layer, "json")
        self.assertNotEqual(setup.type(), "List")
        # KeyValue is the default,
        self.assertEqual(setup.type(), "KeyValue")
        self.assertTrue(layer.rollBack())

        # Cleanup removing the field
        self.assertTrue(layer.startEditing())
        field_idx = layer.fields().indexOf("json")
        self.assertTrue(layer.deleteAttribute(field_idx))
        self.assertTrue(layer.commitChanges())

    def testJSONMemoryLayer(self):

        layer = QgsVectorLayer("Point?", "test", "memory")
        self.verifyJSONTypeFindBest(QVariant.Map, layer)

    def testJSONGeoPackageLayer(self):

        temp_dir = QTemporaryDir()
        uri = temp_dir.filePath("test.gpkg")
        # Create a new geopackage layer using ogr
        driver = ogr.GetDriverByName("GPKG")
        ds = driver.CreateDataSource(uri)
        srs = osr.SpatialReference()
        srs.ImportFromEPSG(4326)
        layer = ds.CreateLayer("test", srs, ogr.wkbPoint)
        del layer
        del ds
        layer = QgsVectorLayer(uri, "test", "ogr")
        self.verifyJSONTypeFindBest(QVariant.Map, layer)


if __name__ == "__main__":
    unittest.main()
