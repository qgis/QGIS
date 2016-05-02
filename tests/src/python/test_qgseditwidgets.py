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

from qgis.core import QgsMapLayerRegistry, QgsFeature, QgsGeometry, QgsPoint, QgsProject, QgsRelation, QgsVectorLayer, NULL
from qgis.gui import QgsEditorWidgetRegistry

from qgis.testing import start_app, unittest

start_app()


class TestQgsTextEditWidget(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        QgsEditorWidgetRegistry.initEditors()

    def createLayerWithOnePoint(self):
        self.layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                    "addfeat", "memory")
        pr = self.layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPoint(QgsPoint(100, 200)))
        assert pr.addFeatures([f])
        assert self.layer.pendingFeatureCount() == 1
        return self.layer

    def doAttributeTest(self, idx, expected):
        reg = QgsEditorWidgetRegistry.instance()
        configWdg = reg.createConfigWidget('TextEdit', self.layer, idx, None)
        config = configWdg.config()
        editwidget = reg.create('TextEdit', self.layer, idx, config, None, None)

        editwidget.setValue('value')
        assert editwidget.value() == expected[0]

        editwidget.setValue(123)
        assert editwidget.value() == expected[1]

        editwidget.setValue(None)
        assert editwidget.value() == expected[2]

        editwidget.setValue(NULL)
        assert editwidget.value() == expected[3]

    def test_SetValue(self):
        self.createLayerWithOnePoint()

        self.doAttributeTest(0, ['value', '123', NULL, NULL])
        self.doAttributeTest(1, [NULL, 123, NULL, NULL])

    def test_ValueRelation_representValue(self):

        first_layer = QgsVectorLayer("none?field=foreign_key:integer",
                                     "first_layer", "memory")
        assert first_layer.isValid()
        second_layer = QgsVectorLayer("none?field=pkid:integer&field=decoded:string",
                                      "second_layer", "memory")
        assert second_layer.isValid()
        QgsMapLayerRegistry.instance().addMapLayer(second_layer)
        f = QgsFeature()
        f.setAttributes([123])
        assert first_layer.dataProvider().addFeatures([f])
        f = QgsFeature()
        f.setAttributes([123, 'decoded_val'])
        assert second_layer.dataProvider().addFeatures([f])

        reg = QgsEditorWidgetRegistry.instance()
        factory = reg.factory("ValueRelation")
        self.assertIsNotNone(factory)

        # Everything valid
        config = {'Layer': second_layer.id(), 'Key': 'pkid', 'Value': 'decoded'}
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '123'), 'decoded_val')

        # Code not find match in foreign layer
        config = {'Layer': second_layer.id(), 'Key': 'pkid', 'Value': 'decoded'}
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '456'), '(456)')

        # Missing Layer
        config = {'Key': 'pkid', 'Value': 'decoded'}
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '456'), '(456)')

        # Invalid Layer
        config = {'Layer': 'invalid', 'Key': 'pkid', 'Value': 'decoded'}
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '456'), '(456)')

        # Invalid Key
        config = {'Layer': second_layer.id(), 'Key': 'invalid', 'Value': 'decoded'}
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '456'), '(456)')

        # Invalid Value
        config = {'Layer': second_layer.id(), 'Key': 'pkid', 'Value': 'invalid'}
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '456'), '(456)')

        QgsMapLayerRegistry.instance().removeMapLayer(second_layer.id())

    def test_RelationReference_representValue(self):

        first_layer = QgsVectorLayer("none?field=foreign_key:integer",
                                     "first_layer", "memory")
        assert first_layer.isValid()
        second_layer = QgsVectorLayer("none?field=pkid:integer&field=decoded:string",
                                      "second_layer", "memory")
        assert second_layer.isValid()
        QgsMapLayerRegistry.instance().addMapLayers([first_layer, second_layer])
        f = QgsFeature()
        f.setAttributes([123])
        assert first_layer.dataProvider().addFeatures([f])
        f = QgsFeature()
        f.setAttributes([123, 'decoded_val'])
        assert second_layer.dataProvider().addFeatures([f])

        relMgr = QgsProject.instance().relationManager()

        reg = QgsEditorWidgetRegistry.instance()
        factory = reg.factory("RelationReference")
        self.assertIsNotNone(factory)

        rel = QgsRelation()
        rel.setRelationId('rel1')
        rel.setRelationName('Relation Number One')
        rel.setReferencingLayer(first_layer.id())
        rel.setReferencedLayer(second_layer.id())
        rel.addFieldPair('foreign_key', 'pkid')
        assert(rel.isValid())

        relMgr.addRelation(rel)

        # Everything valid
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '123'), 'decoded_val')

        # Code not find match in foreign layer
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '456'), '456')

        # Invalid relation id
        config = {'Relation': 'invalid'}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '123'), '123')

        # No display expression
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression(None)
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '123'), '123')

        # Invalid display expression
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('invalid +')
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '123'), '123')

        # Missing relation
        config = {}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '123'), '123')

        # Inconsistent layer provided to representValue()
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(factory.representValue(second_layer, 0, config, None, '123'), '123')

        # Inconsistent idx provided to representValue()
        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(factory.representValue(first_layer, 1, config, None, '123'), '123')

        # Invalid relation
        rel = QgsRelation()
        rel.setRelationId('rel2')
        rel.setRelationName('Relation Number Two')
        rel.setReferencingLayer(first_layer.id())
        rel.addFieldPair('foreign_key', 'pkid')
        self.assertFalse(rel.isValid())

        relMgr.addRelation(rel)

        config = {'Relation': rel.id()}
        second_layer.setDisplayExpression('decoded')
        self.assertEqual(factory.representValue(first_layer, 0, config, None, '123'), '123')

        QgsMapLayerRegistry.instance().removeAllMapLayers()

if __name__ == '__main__':
    unittest.main()
