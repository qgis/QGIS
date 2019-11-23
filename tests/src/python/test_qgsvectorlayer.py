# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'

import qgis  # NOQA

import os
import tempfile
import shutil

from qgis.PyQt.QtCore import QVariant, Qt
from qgis.PyQt.QtGui import QPainter, QColor
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsWkbTypes,
                       QgsAction,
                       QgsCoordinateTransformContext,
                       QgsDataProvider,
                       QgsDefaultValue,
                       QgsEditorWidgetSetup,
                       QgsVectorLayer,
                       QgsRectangle,
                       QgsFeature,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsPointXY,
                       QgsField,
                       QgsFieldConstraints,
                       QgsFields,
                       QgsVectorLayerJoinInfo,
                       QgsSymbol,
                       QgsSingleSymbolRenderer,
                       QgsCoordinateReferenceSystem,
                       QgsVectorLayerCache,
                       QgsReadWriteContext,
                       QgsProject,
                       QgsUnitTypes,
                       QgsAggregateCalculator,
                       QgsPoint,
                       QgsExpressionContext,
                       QgsExpressionContextScope,
                       QgsExpressionContextUtils,
                       QgsLineSymbol,
                       QgsMapLayerStyle,
                       QgsMapLayerDependency,
                       QgsPalLayerSettings,
                       QgsVectorLayerSimpleLabeling,
                       QgsSingleCategoryDiagramRenderer,
                       QgsDiagramLayerSettings,
                       QgsTextFormat,
                       QgsVectorLayerSelectedFeatureSource,
                       QgsExpression,
                       NULL)
from qgis.gui import (QgsAttributeTableModel,
                      QgsGui
                      )
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import start_app, unittest
from featuresourcetestbase import FeatureSourceTestCase
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


def createEmptyLayer():
    layer = QgsVectorLayer("Point", "addfeat", "memory")
    assert layer.featureCount() == 0
    return layer


def createEmptyLayerWithFields():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory")
    assert layer.featureCount() == 0
    return layer


def createLayerWithOnePoint():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    assert pr.addFeatures([f])
    assert layer.featureCount() == 1
    return layer


def createLayerWithTwoPoints():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    f2 = QgsFeature()
    f2.setAttributes(["test2", 457])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    assert pr.addFeatures([f, f2])
    assert layer.featureCount() == 2
    return layer


def createLayerWithFivePoints():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    f2 = QgsFeature()
    f2.setAttributes(["test2", 457])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
    f3 = QgsFeature()
    f3.setAttributes(["test2", 888])
    f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
    f4 = QgsFeature()
    f4.setAttributes(["test3", -1])
    f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(400, 300)))
    f5 = QgsFeature()
    f5.setAttributes(["test4", 0])
    f5.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(0, 0)))
    assert pr.addFeatures([f, f2, f3, f4, f5])
    assert layer.featureCount() == 5
    return layer


def createJoinLayer():
    joinLayer = QgsVectorLayer(
        "Point?field=x:string&field=y:integer&field=z:integer",
        "joinlayer", "memory")
    pr = joinLayer.dataProvider()
    f1 = QgsFeature()
    f1.setAttributes(["foo", 123, 321])
    f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 1)))
    f2 = QgsFeature()
    f2.setAttributes(["bar", 456, 654])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 2)))
    f3 = QgsFeature()
    f3.setAttributes(["qar", 457, 111])
    f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 2)))
    f4 = QgsFeature()
    f4.setAttributes(["a", 458, 19])
    f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 2)))
    assert pr.addFeatures([f1, f2, f3, f4])
    assert joinLayer.featureCount() == 4
    return joinLayer


def dumpFeature(f):
    print("--- FEATURE DUMP ---")
    print(("valid: %d   | id: %d" % (f.isValid(), f.id())))
    geom = f.geometry()
    if geom:
        print(("geometry wkb: %d" % geom.wkbType()))
    else:
        print("no geometry")
    print(("attrs: %s" % str(f.attributes())))


def formatAttributes(attrs):
    return repr([str(a) for a in attrs])


def dumpEditBuffer(layer):
    editBuffer = layer.editBuffer()
    if not editBuffer:
        print("NO EDITING!")
        return
    print("ADDED:")
    for fid, f in editBuffer.addedFeatures().items():
        print(("%d: %s | %s" % (
            f.id(), formatAttributes(f.attributes()),
            f.geometry().asWkt())))
    print("CHANGED GEOM:")
    for fid, geom in editBuffer.changedGeometries().items():
        print(("%d | %s" % (f.id(), f.geometry().asWkt())))


class TestQgsVectorLayerShapefile(unittest.TestCase, FeatureSourceTestCase):

    """
    Tests a vector layer against the feature source tests, using a real layer source (not a memory layer)
    """
    @classmethod
    def getSource(cls):
        vl = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'provider', 'shapefile.shp'), 'test')
        assert (vl.isValid())
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QgsGui.editorWidgetRegistry().initEditors()
        # Create test layer for FeatureSourceTestCase
        cls.source = cls.getSource()


class TestQgsVectorLayer(unittest.TestCase, FeatureSourceTestCase):

    @classmethod
    def getSource(cls):
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

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QgsGui.editorWidgetRegistry().initEditors()
        # Create test layer for FeatureSourceTestCase
        cls.source = cls.getSource()

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def test_FeatureCount(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        myCount = myLayer.featureCount()
        self.assertEqual(myCount, 6)

    # undo stack
    def testUndoStack(self):
        layer = createLayerWithOnePoint()
        layer.startEditing()

        self.assertEqual(layer.undoStack().count(), 0)
        self.assertEqual(layer.undoStack().index(), 0)
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        self.assertTrue(layer.addFeatures([f]))
        self.assertEqual(layer.undoStack().count(), 1)
        self.assertEqual(layer.undoStack().index(), 1)
        self.assertEqual(layer.featureCount(), 2)

        layer.undoStack().undo()
        self.assertEqual(layer.undoStack().count(), 1)
        self.assertEqual(layer.undoStack().index(), 0)
        self.assertEqual(layer.featureCount(), 1)

        layer.undoStack().redo()
        self.assertEqual(layer.undoStack().count(), 1)
        self.assertEqual(layer.undoStack().index(), 1)
        self.assertEqual(layer.featureCount(), 2)

        # macro commands
        layer.beginEditCommand("Test command 1")
        self.assertTrue(layer.addFeatures([f]))
        self.assertTrue(layer.addFeatures([f]))
        layer.endEditCommand()
        self.assertEqual(layer.undoStack().count(), 2)
        self.assertEqual(layer.undoStack().index(), 2)
        self.assertEqual(layer.featureCount(), 4)

        layer.undoStack().undo()
        self.assertEqual(layer.undoStack().count(), 2)
        self.assertEqual(layer.undoStack().index(), 1)
        self.assertEqual(layer.featureCount(), 2)

        layer.undoStack().redo()
        self.assertEqual(layer.undoStack().count(), 2)
        self.assertEqual(layer.undoStack().index(), 2)
        self.assertEqual(layer.featureCount(), 4)

        # throw away a macro command
        layer.beginEditCommand("Test command 1")
        self.assertTrue(layer.addFeatures([f]))
        self.assertTrue(layer.addFeatures([f]))
        self.assertEqual(layer.featureCount(), 6)
        layer.destroyEditCommand()
        self.assertEqual(layer.undoStack().count(), 2)
        self.assertEqual(layer.undoStack().index(), 2)
        self.assertEqual(layer.featureCount(), 4)

    def testSetDataSource(self):
        """
        Test changing a layer's data source
        """
        layer = createLayerWithOnePoint()
        layer.setCrs(QgsCoordinateReferenceSystem("epsg:3111"))
        r = QgsSingleSymbolRenderer(QgsSymbol.defaultSymbol(QgsWkbTypes.PointGeometry))
        layer.setRenderer(r)
        self.assertEqual(layer.renderer().symbol().type(), QgsSymbol.Marker)

        spy = QSignalSpy(layer.dataSourceChanged)

        options = QgsDataProvider.ProviderOptions()
        # change with layer of same type
        points_path = os.path.join(unitTestDataPath(), 'points.shp')
        layer.setDataSource(points_path, 'new name', 'ogr', options)

        self.assertTrue(layer.isValid())
        self.assertEqual(layer.name(), 'new name')
        self.assertEqual(layer.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(layer.crs().authid(), 'EPSG:4326')
        self.assertIn(points_path, layer.dataProvider().dataSourceUri())
        self.assertEqual(len(spy), 1)

        # should have kept the same renderer!
        self.assertEqual(layer.renderer(), r)

        # layer with different type
        lines_path = os.path.join(unitTestDataPath(), 'lines.shp')
        layer.setDataSource(lines_path, 'new name2', 'ogr', options)

        self.assertTrue(layer.isValid())
        self.assertEqual(layer.name(), 'new name2')
        self.assertEqual(layer.wkbType(), QgsWkbTypes.MultiLineString)
        # depending on proj version, the projection for this layer is either 4326 or 4030
        self.assertIn(layer.crs().authid(), ('EPSG:4326', 'EPSG:4030'))
        self.assertIn(lines_path, layer.dataProvider().dataSourceUri())
        self.assertEqual(len(spy), 2)

        # should have reset renderer!
        self.assertNotEqual(layer.renderer(), r)
        self.assertEqual(layer.renderer().symbol().type(), QgsSymbol.Line)

    def testSetDataSourceInvalidToValid(self):
        """
        Test that changing an invalid layer path to valid maintains the renderer
        """
        layer = createLayerWithOnePoint()
        layer.setCrs(QgsCoordinateReferenceSystem("epsg:3111"))
        r = QgsSingleSymbolRenderer(QgsSymbol.defaultSymbol(QgsWkbTypes.PointGeometry))
        layer.setRenderer(r)
        self.assertEqual(layer.renderer().symbol().type(), QgsSymbol.Marker)

        # change to invalid path
        options = QgsDataProvider.ProviderOptions()
        layer.setDataSource('nothing', 'new name', 'ogr', options)

        self.assertFalse(layer.isValid())
        # these properties should be kept intact!
        self.assertEqual(layer.name(), 'new name')
        self.assertEqual(layer.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(layer.crs().authid(), 'EPSG:3111')
        # should have kept the same renderer!
        self.assertEqual(layer.renderer(), r)

        # set to a valid path
        points_path = os.path.join(unitTestDataPath(), 'points.shp')
        layer.setDataSource(points_path, 'new name2', 'ogr', options)

        self.assertTrue(layer.isValid())
        self.assertEqual(layer.name(), 'new name2')
        self.assertEqual(layer.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(layer.crs().authid(), 'EPSG:4326')
        self.assertIn(points_path, layer.dataProvider().dataSourceUri())

        # should STILL have kept renderer!
        self.assertEqual(layer.renderer(), r)

    def testStoreWkbTypeInvalidLayers(self):
        """
        Test that layer wkb types are restored for projects with invalid layer paths
        """
        layer = createLayerWithOnePoint()
        layer.setName('my test layer')
        r = QgsSingleSymbolRenderer(QgsSymbol.defaultSymbol(QgsWkbTypes.PointGeometry))
        r.symbol().setColor(QColor('#123456'))
        layer.setRenderer(r)
        self.assertEqual(layer.renderer().symbol().color().name(), '#123456')

        p = QgsProject()
        p.addMapLayer(layer)

        # reset layer to a bad path
        options = QgsDataProvider.ProviderOptions()
        layer.setDataSource('nothing', 'new name', 'ogr', options)
        # should have kept the same renderer and wkb type!
        self.assertEqual(layer.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(layer.renderer().symbol().color().name(), '#123456')

        # save project to a temporary file
        temp_path = tempfile.mkdtemp()
        temp_project_path = os.path.join(temp_path, 'temp.qgs')
        self.assertTrue(p.write(temp_project_path))

        # restore project
        p2 = QgsProject()
        self.assertTrue(p2.read(temp_project_path))

        l2 = p2.mapLayersByName('new name')[0]
        self.assertFalse(l2.isValid())

        # should have kept the same renderer and wkb type!
        self.assertEqual(l2.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(l2.renderer().symbol().color().name(), '#123456')

        shutil.rmtree(temp_path, True)

    def testFallbackCrsWkbType(self):
        """
        Test fallback CRS and WKB types are used when layer path is invalid
        """
        vl = QgsVectorLayer('this is an outrage!!!')
        self.assertFalse(vl.isValid()) # i'd certainly hope so...
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Unknown)
        self.assertFalse(vl.crs().isValid())

        # with fallback
        options = QgsVectorLayer.LayerOptions()
        options.fallbackWkbType = QgsWkbTypes.CircularString
        options.fallbackCrs = QgsCoordinateReferenceSystem('EPSG:3111')
        vl = QgsVectorLayer("i'm the moon", options=options)
        self.assertFalse(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.CircularString)
        self.assertEqual(vl.crs().authid(), 'EPSG:3111')

    def test_layer_crs(self):
        """
        Test that spatial layers have CRS, and non-spatial don't
        """
        vl = QgsVectorLayer('Point?crs=epsg:3111&field=pk:integer', 'test', 'memory')
        self.assertTrue(vl.isSpatial())
        self.assertTrue(vl.crs().isValid())
        self.assertEqual(vl.crs().authid(), 'EPSG:3111')

        vl = QgsVectorLayer('None?field=pk:integer', 'test', 'memory')
        self.assertFalse(vl.isSpatial())
        self.assertFalse(vl.crs().isValid())

        # even if provider has a crs - we don't respect it for non-spatial layers!
        vl = QgsVectorLayer('None?crs=epsg:3111field=pk:integer', 'test', 'memory')
        self.assertFalse(vl.isSpatial())
        self.assertFalse(vl.crs().isValid())

    # ADD FEATURE

    def test_AddFeature(self):
        layer = createEmptyLayerWithFields()
        feat = QgsFeature(layer.fields())
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))

        def checkAfter():
            self.assertEqual(layer.featureCount(), 1)

            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPointXY(1, 2))

            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2.geometry().asPoint(), QgsPointXY(1, 2))

        def checkBefore():
            self.assertEqual(layer.featureCount(), 0)

            # check select+nextFeature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        checkBefore()

        # try to add feature without editing mode
        self.assertFalse(layer.addFeature(feat))

        # add feature
        layer.startEditing()

        # try adding feature with incorrect number of fields
        bad_feature = QgsFeature()
        self.assertFalse(layer.addFeature(bad_feature))

        # add good feature
        self.assertTrue(layer.addFeature(feat))

        checkAfter()
        self.assertEqual(layer.dataProvider().featureCount(), 0)

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        self.assertTrue(layer.commitChanges())

        checkAfter()
        self.assertEqual(layer.dataProvider().featureCount(), 1)

    # ADD FEATURES

    def test_AddFeatures(self):
        layer = createEmptyLayerWithFields()
        feat1 = QgsFeature(layer.fields())
        feat1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        feat2 = QgsFeature(layer.fields())
        feat2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(11, 12)))

        def checkAfter():
            self.assertEqual(layer.featureCount(), 2)

            # check select+nextFeature
            it = layer.getFeatures()
            f1 = next(it)
            self.assertEqual(f1.geometry().asPoint(), QgsPointXY(1, 2))
            f2 = next(it)
            self.assertEqual(f2.geometry().asPoint(), QgsPointXY(11, 12))

            # check feature at id
            f1_1 = next(layer.getFeatures(QgsFeatureRequest(f1.id())))
            self.assertEqual(f1_1.geometry().asPoint(), QgsPointXY(1, 2))
            f2_1 = next(layer.getFeatures(QgsFeatureRequest(f2.id())))
            self.assertEqual(f2_1.geometry().asPoint(), QgsPointXY(11, 12))

        def checkBefore():
            self.assertEqual(layer.featureCount(), 0)

            # check select+nextFeature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        checkBefore()

        # try to add feature without editing mode
        self.assertFalse(layer.addFeatures([feat1, feat2]))

        # add feature
        layer.startEditing()

        # try adding feature with incorrect number of fields
        bad_feature = QgsFeature()
        self.assertFalse(layer.addFeatures([bad_feature]))

        # add good features
        self.assertTrue(layer.addFeatures([feat1, feat2]))

        checkAfter()
        self.assertEqual(layer.dataProvider().featureCount(), 0)

        # now try undo/redo
        layer.undoStack().undo()
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        layer.undoStack().redo()
        checkAfter()

        self.assertTrue(layer.commitChanges())

        checkAfter()
        self.assertEqual(layer.dataProvider().featureCount(), 2)

    # DELETE FEATURE

    def test_DeleteFeature(self):
        layer = createLayerWithOnePoint()
        fid = 1

        def checkAfter():
            self.assertEqual(layer.featureCount(), 0)

            # check select+nextFeature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

            # check feature at id
            with self.assertRaises(StopIteration):
                next(layer.getFeatures(QgsFeatureRequest(fid)))

        def checkBefore():
            self.assertEqual(layer.featureCount(), 1)

            # check select+nextFeature
            fi = layer.getFeatures()
            f = next(fi)
            self.assertEqual(f.geometry().asPoint(), QgsPointXY(100, 200))
            with self.assertRaises(StopIteration):
                next(fi)

            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(fid)))
            self.assertEqual(f2.id(), fid)

        checkBefore()

        # try to delete feature without editing mode
        self.assertFalse(layer.deleteFeature(fid))

        # delete feature
        layer.startEditing()
        self.assertTrue(layer.deleteFeature(fid))

        checkAfter()

        # make sure calling it twice does not work
        self.assertFalse(layer.deleteFeature(fid))

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        self.assertEqual(layer.dataProvider().featureCount(), 1)

        self.assertTrue(layer.commitChanges())

        checkAfter()
        self.assertEqual(layer.dataProvider().featureCount(), 0)

    def test_DeleteFeatureAfterAddFeature(self):

        layer = createEmptyLayer()
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))

        def checkBefore():
            self.assertEqual(layer.featureCount(), 0)

            # check select+nextFeature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        def checkAfter1():
            self.assertEqual(layer.featureCount(), 1)

        def checkAfter2():
            checkBefore()  # should be the same state: no features

        checkBefore()

        # add feature
        layer.startEditing()
        self.assertTrue(layer.addFeature(feat))
        checkAfter1()
        fid = feat.id()
        self.assertTrue(layer.deleteFeature(fid))
        checkAfter2()

        # now try undo/redo
        layer.undoStack().undo()
        checkAfter1()
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter1()
        layer.undoStack().redo()
        checkAfter2()

        self.assertTrue(layer.commitChanges())
        checkAfter2()

        self.assertEqual(layer.dataProvider().featureCount(), 0)

    def test_DeleteJoinedFeature(self):
        joinLayer = createJoinLayer()
        joinLayer2 = createJoinLayer()
        QgsProject.instance().addMapLayers([joinLayer, joinLayer2])

        layer = createLayerWithOnePoint()

        join = QgsVectorLayerJoinInfo()
        join.setTargetFieldName("fldint")
        join.setJoinLayer(joinLayer)
        join.setJoinFieldName("y")
        join.setUsingMemoryCache(True)
        join.setEditable(True)
        join.setCascadedDelete(True)

        layer.addJoin(join)

        join2 = QgsVectorLayerJoinInfo()
        join2.setTargetFieldName("fldint")
        join2.setJoinLayer(joinLayer2)
        join2.setJoinFieldName("y")
        join2.setUsingMemoryCache(True)
        join2.setPrefix("custom-prefix_")
        join2.setEditable(True)
        join2.setCascadedDelete(False)

        layer.addJoin(join2)

        # check number of features
        self.assertEqual(layer.featureCount(), 1)
        self.assertEqual(joinLayer.featureCount(), 4)
        self.assertEqual(joinLayer2.featureCount(), 4)

        # delete a feature which is also in joined layers
        layer.startEditing()
        joinLayer.startEditing()
        joinLayer2.startEditing()

        filter = QgsExpression.createFieldEqualityExpression('fldint', '123')
        feature = next(layer.getFeatures(QgsFeatureRequest().setFilterExpression(filter)))
        layer.deleteFeature(feature.id())

        # check number of features
        self.assertEqual(layer.featureCount(), 0)
        self.assertEqual(joinLayer.featureCount(), 3)  # deleteCascade activated
        self.assertEqual(joinLayer2.featureCount(), 4)  # deleteCascade deactivated

    # CHANGE ATTRIBUTE

    def test_ChangeAttribute(self):
        layer = createLayerWithOnePoint()
        fid = 1

        def checkAfter():
            # check select+nextFeature
            fi = layer.getFeatures()
            f = next(fi)
            self.assertEqual(f[0], "good")

            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2[0], "good")

        def checkBefore():
            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f[0], "test")

        checkBefore()

        # try to change attribute without editing mode
        self.assertFalse(layer.changeAttributeValue(fid, 0, "good"))

        # change attribute
        layer.startEditing()
        self.assertTrue(layer.changeAttributeValue(fid, 0, "good"))

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        self.assertTrue(layer.commitChanges())
        checkAfter()

    def test_ChangeAttributeAfterAddFeature(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().deleteFeatures([1])  # no need for this feature

        newF = QgsFeature()
        newF.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 1)))
        newF.setAttributes(["hello", 42])

        def checkAfter():
            self.assertEqual(len(layer.fields()), 2)
            # check feature
            fi = layer.getFeatures()
            f = next(fi)
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], "hello")
            self.assertEqual(attrs[1], 12)

            with self.assertRaises(StopIteration):
                next(fi)

            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2[0], "hello")
            self.assertEqual(f2[1], 12)

        def checkBefore():
            # check feature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        checkBefore()

        layer.startEditing()
        layer.beginEditCommand("AddFeature + ChangeAttribute")
        self.assertTrue(layer.addFeature(newF))
        self.assertTrue(layer.changeAttributeValue(newF.id(), 1, 12))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        self.assertTrue(layer.commitChanges())
        checkAfter()

        # print "COMMIT ERRORS:"
        # for item in list(layer.commitErrors()): print item

    # CHANGE GEOMETRY

    def test_ChangeGeometry(self):
        layer = createLayerWithOnePoint()
        fid = 1

        def checkAfter():
            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPointXY(300, 400))
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2.geometry().asPoint(), QgsPointXY(300, 400))

        def checkBefore():
            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPointXY(100, 200))

        # try to change geometry without editing mode
        self.assertFalse(layer.changeGeometry(fid, QgsGeometry.fromPointXY(QgsPointXY(300, 400))))

        checkBefore()

        # change geometry
        layer.startEditing()
        layer.beginEditCommand("ChangeGeometry")
        self.assertTrue(layer.changeGeometry(fid, QgsGeometry.fromPointXY(QgsPointXY(300, 400))))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        self.assertTrue(layer.commitChanges())
        checkAfter()

    def test_ChangeGeometryAfterChangeAttribute(self):
        layer = createLayerWithOnePoint()
        fid = 1

        def checkAfter():
            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPointXY(300, 400))
            self.assertEqual(f[0], "changed")
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2.geometry().asPoint(), QgsPointXY(300, 400))
            self.assertEqual(f2[0], "changed")

        def checkBefore():
            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPointXY(100, 200))
            self.assertEqual(f[0], "test")

        checkBefore()

        # change geometry
        layer.startEditing()
        layer.beginEditCommand("ChangeGeometry + ChangeAttribute")
        self.assertTrue(layer.changeAttributeValue(fid, 0, "changed"))
        self.assertTrue(layer.changeGeometry(fid, QgsGeometry.fromPointXY(QgsPointXY(300, 400))))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        self.assertTrue(layer.commitChanges())
        checkAfter()

    def test_ChangeGeometryAfterAddFeature(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().deleteFeatures([1])  # no need for this feature

        newF = QgsFeature()
        newF.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 1)))
        newF.setAttributes(["hello", 42])

        def checkAfter():
            self.assertEqual(len(layer.fields()), 2)
            # check feature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPointXY(2, 2))
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2.geometry().asPoint(), QgsPointXY(2, 2))

        def checkBefore():
            # check feature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        checkBefore()

        layer.startEditing()
        layer.beginEditCommand("AddFeature+ChangeGeometry")
        self.assertTrue(layer.addFeature(newF))
        self.assertTrue(layer.changeGeometry(newF.id(), QgsGeometry.fromPointXY(QgsPointXY(2, 2))))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        self.assertTrue(layer.commitChanges())
        checkAfter()

        # print "COMMIT ERRORS:"
        # for item in list(layer.commitErrors()): print item

    # updateFeature

    def testUpdateFeature(self):
        layer = createLayerWithFivePoints()
        features = [f for f in layer.getFeatures()]

        # try to change feature without editing mode
        self.assertFalse(layer.updateFeature(features[0]))

        layer.startEditing()

        # no matching feature
        f = QgsFeature(1123)
        self.assertFalse(layer.updateFeature(f))

        # change geometry and attributes
        f = features[0]
        f.setAttributes(['new', 321])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-200, -200)))
        self.assertTrue(layer.updateFeature(f))

        new_feature = next(layer.getFeatures(QgsFeatureRequest(f.id())))
        self.assertEqual(new_feature.attributes(), ['new', 321])
        self.assertEqual(new_feature.geometry().asPoint(), QgsPointXY(-200, -200))

        # add feature with no geometry
        f6 = QgsFeature()
        f6.setAttributes(["test6", 555])
        self.assertTrue(layer.dataProvider().addFeatures([f6]))
        features = [f for f in layer.getFeatures()]

        # update feature with no geometry -> have geometry
        f = features[-1]
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-350, -250)))
        self.assertTrue(layer.updateFeature(f))
        new_feature = next(layer.getFeatures(QgsFeatureRequest(f.id())))
        self.assertEqual(new_feature.attributes(), ['test6', 555])
        self.assertTrue(new_feature.hasGeometry())
        self.assertEqual(new_feature.geometry().asPoint(), QgsPointXY(-350, -250))

        # update feature from geometry -> no geometry
        f = features[1]
        f.clearGeometry()
        self.assertTrue(layer.updateFeature(f))
        new_feature = next(layer.getFeatures(QgsFeatureRequest(f.id())))
        self.assertEqual(new_feature.attributes(), ['test2', 457])
        self.assertFalse(new_feature.hasGeometry())

    # ADD ATTRIBUTE

    def test_AddAttribute(self):
        layer = createLayerWithOnePoint()
        fld1 = QgsField("fld1", QVariant.Int, "integer")

        # fld2 = QgsField("fld2", QVariant.Int, "integer")

        def checkBefore():
            # check fields
            flds = layer.fields()
            self.assertEqual(len(flds), 2)
            self.assertEqual(flds[0].name(), "fldtxt")
            self.assertEqual(flds[1].name(), "fldint")

            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], "test")
            self.assertEqual(attrs[1], 123)

        def checkAfter():
            # check fields
            flds = layer.fields()
            self.assertEqual(len(flds), 3)
            self.assertEqual(flds[0].name(), "fldtxt")
            self.assertEqual(flds[1].name(), "fldint")
            self.assertEqual(flds[2].name(), "fld1")

            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 3)
            self.assertEqual(attrs[0], "test")
            self.assertEqual(attrs[1], 123)
            self.assertTrue(attrs[2] is None)

            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2[0], "test")
            self.assertEqual(f2[1], 123)
            self.assertTrue(f2[2] is None)

        # for nt in layer.dataProvider().nativeTypes():
        #    print (nt.mTypeDesc, nt.mTypeName, nt.mType, nt.mMinLen,
        #          nt.mMaxLen, nt.mMinPrec, nt.mMaxPrec)
        self.assertTrue(layer.dataProvider().supportedType(fld1))

        # without editing mode
        self.assertFalse(layer.addAttribute(fld1))

        layer.startEditing()

        checkBefore()

        self.assertTrue(layer.addAttribute(fld1))
        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        layer.commitChanges()
        checkAfter()

    def test_AddAttributeAfterAddFeature(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().deleteFeatures([1])  # no need for this feature

        newF = QgsFeature()
        newF.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 1)))
        newF.setAttributes(["hello", 42])

        fld1 = QgsField("fld1", QVariant.Int, "integer")

        def checkBefore():
            self.assertEqual(len(layer.fields()), 2)
            # check feature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        def checkAfter():
            self.assertEqual(len(layer.fields()), 3)
            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 3)
            self.assertEqual(attrs[0], "hello")
            self.assertEqual(attrs[1], 42)
            self.assertTrue(attrs[2] is None)
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2[0], "hello")
            self.assertEqual(f2[1], 42)
            self.assertTrue(f2[2] is None)

        layer.startEditing()

        checkBefore()

        layer.beginEditCommand("AddFeature + AddAttribute")
        self.assertTrue(layer.addFeature(newF))
        self.assertTrue(layer.addAttribute(fld1))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        layer.commitChanges()
        checkAfter()

        # print "COMMIT ERRORS:"
        # for item in list(layer.commitErrors()): print item

    def test_AddAttributeAfterChangeValue(self):
        pass  # not interesting to test...?

    def test_AddAttributeAfterDeleteAttribute(self):
        pass  # maybe it would be good to test

    # DELETE ATTRIBUTE

    def test_DeleteAttribute(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().addAttributes(
            [QgsField("flddouble", QVariant.Double, "double")])
        layer.dataProvider().changeAttributeValues(
            {1: {2: 5.5}})

        # without editing mode
        self.assertFalse(layer.deleteAttribute(0))

        def checkBefore():
            flds = layer.fields()
            self.assertEqual(len(flds), 3)
            self.assertEqual(flds[0].name(), "fldtxt")
            self.assertEqual(flds[1].name(), "fldint")
            self.assertEqual(flds[2].name(), "flddouble")

            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 3)
            self.assertEqual(attrs[0], "test")
            self.assertEqual(attrs[1], 123)
            self.assertEqual(attrs[2], 5.5)

        layer.startEditing()

        checkBefore()

        self.assertTrue(layer.deleteAttribute(0))

        def checkAfterOneDelete():
            flds = layer.fields()
            # for fld in flds: print "FLD", fld.name()
            self.assertEqual(len(flds), 2)
            self.assertEqual(flds[0].name(), "fldint")
            self.assertEqual(flds[1].name(), "flddouble")
            self.assertEqual(layer.attributeList(), [0, 1])

            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], 123)
            self.assertEqual(attrs[1], 5.5)

        checkAfterOneDelete()

        # delete last attribute
        self.assertTrue(layer.deleteAttribute(0))

        def checkAfterTwoDeletes():
            self.assertEqual(layer.attributeList(), [0])
            flds = layer.fields()
            # for fld in flds: print "FLD", fld.name()
            self.assertEqual(len(flds), 1)
            self.assertEqual(flds[0].name(), "flddouble")

            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 1)
            self.assertEqual(attrs[0], 5.5)
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(len(f2.attributes()), 1)
            self.assertEqual(f2[0], 5.5)

        checkAfterTwoDeletes()
        layer.undoStack().undo()
        checkAfterOneDelete()
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfterOneDelete()
        layer.undoStack().redo()
        checkAfterTwoDeletes()

        self.assertTrue(layer.commitChanges())  # COMMIT!
        checkAfterTwoDeletes()

    def test_DeleteAttributeAfterAddAttribute(self):
        layer = createLayerWithOnePoint()
        fld1 = QgsField("fld1", QVariant.Int, "integer")

        def checkAfter():  # layer should be unchanged
            flds = layer.fields()
            self.assertEqual(len(flds), 2)
            self.assertEqual(flds[0].name(), "fldtxt")
            self.assertEqual(flds[1].name(), "fldint")

            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], "test")
            self.assertEqual(attrs[1], 123)
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(len(f2.attributes()), 2)
            self.assertEqual(f2[0], "test")
            self.assertEqual(f2[1], 123)

        checkAfter()

        layer.startEditing()

        layer.beginEditCommand("AddAttribute + DeleteAttribute")
        self.assertTrue(layer.addAttribute(fld1))
        self.assertTrue(layer.deleteAttribute(2))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkAfter()
        layer.undoStack().redo()
        checkAfter()

        layer.commitChanges()
        checkAfter()

    def test_DeleteAttributeAfterAddFeature(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().deleteFeatures([1])  # no need for this feature

        newF = QgsFeature()
        newF.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 1)))
        newF.setAttributes(["hello", 42])

        def checkBefore():
            self.assertEqual(len(layer.fields()), 2)
            # check feature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        def checkAfter1():
            self.assertEqual(len(layer.fields()), 2)
            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], "hello")
            self.assertEqual(attrs[1], 42)

        def checkAfter2():
            self.assertEqual(len(layer.fields()), 1)
            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 1)
            self.assertEqual(attrs[0], 42)

        layer.startEditing()

        checkBefore()

        layer.addFeature(newF)
        checkAfter1()
        layer.deleteAttribute(0)
        checkAfter2()

        # now try undo/redo
        layer.undoStack().undo()
        checkAfter1()
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter1()
        layer.undoStack().redo()
        checkAfter2()

        layer.commitChanges()
        checkAfter2()

    def test_DeleteAttributeAfterChangeValue(self):
        layer = createLayerWithOnePoint()

        def checkBefore():
            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], "test")
            self.assertEqual(attrs[1], 123)

        def checkAfter1():
            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], "changed")
            self.assertEqual(attrs[1], 123)

        def checkAfter2():
            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 1)
            self.assertEqual(attrs[0], 123)

        layer.startEditing()

        checkBefore()

        self.assertTrue(layer.changeAttributeValue(1, 0, "changed"))
        checkAfter1()
        self.assertTrue(layer.deleteAttribute(0))
        checkAfter2()

        # now try undo/redo
        layer.undoStack().undo()
        checkAfter1()
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter1()
        layer.undoStack().redo()
        checkAfter2()

        layer.commitChanges()
        checkAfter2()

    # RENAME ATTRIBUTE

    def test_RenameAttribute(self):
        layer = createLayerWithOnePoint()

        # without editing mode
        self.assertFalse(layer.renameAttribute(0, 'renamed'))

        def checkFieldNames(names):
            flds = layer.fields()
            f = next(layer.getFeatures())
            self.assertEqual(flds.count(), len(names))
            self.assertEqual(f.fields().count(), len(names))

            for idx, expected_name in enumerate(names):
                self.assertEqual(flds[idx].name(), expected_name)
                self.assertEqual(f.fields().at(idx).name(), expected_name)

        layer.startEditing()

        checkFieldNames(['fldtxt', 'fldint'])

        self.assertFalse(layer.renameAttribute(-1, 'fldtxt2'))
        self.assertFalse(layer.renameAttribute(10, 'fldtxt2'))
        self.assertFalse(layer.renameAttribute(0, 'fldint'))  # duplicate name

        self.assertTrue(layer.renameAttribute(0, 'fldtxt2'))
        checkFieldNames(['fldtxt2', 'fldint'])

        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt2', 'fldint'])

        # change two fields
        self.assertTrue(layer.renameAttribute(1, 'fldint2'))
        checkFieldNames(['fldtxt2', 'fldint2'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt2', 'fldint'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt2', 'fldint'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt2', 'fldint2'])

        # two renames
        self.assertTrue(layer.renameAttribute(0, 'fldtxt3'))
        checkFieldNames(['fldtxt3', 'fldint2'])
        self.assertTrue(layer.renameAttribute(0, 'fldtxt4'))
        checkFieldNames(['fldtxt4', 'fldint2'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt3', 'fldint2'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt2', 'fldint2'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt3', 'fldint2'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt4', 'fldint2'])

    def test_RenameAttributeAfterAdd(self):
        layer = createLayerWithOnePoint()

        def checkFieldNames(names):
            flds = layer.fields()
            f = next(layer.getFeatures())
            self.assertEqual(flds.count(), len(names))
            self.assertEqual(f.fields().count(), len(names))

            for idx, expected_name in enumerate(names):
                self.assertEqual(flds[idx].name(), expected_name)
                self.assertEqual(f.fields().at(idx).name(), expected_name)

        layer.startEditing()

        checkFieldNames(['fldtxt', 'fldint'])
        self.assertTrue(layer.renameAttribute(1, 'fldint2'))
        checkFieldNames(['fldtxt', 'fldint2'])
        # add an attribute
        self.assertTrue(layer.addAttribute(QgsField("flddouble", QVariant.Double, "double")))
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble'])
        # rename it
        self.assertTrue(layer.renameAttribute(2, 'flddouble2'))
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble2'])
        self.assertTrue(layer.addAttribute(QgsField("flddate", QVariant.Date, "date")))
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble2', 'flddate'])
        self.assertTrue(layer.renameAttribute(2, 'flddouble3'))
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble3', 'flddate'])
        self.assertTrue(layer.renameAttribute(3, 'flddate2'))
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble3', 'flddate2'])

        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble3', 'flddate'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble2', 'flddate'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble2'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint2'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint'])

        layer.undoStack().redo()
        checkFieldNames(['fldtxt', 'fldint2'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble2'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble2', 'flddate'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble3', 'flddate'])
        layer.undoStack().redo()
        checkFieldNames(['fldtxt', 'fldint2', 'flddouble3', 'flddate2'])

    def test_RenameAttributeAndDelete(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().addAttributes(
            [QgsField("flddouble", QVariant.Double, "double")])
        layer.updateFields()

        def checkFieldNames(names):
            flds = layer.fields()
            f = next(layer.getFeatures())
            self.assertEqual(flds.count(), len(names))
            self.assertEqual(f.fields().count(), len(names))

            for idx, expected_name in enumerate(names):
                self.assertEqual(flds[idx].name(), expected_name)
                self.assertEqual(f.fields().at(idx).name(), expected_name)

        layer.startEditing()

        checkFieldNames(['fldtxt', 'fldint', 'flddouble'])
        self.assertTrue(layer.renameAttribute(0, 'fldtxt2'))
        checkFieldNames(['fldtxt2', 'fldint', 'flddouble'])
        self.assertTrue(layer.renameAttribute(2, 'flddouble2'))
        checkFieldNames(['fldtxt2', 'fldint', 'flddouble2'])

        # delete an attribute
        self.assertTrue(layer.deleteAttribute(0))
        checkFieldNames(['fldint', 'flddouble2'])
        # rename remaining
        self.assertTrue(layer.renameAttribute(0, 'fldint2'))
        checkFieldNames(['fldint2', 'flddouble2'])
        self.assertTrue(layer.renameAttribute(1, 'flddouble3'))
        checkFieldNames(['fldint2', 'flddouble3'])
        # delete an attribute
        self.assertTrue(layer.deleteAttribute(0))
        checkFieldNames(['flddouble3'])
        self.assertTrue(layer.renameAttribute(0, 'flddouble4'))
        checkFieldNames(['flddouble4'])

        layer.undoStack().undo()
        checkFieldNames(['flddouble3'])
        layer.undoStack().undo()
        checkFieldNames(['fldint2', 'flddouble3'])
        layer.undoStack().undo()
        checkFieldNames(['fldint2', 'flddouble2'])
        layer.undoStack().undo()
        checkFieldNames(['fldint', 'flddouble2'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt2', 'fldint', 'flddouble2'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt2', 'fldint', 'flddouble'])
        layer.undoStack().undo()
        checkFieldNames(['fldtxt', 'fldint', 'flddouble'])

        # layer.undoStack().redo()
        # checkFieldNames(['fldtxt2', 'fldint'])
        # layer.undoStack().redo()
        # checkFieldNames(['fldint'])

    def test_RenameExpressionField(self):
        layer = createLayerWithOnePoint()
        exp_field_idx = layer.addExpressionField('1+1', QgsField('math_is_hard', QVariant.Int))

        # rename and check
        self.assertTrue(layer.renameAttribute(exp_field_idx, 'renamed'))
        self.assertEqual(layer.fields()[exp_field_idx].name(), 'renamed')
        f = next(layer.getFeatures())
        self.assertEqual(f.fields()[exp_field_idx].name(), 'renamed')

    def test_fields(self):
        layer = createLayerWithOnePoint()

        flds = layer.fields()
        self.assertEqual(flds.indexFromName("fldint"), 1)
        self.assertEqual(flds.indexFromName("fldXXX"), -1)

    def test_getFeatures(self):

        layer = createLayerWithOnePoint()

        f = QgsFeature()
        fi = layer.getFeatures()
        self.assertTrue(fi.nextFeature(f))
        self.assertTrue(f.isValid())
        self.assertEqual(f.id(), 1)
        self.assertEqual(f.geometry().asPoint(), QgsPointXY(100, 200))
        self.assertEqual(f["fldtxt"], "test")
        self.assertEqual(f["fldint"], 123)

        self.assertFalse(fi.nextFeature(f))

        layer2 = createLayerWithFivePoints()

        # getFeature(fid)
        feat = layer2.getFeature(4)
        self.assertTrue(feat.isValid())
        self.assertEqual(feat['fldtxt'], 'test3')
        self.assertEqual(feat['fldint'], -1)
        feat = layer2.getFeature(10)
        self.assertFalse(feat.isValid())

        # getFeatures(expression)
        it = layer2.getFeatures("fldint <= 0")
        fids = [f.id() for f in it]
        self.assertEqual(set(fids), set([4, 5]))

        # getFeatures(fids)
        it = layer2.getFeatures([1, 2])
        fids = [f.id() for f in it]
        self.assertEqual(set(fids), set([1, 2]))

        # getFeatures(rect)
        it = layer2.getFeatures(QgsRectangle(99, 99, 201, 201))
        fids = [f.id() for f in it]
        self.assertEqual(set(fids), set([1, 2]))

    def test_join(self):

        joinLayer = createJoinLayer()
        joinLayer2 = createJoinLayer()
        QgsProject.instance().addMapLayers([joinLayer, joinLayer2])

        layer = createLayerWithOnePoint()

        join = QgsVectorLayerJoinInfo()
        join.setTargetFieldName("fldint")
        join.setJoinLayer(joinLayer)
        join.setJoinFieldName("y")
        join.setUsingMemoryCache(True)

        layer.addJoin(join)

        join2 = QgsVectorLayerJoinInfo()
        join2.setTargetFieldName("fldint")
        join2.setJoinLayer(joinLayer2)
        join2.setJoinFieldName("y")
        join2.setUsingMemoryCache(True)
        join2.setPrefix("custom-prefix_")

        layer.addJoin(join2)

        flds = layer.fields()
        self.assertEqual(len(flds), 6)
        self.assertEqual(flds[2].name(), "joinlayer_x")
        self.assertEqual(flds[3].name(), "joinlayer_z")
        self.assertEqual(flds[4].name(), "custom-prefix_x")
        self.assertEqual(flds[5].name(), "custom-prefix_z")
        self.assertEqual(flds.fieldOrigin(0), QgsFields.OriginProvider)
        self.assertEqual(flds.fieldOrigin(2), QgsFields.OriginJoin)
        self.assertEqual(flds.fieldOrigin(3), QgsFields.OriginJoin)
        self.assertEqual(flds.fieldOriginIndex(0), 0)
        self.assertEqual(flds.fieldOriginIndex(2), 0)
        self.assertEqual(flds.fieldOriginIndex(3), 2)

        f = QgsFeature()
        fi = layer.getFeatures()
        self.assertTrue(fi.nextFeature(f))
        attrs = f.attributes()
        self.assertEqual(len(attrs), 6)
        self.assertEqual(attrs[0], "test")
        self.assertEqual(attrs[1], 123)
        self.assertEqual(attrs[2], "foo")
        self.assertEqual(attrs[3], 321)
        self.assertFalse(fi.nextFeature(f))

        f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
        self.assertEqual(len(f2.attributes()), 6)
        self.assertEqual(f2[2], "foo")
        self.assertEqual(f2[3], 321)

    def test_JoinStats(self):
        """ test calculating min/max/uniqueValues on joined field """
        joinLayer = createJoinLayer()
        layer = createLayerWithTwoPoints()
        QgsProject.instance().addMapLayers([joinLayer, layer])

        join = QgsVectorLayerJoinInfo()
        join.setTargetFieldName("fldint")
        join.setJoinLayer(joinLayer)
        join.setJoinFieldName("y")
        join.setUsingMemoryCache(True)
        layer.addJoin(join)

        # stats on joined fields should only include values present by join
        self.assertEqual(layer.minimumValue(3), 111)
        self.assertEqual(layer.maximumValue(3), 321)
        self.assertEqual(set(layer.uniqueValues(3)), set([111, 321]))

    def test_valid_join_when_opening_project(self):
        join_field = "id"
        fid = 4
        attr_idx = 4
        join_attr_idx = 1
        new_value = 33.0

        # read project and get layers
        myPath = os.path.join(unitTestDataPath(), 'joins.qgs')
        rc = QgsProject.instance().read(myPath)

        layer = QgsProject.instance().mapLayersByName("polys_with_id")[0]
        join_layer = QgsProject.instance().mapLayersByName("polys_overlapping_with_id")[0]

        # create an attribute table for the main_layer and the
        # joined layer
        cache = QgsVectorLayerCache(layer, 100)
        am = QgsAttributeTableModel(cache)
        am.loadLayer()

        join_cache = QgsVectorLayerCache(join_layer, 100)
        join_am = QgsAttributeTableModel(join_cache)
        join_am.loadLayer()

        # check feature value of a joined field from the attribute model
        model_index = am.idToIndex(fid)
        feature_model = am.feature(model_index)

        join_model_index = join_am.idToIndex(fid)
        join_feature_model = join_am.feature(join_model_index)

        self.assertEqual(feature_model.attribute(attr_idx), join_feature_model.attribute(join_attr_idx))

        # change attribute value for a feature of the joined layer
        join_layer.startEditing()
        join_layer.changeAttributeValue(fid, join_attr_idx, new_value)
        join_layer.commitChanges()

        # check the feature previously modified
        join_model_index = join_am.idToIndex(fid)
        join_feature_model = join_am.feature(join_model_index)
        self.assertEqual(join_feature_model.attribute(join_attr_idx), new_value)

        # recreate a new cache and model to simulate the opening of
        # a new attribute table
        cache = QgsVectorLayerCache(layer, 100)
        am = QgsAttributeTableModel(cache)
        am.loadLayer()

        # test that the model is up to date with the joined layer
        model_index = am.idToIndex(fid)
        feature_model = am.feature(model_index)
        self.assertEqual(feature_model.attribute(attr_idx), new_value)

        # restore value
        join_layer.startEditing()
        join_layer.changeAttributeValue(fid, join_attr_idx, 7.0)
        join_layer.commitChanges()

    def testUniqueValue(self):
        """ test retrieving unique values """
        layer = createLayerWithFivePoints()

        # test layer with just provider features
        self.assertEqual(set(layer.uniqueValues(1)), set([123, 457, 888, -1, 0]))

        # add feature with new value
        layer.startEditing()
        f1 = QgsFeature()
        f1.setAttributes(["test2", 999])
        self.assertTrue(layer.addFeature(f1))

        # should be included in unique values
        self.assertEqual(set(layer.uniqueValues(1)), set([123, 457, 888, -1, 0, 999]))
        # add it again, should be no change
        f2 = QgsFeature()
        f2.setAttributes(["test2", 999])
        self.assertTrue(layer.addFeature(f1))
        self.assertEqual(set(layer.uniqueValues(1)), set([123, 457, 888, -1, 0, 999]))
        # add another feature
        f3 = QgsFeature()
        f3.setAttributes(["test2", 9999])
        self.assertTrue(layer.addFeature(f3))
        self.assertEqual(set(layer.uniqueValues(1)), set([123, 457, 888, -1, 0, 999, 9999]))

        # change an attribute value to a new unique value
        f1_id = next(layer.getFeatures()).id()
        self.assertTrue(layer.changeAttributeValue(f1_id, 1, 481523))
        # note - this isn't 100% accurate, since 123 no longer exists - but it avoids looping through all features
        self.assertEqual(set(layer.uniqueValues(1)), set([123, 457, 888, -1, 0, 999, 9999, 481523]))

    def testUniqueStringsMatching(self):
        """ test retrieving unique strings matching subset """
        layer = QgsVectorLayer("Point?field=fldtxt:string", "addfeat", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["apple"])
        f2 = QgsFeature()
        f2.setAttributes(["orange"])
        f3 = QgsFeature()
        f3.setAttributes(["pear"])
        f4 = QgsFeature()
        f4.setAttributes(["BanaNa"])
        f5 = QgsFeature()
        f5.setAttributes(["ApriCot"])
        assert pr.addFeatures([f, f2, f3, f4, f5])
        assert layer.featureCount() == 5

        # test layer with just provider features
        self.assertEqual(set(layer.uniqueStringsMatching(0, 'N')), set(['orange', 'BanaNa']))

        # add feature with new value
        layer.startEditing()
        f1 = QgsFeature()
        f1.setAttributes(["waterMelon"])
        self.assertTrue(layer.addFeature(f1))

        # should be included in unique values
        self.assertEqual(set(layer.uniqueStringsMatching(0, 'N')), set(['orange', 'BanaNa', 'waterMelon']))
        # add it again, should be no change
        f2 = QgsFeature()
        f2.setAttributes(["waterMelon"])
        self.assertTrue(layer.addFeature(f1))
        self.assertEqual(set(layer.uniqueStringsMatching(0, 'N')), set(['orange', 'BanaNa', 'waterMelon']))
        self.assertEqual(set(layer.uniqueStringsMatching(0, 'aN')), set(['orange', 'BanaNa']))
        # add another feature
        f3 = QgsFeature()
        f3.setAttributes(["pineapple"])
        self.assertTrue(layer.addFeature(f3))
        self.assertEqual(set(layer.uniqueStringsMatching(0, 'n')), set(['orange', 'BanaNa', 'waterMelon', 'pineapple']))

        # change an attribute value to a new unique value
        f = QgsFeature()
        f1_id = next(layer.getFeatures()).id()
        self.assertTrue(layer.changeAttributeValue(f1_id, 0, 'coconut'))
        # note - this isn't 100% accurate, since orange no longer exists - but it avoids looping through all features
        self.assertEqual(set(layer.uniqueStringsMatching(0, 'n')),
                         set(['orange', 'BanaNa', 'waterMelon', 'pineapple', 'coconut']))

    def test_subsetString(self):
        subset_string_changed = False

        def onSubsetStringChanged():
            nonlocal subset_string_changed
            subset_string_changed = True

        path = os.path.join(unitTestDataPath(), 'lines.shp')
        layer = QgsVectorLayer(path, 'test', 'ogr')
        layer.subsetStringChanged.connect(onSubsetStringChanged)
        layer.setSubsetString("\"Name\" = 'Highway'")
        self.assertTrue(subset_string_changed)
        self.assertEqual(layer.featureCount(), 2)

    def testMinValue(self):
        """ test retrieving minimum values """
        layer = createLayerWithFivePoints()

        # test layer with just provider features
        self.assertEqual(layer.minimumValue(1), -1)

        # add feature with new value
        layer.startEditing()
        f1 = QgsFeature()
        f1.setAttributes(["test2", -999])
        self.assertTrue(layer.addFeature(f1))

        # should be new minimum value
        self.assertEqual(layer.minimumValue(1), -999)
        # add it again, should be no change
        f2 = QgsFeature()
        f2.setAttributes(["test2", -999])
        self.assertTrue(layer.addFeature(f1))
        self.assertEqual(layer.minimumValue(1), -999)
        # add another feature
        f3 = QgsFeature()
        f3.setAttributes(["test2", -1000])
        self.assertTrue(layer.addFeature(f3))
        self.assertEqual(layer.minimumValue(1), -1000)

        # change an attribute value to a new minimum value
        f1_id = next(layer.getFeatures()).id()
        self.assertTrue(layer.changeAttributeValue(f1_id, 1, -1001))
        self.assertEqual(layer.minimumValue(1), -1001)

    def testMaxValue(self):
        """ test retrieving maximum values """
        layer = createLayerWithFivePoints()

        # test layer with just provider features
        self.assertEqual(layer.maximumValue(1), 888)

        # add feature with new value
        layer.startEditing()
        f1 = QgsFeature()
        f1.setAttributes(["test2", 999])
        self.assertTrue(layer.addFeature(f1))

        # should be new maximum value
        self.assertEqual(layer.maximumValue(1), 999)
        # add it again, should be no change
        f2 = QgsFeature()
        f2.setAttributes(["test2", 999])
        self.assertTrue(layer.addFeature(f1))
        self.assertEqual(layer.maximumValue(1), 999)
        # add another feature
        f3 = QgsFeature()
        f3.setAttributes(["test2", 1000])
        self.assertTrue(layer.addFeature(f3))
        self.assertEqual(layer.maximumValue(1), 1000)

        # change an attribute value to a new maximum value
        f1_id = next(layer.getFeatures()).id()
        self.assertTrue(layer.changeAttributeValue(f1_id, 1, 1001))
        self.assertEqual(layer.maximumValue(1), 1001)

    def test_InvalidOperations(self):
        layer = createLayerWithOnePoint()

        layer.startEditing()

        # ADD FEATURE

        newF1 = QgsFeature()
        self.assertFalse(layer.addFeature(newF1))  # need attributes like the layer has)

        # DELETE FEATURE

        self.assertFalse(layer.deleteFeature(-333))
        # we do not check for existence of the feature id if it's
        # not newly added feature
        # self.assertFalse(layer.deleteFeature(333))

        # CHANGE GEOMETRY

        self.assertFalse(layer.changeGeometry(
            -333, QgsGeometry.fromPointXY(QgsPointXY(1, 1))))

        # CHANGE VALUE

        self.assertFalse(layer.changeAttributeValue(-333, 0, 1))
        self.assertFalse(layer.changeAttributeValue(1, -1, 1))

        # ADD ATTRIBUTE

        self.assertFalse(layer.addAttribute(QgsField()))

        # DELETE ATTRIBUTE

        self.assertFalse(layer.deleteAttribute(-1))

    def onBlendModeChanged(self, mode):
        self.blendModeTest = mode

    def test_setBlendMode(self):
        layer = createLayerWithOnePoint()

        self.blendModeTest = 0
        layer.blendModeChanged.connect(self.onBlendModeChanged)
        layer.setBlendMode(QPainter.CompositionMode_Screen)

        self.assertEqual(self.blendModeTest, QPainter.CompositionMode_Screen)
        self.assertEqual(layer.blendMode(), QPainter.CompositionMode_Screen)

    def test_setFeatureBlendMode(self):
        layer = createLayerWithOnePoint()

        self.blendModeTest = 0
        layer.featureBlendModeChanged.connect(self.onBlendModeChanged)
        layer.setFeatureBlendMode(QPainter.CompositionMode_Screen)

        self.assertEqual(self.blendModeTest, QPainter.CompositionMode_Screen)
        self.assertEqual(layer.featureBlendMode(), QPainter.CompositionMode_Screen)

    def test_ExpressionField(self):
        layer = createLayerWithOnePoint()

        cnt = layer.fields().count()

        idx = layer.addExpressionField('5', QgsField('test', QVariant.LongLong))

        fet = next(layer.getFeatures())
        self.assertEqual(fet[idx], 5)
        # check fields
        self.assertEqual(layer.fields().count(), cnt + 1)
        self.assertEqual(fet.fields(), layer.fields())

        # retrieve single feature and check fields
        fet = next(layer.getFeatures(QgsFeatureRequest().setFilterFid(1)))
        self.assertEqual(fet.fields(), layer.fields())

        layer.updateExpressionField(idx, '9')

        self.assertEqual(next(layer.getFeatures())[idx], 9)

        layer.removeExpressionField(idx)

        self.assertEqual(layer.fields().count(), cnt)

        # expression field which references itself
        idx = layer.addExpressionField('sum(test2)', QgsField('test2', QVariant.LongLong))
        fet = next(layer.getFeatures())
        self.assertEqual(fet['test2'], 0)

    def test_ExpressionFieldEllipsoidLengthCalculation(self):
        # create a temporary layer
        temp_layer = QgsVectorLayer("LineString?crs=epsg:3111&field=pk:int", "vl", "memory")
        self.assertTrue(temp_layer.isValid())
        f1 = QgsFeature(temp_layer.dataProvider().fields(), 1)
        f1.setAttribute("pk", 1)
        f1.setGeometry(QgsGeometry.fromPolylineXY([QgsPointXY(2484588, 2425722), QgsPointXY(2482767, 2398853)]))
        temp_layer.dataProvider().addFeatures([f1])

        # set project CRS and ellipsoid
        srs = QgsCoordinateReferenceSystem(3111, QgsCoordinateReferenceSystem.EpsgCrsId)
        QgsProject.instance().setCrs(srs)
        QgsProject.instance().setEllipsoid("WGS84")
        QgsProject.instance().setDistanceUnits(QgsUnitTypes.DistanceMeters)

        idx = temp_layer.addExpressionField('$length', QgsField('length', QVariant.Double))  # NOQA

        # check value
        f = next(temp_layer.getFeatures())
        expected = 26932.156
        self.assertAlmostEqual(f['length'], expected, 3)

        # change project length unit, check calculation respects unit
        QgsProject.instance().setDistanceUnits(QgsUnitTypes.DistanceFeet)
        f = next(temp_layer.getFeatures())
        expected = 88360.0918635
        self.assertAlmostEqual(f['length'], expected, 3)

    def test_ExpressionFieldEllipsoidAreaCalculation(self):
        # create a temporary layer
        temp_layer = QgsVectorLayer("Polygon?crs=epsg:3111&field=pk:int", "vl", "memory")
        self.assertTrue(temp_layer.isValid())
        f1 = QgsFeature(temp_layer.dataProvider().fields(), 1)
        f1.setAttribute("pk", 1)
        f1.setGeometry(QgsGeometry.fromPolygonXY([[QgsPointXY(2484588, 2425722), QgsPointXY(2482767, 2398853),
                                                   QgsPointXY(2520109, 2397715), QgsPointXY(2520792, 2425494),
                                                   QgsPointXY(2484588, 2425722)]]))
        temp_layer.dataProvider().addFeatures([f1])

        # set project CRS and ellipsoid
        srs = QgsCoordinateReferenceSystem(3111, QgsCoordinateReferenceSystem.EpsgCrsId)
        QgsProject.instance().setCrs(srs)
        QgsProject.instance().setEllipsoid("WGS84")
        QgsProject.instance().setAreaUnits(QgsUnitTypes.AreaSquareMeters)

        idx = temp_layer.addExpressionField('$area', QgsField('area', QVariant.Double))  # NOQA

        # check value
        f = next(temp_layer.getFeatures())
        expected = 1005721496.7800847
        self.assertAlmostEqual(f['area'], expected, delta=1.0)

        # change project area unit, check calculation respects unit
        QgsProject.instance().setAreaUnits(QgsUnitTypes.AreaSquareMiles)
        f = next(temp_layer.getFeatures())
        expected = 388.31124079933016
        self.assertAlmostEqual(f['area'], expected, 3)

    def test_ExpressionFilter(self):
        layer = createLayerWithOnePoint()

        idx = layer.addExpressionField('5', QgsField('test', QVariant.LongLong))  # NOQA

        features = layer.getFeatures(QgsFeatureRequest().setFilterExpression('"test" = 6'))

        assert (len(list(features)) == 0)

        features = layer.getFeatures(QgsFeatureRequest().setFilterExpression('"test" = 5'))

        assert (len(list(features)) == 1)

    def testSelectByIds(self):
        """ Test selecting by ID"""
        layer = QgsVectorLayer(os.path.join(unitTestDataPath(), 'points.shp'), 'Points', 'ogr')

        # SetSelection
        layer.selectByIds([1, 3, 5, 7], QgsVectorLayer.SetSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([1, 3, 5, 7]))
        # check that existing selection is cleared
        layer.selectByIds([2, 4, 6], QgsVectorLayer.SetSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 4, 6]))

        # AddToSelection
        layer.selectByIds([3, 5], QgsVectorLayer.AddToSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 3, 4, 5, 6]))
        layer.selectByIds([1], QgsVectorLayer.AddToSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([1, 2, 3, 4, 5, 6]))

        # IntersectSelection
        layer.selectByIds([1, 3, 5, 6], QgsVectorLayer.IntersectSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([1, 3, 5, 6]))
        layer.selectByIds([1, 2, 5, 6], QgsVectorLayer.IntersectSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([1, 5, 6]))

        # RemoveFromSelection
        layer.selectByIds([2, 6, 7], QgsVectorLayer.RemoveFromSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([1, 5]))
        layer.selectByIds([1, 5], QgsVectorLayer.RemoveFromSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([]))

    def testSelectByExpression(self):
        """ Test selecting by expression """
        layer = QgsVectorLayer(os.path.join(unitTestDataPath(), 'points.shp'), 'Points', 'ogr')

        # SetSelection
        layer.selectByExpression('"Class"=\'B52\' and "Heading" > 10 and "Heading" <70', QgsVectorLayer.SetSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([10, 11]))
        # check that existing selection is cleared
        layer.selectByExpression('"Class"=\'Biplane\'', QgsVectorLayer.SetSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([1, 5, 6, 7, 8]))
        # SetSelection no matching
        layer.selectByExpression('"Class"=\'A380\'', QgsVectorLayer.SetSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([]))

        # AddToSelection
        layer.selectByExpression('"Importance"=3', QgsVectorLayer.AddToSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([0, 2, 3, 4, 14]))
        layer.selectByExpression('"Importance"=4', QgsVectorLayer.AddToSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([0, 2, 3, 4, 13, 14]))

        # IntersectSelection
        layer.selectByExpression('"Heading"<100', QgsVectorLayer.IntersectSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([0, 2, 3, 4]))
        layer.selectByExpression('"Cabin Crew"=1', QgsVectorLayer.IntersectSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 3]))

        # RemoveFromSelection
        layer.selectByExpression('"Heading"=85', QgsVectorLayer.RemoveFromSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([3]))
        layer.selectByExpression('"Heading"=95', QgsVectorLayer.RemoveFromSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([]))

    def testSelectByRect(self):
        """ Test selecting by rectangle """
        layer = QgsVectorLayer(os.path.join(unitTestDataPath(), 'points.shp'), 'Points', 'ogr')

        # SetSelection
        layer.selectByRect(QgsRectangle(-112, 30, -94, 45), QgsVectorLayer.SetSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 3, 7, 10, 11, 15]))
        # check that existing selection is cleared
        layer.selectByRect(QgsRectangle(-112, 30, -94, 37), QgsVectorLayer.SetSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 3, 10, 15]))
        # SetSelection no matching
        layer.selectByRect(QgsRectangle(112, 30, 115, 45), QgsVectorLayer.SetSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([]))

        # AddToSelection
        layer.selectByRect(QgsRectangle(-112, 30, -94, 37), QgsVectorLayer.AddToSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 3, 10, 15]))
        layer.selectByRect(QgsRectangle(-112, 37, -94, 45), QgsVectorLayer.AddToSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 3, 7, 10, 11, 15]))

        # IntersectSelection
        layer.selectByRect(QgsRectangle(-112, 30, -94, 37), QgsVectorLayer.IntersectSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 3, 10, 15]))
        layer.selectByIds([2, 10, 13])
        layer.selectByRect(QgsRectangle(-112, 30, -94, 37), QgsVectorLayer.IntersectSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([2, 10]))

        # RemoveFromSelection
        layer.selectByRect(QgsRectangle(-112, 30, -94, 45), QgsVectorLayer.SetSelection)
        layer.selectByRect(QgsRectangle(-112, 30, -94, 37), QgsVectorLayer.RemoveFromSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([7, 11]))
        layer.selectByRect(QgsRectangle(-112, 30, -94, 45), QgsVectorLayer.RemoveFromSelection)
        self.assertEqual(set(layer.selectedFeatureIds()), set([]))

    def testReselect(self):
        layer = QgsVectorLayer(os.path.join(unitTestDataPath(), 'points.shp'), 'Points', 'ogr')

        layer.selectByIds([1, 3, 5, 7], QgsVectorLayer.SetSelection)
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5, 7])

        layer.reselect() # no effect, selection has not been cleared
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5, 7])

        # clear selection
        layer.removeSelection()
        self.assertCountEqual(layer.selectedFeatureIds(), [])
        # reselect should bring this back
        layer.reselect()
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5, 7])
        layer.reselect() # no change
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5, 7])

        # change an existing selection
        layer.selectByIds([1, 3, 5], QgsVectorLayer.SetSelection)
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5])
        layer.reselect()  # no change
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5])

        layer.removeSelection()
        self.assertCountEqual(layer.selectedFeatureIds(), [])
        # reselect should bring this back
        layer.reselect()
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5])

        layer.select(7)
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5, 7])
        layer.reselect()
        self.assertCountEqual(layer.selectedFeatureIds(), [1, 3, 5, 7])
        layer.removeSelection()
        layer.select([3, 5])
        self.assertCountEqual(layer.selectedFeatureIds(), [3, 5])
        layer.reselect()
        self.assertCountEqual(layer.selectedFeatureIds(), [3, 5])
        layer.deselect([5])
        self.assertCountEqual(layer.selectedFeatureIds(), [3])
        layer.reselect()
        self.assertCountEqual(layer.selectedFeatureIds(), [3])
        layer.modifySelection([5], [3])
        self.assertCountEqual(layer.selectedFeatureIds(), [5])
        layer.reselect()
        self.assertCountEqual(layer.selectedFeatureIds(), [5])

    def testAggregate(self):
        """ Test aggregate calculation """
        layer = QgsVectorLayer("Point?field=fldint:integer", "layer", "memory")
        pr = layer.dataProvider()

        int_values = [4, 2, 3, 2, 5, None, 8]
        features = []
        for i in int_values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([i])
            features.append(f)
        assert pr.addFeatures(features)

        tests = [[QgsAggregateCalculator.Count, 6],
                 [QgsAggregateCalculator.Sum, 24],
                 [QgsAggregateCalculator.Mean, 4],
                 [QgsAggregateCalculator.StDev, 2.0816],
                 [QgsAggregateCalculator.StDevSample, 2.2803],
                 [QgsAggregateCalculator.Min, 2],
                 [QgsAggregateCalculator.Max, 8],
                 [QgsAggregateCalculator.Range, 6],
                 [QgsAggregateCalculator.Median, 3.5],
                 [QgsAggregateCalculator.CountDistinct, 5],
                 [QgsAggregateCalculator.CountMissing, 1],
                 [QgsAggregateCalculator.FirstQuartile, 2],
                 [QgsAggregateCalculator.ThirdQuartile, 5.0],
                 [QgsAggregateCalculator.InterQuartileRange, 3.0]
                 ]

        for t in tests:
            val, ok = layer.aggregate(t[0], 'fldint')
            self.assertTrue(ok)
            if isinstance(t[1], int):
                self.assertEqual(val, t[1])
            else:
                self.assertAlmostEqual(val, t[1], 3)

        # test with parameters
        layer = QgsVectorLayer("Point?field=fldstring:string", "layer", "memory")
        pr = layer.dataProvider()

        string_values = ['this', 'is', 'a', 'test', 'a', 'nice', 'test']
        features = []
        for s in string_values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([s])
            features.append(f)
        assert pr.addFeatures(features)
        params = QgsAggregateCalculator.AggregateParameters()
        params.delimiter = ' '
        val, ok = layer.aggregate(QgsAggregateCalculator.StringConcatenate, 'fldstring', params)
        self.assertTrue(ok)
        self.assertEqual(val, 'this is a test a nice test')
        val, ok = layer.aggregate(QgsAggregateCalculator.StringConcatenateUnique, 'fldstring', params)
        self.assertTrue(ok)
        self.assertEqual(val, 'this is a test nice')

    def testAggregateInVirtualField(self):
        """
        Test aggregates in a virtual field
        """
        layer = QgsVectorLayer("Point?field=fldint:integer", "layer", "memory")
        pr = layer.dataProvider()

        int_values = [4, 2, 3, 2, 5, None, 8]
        features = []
        for i in int_values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([i])
            features.append(f)
        assert pr.addFeatures(features)

        field = QgsField('virtual', QVariant.Double)
        layer.addExpressionField('sum(fldint*2)', field)
        vals = [f['virtual'] for f in layer.getFeatures()]
        self.assertEqual(vals, [48, 48, 48, 48, 48, 48, 48])

    def testAggregateFilter(self):
        """ Test aggregate calculation """
        layer = QgsVectorLayer("Point?field=fldint:integer", "layer", "memory")
        pr = layer.dataProvider()

        int_values = [4, 2, 3, 2, 5, None, 8]
        features = []
        for i in int_values:
            f = QgsFeature()
            f.setFields(layer.fields())
            f.setAttributes([i])
            features.append(f)
        assert pr.addFeatures(features)

        val, ok = layer.aggregate(QgsAggregateCalculator.Sum, 'fldint', fids=[1, 2])
        self.assertTrue(ok)
        self.assertEqual(val, 6.0)

    def onLayerOpacityChanged(self, tr):
        self.opacityTest = tr

    def test_setLayerOpacity(self):
        layer = createLayerWithOnePoint()

        self.opacityTest = 0
        layer.opacityChanged.connect(self.onLayerOpacityChanged)
        layer.setOpacity(0.5)
        self.assertEqual(self.opacityTest, 0.5)
        self.assertEqual(layer.opacity(), 0.5)

    def onRendererChanged(self):
        self.rendererChanged = True

    def test_setRenderer(self):
        layer = createLayerWithOnePoint()

        self.rendererChanged = False
        layer.rendererChanged.connect(self.onRendererChanged)

        r = QgsSingleSymbolRenderer(QgsSymbol.defaultSymbol(QgsWkbTypes.PointGeometry))
        layer.setRenderer(r)
        self.assertTrue(self.rendererChanged)
        self.assertEqual(layer.renderer(), r)

    def testGetSetAliases(self):
        """ test getting and setting aliases """
        layer = createLayerWithOnePoint()

        self.assertFalse(layer.attributeAlias(0))
        self.assertFalse(layer.attributeAlias(1))
        self.assertFalse(layer.attributeAlias(2))

        layer.setFieldAlias(0, "test")
        self.assertEqual(layer.attributeAlias(0), "test")
        self.assertFalse(layer.attributeAlias(1))
        self.assertFalse(layer.attributeAlias(2))
        self.assertEqual(layer.fields().at(0).alias(), "test")

        layer.setFieldAlias(1, "test2")
        self.assertEqual(layer.attributeAlias(0), "test")
        self.assertEqual(layer.attributeAlias(1), "test2")
        self.assertFalse(layer.attributeAlias(2))
        self.assertEqual(layer.fields().at(0).alias(), "test")
        self.assertEqual(layer.fields().at(1).alias(), "test2")

        layer.setFieldAlias(1, None)
        self.assertEqual(layer.attributeAlias(0), "test")
        self.assertFalse(layer.attributeAlias(1))
        self.assertFalse(layer.attributeAlias(2))
        self.assertEqual(layer.fields().at(0).alias(), "test")
        self.assertFalse(layer.fields().at(1).alias())

        layer.removeFieldAlias(0)
        self.assertFalse(layer.attributeAlias(0))
        self.assertFalse(layer.attributeAlias(1))
        self.assertFalse(layer.attributeAlias(2))
        self.assertFalse(layer.fields().at(0).alias())
        self.assertFalse(layer.fields().at(1).alias())

    def testSaveRestoreAliases(self):
        """ test saving and restoring aliases from xml"""
        layer = createLayerWithOnePoint()

        # no default expressions
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer2 = createLayerWithOnePoint()
        self.assertTrue(layer2.readXml(elem, QgsReadWriteContext()))
        self.assertFalse(layer2.attributeAlias(0))
        self.assertFalse(layer2.attributeAlias(1))

        # set some aliases
        layer.setFieldAlias(0, "test")
        layer.setFieldAlias(1, "test2")

        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer3 = createLayerWithOnePoint()
        self.assertTrue(layer3.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(layer3.attributeAlias(0), "test")
        self.assertEqual(layer3.attributeAlias(1), "test2")
        self.assertEqual(layer3.fields().at(0).alias(), "test")
        self.assertEqual(layer3.fields().at(1).alias(), "test2")

    def testGetSetDefaults(self):
        """ test getting and setting default expressions """
        layer = createLayerWithOnePoint()

        self.assertFalse(layer.defaultValueDefinition(0))
        self.assertFalse(layer.defaultValueDefinition(0).expression())
        self.assertFalse(layer.defaultValueDefinition(0).applyOnUpdate())
        self.assertFalse(layer.defaultValueDefinition(1))
        self.assertFalse(layer.defaultValueDefinition(2))

        layer.setDefaultValueDefinition(0, QgsDefaultValue("'test'"))
        self.assertTrue(layer.defaultValueDefinition(0))
        self.assertEqual(layer.defaultValueDefinition(0).expression(), "'test'")
        self.assertFalse(layer.defaultValueDefinition(0).applyOnUpdate())
        self.assertFalse(layer.defaultValueDefinition(1))
        self.assertFalse(layer.defaultValueDefinition(1).applyOnUpdate())
        self.assertFalse(layer.defaultValueDefinition(2))
        self.assertFalse(layer.defaultValueDefinition(2).applyOnUpdate())
        self.assertEqual(layer.fields().at(0).defaultValueDefinition().expression(), "'test'")

        layer.setDefaultValueDefinition(1, QgsDefaultValue("2+2"))
        self.assertEqual(layer.defaultValueDefinition(0).expression(), "'test'")
        self.assertFalse(layer.defaultValueDefinition(0).applyOnUpdate())
        self.assertEqual(layer.defaultValueDefinition(1).expression(), "2+2")
        self.assertFalse(layer.defaultValueDefinition(1).applyOnUpdate())
        self.assertFalse(layer.defaultValueDefinition(2))
        self.assertFalse(layer.defaultValueDefinition(2).applyOnUpdate())
        self.assertEqual(layer.fields().at(0).defaultValueDefinition().expression(), "'test'")
        self.assertEqual(layer.fields().at(1).defaultValueDefinition().expression(), "2+2")

        layer.setDefaultValueDefinition(1, QgsDefaultValue("2+2", True))
        self.assertEqual(layer.defaultValueDefinition(0).expression(), "'test'")
        self.assertFalse(layer.defaultValueDefinition(0).applyOnUpdate())
        self.assertEqual(layer.defaultValueDefinition(1).expression(), "2+2")
        self.assertTrue(layer.defaultValueDefinition(1).applyOnUpdate())
        self.assertEqual(layer.fields().at(0).defaultValueDefinition().expression(), "'test'")
        self.assertEqual(layer.fields().at(1).defaultValueDefinition().expression(), "2+2")

    def testSaveRestoreDefaults(self):
        """ test saving and restoring default expressions from xml"""
        layer = createLayerWithOnePoint()

        # no default expressions
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer2 = createLayerWithOnePoint()
        self.assertTrue(layer2.readXml(elem, QgsReadWriteContext()))
        self.assertFalse(layer2.defaultValueDefinition(0))
        self.assertFalse(layer2.defaultValueDefinition(1))

        # set some default expressions
        layer.setDefaultValueDefinition(0, QgsDefaultValue("'test'"))
        layer.setDefaultValueDefinition(1, QgsDefaultValue("2+2"))

        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer3 = createLayerWithOnePoint()
        self.assertTrue(layer3.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(layer3.defaultValueDefinition(0).expression(), "'test'")
        self.assertEqual(layer3.defaultValueDefinition(1).expression(), "2+2")
        self.assertEqual(layer3.fields().at(0).defaultValueDefinition().expression(), "'test'")
        self.assertEqual(layer3.fields().at(1).defaultValueDefinition().expression(), "2+2")

    def testEvaluatingDefaultExpressions(self):
        """ tests calculation of default values"""
        layer = createLayerWithOnePoint()
        layer.setDefaultValueDefinition(0, QgsDefaultValue("'test'"))
        layer.setDefaultValueDefinition(1, QgsDefaultValue("2+2"))
        self.assertEqual(layer.defaultValue(0), 'test')
        self.assertEqual(layer.defaultValue(1), 4)

        # using feature
        layer.setDefaultValueDefinition(1, QgsDefaultValue('$id * 2'))
        feature = QgsFeature(4)
        feature.setValid(True)
        feature.setFields(layer.fields())
        # no feature:
        self.assertFalse(layer.defaultValue(1))
        # with feature:
        self.assertEqual(layer.defaultValue(0, feature), 'test')
        self.assertEqual(layer.defaultValue(1, feature), 8)

        # using feature geometry
        layer.setDefaultValueDefinition(1, QgsDefaultValue('$x * 2'))
        feature.setGeometry(QgsGeometry(QgsPoint(6, 7)))
        self.assertEqual(layer.defaultValue(1, feature), 12)

        # using contexts
        scope = QgsExpressionContextScope()
        scope.setVariable('var1', 16)
        context = QgsExpressionContext()
        context.appendScope(scope)
        layer.setDefaultValueDefinition(1, QgsDefaultValue('$id + @var1'))
        self.assertEqual(layer.defaultValue(1, feature, context), 20)

        # if no scope passed, should use a default constructed one including layer variables
        QgsExpressionContextUtils.setLayerVariable(layer, 'var2', 4)
        QgsExpressionContextUtils.setProjectVariable(QgsProject.instance(), 'var3', 8)
        layer.setDefaultValueDefinition(1, QgsDefaultValue('to_int(@var2) + to_int(@var3) + $id'))
        self.assertEqual(layer.defaultValue(1, feature), 16)

        # bad expression
        layer.setDefaultValueDefinition(1, QgsDefaultValue('not a valid expression'))
        self.assertFalse(layer.defaultValue(1))

    def testApplyOnUpdateDefaultExpressions(self):
        """tests apply on update of default values"""
        layer = createLayerWithOnePoint()

        layer.setDefaultValueDefinition(0, QgsDefaultValue("CONCAT('l: ', @number, ',f: ', \"fldint\" )", True))
        layer.setDefaultValueDefinition(1, QgsDefaultValue("1 * @number", False))

        QgsExpressionContextUtils.setLayerVariable(layer, 'number', 4)

        layer.startEditing()
        feature = QgsFeature()
        feature.setFields(layer.fields())
        feature.setValid(True)

        # Both default values should be set on feature create
        feature.setAttribute(1, layer.defaultValue(1, feature))
        feature.setAttribute(0, layer.defaultValue(0, feature))

        self.assertTrue(layer.addFeature(feature))
        fid = feature.id()
        self.assertEqual(layer.getFeature(fid)['fldtxt'], 'l: 4,f: 4')
        self.assertEqual(layer.getFeature(fid)['fldint'], 4)

        # ApplyOnUpdateDefaultValue should be set on changeAttributeValue
        layer.changeAttributeValue(fid, 1, 20)
        self.assertEqual(layer.getFeature(fid)['fldtxt'], 'l: 4,f: 20')
        self.assertEqual(layer.getFeature(fid)['fldint'], 20)

        # When changing the value of the "derived" attribute, only this one
        # should be updated
        QgsExpressionContextUtils.setLayerVariable(layer, 'number', 8)
        layer.changeAttributeValue(fid, 0, 0)
        self.assertEqual(layer.getFeature(fid)['fldtxt'], 'l: 8,f: 20')
        self.assertEqual(layer.getFeature(fid)['fldint'], 20)

        # Check update on geometry change
        layer.setDefaultValueDefinition(1, QgsDefaultValue("x($geometry)", True))
        layer.changeGeometry(fid, QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
        self.assertEqual(layer.getFeature(fid)['fldint'], 300)

    def testGetSetConstraints(self):
        """ test getting and setting field constraints """
        layer = createLayerWithOnePoint()

        self.assertFalse(layer.fieldConstraints(0))
        self.assertFalse(layer.fieldConstraints(1))
        self.assertFalse(layer.fieldConstraints(2))

        layer.setFieldConstraint(0, QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(layer.fieldConstraints(0), QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(layer.fieldConstraints(1))
        self.assertFalse(layer.fieldConstraints(2))
        self.assertEqual(layer.fields().at(0).constraints().constraints(), QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(layer.fields().at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer.fields().at(0).constraints().constraintStrength(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintStrengthHard)

        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull)
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(layer.fieldConstraints(0), QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(layer.fieldConstraints(1),
                         QgsFieldConstraints.ConstraintNotNull | QgsFieldConstraints.ConstraintUnique)
        self.assertFalse(layer.fieldConstraints(2))
        self.assertEqual(layer.fields().at(0).constraints().constraints(), QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(layer.fields().at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer.fields().at(0).constraints().constraintStrength(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintStrengthHard)
        self.assertEqual(layer.fields().at(1).constraints().constraints(),
                         QgsFieldConstraints.ConstraintNotNull | QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(layer.fields().at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer.fields().at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer.fields().at(1).constraints().constraintStrength(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintStrengthHard)
        self.assertEqual(layer.fields().at(1).constraints().constraintStrength(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintStrengthHard)

        layer.removeFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull)
        layer.removeFieldConstraint(1, QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(layer.fieldConstraints(0), QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(layer.fieldConstraints(1))
        self.assertFalse(layer.fieldConstraints(2))
        self.assertEqual(layer.fields().at(0).constraints().constraints(), QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(layer.fields().at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer.fields().at(0).constraints().constraintStrength(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintStrengthHard)
        self.assertFalse(layer.fields().at(1).constraints().constraints())
        self.assertEqual(layer.fields().at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginNotSet)
        self.assertEqual(layer.fields().at(1).constraints().constraintStrength(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintStrengthNotSet)

    def testSaveRestoreConstraints(self):
        """ test saving and restoring constraints from xml"""
        layer = createLayerWithOnePoint()

        # no constraints
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer2 = createLayerWithOnePoint()
        self.assertTrue(layer2.readXml(elem, QgsReadWriteContext()))
        self.assertFalse(layer2.fieldConstraints(0))
        self.assertFalse(layer2.fieldConstraints(1))

        # set some constraints
        layer.setFieldConstraint(0, QgsFieldConstraints.ConstraintNotNull)
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull, QgsFieldConstraints.ConstraintStrengthSoft)
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintUnique)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer3 = createLayerWithOnePoint()
        self.assertTrue(layer3.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(layer3.fieldConstraints(0), QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(layer3.fieldConstraints(1),
                         QgsFieldConstraints.ConstraintNotNull | QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(layer3.fields().at(0).constraints().constraints(), QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(layer3.fields().at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer.fields().at(0).constraints().constraintStrength(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintStrengthHard)
        self.assertEqual(layer3.fields().at(1).constraints().constraints(),
                         QgsFieldConstraints.ConstraintNotNull | QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(layer3.fields().at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer3.fields().at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer.fields().at(1).constraints().constraintStrength(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintStrengthSoft)
        self.assertEqual(layer.fields().at(1).constraints().constraintStrength(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintStrengthHard)

    def testGetSetConstraintExpressions(self):
        """ test getting and setting field constraint expressions """
        layer = createLayerWithOnePoint()

        self.assertFalse(layer.constraintExpression(0))
        self.assertFalse(layer.constraintExpression(1))
        self.assertFalse(layer.constraintExpression(2))

        layer.setConstraintExpression(0, '1+2')
        self.assertEqual(layer.constraintExpression(0), '1+2')
        self.assertFalse(layer.constraintExpression(1))
        self.assertFalse(layer.constraintExpression(2))
        self.assertEqual(layer.fields().at(0).constraints().constraintExpression(), '1+2')

        layer.setConstraintExpression(1, '3+4', 'desc')
        self.assertEqual(layer.constraintExpression(0), '1+2')
        self.assertEqual(layer.constraintExpression(1), '3+4')
        self.assertEqual(layer.constraintDescription(1), 'desc')
        self.assertFalse(layer.constraintExpression(2))
        self.assertEqual(layer.fields().at(0).constraints().constraintExpression(), '1+2')
        self.assertEqual(layer.fields().at(1).constraints().constraintExpression(), '3+4')
        self.assertEqual(layer.fields().at(1).constraints().constraintDescription(), 'desc')

        layer.setConstraintExpression(1, None)
        self.assertEqual(layer.constraintExpression(0), '1+2')
        self.assertFalse(layer.constraintExpression(1))
        self.assertFalse(layer.constraintExpression(2))
        self.assertEqual(layer.fields().at(0).constraints().constraintExpression(), '1+2')
        self.assertFalse(layer.fields().at(1).constraints().constraintExpression())

    def testSaveRestoreConstraintExpressions(self):
        """ test saving and restoring constraint expressions from xml"""
        layer = createLayerWithOnePoint()

        # no constraints
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer2 = createLayerWithOnePoint()
        self.assertTrue(layer2.readXml(elem, QgsReadWriteContext()))
        self.assertFalse(layer2.constraintExpression(0))
        self.assertFalse(layer2.constraintExpression(1))

        # set some constraints
        layer.setConstraintExpression(0, '1+2')
        layer.setConstraintExpression(1, '3+4', 'desc')

        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeXml(elem, doc, QgsReadWriteContext()))

        layer3 = createLayerWithOnePoint()
        self.assertTrue(layer3.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(layer3.constraintExpression(0), '1+2')
        self.assertEqual(layer3.constraintExpression(1), '3+4')
        self.assertEqual(layer3.constraintDescription(1), 'desc')
        self.assertEqual(layer3.fields().at(0).constraints().constraintExpression(), '1+2')
        self.assertEqual(layer3.fields().at(1).constraints().constraintExpression(), '3+4')
        self.assertEqual(layer3.fields().at(1).constraints().constraintDescription(), 'desc')
        self.assertEqual(layer3.fields().at(0).constraints().constraints(), QgsFieldConstraints.ConstraintExpression)
        self.assertEqual(layer3.fields().at(1).constraints().constraints(), QgsFieldConstraints.ConstraintExpression)
        self.assertEqual(layer3.fields().at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintExpression),
                         QgsFieldConstraints.ConstraintOriginLayer)
        self.assertEqual(layer3.fields().at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintExpression),
                         QgsFieldConstraints.ConstraintOriginLayer)

    def testGetFeatureLimitWithEdits(self):
        """ test getting features with a limit, when edits are present """
        layer = createLayerWithOnePoint()
        # now has one feature with id 0

        pr = layer.dataProvider()

        f1 = QgsFeature(1)
        f1.setAttributes(["test", 3])
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
        f2 = QgsFeature(2)
        f2.setAttributes(["test", 3])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f3 = QgsFeature(3)
        f3.setAttributes(["test", 3])
        f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        self.assertTrue(pr.addFeatures([f1, f2, f3]))

        req = QgsFeatureRequest().setLimit(2)
        self.assertEqual(len(list(layer.getFeatures(req))), 2)

        # now delete feature f1
        layer.startEditing()
        self.assertTrue(layer.deleteFeature(1))
        req = QgsFeatureRequest().setLimit(2)
        self.assertEqual(len(list(layer.getFeatures(req))), 2)
        layer.rollBack()

        # change an attribute value required by filter
        layer.startEditing()
        req = QgsFeatureRequest().setFilterExpression('fldint=3').setLimit(2)
        self.assertTrue(layer.changeAttributeValue(2, 1, 4))
        self.assertEqual(len(list(layer.getFeatures(req))), 2)
        layer.rollBack()

        layer.startEditing()
        req = QgsFeatureRequest().setFilterRect(QgsRectangle(50, 100, 150, 300)).setLimit(2)
        self.assertTrue(layer.changeGeometry(2, QgsGeometry.fromPointXY(QgsPointXY(500, 600))))
        self.assertEqual(len(list(layer.getFeatures(req))), 2)
        layer.rollBack()

    def testClone(self):
        # init crs
        srs = QgsCoordinateReferenceSystem(3111, QgsCoordinateReferenceSystem.EpsgCrsId)

        # init map layer styles
        tmplayer = createLayerWithTwoPoints()
        sym1 = QgsLineSymbol()
        sym1.setColor(Qt.magenta)
        tmplayer.setRenderer(QgsSingleSymbolRenderer(sym1))

        style0 = QgsMapLayerStyle()
        style0.readFromLayer(tmplayer)
        style1 = QgsMapLayerStyle()
        style1.readFromLayer(tmplayer)

        # init dependencies layers
        ldep = createLayerWithTwoPoints()
        dep = QgsMapLayerDependency(ldep.id())

        # init layer
        layer = createLayerWithTwoPoints()
        layer.setBlendMode(QPainter.CompositionMode_Screen)
        layer.styleManager().addStyle('style0', style0)
        layer.styleManager().addStyle('style1', style1)
        layer.setName('MyName')
        layer.setShortName('MyShortName')
        layer.setMaximumScale(0.5)
        layer.setMinimumScale(1.5)
        layer.setScaleBasedVisibility(True)
        layer.setTitle('MyTitle')
        layer.setAbstract('MyAbstract')
        layer.setKeywordList('MyKeywordList')
        layer.setDataUrl('MyDataUrl')
        layer.setDataUrlFormat('MyDataUrlFormat')
        layer.setAttribution('MyAttribution')
        layer.setAttributionUrl('MyAttributionUrl')
        layer.setMetadataUrl('MyMetadataUrl')
        layer.setMetadataUrlType('MyMetadataUrlType')
        layer.setMetadataUrlFormat('MyMetadataUrlFormat')
        layer.setLegendUrl('MyLegendUrl')
        layer.setLegendUrlFormat('MyLegendUrlFormat')
        layer.setDependencies([dep])
        layer.setCrs(srs)

        layer.setCustomProperty('MyKey0', 'MyValue0')
        layer.setCustomProperty('MyKey1', 'MyValue1')

        layer.setOpacity(0.66)
        layer.setProviderEncoding('latin9')
        layer.setDisplayExpression('MyDisplayExpression')
        layer.setMapTipTemplate('MyMapTipTemplate')
        layer.setExcludeAttributesWfs(['MyExcludeAttributeWFS'])
        layer.setExcludeAttributesWms(['MyExcludeAttributeWMS'])

        layer.setFeatureBlendMode(QPainter.CompositionMode_Xor)

        sym = QgsLineSymbol()
        sym.setColor(Qt.magenta)
        layer.setRenderer(QgsSingleSymbolRenderer(sym))

        simplify = layer.simplifyMethod()
        simplify.setTolerance(33.3)
        simplify.setThreshold(0.333)
        layer.setSimplifyMethod(simplify)

        layer.setFieldAlias(0, 'MyAlias0')
        layer.setFieldAlias(1, 'MyAlias1')

        jl0 = createLayerWithTwoPoints()
        j0 = QgsVectorLayerJoinInfo()
        j0.setJoinLayer(jl0)

        jl1 = createLayerWithTwoPoints()
        j1 = QgsVectorLayerJoinInfo()
        j1.setJoinLayer(jl1)

        layer.addJoin(j0)
        layer.addJoin(j1)

        fids = layer.allFeatureIds()
        selected_fids = fids[0:3]
        layer.selectByIds(selected_fids)

        cfg = layer.attributeTableConfig()
        cfg.setSortOrder(Qt.DescendingOrder)  # by default AscendingOrder
        layer.setAttributeTableConfig(cfg)

        pal = QgsPalLayerSettings()
        text_format = QgsTextFormat()
        text_format.setSize(33)
        text_format.setColor(Qt.magenta)
        pal.setFormat(text_format)

        labeling = QgsVectorLayerSimpleLabeling(pal)
        layer.setLabeling(labeling)

        diag_renderer = QgsSingleCategoryDiagramRenderer()
        diag_renderer.setAttributeLegend(False)  # true by default
        layer.setDiagramRenderer(diag_renderer)

        diag_settings = QgsDiagramLayerSettings()
        diag_settings.setPriority(3)
        diag_settings.setZIndex(0.33)
        layer.setDiagramLayerSettings(diag_settings)

        edit_form_config = layer.editFormConfig()
        edit_form_config.setUiForm("MyUiForm")
        edit_form_config.setInitFilePath("MyInitFilePath")
        layer.setEditFormConfig(edit_form_config)

        widget_setup = QgsEditorWidgetSetup("MyWidgetSetupType", {})
        layer.setEditorWidgetSetup(0, widget_setup)

        layer.setConstraintExpression(0, "MyFieldConstraintExpression")
        layer.setFieldConstraint(0, QgsFieldConstraints.ConstraintUnique, QgsFieldConstraints.ConstraintStrengthHard)
        layer.setDefaultValueDefinition(0, QgsDefaultValue("MyDefaultValueExpression"))

        action = QgsAction(QgsAction.Unix, "MyActionDescription", "MyActionCmd")
        layer.actions().addAction(action)

        # clone layer
        clone = layer.clone()

        # generate xml from layer
        layer_doc = QDomDocument("doc")
        layer_elem = layer_doc.createElement("maplayer")
        layer.writeLayerXml(layer_elem, layer_doc, QgsReadWriteContext())

        # generate xml from clone
        clone_doc = QDomDocument("doc")
        clone_elem = clone_doc.createElement("maplayer")
        clone.writeLayerXml(clone_elem, clone_doc, QgsReadWriteContext())

        # replace id within xml of clone
        clone_id_elem = clone_elem.firstChildElement("id")
        clone_id_elem_patch = clone_doc.createElement("id")
        clone_id_elem_patch_value = clone_doc.createTextNode(layer.id())
        clone_id_elem_patch.appendChild(clone_id_elem_patch_value)
        clone_elem.replaceChild(clone_id_elem_patch, clone_id_elem)

        # update doc
        clone_doc.appendChild(clone_elem)
        layer_doc.appendChild(layer_elem)

        # compare xml documents
        self.assertEqual(layer_doc.toString(), clone_doc.toString())

    def testQgsVectorLayerSelectedFeatureSource(self):
        """
        test QgsVectorLayerSelectedFeatureSource
        """

        layer = QgsVectorLayer("Point?crs=epsg:3111&field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = layer.dataProvider()
        f1 = QgsFeature(1)
        f1.setAttributes(["test", 123])
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature(2)
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
        f3 = QgsFeature(3)
        f3.setAttributes(["test2", 888])
        f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
        f4 = QgsFeature(4)
        f4.setAttributes(["test3", -1])
        f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(400, 300)))
        f5 = QgsFeature(5)
        f5.setAttributes(["test4", 0])
        f5.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(0, 0)))
        self.assertTrue(pr.addFeatures([f1, f2, f3, f4, f5]))
        self.assertEqual(layer.featureCount(), 5)

        source = QgsVectorLayerSelectedFeatureSource(layer)
        self.assertEqual(source.sourceCrs().authid(), 'EPSG:3111')
        self.assertEqual(source.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(source.fields(), layer.fields())

        # no selection
        self.assertEqual(source.featureCount(), 0)
        it = source.getFeatures()
        f = QgsFeature()
        self.assertFalse(it.nextFeature(f))

        # with selection
        layer.selectByIds([f1.id(), f3.id(), f5.id()])
        source = QgsVectorLayerSelectedFeatureSource(layer)
        self.assertEqual(source.featureCount(), 3)
        ids = set([f.id() for f in source.getFeatures()])
        self.assertEqual(ids, {f1.id(), f3.id(), f5.id()})

        # test that requesting subset of ids intersects this request with the selected ids
        ids = set([f.id() for f in source.getFeatures(QgsFeatureRequest().setFilterFids([f1.id(), f2.id(), f5.id()]))])
        self.assertEqual(ids, {f1.id(), f5.id()})

        # test that requesting id works
        ids = set([f.id() for f in source.getFeatures(QgsFeatureRequest().setFilterFid(f1.id()))])
        self.assertEqual(ids, {f1.id()})
        ids = set([f.id() for f in source.getFeatures(QgsFeatureRequest().setFilterFid(f5.id()))])
        self.assertEqual(ids, {f5.id()})

        # test that source has stored snapshot of selected features
        layer.selectByIds([f2.id(), f4.id()])
        self.assertEqual(source.featureCount(), 3)
        ids = set([f.id() for f in source.getFeatures()])
        self.assertEqual(ids, {f1.id(), f3.id(), f5.id()})

        # test that source is not dependent on layer
        del layer
        ids = set([f.id() for f in source.getFeatures()])
        self.assertEqual(ids, {f1.id(), f3.id(), f5.id()})

    def testFeatureRequestWithReprojectionAndVirtualFields(self):
        layer = self.getSource()
        field = QgsField('virtual', QVariant.Double)
        layer.addExpressionField('$x', field)
        virtual_values = [f['virtual'] for f in layer.getFeatures()]
        self.assertAlmostEqual(virtual_values[0], -71.123, 2)
        self.assertEqual(virtual_values[1], NULL)
        self.assertAlmostEqual(virtual_values[2], -70.332, 2)
        self.assertAlmostEqual(virtual_values[3], -68.2, 2)
        self.assertAlmostEqual(virtual_values[4], -65.32, 2)

        # repeat, with reprojection on request
        request = QgsFeatureRequest().setDestinationCrs(QgsCoordinateReferenceSystem('epsg:3785'),
                                                        QgsProject.instance().transformContext())
        features = [f for f in layer.getFeatures(request)]
        # virtual field value should not change, even though geometry has
        self.assertAlmostEqual(features[0]['virtual'], -71.123, 2)
        self.assertAlmostEqual(features[0].geometry().constGet().x(), -7917376, -5)
        self.assertEqual(features[1]['virtual'], NULL)
        self.assertFalse(features[1].hasGeometry())
        self.assertAlmostEqual(features[2]['virtual'], -70.332, 2)
        self.assertAlmostEqual(features[2].geometry().constGet().x(), -7829322, -5)
        self.assertAlmostEqual(features[3]['virtual'], -68.2, 2)
        self.assertAlmostEqual(features[3].geometry().constGet().x(), -7591989, -5)
        self.assertAlmostEqual(features[4]['virtual'], -65.32, 2)
        self.assertAlmostEqual(features[4].geometry().constGet().x(), -7271389, -5)

    def testPrecision(self):
        layer = QgsVectorLayer("Polygon?crs=epsg:2056&field=pk:int", "vl", "memory")
        layer.geometryOptions().setGeometryPrecision(10)
        geom = QgsGeometry.fromWkt('Polygon ((2596411 1224654, 2596400 1224652, 2596405 1224640, 2596410 1224641, 2596411 1224654))')
        feature = QgsFeature(layer.fields())
        feature.setGeometry(geom)
        layer.startEditing()
        layer.addFeature(feature)
        self.assertGeometriesEqual(QgsGeometry.fromWkt('Polygon ((2596410 1224650, 2596400 1224650, 2596410 1224640, 2596410 1224650))'), feature.geometry(), 'geometry with unsnapped nodes', 'fixed geometry')
        layer.geometryOptions().setGeometryPrecision(0.0)
        feature.setGeometry(QgsGeometry.fromWkt('Polygon ((2596411 1224654, 2596400 1224652, 2596405 1224640, 2596410 1224641, 2596411 1224654))'))
        layer.addFeature(feature)
        self.assertGeometriesEqual(QgsGeometry.fromWkt('Polygon ((2596411 1224654, 2596400 1224652, 2596405 1224640, 2596410 1224641, 2596411 1224654))'), feature.geometry(), 'geometry with duplicates', 'unchanged geometry')

    def testRemoveDuplicateNodes(self):
        layer = QgsVectorLayer("Polygon?crs=epsg:2056&field=pk:int", "vl", "memory")
        layer.geometryOptions().setRemoveDuplicateNodes(True)
        geom = QgsGeometry.fromWkt('Polygon ((70 80, 80 90, 80 90, 60 50, 70 80))')
        feature = QgsFeature(layer.fields())
        feature.setGeometry(geom)
        layer.startEditing()
        layer.addFeature(feature)
        self.assertGeometriesEqual(feature.geometry(), QgsGeometry.fromWkt('Polygon ((70 80, 80 90, 60 50, 70 80))'), 'fixed geometry', 'geometry with duplicates')
        layer.geometryOptions().setRemoveDuplicateNodes(False)
        feature.setGeometry(QgsGeometry.fromWkt('Polygon ((70 80, 80 90, 80 90, 60 50, 70 80))'))
        layer.addFeature(feature)
        self.assertGeometriesEqual(feature.geometry(), QgsGeometry.fromWkt('Polygon ((70 80, 80 90, 80 90, 60 50, 70 80))'), 'unchanged geometry', 'geometry with duplicates')

    def testPrecisionAndDuplicateNodes(self):
        layer = QgsVectorLayer("Polygon?crs=epsg:2056&field=pk:int", "vl", "memory")
        layer.geometryOptions().setGeometryPrecision(10)
        layer.geometryOptions().setRemoveDuplicateNodes(True)
        geom = QgsGeometry.fromWkt('Polygon ((2596411 1224654, 2596400 1224652, 2596402 1224653, 2596405 1224640, 2596410 1224641, 2596411 1224654))')
        feature = QgsFeature(layer.fields())
        feature.setGeometry(geom)
        layer.startEditing()
        layer.addFeature(feature)
        self.assertGeometriesEqual(QgsGeometry.fromWkt('Polygon ((2596410 1224650, 2596400 1224650, 2596410 1224640, 2596410 1224650))'), feature.geometry(), 'geometry with unsnapped nodes', 'fixed geometry')

    def testDefaultDisplayExpression(self):
        """
        Test that default display expression gravitates to most interesting column names
        """
        layer = QgsVectorLayer("Polygon?crs=epsg:2056&field=pk:int", "vl", "memory")
        self.assertEqual(layer.displayExpression(), '"pk"')
        self.assertEqual(layer.displayField(), 'pk')
        layer = QgsVectorLayer("Polygon?crs=epsg:2056&field=pk:int&field=fid:int", "vl", "memory")
        self.assertEqual(layer.displayExpression(), '"fid"')
        self.assertEqual(layer.displayField(), 'fid')
        layer = QgsVectorLayer("Polygon?crs=epsg:2056&field=pk:int&field=DESCRIPTION:string&field=fid:int", "vl", "memory")
        self.assertEqual(layer.displayExpression(), '"DESCRIPTION"')
        self.assertEqual(layer.displayField(), 'DESCRIPTION')
        layer = QgsVectorLayer("Polygon?crs=epsg:2056&field=pk:int&field=DESCRIPTION:string&field=fid:int&field=NAME:string", "vl", "memory")
        self.assertEqual(layer.displayExpression(), '"NAME"')
        self.assertEqual(layer.displayField(), 'NAME')
        layer = QgsVectorLayer("Polygon?crs=epsg:2056&field=pk:int&field=DESCRIPTION:string&field=fid:int&field=BETTER_NAME:string&field=NAME:string", "vl", "memory")
        self.assertEqual(layer.displayExpression(), '"BETTER_NAME"')
        self.assertEqual(layer.displayField(), 'BETTER_NAME')


class TestQgsVectorLayerSourceAddedFeaturesInBuffer(unittest.TestCase, FeatureSourceTestCase):

    @classmethod
    def getSource(cls):
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

        # create a layer with features only in the added features buffer - not the provider
        vl.startEditing()
        vl.addFeatures([f1, f2, f3, f4, f5])
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer for FeatureSourceTestCase
        cls.source = cls.getSource()

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testOrderBy(self):
        """ Skip order by tests - edited features are not sorted in iterators.
        (Maybe they should be??)
        """
        pass

    def testMinimumValue(self):
        """ Skip min values test - due to inconsistencies in how null values are treated by providers.
        They are included here, but providers don't include them.... which is right?
        """
        pass


class TestQgsVectorLayerSourceChangedGeometriesInBuffer(unittest.TestCase, FeatureSourceTestCase):

    @classmethod
    def getSource(cls):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')
        assert (vl.isValid())

        f1 = QgsFeature()
        f1.setAttributes([5, -200, NULL, 'NuLl', '5'])

        f2 = QgsFeature()
        f2.setAttributes([3, 300, 'Pear', 'PEaR', '3'])
        f2.setGeometry(QgsGeometry.fromWkt('Point (-70.5 65.2)'))

        f3 = QgsFeature()
        f3.setAttributes([1, 100, 'Orange', 'oranGe', '1'])

        f4 = QgsFeature()
        f4.setAttributes([2, 200, 'Apple', 'Apple', '2'])

        f5 = QgsFeature()
        f5.setAttributes([4, 400, 'Honey', 'Honey', '4'])

        vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])

        ids = {f['pk']: f.id() for f in vl.getFeatures()}

        # modify geometries in buffer
        vl.startEditing()
        vl.changeGeometry(ids[5], QgsGeometry.fromWkt('Point (-71.123 78.23)'))
        vl.changeGeometry(ids[3], QgsGeometry())
        vl.changeGeometry(ids[1], QgsGeometry.fromWkt('Point (-70.332 66.33)'))
        vl.changeGeometry(ids[2], QgsGeometry.fromWkt('Point (-68.2 70.8)'))
        vl.changeGeometry(ids[4], QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer for FeatureSourceTestCase
        cls.source = cls.getSource()

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testOrderBy(self):
        """ Skip order by tests - edited features are not sorted in iterators.
        (Maybe they should be??)
        """
        pass


class TestQgsVectorLayerSourceChangedAttributesInBuffer(unittest.TestCase, FeatureSourceTestCase):

    @classmethod
    def getSource(cls):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')
        assert (vl.isValid())

        f1 = QgsFeature()
        f1.setAttributes([5, 200, 'a', 'b', 'c'])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f2 = QgsFeature()
        f2.setAttributes([3, -200, 'd', 'e', 'f'])

        f3 = QgsFeature()
        f3.setAttributes([1, -100, 'g', 'h', 'i'])
        f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f4 = QgsFeature()
        f4.setAttributes([2, -200, 'j', 'k', 'l'])
        f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f5 = QgsFeature()
        f5.setAttributes([4, 400, 'm', 'n', 'o'])
        f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])

        ids = {f['pk']: f.id() for f in vl.getFeatures()}

        # modify geometries in buffer
        vl.startEditing()
        vl.changeAttributeValue(ids[5], 1, -200)
        vl.changeAttributeValue(ids[5], 2, NULL)
        vl.changeAttributeValue(ids[5], 3, 'NuLl')
        vl.changeAttributeValue(ids[5], 4, '5')

        vl.changeAttributeValue(ids[3], 1, 300)
        vl.changeAttributeValue(ids[3], 2, 'Pear')
        vl.changeAttributeValue(ids[3], 3, 'PEaR')
        vl.changeAttributeValue(ids[3], 4, '3')

        vl.changeAttributeValue(ids[1], 1, 100)
        vl.changeAttributeValue(ids[1], 2, 'Orange')
        vl.changeAttributeValue(ids[1], 3, 'oranGe')
        vl.changeAttributeValue(ids[1], 4, '1')

        vl.changeAttributeValue(ids[2], 1, 200)
        vl.changeAttributeValue(ids[2], 2, 'Apple')
        vl.changeAttributeValue(ids[2], 3, 'Apple')
        vl.changeAttributeValue(ids[2], 4, '2')

        vl.changeAttributeValue(ids[4], 1, 400)
        vl.changeAttributeValue(ids[4], 2, 'Honey')
        vl.changeAttributeValue(ids[4], 3, 'Honey')
        vl.changeAttributeValue(ids[4], 4, '4')

        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer for FeatureSourceTestCase
        cls.source = cls.getSource()

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testOrderBy(self):
        """ Skip order by tests - edited features are not sorted in iterators.
        (Maybe they should be??)
        """
        pass

    def testUniqueValues(self):
        """ Skip unique values test - as noted in the docs this is unreliable when features are in the buffer
        """
        pass

    def testMinimumValue(self):
        """ Skip min values test - as noted in the docs this is unreliable when features are in the buffer
        """
        pass

    def testMaximumValue(self):
        """ Skip max values test - as noted in the docs this is unreliable when features are in the buffer
        """
        pass


class TestQgsVectorLayerSourceDeletedFeaturesInBuffer(unittest.TestCase, FeatureSourceTestCase):

    @classmethod
    def getSource(cls):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')
        assert (vl.isValid())

        # add a bunch of similar features to the provider
        b1 = QgsFeature()
        b1.setAttributes([5, -300, 'Apple', 'PEaR', '1'])
        b1.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        b2 = QgsFeature()
        b2.setAttributes([3, 100, 'Orange', 'NuLl', '2'])
        b2.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        b3 = QgsFeature()
        b3.setAttributes([1, -200, 'Honey', 'oranGe', '5'])

        b4 = QgsFeature()
        b4.setAttributes([2, 400, 'Pear', 'Honey', '3'])
        b4.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        b5 = QgsFeature()
        b5.setAttributes([4, 200, NULL, 'oranGe', '3'])
        b5.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        vl.dataProvider().addFeatures([b1, b2, b3, b4, b5])

        bad_ids = [f['pk'] for f in vl.getFeatures()]

        # here's our good features
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

        # delete the bad features, but don't commit
        vl.startEditing()
        vl.deleteFeatures(bad_ids)
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer for FeatureSourceTestCase
        cls.source = cls.getSource()

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testOrderBy(self):
        """ Skip order by tests - edited features are not sorted in iterators.
        (Maybe they should be??)
        """
        pass

    def testUniqueValues(self):
        """ Skip unique values test - as noted in the docs this is unreliable when features are in the buffer
        """
        pass

    def testMinimumValue(self):
        """ Skip min values test - as noted in the docs this is unreliable when features are in the buffer
        """
        pass

    def testMaximumValue(self):
        """ Skip max values test - as noted in the docs this is unreliable when features are in the buffer
        """
        pass


class TestQgsVectorLayerTransformContext(unittest.TestCase):

    def setUp(self):
        """Prepare tc"""
        super(TestQgsVectorLayerTransformContext, self).setUp()
        self.ctx = QgsCoordinateTransformContext()
        self.ctx.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857), 1234, 1235)

    def testTransformContextIsSetInCtor(self):
        """Test transform context can be set from ctor"""

        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')
        self.assertFalse(vl.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))

        options = QgsVectorLayer.LayerOptions(self.ctx)
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory', options)
        self.assertTrue(vl.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))

    def testTransformContextInheritsFromProject(self):
        """Test that when a layer is added to a project it inherits its context"""

        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')
        self.assertFalse(vl.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))

        p = QgsProject()
        self.assertFalse(p.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))
        p.setTransformContext(self.ctx)
        self.assertTrue(p.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))

        p.addMapLayers([vl])
        self.assertTrue(vl.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))

    def testTransformContextIsSyncedFromProject(self):
        """Test that when a layer is synced when project context changes"""

        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')
        self.assertFalse(vl.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))

        p = QgsProject()
        self.assertFalse(p.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))
        p.setTransformContext(self.ctx)
        self.assertTrue(p.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))

        p.addMapLayers([vl])
        self.assertTrue(vl.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))

        # Now change the project context
        tc2 = QgsCoordinateTransformContext()
        p.setTransformContext(tc2)
        self.assertFalse(p.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))
        self.assertFalse(vl.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))
        p.setTransformContext(self.ctx)
        self.assertTrue(p.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))
        self.assertTrue(vl.transformContext().hasTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857)))


# TODO:
# - fetch rect: feat with changed geometry: 1. in rect, 2. out of rect
# - more join tests
# - import


if __name__ == '__main__':
    unittest.main()
