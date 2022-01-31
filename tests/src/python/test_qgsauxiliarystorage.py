# -*- coding: utf-8 -*-
"""QGIS Unit tests for Auxiliary Storage.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Paul Blottiere'
__date__ = '06/09/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QTemporaryFile, QVariant
from qgis.core import (QgsAuxiliaryStorage,
                       QgsAuxiliaryLayer,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPropertyDefinition,
                       QgsProperty,
                       QgsProject,
                       QgsProjectArchive,
                       QgsFeatureRequest,
                       QgsPalLayerSettings,
                       QgsSymbolLayer,
                       QgsVectorLayerSimpleLabeling,
                       QgsField,
                       QgsCallout,
                       QgsSimpleLineCallout,
                       NULL,
                       QgsDiagramLayerSettings,
                       QgsSingleCategoryDiagramRenderer)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath, writeShape

start_app()


def tmpPath():
    f = QTemporaryFile()
    f.open()
    f.close()
    os.remove(f.fileName())

    return f.fileName().replace('.', '_')


def createLayer():
    vl = QgsVectorLayer(
        'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
        'test', 'memory')
    assert (vl.isValid())

    f1 = QgsFeature()
    f1.setAttributes([5, -200, NULL, 'NuLl', '5'])
    f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

    f2 = QgsFeature()
    f2.setAttributes([3, 300, 'Pear', 'PEaR', '3'])

    f3 = QgsFeature()
    f3.setAttributes([1, 100, 'Orange', 'oranGe', '1'])
    f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

    f4 = QgsFeature()
    f4.setAttributes([2, 200, 'Apple', 'Apple', '2'])
    f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

    f5 = QgsFeature()
    f5.setAttributes([4, 400, 'Honey', 'Honey', '4'])
    f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

    vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])
    return vl


class TestQgsAuxiliaryStorage(unittest.TestCase):

    def testCreateSaveOpenStorageWithString(self):
        # Empty string in copy mode. A new database is created in a temporary
        # file.
        s0 = QgsAuxiliaryStorage()
        self.assertTrue(s0.isValid())

        # saveAs should be used instead of save in case of an empty string
        # given to the constructor of QgsAuxiliaryStorage
        self.assertEqual(s0.fileName(), "")
        self.assertFalse(s0.save())

        # Create a new auxiliary layer with 'pk' as key
        vl0 = createLayer()
        pkf = vl0.fields().field(vl0.fields().indexOf('pk'))
        al0 = s0.createAuxiliaryLayer(pkf, vl0)
        self.assertTrue(al0.isValid())

        # Test the auxiliary key
        key = al0.joinInfo().targetFieldName()
        self.assertEqual(key, 'pk')

        # Add a field in auxiliary layer
        p = QgsPropertyDefinition('propName', QgsPropertyDefinition.DataTypeNumeric, '', '', 'user')
        self.assertTrue(al0.addAuxiliaryField(p))

        # saveAs without saving the auxiliary layer, the auxiliary field is lost
        f = tmpPath()
        self.assertTrue(s0.saveAs(f))

        # Open the previous database.
        s1 = QgsAuxiliaryStorage(f)
        self.assertTrue(s1.isValid())

        # Load the auxiliary layer from auxiliary storage
        self.assertTrue(vl0.loadAuxiliaryLayer(s1, key))

        # As the vl0 has not been saved before saving the storage, there
        # shouldn't have auxiliary fields
        self.assertEqual(len(vl0.auxiliaryLayer().auxiliaryFields()), 0)

        # Save the layer before saving the storage
        self.assertTrue(al0.save())
        self.assertTrue(s0.saveAs(f))

        # Open the previous database.
        s2 = QgsAuxiliaryStorage(f)
        self.assertTrue(s2.isValid())

        # Load the auxiliary layer from auxiliary storage
        self.assertTrue(vl0.loadAuxiliaryLayer(s2, key))

        # As the vl0 has been saved before saving the storage, there
        # should have 1 auxiliary field
        self.assertEqual(len(vl0.auxiliaryLayer().auxiliaryFields()), 1)

        # save is available on s2
        self.assertTrue(s2.save())

    def testCreateSaveOpenStorageWithProject(self):
        # New project without fileName
        p = QgsProject()

        # Create storage
        s0 = QgsAuxiliaryStorage(p)
        self.assertTrue(s0.isValid())

        # saveAs should be used instead of save in case of an empty string
        # given to the constructor of QgsAuxiliaryStorage
        self.assertEqual(s0.fileName(), "")
        self.assertFalse(s0.save())

        # saveAs
        f = tmpPath()
        self.assertTrue(s0.saveAs(f))

    def testProjectStorage(self):
        # New project without fileName
        p0 = QgsProject()
        self.assertTrue(p0.auxiliaryStorage().isValid())

        # Create new layers with key otherwise auxiliary layers are not
        # automacially created when added in project
        vl0 = createLayer()
        vl0Shp = writeShape(vl0, 'vl0.shp')

        vl1 = createLayer()
        vl1Shp = writeShape(vl1, 'vl1.shp')

        vl0 = QgsVectorLayer(vl0Shp, 'points', 'ogr')
        self.assertTrue(vl0.isValid())

        vl1 = QgsVectorLayer(vl1Shp, 'points', 'ogr')
        self.assertTrue(vl1.isValid())

        # Add layers to project and check underlying auxiliary layers
        p0.addMapLayers([vl0, vl1])

        self.assertTrue(vl0.loadAuxiliaryLayer(p0.auxiliaryStorage(), 'pk'))
        self.assertTrue(vl1.loadAuxiliaryLayer(p0.auxiliaryStorage(), 'num_char'))

        al0 = vl0.auxiliaryLayer()
        al1 = vl1.auxiliaryLayer()

        self.assertEqual(al0.joinInfo().targetFieldName(), 'pk')
        self.assertEqual(al1.joinInfo().targetFieldName(), 'num_char')

        # Add a field in auxiliary layers
        pdef0 = QgsPropertyDefinition('propname', QgsPropertyDefinition.DataTypeNumeric, '', '', 'ut')
        self.assertTrue(al0.addAuxiliaryField(pdef0))

        pdef1 = QgsPropertyDefinition('propname1', QgsPropertyDefinition.DataTypeString, '', '', 'ut')
        self.assertTrue(al1.addAuxiliaryField(pdef1))

        # Check auxiliary fields names
        af0Name = QgsAuxiliaryLayer.nameFromProperty(pdef0, False)
        self.assertEqual(af0Name, 'ut_propname')
        af1Name = QgsAuxiliaryLayer.nameFromProperty(pdef1, False)
        self.assertEqual(af1Name, 'ut_propname1')

        # Set value for auxiliary fields
        req = QgsFeatureRequest().setFilterExpression("name = 'Honey'")
        f = QgsFeature()
        vl0.getFeatures(req).nextFeature(f)
        self.assertTrue(f.isValid())
        af0Name = QgsAuxiliaryLayer.nameFromProperty(pdef0, True)
        index0 = vl0.fields().indexOf(af0Name)
        vl0.changeAttributeValue(f.id(), index0, 333)

        req = QgsFeatureRequest().setFilterExpression("name = 'Apple'")
        f = QgsFeature()
        vl1.getFeatures(req).nextFeature(f)
        self.assertTrue(f.isValid())
        af1Name = QgsAuxiliaryLayer.nameFromProperty(pdef1, True)
        index1 = vl1.fields().indexOf(af1Name)
        vl1.changeAttributeValue(f.id(), index0, 'myvalue')

        req = QgsFeatureRequest().setFilterExpression("name = 'Orange'")
        f = QgsFeature()
        vl1.getFeatures(req).nextFeature(f)
        self.assertTrue(f.isValid())
        vl1.changeAttributeValue(f.id(), index0, 'myvalue1')

        # Save the project in a zip file
        f = tmpPath() + '.qgz'
        p0.write(f)

        # Open the zip file with embedded auxiliary storage
        p1 = QgsProject()
        p1.read(f)

        # Check that auxiliary fields are well loaded in layers
        self.assertEqual(len(p1.mapLayers().values()), 2)

        for vl in p1.mapLayers().values():
            al = vl.auxiliaryLayer()
            self.assertEqual(len(al.auxiliaryFields()), 1)

            af = al.auxiliaryFields()[0]
            afPropDef = QgsAuxiliaryLayer.propertyDefinitionFromField(af)
            self.assertEqual(afPropDef.origin(), 'ut')

            if vl.auxiliaryLayer().joinInfo().targetFieldName() == 'pk':
                self.assertEqual(afPropDef.name(), 'propname')
                self.assertEqual(al.featureCount(), 1)

                req = QgsFeatureRequest().setFilterExpression("name = 'Honey'")
                f = QgsFeature()
                vl.getFeatures(req).nextFeature(f)
                self.assertTrue(f.isValid())
                self.assertEqual(f.attributes()[index0], 333.0)
            else:  # num_char
                self.assertEqual(al.featureCount(), 2)
                self.assertEqual(afPropDef.name(), 'propname1')

                req = QgsFeatureRequest().setFilterExpression("name = 'Apple'")
                f = QgsFeature()
                vl.getFeatures(req).nextFeature(f)
                self.assertTrue(f.isValid())
                self.assertEqual(f.attributes()[index1], 'myvalue')

                req = QgsFeatureRequest().setFilterExpression("name = 'Orange'")
                f = QgsFeature()
                vl.getFeatures(req).nextFeature(f)
                self.assertTrue(f.isValid())
                self.assertEqual(f.attributes()[index1], 'myvalue1')

    def testAuxiliaryFieldWidgets(self):
        # Init storage
        s = QgsAuxiliaryStorage()
        self.assertTrue(s.isValid())

        # Create a new auxiliary layer with 'pk' as key
        vl = createLayer()
        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = s.createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())

        # Set the auxiliary layer to the vector layer
        vl.setAuxiliaryLayer(al)

        # Add a visible property
        p = QgsPropertyDefinition('propName', QgsPropertyDefinition.DataTypeNumeric, '', '', 'user')
        self.assertTrue(al.addAuxiliaryField(p))

        index = al.indexOfPropertyDefinition(p)
        self.assertFalse(al.isHiddenProperty(index))

        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        index = vl.fields().indexOf(afName)
        setup = vl.editorWidgetSetup(index)
        self.assertEqual(setup.type(), '')

        tested = False
        for c in vl.attributeTableConfig().columns():
            if c.name == afName:
                self.assertFalse(c.hidden)
                tested = True
                break
        self.assertTrue(tested)

        # Add a hidden property
        p = QgsPalLayerSettings.propertyDefinitions()[QgsPalLayerSettings.PositionX]
        self.assertTrue(al.addAuxiliaryField(p))

        index = al.indexOfPropertyDefinition(p)
        self.assertTrue(al.isHiddenProperty(index))

        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        index = vl.fields().indexOf(afName)
        setup = vl.editorWidgetSetup(index)
        self.assertEqual(setup.type(), 'Hidden')

        tested = False
        for c in vl.attributeTableConfig().columns():
            if c.name == afName:
                self.assertTrue(c.hidden)
                tested = True
                break
        self.assertTrue(tested)

        # Add a color property
        p = QgsSymbolLayer.propertyDefinitions()[QgsSymbolLayer.PropertyFillColor]
        self.assertTrue(al.addAuxiliaryField(p))

        index = al.indexOfPropertyDefinition(p)
        self.assertFalse(al.isHiddenProperty(index))

        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        index = vl.fields().indexOf(afName)
        setup = vl.editorWidgetSetup(index)
        self.assertEqual(setup.type(), 'Color')

    def testClear(self):
        s = QgsAuxiliaryStorage()
        self.assertTrue(s.isValid())

        # Create a new auxiliary layer with 'pk' as key
        vl = createLayer()
        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = s.createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())
        vl.setAuxiliaryLayer(al)

        # Add a field in auxiliary layer
        p = QgsPropertyDefinition('myprop', QgsPropertyDefinition.DataTypeNumeric, '', '', 'me')
        self.assertFalse(al.exists(p))
        self.assertTrue(al.addAuxiliaryField(p))
        self.assertTrue(al.exists(p))

        # Count auxiliary features
        self.assertEqual(al.featureCount(), 0)

        # Set value for auxiliary fields
        req = QgsFeatureRequest().setFilterExpression("name = 'Honey'")
        f = QgsFeature()
        vl.getFeatures(req).nextFeature(f)
        self.assertTrue(f.isValid())
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        index = vl.fields().indexOf(afName)
        vl.changeAttributeValue(f.id(), index, 333)

        # Count auxiliary features
        self.assertEqual(al.featureCount(), 1)

        # Clear and count
        al.clear()
        self.assertEqual(al.featureCount(), 0)

    def testSetAuxiliaryLayer(self):
        s = QgsAuxiliaryStorage()
        self.assertTrue(s.isValid())

        # Create a new auxiliary layer with 'pk' as key
        vl = createLayer()
        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = s.createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())
        vl.setAuxiliaryLayer(al)

        self.assertIsNotNone(vl.auxiliaryLayer())

        # Clear auxiliary layer
        al.clear()
        # Remove auxiliary layer
        vl.setAuxiliaryLayer()

        self.assertIsNone(vl.auxiliaryLayer())

    def testCreateProperty(self):
        s = QgsAuxiliaryStorage()
        self.assertTrue(s.isValid())

        # Create a new auxiliary layer with 'pk' as key
        vl = createLayer()
        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = s.createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())
        vl.setAuxiliaryLayer(al)

        # Create a new labeling property on layer without labels
        key = QgsPalLayerSettings.PositionX
        index = QgsAuxiliaryLayer.createProperty(key, vl)
        self.assertEqual(index, -1)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(QgsPalLayerSettings()))
        index = QgsAuxiliaryLayer.createProperty(key, vl)

        p = QgsPalLayerSettings.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        # with existing property
        key = QgsPalLayerSettings.PositionY
        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(key, QgsProperty.fromExpression('$y + 20'))
        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        # without overwriting existing, property should be upgraded to coalesce("aux field", 'existing expression') type
        index = QgsAuxiliaryLayer.createProperty(key, vl, False)
        p = QgsPalLayerSettings.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.labeling().settings()
        self.assertTrue(settings.dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.dataDefinedProperties().property(key).asExpression(), 'coalesce("auxiliary_storage_labeling_positiony",$y + 20)')

        # with existing but invalid field name
        key = QgsPalLayerSettings.PositionY
        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(key, QgsProperty.fromField(''))
        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        # even when asked to not overwrite existing, this is an invalid property and should be overwritten
        index = QgsAuxiliaryLayer.createProperty(key, vl, False)
        p = QgsPalLayerSettings.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.labeling().settings()
        self.assertTrue(settings.dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.dataDefinedProperties().property(key).asExpression(), '"auxiliary_storage_labeling_positiony"')

        # with existing valid field name
        key = QgsPalLayerSettings.PositionY
        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(key, QgsProperty.fromField('asd'))
        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        index = QgsAuxiliaryLayer.createProperty(key, vl, False)
        p = QgsPalLayerSettings.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.labeling().settings()
        self.assertTrue(settings.dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.dataDefinedProperties().property(key).asExpression(), 'coalesce("auxiliary_storage_labeling_positiony","asd")')

        # with overwrite existing
        key = QgsPalLayerSettings.Show
        settings = QgsPalLayerSettings()
        settings.dataDefinedProperties().setProperty(key, QgsProperty.fromExpression('$y > 20'))
        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        # existing property should be discarded
        index = QgsAuxiliaryLayer.createProperty(key, vl, True)
        p = QgsPalLayerSettings.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.labeling().settings()
        self.assertTrue(settings.dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.dataDefinedProperties().property(key).field(), 'auxiliary_storage_labeling_show')

    def testCreateCalloutProperty(self):
        s = QgsAuxiliaryStorage()
        self.assertTrue(s.isValid())

        # Create a new auxiliary layer with 'pk' as key
        vl = createLayer()
        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = s.createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())
        vl.setAuxiliaryLayer(al)

        # Create a new callout property on layer without labels
        key = QgsCallout.DestinationX
        index = QgsAuxiliaryLayer.createProperty(key, vl)
        self.assertEqual(index, -1)

        # Labeling, but no callouts
        settings = QgsPalLayerSettings()
        settings.setCallout(None)
        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        index = QgsAuxiliaryLayer.createProperty(key, vl)
        self.assertEqual(index, -1)

        # callouts
        settings = QgsPalLayerSettings()
        callout = QgsSimpleLineCallout()
        callout.setEnabled(True)
        settings.setCallout(callout)
        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        index = QgsAuxiliaryLayer.createProperty(key, vl)

        p = QgsCallout.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.labeling().settings()
        self.assertEqual(settings.callout().dataDefinedProperties().property(key), QgsProperty.fromField('auxiliary_storage_callouts_destinationx'))

        key2 = QgsCallout.DestinationY
        index = QgsAuxiliaryLayer.createProperty(key2, vl)

        p = QgsCallout.propertyDefinitions()[key2]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.labeling().settings()
        self.assertEqual(settings.callout().dataDefinedProperties().property(key),
                         QgsProperty.fromField('auxiliary_storage_callouts_destinationx'))
        self.assertEqual(settings.callout().dataDefinedProperties().property(key2),
                         QgsProperty.fromField('auxiliary_storage_callouts_destinationy'))

        # with existing property
        key = QgsCallout.OriginX
        settings = QgsPalLayerSettings()
        callout = QgsSimpleLineCallout()
        callout.dataDefinedProperties().setProperty(key, QgsProperty.fromExpression('$x + 20'))
        callout.setEnabled(True)
        settings.setCallout(callout)
        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        # without overwriting existing, property should be upgraded to coalesce("aux field", 'existing expression') type
        index = QgsAuxiliaryLayer.createProperty(key, vl, False)
        p = QgsCallout.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.labeling().settings()
        self.assertTrue(settings.callout().dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.callout().dataDefinedProperties().property(key).asExpression(), 'coalesce("auxiliary_storage_callouts_originx",$x + 20)')

        # with overwrite existing
        key = QgsCallout.OriginY
        callout = QgsSimpleLineCallout()
        callout.dataDefinedProperties().setProperty(key, QgsProperty.fromExpression('$y + 20'))
        settings.setCallout(callout)
        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        # existing property should be discarded
        index = QgsAuxiliaryLayer.createProperty(key, vl, True)
        p = QgsCallout.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.labeling().settings()
        self.assertTrue(settings.callout().dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.callout().dataDefinedProperties().property(key).field(), 'auxiliary_storage_callouts_originy')

    def testCreatePropertyDiagram(self):
        s = QgsAuxiliaryStorage()
        self.assertTrue(s.isValid())

        # Create a new auxiliary layer with 'pk' as key
        vl = createLayer()
        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = s.createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())
        vl.setAuxiliaryLayer(al)

        # Create a new labeling property on layer without labels
        key = QgsDiagramLayerSettings.PositionX
        index = QgsAuxiliaryLayer.createProperty(key, vl)
        self.assertEqual(index, -1)

        renderer = QgsSingleCategoryDiagramRenderer()
        diagram_settings = QgsDiagramLayerSettings()
        vl.setDiagramRenderer(renderer)
        vl.setDiagramLayerSettings(diagram_settings)
        index = QgsAuxiliaryLayer.createProperty(key, vl)

        p = QgsDiagramLayerSettings.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.diagramLayerSettings()
        self.assertTrue(settings.dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.dataDefinedProperties().property(key).field(), "auxiliary_storage_diagram_positionx")

        # with existing property
        key = QgsDiagramLayerSettings.Distance
        settings = QgsDiagramLayerSettings()
        settings.dataDefinedProperties().setProperty(key, QgsProperty.fromExpression('$y + 20'))
        vl.setDiagramLayerSettings(settings)

        # without overwriting existing, property should be upgraded to coalesce("aux field", 'existing expression') type
        index = QgsAuxiliaryLayer.createProperty(key, vl, False)
        p = QgsDiagramLayerSettings.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.diagramLayerSettings()
        self.assertTrue(settings.dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.dataDefinedProperties().property(key).asExpression(), 'coalesce("auxiliary_storage_diagram_distance",$y + 20)')

        # with overwrite existing
        key = QgsDiagramLayerSettings.PositionY
        settings = QgsDiagramLayerSettings()
        settings.dataDefinedProperties().setProperty(key, QgsProperty.fromExpression('$y > 20'))
        vl.setDiagramLayerSettings(settings)

        # existing property should be discarded
        index = QgsAuxiliaryLayer.createProperty(key, vl, True)
        p = QgsDiagramLayerSettings.propertyDefinitions()[key]
        afName = QgsAuxiliaryLayer.nameFromProperty(p, True)
        afIndex = vl.fields().indexOf(afName)
        self.assertEqual(index, afIndex)

        settings = vl.diagramLayerSettings()
        self.assertTrue(settings.dataDefinedProperties().property(key).isActive())
        self.assertEqual(settings.dataDefinedProperties().property(key).field(), 'auxiliary_storage_diagram_positiony')

    def testCreateField(self):
        s = QgsAuxiliaryStorage()
        self.assertTrue(s.isValid())

        # Create a new auxiliary layer with 'pk' as key
        vl = createLayer()
        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = s.createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())
        vl.setAuxiliaryLayer(al)

        prop = QgsPropertyDefinition()
        prop.setComment('test_field')
        prop.setDataType(QgsPropertyDefinition.DataTypeNumeric)
        prop.setOrigin('user')
        prop.setName('custom')
        self.assertTrue(al.addAuxiliaryField(prop))

        prop = QgsPropertyDefinition()
        prop.setComment('test_field_string')
        prop.setDataType(QgsPropertyDefinition.DataTypeString)
        prop.setOrigin('user')
        prop.setName('custom')
        self.assertTrue(al.addAuxiliaryField(prop))

        self.assertEqual(len(al.auxiliaryFields()), 2)
        self.assertEqual(al.auxiliaryFields()[0].name(), 'user_custom_test_field')
        self.assertEqual(al.auxiliaryFields()[0].type(), QVariant.Double)
        self.assertEqual(al.auxiliaryFields()[0].typeName(), 'Real')
        self.assertEqual(al.auxiliaryFields()[1].name(), 'user_custom_test_field_string')
        self.assertEqual(al.auxiliaryFields()[1].type(), QVariant.String)
        self.assertEqual(al.auxiliaryFields()[1].typeName(), 'String')

    def testQgdCreation(self):
        # New project
        p = QgsProject()
        self.assertTrue(p.auxiliaryStorage().isValid())

        # Save the project
        path = tmpPath()
        qgs = path + '.qgs'
        self.assertTrue(p.write(qgs))
        self.assertTrue(os.path.exists(qgs))

        # Auxiliary storage is empty so .qgd file should not be saved
        qgd = path + '.qgd'
        self.assertFalse(os.path.exists(qgd))

        # Add a vector layer and an auxiliary layer in the project
        vl = createLayer()
        self.assertTrue(vl.isValid())
        p.addMapLayers([vl])

        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = p.auxiliaryStorage().createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())
        vl.setAuxiliaryLayer(al)

        # Add an auxiliary field to have a non empty auxiliary storage
        pdef = QgsPropertyDefinition('propname', QgsPropertyDefinition.DataTypeNumeric, '', '', 'ut')
        self.assertTrue(al.addAuxiliaryField(pdef))

        # Save the project
        newpath = tmpPath()
        qgs = newpath + '.qgs'
        self.assertTrue(p.write(qgs))
        self.assertTrue(os.path.exists(qgs))

        # Auxiliary storage is NOT empty so .qgd file should be saved now
        qgd = newpath + '.qgd'
        self.assertTrue(os.path.exists(qgd))

    def testInvalidPrimaryKey(self):
        # create layer
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&key=pk',
            'test', 'memory')
        assert (vl.isValid())

        # add a field with an invalid typename
        field = QgsField(name="invalid_pk", type=QVariant.Int, typeName="xsd:int")
        vl.startEditing()
        vl.addAttribute(field)
        vl.commitChanges()

        # create auxiliary storage based on the invalid field
        s = QgsAuxiliaryStorage()
        pkf = field
        al = s.createAuxiliaryLayer(pkf, vl)

        self.assertEqual(al, None)
        self.assertTrue("CREATE TABLE IF NOT EXISTS" in s.errorString())

    def testQgdCreationInQgz(self):
        # New project
        p = QgsProject()
        self.assertTrue(p.auxiliaryStorage().isValid())

        # Save the project
        path = tmpPath()
        qgz = path + '.qgz'
        self.assertTrue(p.write(qgz))
        self.assertTrue(os.path.exists(qgz))

        # Check the content of the archive: auxiliary database doesn't exist
        # because it's empty
        archive = QgsProjectArchive()
        archive.unzip(qgz)
        self.assertEqual(archive.auxiliaryStorageFile(), "")

        # Add a vector layer and an auxiliary layer in the project
        vl = createLayer()
        self.assertTrue(vl.isValid())
        p.addMapLayers([vl])

        pkf = vl.fields().field(vl.fields().indexOf('pk'))
        al = p.auxiliaryStorage().createAuxiliaryLayer(pkf, vl)
        self.assertTrue(al.isValid())
        vl.setAuxiliaryLayer(al)

        # Add an auxiliary field to have a non empty auxiliary storage
        pdef = QgsPropertyDefinition('propname', QgsPropertyDefinition.DataTypeNumeric, '', '', 'ut')
        self.assertTrue(al.addAuxiliaryField(pdef))

        # Save the project
        path = tmpPath()
        qgz = path + '.qgz'
        self.assertTrue(p.write(qgz))
        self.assertTrue(os.path.exists(qgz))

        # Check the content of the archive: auxiliary database exist
        # because it's not empty
        archive = QgsProjectArchive()
        archive.unzip(qgz)
        self.assertNotEqual(archive.auxiliaryStorageFile(), '')


if __name__ == '__main__':
    unittest.main()
