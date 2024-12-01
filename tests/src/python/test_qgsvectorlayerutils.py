"""QGIS Unit tests for QgsVectorLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "25/10/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

import shutil
import tempfile

from qgis.PyQt.QtCore import QVariant
from qgis.core import (
    NULL,
    Qgis,
    QgsDefaultValue,
    QgsFeature,
    QgsField,
    QgsFieldConstraints,
    QgsFields,
    QgsGeometry,
    QgsPointXY,
    QgsProject,
    QgsRelation,
    QgsVectorLayer,
    QgsVectorLayerJoinInfo,
    QgsVectorLayerUtils,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


def createLayerWithOnePoint():
    layer = QgsVectorLayer(
        "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
    )
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    assert pr.addFeatures([f])
    assert layer.featureCount() == 1
    return layer


class TestQgsVectorLayerUtils(QgisTestCase):

    def test_field_is_read_only(self):
        """
        Test fieldIsReadOnly
        """
        layer = createLayerWithOnePoint()
        # layer is not editable => all fields are read only
        self.assertTrue(QgsVectorLayerUtils.fieldIsReadOnly(layer, 0))
        self.assertTrue(QgsVectorLayerUtils.fieldIsReadOnly(layer, 1))

        layer.startEditing()
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 0))
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 1))

        field = QgsField("test", QVariant.String)
        layer.addAttribute(field)
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 0))
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 1))
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 2))

        # simulate read-only field from provider
        field = QgsField("test2", QVariant.String)
        field.setReadOnly(True)
        layer.addAttribute(field)
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 0))
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 1))
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 2))
        self.assertTrue(QgsVectorLayerUtils.fieldIsReadOnly(layer, 3))

        layer.rollBack()
        layer.startEditing()

        # edit form config specifies read only
        form_config = layer.editFormConfig()
        form_config.setReadOnly(1, True)
        layer.setEditFormConfig(form_config)
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 0))
        self.assertTrue(QgsVectorLayerUtils.fieldIsReadOnly(layer, 1))
        form_config.setReadOnly(1, False)
        layer.setEditFormConfig(form_config)
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 0))
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 1))

        # joined field
        layer2 = QgsVectorLayer(
            "Point?field=fldtxt2:string&field=fldint:integer", "addfeat", "memory"
        )
        join_info = QgsVectorLayerJoinInfo()
        join_info.setJoinLayer(layer2)
        join_info.setJoinFieldName("fldint")
        join_info.setTargetFieldName("fldint")
        join_info.setUsingMemoryCache(True)
        layer.addJoin(join_info)
        layer.updateFields()

        self.assertEqual(
            [f.name() for f in layer.fields()], ["fldtxt", "fldint", "addfeat_fldtxt2"]
        )
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 0))
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 1))
        # join layer is not editable
        self.assertTrue(QgsVectorLayerUtils.fieldIsReadOnly(layer, 2))

        # make join editable
        layer.removeJoin(layer2.id())
        join_info.setEditable(True)
        layer.addJoin(join_info)
        layer.updateFields()
        self.assertEqual(
            [f.name() for f in layer.fields()], ["fldtxt", "fldint", "addfeat_fldtxt2"]
        )

        # should still be read only -- the join layer itself is not editable
        self.assertTrue(QgsVectorLayerUtils.fieldIsReadOnly(layer, 2))

        layer2.startEditing()
        self.assertFalse(QgsVectorLayerUtils.fieldIsReadOnly(layer, 2))

        # but now we set a property on the join layer which blocks editing for the feature...
        form_config = layer2.editFormConfig()
        form_config.setReadOnly(0, True)
        layer2.setEditFormConfig(form_config)
        # should now be read only -- the joined layer edit form config prohibits edits
        self.assertTrue(QgsVectorLayerUtils.fieldIsReadOnly(layer, 2))

    def test_field_editability_depends_on_feature(self):
        """
        Test QgsVectorLayerUtils.fieldEditabilityDependsOnFeature
        """
        layer = createLayerWithOnePoint()

        # not joined fields, so answer should be False
        self.assertFalse(QgsVectorLayerUtils.fieldEditabilityDependsOnFeature(layer, 0))
        self.assertFalse(QgsVectorLayerUtils.fieldEditabilityDependsOnFeature(layer, 1))

        # joined field
        layer2 = QgsVectorLayer(
            "Point?field=fldtxt2:string&field=fldint:integer", "addfeat", "memory"
        )
        join_info = QgsVectorLayerJoinInfo()
        join_info.setJoinLayer(layer2)
        join_info.setJoinFieldName("fldint")
        join_info.setTargetFieldName("fldint")
        join_info.setUsingMemoryCache(True)
        layer.addJoin(join_info)
        layer.updateFields()

        self.assertEqual(
            [f.name() for f in layer.fields()], ["fldtxt", "fldint", "addfeat_fldtxt2"]
        )
        self.assertFalse(QgsVectorLayerUtils.fieldEditabilityDependsOnFeature(layer, 0))
        self.assertFalse(QgsVectorLayerUtils.fieldEditabilityDependsOnFeature(layer, 1))
        # join layer is not editable => regardless of the feature, the field will always be read-only
        self.assertFalse(QgsVectorLayerUtils.fieldEditabilityDependsOnFeature(layer, 2))

        # make join editable
        layer.removeJoin(layer2.id())
        join_info.setEditable(True)
        join_info.setUpsertOnEdit(True)
        layer.addJoin(join_info)
        layer.updateFields()
        self.assertEqual(
            [f.name() for f in layer.fields()], ["fldtxt", "fldint", "addfeat_fldtxt2"]
        )

        # has upsert on edit => regardless of feature, we can create the join target to make the field editable
        self.assertFalse(QgsVectorLayerUtils.fieldEditabilityDependsOnFeature(layer, 2))

        layer.removeJoin(layer2.id())
        join_info.setEditable(True)
        join_info.setUpsertOnEdit(False)
        layer.addJoin(join_info)
        layer.updateFields()
        self.assertEqual(
            [f.name() for f in layer.fields()], ["fldtxt", "fldint", "addfeat_fldtxt2"]
        )

        # No upsert on edit => depending on feature, we either can edit the field or not, depending on whether
        # the join target feature already exists or not
        self.assertTrue(QgsVectorLayerUtils.fieldEditabilityDependsOnFeature(layer, 2))

    def test_value_exists(self):
        layer = createLayerWithOnePoint()
        # add some more features
        f1 = QgsFeature(2)
        f1.setAttributes(["test1", 124])
        f2 = QgsFeature(3)
        f2.setAttributes(["test2", 125])
        f3 = QgsFeature(4)
        f3.setAttributes(["test3", 126])
        f4 = QgsFeature(5)
        f4.setAttributes(["test4", 127])
        layer.dataProvider().addFeatures([f1, f2, f3, f4])

        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, "test"))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, "test1"))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, "test4"))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, "not present!"))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 123))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 124))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 127))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 99))

        # no layer
        self.assertFalse(QgsVectorLayerUtils.valueExists(None, 1, 123))
        # bad field indexes
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, -1, "test"))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 100, "test"))

        # with ignore list
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, "test1", [3, 4, 5]))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, "test1", [999999]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, "test1", [2]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, "test1", [99999, 2]))
        self.assertFalse(
            QgsVectorLayerUtils.valueExists(layer, 0, "test1", [3, 4, 5, 2])
        )

        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 125, [2, 4, 5]))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 125, [999999]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [3]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [99999, 3]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [2, 4, 5, 3]))

    def test_value_exists_joins(self):
        """Test that unique values in fields from joined layers, see GH #36167"""

        p = QgsProject()
        main_layer = QgsVectorLayer("Point?field=fid:integer", "main_layer", "memory")
        self.assertTrue(main_layer.isValid())
        # Attr layer is joined with layer on fk ->
        attr_layer = QgsVectorLayer(
            "Point?field=id:integer&field=fk:integer", "attr_layer", "memory"
        )
        self.assertTrue(attr_layer.isValid())

        p.addMapLayers([main_layer, attr_layer])
        join_info = QgsVectorLayerJoinInfo()
        join_info.setJoinLayer(attr_layer)
        join_info.setJoinFieldName("fk")
        join_info.setTargetFieldName("fid")
        join_info.setUsingMemoryCache(True)
        main_layer.addJoin(join_info)
        main_layer.updateFields()
        join_buffer = main_layer.joinBuffer()
        self.assertTrue(join_buffer.containsJoins())
        self.assertEqual(main_layer.fields().names(), ["fid", "attr_layer_id"])

        f = QgsFeature(main_layer.fields())
        f.setAttributes([1])
        main_layer.dataProvider().addFeatures([f])

        f = QgsFeature(attr_layer.fields())
        f.setAttributes([1, 1])
        attr_layer.dataProvider().addFeatures([f])

        self.assertTrue(QgsVectorLayerUtils.valueExists(main_layer, 0, 1))
        self.assertTrue(QgsVectorLayerUtils.valueExists(main_layer, 1, 1))
        self.assertFalse(QgsVectorLayerUtils.valueExists(main_layer, 0, 2))
        self.assertFalse(QgsVectorLayerUtils.valueExists(main_layer, 1, 2))

    def test_validate_attribute(self):
        """test validating attributes against constraints"""
        layer = createLayerWithOnePoint()

        # field expression check
        self.assertFalse(QgsVectorLayerUtils.attributeHasConstraints(layer, 1))
        layer.setConstraintExpression(1, "fldint>5")
        self.assertTrue(QgsVectorLayerUtils.attributeHasConstraints(layer, 1))

        f = QgsFeature(2)
        f.setAttributes(["test123", 6])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)
        f.setAttributes(["test123", 2])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)
        # checking only for provider constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(
            layer,
            f,
            1,
            origin=QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider,
        )
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # bad field expression check
        layer.setConstraintExpression(1, "fldint>")
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)

        layer.setConstraintExpression(1, None)

        # not null constraint
        f.setAttributes(["test123", NULL])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        self.assertFalse(QgsVectorLayerUtils.attributeHasConstraints(layer, 1))
        layer.setFieldConstraint(1, QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertTrue(QgsVectorLayerUtils.attributeHasConstraints(layer, 1))

        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)

        # checking only for provider constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(
            layer,
            f,
            1,
            origin=QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider,
        )
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # unique constraint
        f.setAttributes(["test123", 123])
        layer.removeFieldConstraint(1, QgsFieldConstraints.Constraint.ConstraintNotNull)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        self.assertFalse(QgsVectorLayerUtils.attributeHasConstraints(layer, 1))
        layer.setFieldConstraint(1, QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertTrue(QgsVectorLayerUtils.attributeHasConstraints(layer, 1))

        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)

        # checking only for provider constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(
            layer,
            f,
            1,
            origin=QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider,
        )
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # checking only for soft constraints
        layer.setFieldConstraint(
            1,
            QgsFieldConstraints.Constraint.ConstraintUnique,
            QgsFieldConstraints.ConstraintStrength.ConstraintStrengthHard,
        )
        res, errors = QgsVectorLayerUtils.validateAttribute(
            layer,
            f,
            1,
            strength=QgsFieldConstraints.ConstraintStrength.ConstraintStrengthSoft,
        )
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)
        # checking for hard constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(
            layer,
            f,
            1,
            strength=QgsFieldConstraints.ConstraintStrength.ConstraintStrengthHard,
        )
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)

        # check - same id should be ignored when testing for uniqueness
        f1 = QgsFeature(1)
        f1.setAttributes(["test123", 123])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f1, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # test double constraint failure
        layer.setConstraintExpression(1, "fldint>5")
        layer.removeFieldConstraint(1, QgsFieldConstraints.Constraint.ConstraintUnique)
        layer.setFieldConstraint(1, QgsFieldConstraints.Constraint.ConstraintNotNull)
        f.setAttributes(["test123", NULL])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 2)
        print(errors)

    def testCreateUniqueValue(self):
        """test creating a unique value"""
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=flddbl:double",
            "addfeat",
            "memory",
        )
        # add a bunch of features
        f = QgsFeature()
        f.setAttributes(["test", 123, 1.0])
        f1 = QgsFeature(2)
        f1.setAttributes(["test_1", 124, 1.1])
        f2 = QgsFeature(3)
        f2.setAttributes(["test_2", 125, 2.4])
        f3 = QgsFeature(4)
        f3.setAttributes(["test_3", 126, 1.7])
        f4 = QgsFeature(5)
        f4.setAttributes(["superpig", 127, 0.8])
        self.assertTrue(layer.dataProvider().addFeatures([f, f1, f2, f3, f4]))

        # bad field indices
        self.assertFalse(QgsVectorLayerUtils.createUniqueValue(layer, -10))
        self.assertFalse(QgsVectorLayerUtils.createUniqueValue(layer, 10))

        # integer field
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 1), 128)

        # double field
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 2), 3.0)

        # string field
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0), "test_4")
        self.assertEqual(
            QgsVectorLayerUtils.createUniqueValue(layer, 0, "test_1"), "test_4"
        )
        self.assertEqual(
            QgsVectorLayerUtils.createUniqueValue(layer, 0, "seed"), "seed"
        )
        self.assertEqual(
            QgsVectorLayerUtils.createUniqueValue(layer, 0, "superpig"), "superpig_1"
        )

    def testCreateFeature(self):
        """test creating a feature respecting defaults and constraints"""
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=flddbl:double",
            "addfeat",
            "memory",
        )
        # add a bunch of features
        f = QgsFeature()
        f.setAttributes(["test", 123, 1.0])
        f1 = QgsFeature(2)
        f1.setAttributes(["test_1", 124, 1.1])
        f2 = QgsFeature(3)
        f2.setAttributes(["test_2", 125, 2.4])
        f3 = QgsFeature(4)
        f3.setAttributes(["test_3", 126, 1.7])
        f4 = QgsFeature(5)
        f4.setAttributes(["superpig", 127, 0.8])
        self.assertTrue(layer.dataProvider().addFeatures([f, f1, f2, f3, f4]))

        # no layer
        self.assertFalse(QgsVectorLayerUtils.createFeature(None).isValid())

        # basic tests
        f = QgsVectorLayerUtils.createFeature(layer)
        self.assertTrue(f.isValid())
        self.assertEqual(f.fields(), layer.fields())
        self.assertFalse(f.hasGeometry())
        self.assertEqual(f.attributes(), [NULL, NULL, NULL])

        # set geometry
        g = QgsGeometry.fromPointXY(QgsPointXY(100, 200))
        f = QgsVectorLayerUtils.createFeature(layer, g)
        self.assertTrue(f.hasGeometry())
        self.assertEqual(f.geometry().asWkt(), g.asWkt())

        # using attribute map
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: "a", 2: 6.0})
        self.assertEqual(f.attributes(), ["a", NULL, 6.0])

        # layer with default value expression
        layer.setDefaultValueDefinition(2, QgsDefaultValue("3*4"))
        f = QgsVectorLayerUtils.createFeature(layer)
        self.assertEqual(f.attributes(), [NULL, NULL, 12])
        # we do not expect the default value expression to take precedence over the attribute map
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: "a", 2: 6.0})
        self.assertEqual(f.attributes(), ["a", NULL, 6.0])
        # default value takes precedence if it's apply on update
        layer.setDefaultValueDefinition(2, QgsDefaultValue("3*4", True))
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: "a", 2: 6.0})
        self.assertEqual(f.attributes(), ["a", NULL, 12.0])
        # layer with default value expression based on geometry
        layer.setDefaultValueDefinition(2, QgsDefaultValue("3*$x"))
        f = QgsVectorLayerUtils.createFeature(layer, g)
        # adjusted so that input value and output feature are the same
        self.assertEqual(f.attributes(), [NULL, NULL, 300.0])
        layer.setDefaultValueDefinition(2, QgsDefaultValue(None))

        # test with violated unique constraints
        layer.setFieldConstraint(1, QgsFieldConstraints.Constraint.ConstraintUnique)
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: "test_1", 1: 123})
        # since field 1 has Unique Constraint, it ignores value 123 that already has been set and sets to 128
        self.assertEqual(f.attributes(), ["test_1", 128, NULL])
        layer.setFieldConstraint(0, QgsFieldConstraints.Constraint.ConstraintUnique)
        # since field 0 and 1 already have values test_1 and 123, the output must be a new unique value
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: "test_1", 1: 123})
        self.assertEqual(f.attributes(), ["test_4", 128, NULL])

        # test with violated unique constraints and default value expression providing unique value
        layer.setDefaultValueDefinition(1, QgsDefaultValue("130"))
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: "test_1", 1: 123})
        # since field 1 has Unique Constraint, it ignores value 123 that already has been set and adds the default value
        self.assertEqual(f.attributes(), ["test_4", 130, NULL])
        # fallback: test with violated unique constraints and default value expression providing already existing value
        # add the feature with the default value:
        self.assertTrue(layer.dataProvider().addFeatures([f]))
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: "test_1", 1: 123})
        # since field 1 has Unique Constraint, it ignores value 123 that already has been set and adds the default value
        # and since the default value providing an already existing value (130) it generates a unique value (next int: 131)
        self.assertEqual(f.attributes(), ["test_5", 131, NULL])
        layer.setDefaultValueDefinition(1, QgsDefaultValue(None))

        # test with manually correct unique constraint
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: "test_1", 1: 132})
        self.assertEqual(f.attributes(), ["test_5", 132, NULL])

    def testDuplicateFeature(self):
        """test duplicating a feature with relations"""

        project = QgsProject().instance()

        # LAYERS
        # - add first layer (parent)
        layer1 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=pkid:integer&field=policycheck1value:text&field=policycheck2value:text&field=policycheck3value:text",
            "parentlayer",
            "memory",
        )
        # > check first layer (parent)
        self.assertTrue(layer1.isValid())
        # -  set the default values for pk and policy check and the field policy
        layer1.setDefaultValueDefinition(1, QgsDefaultValue("rand(1000,2000)"))
        layer1.setDefaultValueDefinition(2, QgsDefaultValue("'Def Blabla L1'"))
        layer1.setDefaultValueDefinition(3, QgsDefaultValue("'Def Blabla L1'"))
        layer1.setDefaultValueDefinition(4, QgsDefaultValue("'Def Blabla L1'"))
        layer1.setFieldDuplicatePolicy(2, Qgis.FieldDuplicatePolicy.Duplicate)
        layer1.setFieldDuplicatePolicy(3, Qgis.FieldDuplicatePolicy.DefaultValue)
        layer1.setFieldDuplicatePolicy(4, Qgis.FieldDuplicatePolicy.UnsetField)
        # > check first layer (parent)
        self.assertTrue(layer1.isValid())
        # - add second layer (child)
        layer2 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=id:integer&field=foreign_key:integer&field=policycheck1value:text&field=policycheck2value:text&field=policycheck3value:text",
            "childlayer1",
            "memory",
        )
        # > check second layer (child)
        self.assertTrue(layer2.isValid())
        # -  set the default values for pk and policy check and the field policy
        layer2.setDefaultValueDefinition(3, QgsDefaultValue("'Def Blabla L2'"))
        layer2.setDefaultValueDefinition(4, QgsDefaultValue("'Def Blabla L2'"))
        layer2.setDefaultValueDefinition(5, QgsDefaultValue("'Def Blabla L2'"))
        layer2.setFieldDuplicatePolicy(3, Qgis.FieldDuplicatePolicy.Duplicate)
        layer2.setFieldDuplicatePolicy(4, Qgis.FieldDuplicatePolicy.DefaultValue)
        layer2.setFieldDuplicatePolicy(5, Qgis.FieldDuplicatePolicy.UnsetField)
        # > check second layer (child)
        self.assertTrue(layer2.isValid())
        # - add third layer (child)
        layer3 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=id:integer&field=foreign_key:integer&field=policycheck1value:text&field=policycheck2value:text&field=policycheck3value:text",
            "childlayer2",
            "memory",
        )
        # > check third layer (child)
        self.assertTrue(layer3.isValid())
        # -  set the default values for pk and policy check and the field policy
        layer3.setDefaultValueDefinition(3, QgsDefaultValue("'Def Blabla L3'"))
        layer3.setDefaultValueDefinition(4, QgsDefaultValue("'Def Blabla L3'"))
        layer3.setDefaultValueDefinition(5, QgsDefaultValue("'Def Blabla L3'"))
        layer3.setFieldDuplicatePolicy(3, Qgis.FieldDuplicatePolicy.Duplicate)
        layer3.setFieldDuplicatePolicy(4, Qgis.FieldDuplicatePolicy.DefaultValue)
        layer3.setFieldDuplicatePolicy(5, Qgis.FieldDuplicatePolicy.UnsetField)
        # > check third layer (child)
        self.assertTrue(layer3.isValid())
        # - add layers
        project.addMapLayers([layer1, layer2, layer3])

        # FEATURES
        # - add 2 features on layer1 (parent)
        l1f1orig = QgsFeature()
        l1f1orig.setFields(layer1.fields())
        l1f1orig.setAttributes(
            ["F_l1f1", 100, "Orig Blabla L1", "Orig Blabla L1", "Orig Blabla L1"]
        )
        l1f2orig = QgsFeature()
        l1f2orig.setFields(layer1.fields())
        l1f2orig.setAttributes(
            ["F_l1f2", 101, "Orig Blabla L1", "Orig Blabla L1", "Orig Blabla L1"]
        )
        # > check by adding features
        self.assertTrue(layer1.dataProvider().addFeatures([l1f1orig, l1f2orig]))
        # add 4 features on layer2 (child)
        l2f1orig = QgsFeature()
        l2f1orig.setFields(layer2.fields())
        l2f1orig.setAttributes(
            ["F_l2f1", 201, 100, "Orig Blabla L2", "Orig Blabla L2", "Orig Blabla L2"]
        )
        l2f2orig = QgsFeature()
        l2f2orig.setFields(layer2.fields())
        l2f2orig.setAttributes(
            ["F_l2f2", 202, 100, "Orig Blabla L2", "Orig Blabla L2", "Orig Blabla L2"]
        )
        l2f3orig = QgsFeature()
        l2f3orig.setFields(layer2.fields())
        l2f3orig.setAttributes(
            ["F_l2f3", 203, 100, "Orig Blabla L2", "Orig Blabla L2", "Orig Blabla L2"]
        )
        l2f4orig = QgsFeature()
        l2f4orig.setFields(layer2.fields())
        l2f4orig.setAttributes(
            ["F_l2f4", 204, 101, "Orig Blabla L2", "Orig Blabla L2", "Orig Blabla L2"]
        )
        # > check by adding features
        self.assertTrue(
            layer2.dataProvider().addFeatures([l2f1orig, l2f2orig, l2f3orig, l2f4orig])
        )
        # add 2 features on layer3 (child)
        l3f1orig = QgsFeature()
        l3f1orig.setFields(layer3.fields())
        l3f1orig.setAttributes(
            ["F_l3f1", 301, 100, "Orig Blabla L3", "Orig Blabla L3", "Orig Blabla L3"]
        )
        l3f2orig = QgsFeature()
        l3f2orig.setFields(layer2.fields())
        l3f2orig.setAttributes(
            ["F_l3f2", 302, 100, "Orig Blabla L3", "Orig Blabla L3", "Orig Blabla L3"]
        )
        # > check by adding features
        self.assertTrue(layer3.dataProvider().addFeatures([l3f1orig, l3f2orig]))

        # RELATION
        # - create the relationmanager
        relMgr = project.relationManager()
        # - create the first relation
        rel1 = QgsRelation()
        rel1.setId("rel1")
        rel1.setName("childrel1")
        rel1.setReferencingLayer(layer2.id())
        rel1.setReferencedLayer(layer1.id())
        rel1.addFieldPair("foreign_key", "pkid")
        rel1.setStrength(QgsRelation.RelationStrength.Composition)
        # > check relation
        self.assertTrue(rel1.isValid())
        # - add relation
        relMgr.addRelation(rel1)
        # > check if referencedLayer is layer1
        self.assertEqual(rel1.referencedLayer(), layer1)
        # > check if referencingLayer is layer2
        self.assertEqual(rel1.referencingLayer(), layer2)
        # - create the second relation
        rel2 = QgsRelation()
        rel2.setId("rel2")
        rel2.setName("childrel2")
        rel2.setReferencingLayer(layer3.id())
        rel2.setReferencedLayer(layer1.id())
        rel2.addFieldPair("foreign_key", "pkid")
        rel2.setStrength(QgsRelation.RelationStrength.Composition)
        # > check relation
        self.assertTrue(rel2.isValid())
        # - add relation
        relMgr.addRelation(rel2)
        # > check if referencedLayer is layer1
        self.assertEqual(rel2.referencedLayer(), layer1)
        # > check if referencingLayer is layer2
        self.assertEqual(rel2.referencingLayer(), layer3)

        # > check if the layers are correct in relation when loading from relationManager
        relations = project.relationManager().relationsByName("childrel1")
        relation = relations[0]
        # > check if referencedLayer is layer1
        self.assertEqual(relation.referencedLayer(), layer1)
        # > check if referencingLayer is layer2
        self.assertEqual(relation.referencingLayer(), layer2)
        relations = project.relationManager().relationsByName("childrel2")
        relation = relations[0]
        # > check if referencedLayer is layer1
        self.assertEqual(relation.referencedLayer(), layer1)
        # > check if referencingLayer is layer2
        self.assertEqual(relation.referencingLayer(), layer3)

        """
        # testoutput 1
        print( "\nAll Features and relations")
        featit=layer1.getFeatures()
        f=QgsFeature()
        while featit.nextFeature(f):
            print( f.attributes())
            childFeature = QgsFeature()
            relfeatit=rel.getRelatedFeatures(f)
            while relfeatit.nextFeature(childFeature):
                 print( childFeature.attributes() )
        print( "\n--------------------------")

        print( "\nFeatures on layer1")
        for f in layer1.getFeatures():
            print( f.attributes() )

        print( "\nFeatures on layer2")
        for f in layer2.getFeatures():
            print( f.attributes() )
        """

        # DUPLICATION
        # - duplicate feature l1f1orig with children
        layer1.startEditing()
        results = QgsVectorLayerUtils.duplicateFeature(layer1, l1f1orig, project, 0)

        # > check if name is name of duplicated (pk is different)
        # > and duplicate policy is concerned
        result_feature = results[0]
        self.assertEqual(
            result_feature.attribute("fldtxt"), l1f1orig.attribute("fldtxt")
        )
        self.assertEqual(
            result_feature.attribute("policycheck1value"), "Orig Blabla L1"
        )  # duplicated
        self.assertEqual(
            result_feature.attribute("policycheck2value"), "Def Blabla L1"
        )  # default Value
        self.assertEqual(result_feature.attribute("policycheck3value"), None)  # unset
        # > check duplicated children occurred on both layers
        self.assertEqual(len(results[1].layers()), 2)
        idx = results[1].layers().index(layer2)
        self.assertEqual(results[1].layers()[idx], layer2)
        self.assertTrue(results[1].duplicatedFeatures(layer2))
        for child_fid in results[1].duplicatedFeatures(layer2):
            child_feature = layer2.getFeature(child_fid)
            self.assertEqual(
                child_feature.attribute("policycheck1value"), "Orig Blabla L2"
            )  # duplicated
            self.assertEqual(
                child_feature.attribute("policycheck2value"), "Def Blabla L2"
            )  # default Value
            self.assertEqual(
                child_feature.attribute("policycheck3value"), None
            )  # unset
        idx = results[1].layers().index(layer3)
        self.assertEqual(results[1].layers()[idx], layer3)
        self.assertTrue(results[1].duplicatedFeatures(layer3))
        for child_fid in results[1].duplicatedFeatures(layer3):
            child_feature = layer3.getFeature(child_fid)
            self.assertEqual(
                child_feature.attribute("policycheck1value"), "Orig Blabla L3"
            )  # duplicated
            self.assertEqual(
                child_feature.attribute("policycheck2value"), "Def Blabla L3"
            )  # default Value
            self.assertEqual(
                child_feature.attribute("policycheck3value"), None
            )  # unset

        """
        # testoutput 2
        print( "\nFeatures on layer1 (after duplication)")
        for f in layer1.getFeatures():
            print( f.attributes() )

        print( "\nFeatures on layer2 (after duplication)")
        for f in layer2.getFeatures():
            print( f.attributes() )

        print( "\nAll Features and relations")
        featit=layer1.getFeatures()
        f=QgsFeature()
        while featit.nextFeature(f):
            print( f.attributes())
            childFeature = QgsFeature()
            relfeatit=rel.getRelatedFeatures(f)
            while relfeatit.nextFeature(childFeature):
                 print( childFeature.attributes() )
        """

        # > compare text of parent feature
        self.assertEqual(
            result_feature.attribute("fldtxt"), l1f1orig.attribute("fldtxt")
        )

        # - create copyValueList
        childFeature = QgsFeature()
        relfeatit = rel1.getRelatedFeatures(result_feature)
        copyValueList = []
        while relfeatit.nextFeature(childFeature):
            copyValueList.append(childFeature.attribute("fldtxt"))
        # - create origValueList
        childFeature = QgsFeature()
        relfeatit = rel1.getRelatedFeatures(l1f1orig)
        origValueList = []
        while relfeatit.nextFeature(childFeature):
            origValueList.append(childFeature.attribute("fldtxt"))

        # - check if the ids are still the same
        self.assertEqual(copyValueList, origValueList)

    def test_make_features_compatible_attributes(self):
        """Test corner cases for attributes"""

        # Test feature with attributes
        fields = QgsFields()
        fields.append(QgsField("int_f", QVariant.Int))
        fields.append(QgsField("str_f", QVariant.String))
        f1 = QgsFeature(fields)
        f1["int_f"] = 1
        f1["str_f"] = "str"
        f1.setGeometry(QgsGeometry.fromWkt("Point(9 45)"))
        f2 = f1
        QgsVectorLayerUtils.matchAttributesToFields(f2, fields)
        self.assertEqual(f1.attributes(), f2.attributes())
        self.assertTrue(f1.geometry().asWkt(), f2.geometry().asWkt())

        # Test pad with 0 with fields
        f1.setAttributes([])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 2)
        self.assertEqual(f1.attributes()[0], NULL)
        self.assertEqual(f1.attributes()[1], NULL)

        # Test pad with 0 without fields
        f1 = QgsFeature()
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 2)
        self.assertEqual(f1.attributes()[0], NULL)
        self.assertEqual(f1.attributes()[1], NULL)

        # Test drop extra attrs
        f1 = QgsFeature(fields)
        f1.setAttributes([1, "foo", "extra"])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 2)
        self.assertEqual(f1.attributes()[0], 1)
        self.assertEqual(f1.attributes()[1], "foo")

        # Rearranged fields
        fields2 = QgsFields()
        fields2.append(QgsField("str_f", QVariant.String))
        fields2.append(QgsField("int_f", QVariant.Int))
        f1 = QgsFeature(fields2)
        f1.setAttributes([1, "foo", "extra"])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 2)
        self.assertEqual(f1.attributes()[0], "foo")
        self.assertEqual(f1.attributes()[1], 1)

        # mixed
        fields2.append(QgsField("extra", QVariant.String))
        fields.append(QgsField("extra2", QVariant.Int))
        f1.setFields(fields2)
        f1.setAttributes([1, "foo", "blah"])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 3)
        self.assertEqual(f1.attributes()[0], "foo")
        self.assertEqual(f1.attributes()[1], 1)
        self.assertEqual(f1.attributes()[2], NULL)

        fields.append(QgsField("extra", QVariant.Int))
        f1.setAttributes([1, "foo", "blah"])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 4)
        self.assertEqual(f1.attributes()[0], 1)
        self.assertEqual(f1.attributes()[1], "foo")
        self.assertEqual(f1.attributes()[2], "blah")
        self.assertEqual(f1.attributes()[3], NULL)

        # case insensitive
        fields2.append(QgsField("extra3", QVariant.String))
        fields.append(QgsField("EXTRA3", QVariant.Int))
        f1.setFields(fields2)
        f1.setAttributes([1, "foo", "blah", "blergh"])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 5)
        self.assertEqual(f1.attributes()[0], "foo")
        self.assertEqual(f1.attributes()[1], 1)
        self.assertEqual(f1.attributes()[2], NULL)
        self.assertEqual(f1.attributes()[3], "blah")
        self.assertEqual(f1.attributes()[4], "blergh")

    def test_create_multiple_unique_constraint(self):
        """Test create multiple features with unique constraint"""

        vl = createLayerWithOnePoint()
        vl.setFieldConstraint(1, QgsFieldConstraints.Constraint.ConstraintUnique)

        features_data = []
        context = vl.createExpressionContext()
        for i in range(2):
            features_data.append(
                QgsVectorLayerUtils.QgsFeatureData(
                    QgsGeometry.fromWkt("Point (7 44)"), {0: f"test_{i}", 1: 123}
                )
            )
        features = QgsVectorLayerUtils.createFeatures(vl, features_data, context)

        self.assertEqual(features[0].attributes()[1], 124)
        self.assertEqual(features[1].attributes()[1], 125)

    def test_create_nulls_and_defaults(self):
        """Test bug #21304 when pasting features from another layer and default values are not honored"""

        vl = createLayerWithOnePoint()
        vl.setDefaultValueDefinition(1, QgsDefaultValue("300"))

        features_data = []
        context = vl.createExpressionContext()
        features_data.append(
            QgsVectorLayerUtils.QgsFeatureData(
                QgsGeometry.fromWkt("Point (7 44)"), {0: "test_1", 1: None}
            )
        )
        features_data.append(
            QgsVectorLayerUtils.QgsFeatureData(
                QgsGeometry.fromWkt("Point (7 45)"), {0: "test_2", 1: NULL}
            )
        )
        features_data.append(
            QgsVectorLayerUtils.QgsFeatureData(
                QgsGeometry.fromWkt("Point (7 46)"), {0: "test_3", 1: NULL}
            )
        )
        features_data.append(
            QgsVectorLayerUtils.QgsFeatureData(
                QgsGeometry.fromWkt("Point (7 46)"), {0: "test_4"}
            )
        )
        features = QgsVectorLayerUtils.createFeatures(vl, features_data, context)

        for f in features:
            self.assertEqual(f.attributes()[1], 300, f.id())

        vl = createLayerWithOnePoint()
        vl.setDefaultValueDefinition(0, QgsDefaultValue("'my_default'"))

        features_data = []
        context = vl.createExpressionContext()
        features_data.append(
            QgsVectorLayerUtils.QgsFeatureData(
                QgsGeometry.fromWkt("Point (7 44)"), {0: None}
            )
        )
        features_data.append(
            QgsVectorLayerUtils.QgsFeatureData(
                QgsGeometry.fromWkt("Point (7 45)"), {0: NULL}
            )
        )
        features_data.append(
            QgsVectorLayerUtils.QgsFeatureData(
                QgsGeometry.fromWkt("Point (7 46)"), {0: NULL}
            )
        )
        features_data.append(
            QgsVectorLayerUtils.QgsFeatureData(QgsGeometry.fromWkt("Point (7 46)"), {})
        )
        features = QgsVectorLayerUtils.createFeatures(vl, features_data, context)

        for f in features:
            self.assertEqual(f.attributes()[0], "my_default", f.id())

    def test_unique_pk_when_subset(self):
        """Test unique values on filtered layer GH #30062"""

        src = unitTestDataPath("points_gpkg.gpkg")
        dest = tempfile.mktemp() + ".gpkg"
        shutil.copy(src, dest)
        vl = QgsVectorLayer(dest, "vl", "ogr")
        self.assertTrue(vl.isValid())
        features_data = []
        it = vl.getFeatures()
        for _ in range(3):
            f = next(it)
            features_data.append(
                QgsVectorLayerUtils.QgsFeatureData(
                    f.geometry(), dict(zip(range(f.fields().count()), f.attributes()))
                )
            )
        # Set a filter
        vl.setSubsetString('"fid" in (4,5,6)')
        self.assertTrue(vl.isValid())
        context = vl.createExpressionContext()
        features = QgsVectorLayerUtils.createFeatures(vl, features_data, context)
        self.assertTrue(vl.startEditing())
        vl.addFeatures(features)
        self.assertTrue(vl.commitChanges())

    def testGuessFriendlyIdentifierField(self):
        """
        Test guessing a user friendly identifier field
        """
        fields = QgsFields()
        self.assertFalse(QgsVectorLayerUtils.guessFriendlyIdentifierField(fields))

        fields.append(QgsField("id", QVariant.Int))
        self.assertEqual(QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "id")
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierFieldV2(fields), ("id", False)
        )

        fields.append(QgsField("name", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "name"
        )
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierFieldV2(fields), ("name", True)
        )

        fields.append(QgsField("title", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "name"
        )
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierFieldV2(fields), ("name", True)
        )

        # regardless of actual field order, we prefer "name" over "title"
        fields = QgsFields()
        fields.append(QgsField("title", QVariant.String))
        fields.append(QgsField("name", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "name"
        )

        # test with an "anti candidate", which is a substring which makes a field containing "name" less preferred...
        fields = QgsFields()
        fields.append(QgsField("id", QVariant.Int))
        fields.append(QgsField("typename", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "typename"
        )
        fields.append(QgsField("title", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "title"
        )

        fields = QgsFields()
        fields.append(QgsField("id", QVariant.Int))
        fields.append(QgsField("classname", QVariant.String))
        fields.append(QgsField("x", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "classname"
        )
        fields.append(QgsField("desc", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "desc"
        )

        fields = QgsFields()
        fields.append(QgsField("id", QVariant.Int))
        fields.append(QgsField("areatypename", QVariant.String))
        fields.append(QgsField("areaadminname", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "areaadminname"
        )

        # if no good matches by name found, the first string field should be used
        fields = QgsFields()
        fields.append(QgsField("id", QVariant.Int))
        fields.append(QgsField("date", QVariant.Date))
        fields.append(QgsField("station", QVariant.String))
        fields.append(QgsField("org", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "station"
        )

        # Particular case for WFS layers analyzed with the GMLAS driver.
        # We prioritize a field ending with _name, but which is not gml_name
        fields = QgsFields()
        fields.append(QgsField("id", QVariant.String))
        fields.append(QgsField("gml_name", QVariant.String))
        fields.append(QgsField("other_name", QVariant.String))
        self.assertEqual(
            QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "other_name"
        )

        fields = QgsFields()
        fields.append(QgsField("id", QVariant.String))
        fields.append(QgsField("gml_name", QVariant.String))
        self.assertEqual(QgsVectorLayerUtils.guessFriendlyIdentifierField(fields), "id")


if __name__ == "__main__":
    unittest.main()
