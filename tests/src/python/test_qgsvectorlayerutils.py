# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '25/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsProject,
                       QgsVectorLayer,
                       QgsVectorLayerUtils,
                       QgsFieldConstraints,
                       QgsFeature,
                       QgsFeatureIterator,
                       QgsGeometry,
                       QgsPointXY,
                       QgsDefaultValue,
                       QgsRelation,
                       QgsFields,
                       QgsField,
                       QgsMemoryProviderUtils,
                       QgsWkbTypes,
                       QgsCoordinateReferenceSystem,
                       NULL
                       )
from qgis.testing import start_app, unittest


start_app()


def createLayerWithOnePoint():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    assert pr.addFeatures([f])
    assert layer.featureCount() == 1
    return layer


class TestQgsVectorLayerUtils(unittest.TestCase):

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

        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test'))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test1'))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test4'))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, 'not present!'))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 123))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 124))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 127))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 99))

        # no layer
        self.assertFalse(QgsVectorLayerUtils.valueExists(None, 1, 123))
        # bad field indexes
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, -1, 'test'))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 100, 'test'))

        # with ignore list
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [3, 4, 5]))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [999999]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [2]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [99999, 2]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [3, 4, 5, 2]))

        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 125, [2, 4, 5]))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 125, [999999]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [3]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [99999, 3]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [2, 4, 5, 3]))

    def test_validate_attribute(self):
        """ test validating attributes against constraints """
        layer = createLayerWithOnePoint()

        # field expression check
        layer.setConstraintExpression(1, 'fldint>5')

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
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1, origin=QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # bad field expression check
        layer.setConstraintExpression(1, 'fldint>')
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

        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)

        # checking only for provider constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1, origin=QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # unique constraint
        f.setAttributes(["test123", 123])
        layer.removeFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintUnique)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)

        # checking only for provider constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1, origin=QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # checking only for soft constraints
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintUnique, QgsFieldConstraints.ConstraintStrengthHard)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1, strength=QgsFieldConstraints.ConstraintStrengthSoft)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)
        # checking for hard constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1,
                                                            strength=QgsFieldConstraints.ConstraintStrengthHard)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)

        # check - same id should be ignored when testing for uniqueness
        f1 = QgsFeature(1)
        f1.setAttributes(["test123", 123])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f1, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # test double constraint failure
        layer.setConstraintExpression(1, 'fldint>5')
        layer.removeFieldConstraint(1, QgsFieldConstraints.ConstraintUnique)
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull)
        f.setAttributes(["test123", NULL])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 2)
        print(errors)

    def testCreateUniqueValue(self):
        """ test creating a unique value """
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=flddbl:double",
                               "addfeat", "memory")
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
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0), 'test_4')
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0, 'test_1'), 'test_4')
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0, 'seed'), 'seed')
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0, 'superpig'), 'superpig_1')

    def testCreateFeature(self):
        """ test creating a feature respecting defaults and constraints """
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=flddbl:double",
                               "addfeat", "memory")
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
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: 'a', 2: 6.0})
        self.assertEqual(f.attributes(), ['a', NULL, 6.0])

        # layer with default value expression
        layer.setDefaultValueDefinition(2, QgsDefaultValue('3*4'))
        f = QgsVectorLayerUtils.createFeature(layer)
        self.assertEqual(f.attributes(), [NULL, NULL, 12])
        # we do not expect the default value expression to take precedence over the attribute map
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: 'a', 2: 6.0})
        self.assertEqual(f.attributes(), ['a', NULL, 6.0])
        # layer with default value expression based on geometry
        layer.setDefaultValueDefinition(2, QgsDefaultValue('3*$x'))
        f = QgsVectorLayerUtils.createFeature(layer, g)
        #adjusted so that input value and output feature are the same
        self.assertEqual(f.attributes(), [NULL, NULL, 300.0])
        layer.setDefaultValueDefinition(2, QgsDefaultValue(None))

        # test with violated unique constraints
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintUnique)
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: 'test_1', 1: 123})
        # since field 1 has Unique Constraint, it ignores value 123 that already has been set and sets to 128
        self.assertEqual(f.attributes(), ['test_1', 128, NULL])
        layer.setFieldConstraint(0, QgsFieldConstraints.ConstraintUnique)
        # since field 0 and 1 already have values test_1 and 123, the output must be a new unique value
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: 'test_1', 1: 123})
        self.assertEqual(f.attributes(), ['test_4', 128, NULL])

        # test with violated unique constraints and default value expression providing unique value
        layer.setDefaultValueDefinition(1, QgsDefaultValue('130'))
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: 'test_1', 1: 123})
        # since field 1 has Unique Constraint, it ignores value 123 that already has been set and adds the default value
        self.assertEqual(f.attributes(), ['test_4', 130, NULL])
        # fallback: test with violated unique constraints and default value expression providing already existing value
        # add the feature with the default value:
        self.assertTrue(layer.dataProvider().addFeatures([f]))
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: 'test_1', 1: 123})
        # since field 1 has Unique Constraint, it ignores value 123 that already has been set and adds the default value
        # and since the default value providing an already existing value (130) it generates a unique value (next int: 131)
        self.assertEqual(f.attributes(), ['test_5', 131, NULL])
        layer.setDefaultValueDefinition(1, QgsDefaultValue(None))

        # test with manually correct unique constraint
        f = QgsVectorLayerUtils.createFeature(layer, attributes={0: 'test_1', 1: 132})
        self.assertEqual(f.attributes(), ['test_5', 132, NULL])

    def testDuplicateFeature(self):
        """ test duplicating a feature """

        project = QgsProject().instance()

        # LAYERS
        # - add first layer (parent)
        layer1 = QgsVectorLayer("Point?field=fldtxt:string&field=pkid:integer",
                                "parentlayer", "memory")
        # > check first layer (parent)
        self.assertTrue(layer1.isValid())
        # -  set the value for the copy
        layer1.setDefaultValueDefinition(1, QgsDefaultValue("rand(1000,2000)"))
        # > check first layer (parent)
        self.assertTrue(layer1.isValid())
        # - add second layer (child)
        layer2 = QgsVectorLayer("Point?field=fldtxt:string&field=id:integer&field=foreign_key:integer",
                                "childlayer", "memory")
        # > check second layer (child)
        self.assertTrue(layer2.isValid())
        # - add layers
        project.addMapLayers([layer1, layer2])

        # FEATURES
        # - add 2 features on layer1 (parent)
        l1f1orig = QgsFeature()
        l1f1orig.setFields(layer1.fields())
        l1f1orig.setAttributes(["F_l1f1", 100])
        l1f2orig = QgsFeature()
        l1f2orig.setFields(layer1.fields())
        l1f2orig.setAttributes(["F_l1f2", 101])
        # > check by adding features
        self.assertTrue(layer1.dataProvider().addFeatures([l1f1orig, l1f2orig]))
        # add 4 features on layer2 (child)
        l2f1orig = QgsFeature()
        l2f1orig.setFields(layer2.fields())
        l2f1orig.setAttributes(["F_l2f1", 201, 100])
        l2f2orig = QgsFeature()
        l2f2orig.setFields(layer2.fields())
        l2f2orig.setAttributes(["F_l2f2", 202, 100])
        l2f3orig = QgsFeature()
        l2f3orig.setFields(layer2.fields())
        l2f3orig.setAttributes(["F_l2f3", 203, 100])
        l2f4orig = QgsFeature()
        l2f4orig.setFields(layer2.fields())
        l2f4orig.setAttributes(["F_l2f4", 204, 101])
        # > check by adding features
        self.assertTrue(layer2.dataProvider().addFeatures([l2f1orig, l2f2orig, l2f3orig, l2f4orig]))

        # RELATION
        # - create the relationmanager
        relMgr = project.relationManager()
        # - create the relation
        rel = QgsRelation()
        rel.setId('rel1')
        rel.setName('childrel')
        rel.setReferencingLayer(layer2.id())
        rel.setReferencedLayer(layer1.id())
        rel.addFieldPair('foreign_key', 'pkid')
        rel.setStrength(QgsRelation.Composition)
        # > check relation
        self.assertTrue(rel.isValid())
        # - add relation
        relMgr.addRelation(rel)
        # > check if referencedLayer is layer1
        self.assertEqual(rel.referencedLayer(), layer1)
        # > check if referencingLayer is layer2
        self.assertEqual(rel.referencingLayer(), layer2)
        # > check if the layers are correct in relation when loading from relationManager
        relations = project.relationManager().relations()
        relation = relations[list(relations.keys())[0]]
        # > check if referencedLayer is layer1
        self.assertEqual(relation.referencedLayer(), layer1)
        # > check if referencingLayer is layer2
        self.assertEqual(relation.referencingLayer(), layer2)
        # > check the relatedfeatures

        '''
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
        '''

        # DUPLICATION
        # - duplicate feature l1f1orig with children
        layer1.startEditing()
        results = QgsVectorLayerUtils.duplicateFeature(layer1, l1f1orig, project, 0)

        # > check if name is name of duplicated (pk is different)
        result_feature = results[0]
        self.assertEqual(result_feature.attribute('fldtxt'), l1f1orig.attribute('fldtxt'))
        # > check duplicated child layer
        result_layer = results[1].layers()[0]
        self.assertEqual(result_layer, layer2)
        #  > check duplicated child features
        self.assertTrue(results[1].duplicatedFeatures(result_layer))

        '''
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
        '''

        # > compare text of parent feature
        self.assertEqual(result_feature.attribute('fldtxt'), l1f1orig.attribute('fldtxt'))

        # - create copyValueList
        childFeature = QgsFeature()
        relfeatit = rel.getRelatedFeatures(result_feature)
        copyValueList = []
        while relfeatit.nextFeature(childFeature):
            copyValueList.append(childFeature.attribute('fldtxt'))
        # - create origValueList
        childFeature = QgsFeature()
        relfeatit = rel.getRelatedFeatures(l1f1orig)
        origValueList = []
        while relfeatit.nextFeature(childFeature):
            origValueList.append(childFeature.attribute('fldtxt'))

        # - check if the ids are still the same
        self.assertEqual(copyValueList, origValueList)

    def test_make_features_compatible_attributes(self):
        """Test corner cases for attributes"""

        # Test feature with attributes
        fields = QgsFields()
        fields.append(QgsField('int_f', QVariant.Int))
        fields.append(QgsField('str_f', QVariant.String))
        f1 = QgsFeature(fields)
        f1['int_f'] = 1
        f1['str_f'] = 'str'
        f1.setGeometry(QgsGeometry.fromWkt('Point(9 45)'))
        f2 = f1
        QgsVectorLayerUtils.matchAttributesToFields(f2, fields)
        self.assertEqual(f1.attributes(), f2.attributes())
        self.assertTrue(f1.geometry().asWkt(), f2.geometry().asWkt())

        # Test pad with 0 with fields
        f1.setAttributes([])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 2)
        self.assertEqual(f1.attributes()[0], QVariant())
        self.assertEqual(f1.attributes()[1], QVariant())

        # Test pad with 0 without fields
        f1 = QgsFeature()
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 2)
        self.assertEqual(f1.attributes()[0], QVariant())
        self.assertEqual(f1.attributes()[1], QVariant())

        # Test drop extra attrs
        f1 = QgsFeature(fields)
        f1.setAttributes([1, 'foo', 'extra'])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 2)
        self.assertEqual(f1.attributes()[0], 1)
        self.assertEqual(f1.attributes()[1], 'foo')

        # Rearranged fields
        fields2 = QgsFields()
        fields2.append(QgsField('str_f', QVariant.String))
        fields2.append(QgsField('int_f', QVariant.Int))
        f1 = QgsFeature(fields2)
        f1.setAttributes([1, 'foo', 'extra'])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 2)
        self.assertEqual(f1.attributes()[0], 'foo')
        self.assertEqual(f1.attributes()[1], 1)

        # mixed
        fields2.append(QgsField('extra', QVariant.String))
        fields.append(QgsField('extra2', QVariant.Int))
        f1.setFields(fields2)
        f1.setAttributes([1, 'foo', 'blah'])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 3)
        self.assertEqual(f1.attributes()[0], 'foo')
        self.assertEqual(f1.attributes()[1], 1)
        self.assertEqual(f1.attributes()[2], QVariant())

        fields.append(QgsField('extra', QVariant.Int))
        f1.setAttributes([1, 'foo', 'blah'])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 4)
        self.assertEqual(f1.attributes()[0], 'foo')
        self.assertEqual(f1.attributes()[1], 1)
        self.assertEqual(f1.attributes()[2], QVariant())
        self.assertEqual(f1.attributes()[3], 'blah')

        # case insensitive
        fields2.append(QgsField('extra3', QVariant.String))
        fields.append(QgsField('EXTRA3', QVariant.Int))
        f1.setFields(fields2)
        f1.setAttributes([1, 'foo', 'blah', 'blergh'])
        QgsVectorLayerUtils.matchAttributesToFields(f1, fields)
        self.assertEqual(len(f1.attributes()), 5)
        self.assertEqual(f1.attributes()[0], 'foo')
        self.assertEqual(f1.attributes()[1], 1)
        self.assertEqual(f1.attributes()[2], QVariant())
        self.assertEqual(f1.attributes()[3], 'blah')
        self.assertEqual(f1.attributes()[4], 'blergh')


if __name__ == '__main__':
    unittest.main()
