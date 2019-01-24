# -*- coding: utf-8 -*-
"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '20/05/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsProject, QgsFeature, QgsGeometry, QgsPointXY, QgsVectorLayer, NULL, QgsField)
from qgis.gui import QgsGui

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import Qt, QVariant
from qgis.PyQt.QtWidgets import QTextEdit, QTableWidgetItem

start_app()


class TestQgsTextEditWidget(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        QgsGui.editorWidgetRegistry().initEditors()

    def createLayerWithOnePoint(self):
        self.layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                    "addfeat", "memory")
        pr = self.layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        self.assertTrue(pr.addFeatures([f]))
        self.assertEqual(self.layer.featureCount(), 1)
        return self.layer

    def doAttributeTest(self, idx, expected):
        reg = QgsGui.editorWidgetRegistry()
        configWdg = reg.createConfigWidget('TextEdit', self.layer, idx, None)
        config = configWdg.config()
        editwidget = reg.create('TextEdit', self.layer, idx, config, None, None)

        editwidget.setValue('value')
        self.assertEqual(editwidget.value(), expected[0])

        editwidget.setValue(123)
        self.assertEqual(editwidget.value(), expected[1])

        editwidget.setValue(None)
        self.assertEqual(editwidget.value(), expected[2])

        editwidget.setValue(NULL)
        self.assertEqual(editwidget.value(), expected[3])

    def test_SetValue(self):
        self.createLayerWithOnePoint()

        self.doAttributeTest(0, ['value', '123', NULL, NULL])
        self.doAttributeTest(1, [NULL, 123, NULL, NULL])

    def testStringWithMaxLen(self):
        """ tests that text edit wrappers correctly handle string fields with a maximum length """
        layer = QgsVectorLayer("none?field=fldint:integer", "layer", "memory")
        self.assertTrue(layer.isValid())
        layer.dataProvider().addAttributes([QgsField('max', QVariant.String, 'string', 10),
                                            QgsField('nomax', QVariant.String, 'string', 0)])
        layer.updateFields()
        QgsProject.instance().addMapLayer(layer)

        reg = QgsGui.editorWidgetRegistry()
        config = {'IsMultiline': 'True'}

        # first test for field without character limit
        editor = QTextEdit()
        editor.setPlainText('this_is_a_long_string')
        w = reg.create('TextEdit', layer, 2, config, editor, None)
        self.assertEqual(w.value(), 'this_is_a_long_string')

        # next test for field with character limit
        editor = QTextEdit()
        editor.setPlainText('this_is_a_long_string')
        w = reg.create('TextEdit', layer, 1, config, editor, None)

        self.assertEqual(w.value(), 'this_is_a_')

        QgsProject.instance().removeAllMapLayers()


class TestQgsValueRelationWidget(unittest.TestCase):

    def test_enableDisable(self):
        reg = QgsGui.editorWidgetRegistry()
        layer = QgsVectorLayer("none?field=number:integer", "layer", "memory")
        wrapper = reg.create('ValueRelation', layer, 0, {}, None, None)

        widget = wrapper.widget()

        self.assertTrue(widget.isEnabled())
        wrapper.setEnabled(False)
        self.assertFalse(widget.isEnabled())
        wrapper.setEnabled(True)
        self.assertTrue(widget.isEnabled())

    def test_enableDisableOnTableWidget(self):
        reg = QgsGui.editorWidgetRegistry()
        layer = QgsVectorLayer("none?field=number:integer", "layer", "memory")
        wrapper = reg.create('ValueRelation', layer, 0, {'AllowMulti': 'True'}, None, None)

        widget = wrapper.widget()
        item = QTableWidgetItem('first item')
        widget.setItem(0, 0, item)

        # does not change the state the whole widget but the single items instead
        wrapper.setEnabled(False)
        # widget still true, but items false
        self.assertTrue(widget.isEnabled())
        self.assertNotEqual(widget.item(0, 0).flags(), widget.item(0, 0).flags() | Qt.ItemIsEnabled)
        wrapper.setEnabled(True)
        self.assertTrue(widget.isEnabled())
        self.assertEqual(widget.item(0, 0).flags(), widget.item(0, 0).flags() | Qt.ItemIsEnabled)


class TestQgsValueMapEditWidget(unittest.TestCase):
    VALUEMAP_NULL_TEXT = "{2839923C-8B7D-419E-B84B-CA2FE9B80EC7}"

    def test_ValueMap_set_get(self):
        layer = QgsVectorLayer("none?field=number:integer", "layer", "memory")
        self.assertTrue(layer.isValid())
        QgsProject.instance().addMapLayer(layer)
        reg = QgsGui.editorWidgetRegistry()
        configWdg = reg.createConfigWidget('ValueMap', layer, 0, None)

        config = {'map': [{'two': '2'}, {'twoandhalf': '2.5'}, {'NULL text': 'NULL'}, {'nothing': self.VALUEMAP_NULL_TEXT}]}

        # Set a configuration containing values and NULL and check if it
        # is returned intact.
        configWdg.setConfig(config)
        self.assertEqual(configWdg.config(), config)

        QgsProject.instance().removeAllMapLayers()


if __name__ == "__main__":
    unittest.main()
