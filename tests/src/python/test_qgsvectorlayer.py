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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QPainter

from qgis.core import (QGis,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsPoint,
                       QgsField,
                       QgsFields,
                       QgsMapLayerRegistry,
                       QgsVectorJoinInfo,
                       QgsSymbolV2,
                       QgsSingleSymbolRendererV2,
                       QgsCoordinateReferenceSystem,
                       QgsProject,
                       QgsUnitTypes)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
start_app()


def createEmptyLayer():
    layer = QgsVectorLayer("Point", "addfeat", "memory")
    assert layer.pendingFeatureCount() == 0
    return layer


def createLayerWithOnePoint():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPoint(QgsPoint(100, 200)))
    assert pr.addFeatures([f])
    assert layer.pendingFeatureCount() == 1
    return layer


def createJoinLayer():
    joinLayer = QgsVectorLayer(
        "Point?field=x:string&field=y:integer&field=z:integer",
        "joinlayer", "memory")
    pr = joinLayer.dataProvider()
    f1 = QgsFeature()
    f1.setAttributes(["foo", 123, 321])
    f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 1)))
    f2 = QgsFeature()
    f2.setAttributes(["bar", 456, 654])
    f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(2, 2)))
    assert pr.addFeatures([f1, f2])
    assert joinLayer.pendingFeatureCount() == 2
    return joinLayer


def dumpFeature(f):
    print("--- FEATURE DUMP ---")
    print("valid: %d   | id: %d" % (f.isValid(), f.id()))
    geom = f.geometry()
    if geom:
        print("geometry wkb: %d" % geom.wkbType())
    else:
        print("no geometry")
    print("attrs: %s" % str(f.attributes()))


def formatAttributes(attrs):
    return repr([unicode(a) for a in attrs])


def dumpEditBuffer(layer):
    editBuffer = layer.editBuffer()
    if not editBuffer:
        print("NO EDITING!")
        return
    print("ADDED:")
    for fid, f in editBuffer.addedFeatures().iteritems():
        print("%d: %s | %s" % (
            f.id(), formatAttributes(f.attributes()),
            f.geometry().exportToWkt()))
    print("CHANGED GEOM:")
    for fid, geom in editBuffer.changedGeometries().iteritems():
        print("%d | %s" % (f.id(), f.geometry().exportToWkt()))


class TestQgsVectorLayer(unittest.TestCase):

    def test_FeatureCount(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        myCount = myLayer.featureCount()
        self.assertEqual(myCount, 6)

    # ADD FEATURE

    def test_AddFeature(self):
        layer = createEmptyLayer()
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 2)))

        def checkAfter():
            self.assertEqual(layer.pendingFeatureCount(), 1)

            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPoint(1, 2))

            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2.geometry().asPoint(), QgsPoint(1, 2))

        def checkBefore():
            self.assertEqual(layer.pendingFeatureCount(), 0)

            # check select+nextFeature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        checkBefore()

        # try to add feature without editing mode
        self.assertFalse(layer.addFeature(feat))

        # add feature
        layer.startEditing()
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

    # DELETE FEATURE

    def test_DeleteFeature(self):
        layer = createLayerWithOnePoint()
        fid = 1

        def checkAfter():
            self.assertEqual(layer.pendingFeatureCount(), 0)

            # check select+nextFeature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

            # check feature at id
            with self.assertRaises(StopIteration):
                next(layer.getFeatures(QgsFeatureRequest(fid)))

        def checkBefore():
            self.assertEqual(layer.pendingFeatureCount(), 1)

            # check select+nextFeature
            fi = layer.getFeatures()
            f = next(fi)
            self.assertEqual(f.geometry().asPoint(), QgsPoint(100, 200))
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
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 2)))

        def checkBefore():
            self.assertEqual(layer.pendingFeatureCount(), 0)

            # check select+nextFeature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        def checkAfter1():
            self.assertEqual(layer.pendingFeatureCount(), 1)

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
        newF.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 1)))
        newF.setAttributes(["hello", 42])

        def checkAfter():
            self.assertEqual(len(layer.pendingFields()), 2)
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
            self.assertEqual(f.geometry().asPoint(), QgsPoint(300, 400))
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2.geometry().asPoint(), QgsPoint(300, 400))

        def checkBefore():
            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPoint(100, 200))

        # try to change geometry without editing mode
        self.assertFalse(layer.changeGeometry(fid, QgsGeometry.fromPoint(QgsPoint(300, 400))))

        checkBefore()

        # change geometry
        layer.startEditing()
        layer.beginEditCommand("ChangeGeometry")
        self.assertTrue(layer.changeGeometry(fid, QgsGeometry.fromPoint(QgsPoint(300, 400))))
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
            self.assertEqual(f.geometry().asPoint(), QgsPoint(300, 400))
            self.assertEqual(f[0], "changed")
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2.geometry().asPoint(), QgsPoint(300, 400))
            self.assertEqual(f2[0], "changed")

        def checkBefore():
            # check select+nextFeature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPoint(100, 200))
            self.assertEqual(f[0], "test")

        checkBefore()

        # change geometry
        layer.startEditing()
        layer.beginEditCommand("ChangeGeometry + ChangeAttribute")
        self.assertTrue(layer.changeAttributeValue(fid, 0, "changed"))
        self.assertTrue(layer.changeGeometry(fid, QgsGeometry.fromPoint(QgsPoint(300, 400))))
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
        newF.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 1)))
        newF.setAttributes(["hello", 42])

        def checkAfter():
            self.assertEqual(len(layer.pendingFields()), 2)
            # check feature
            f = next(layer.getFeatures())
            self.assertEqual(f.geometry().asPoint(), QgsPoint(2, 2))
            # check feature at id
            f2 = next(layer.getFeatures(QgsFeatureRequest(f.id())))
            self.assertEqual(f2.geometry().asPoint(), QgsPoint(2, 2))

        def checkBefore():
            # check feature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        checkBefore()

        layer.startEditing()
        layer.beginEditCommand("AddFeature+ChangeGeometry")
        self.assertTrue(layer.addFeature(newF))
        self.assertTrue(layer.changeGeometry(newF.id(), QgsGeometry.fromPoint(QgsPoint(2, 2))))
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

    # ADD ATTRIBUTE

    def test_AddAttribute(self):
        layer = createLayerWithOnePoint()
        fld1 = QgsField("fld1", QVariant.Int, "integer")
        #fld2 = QgsField("fld2", QVariant.Int, "integer")

        def checkBefore():
            # check fields
            flds = layer.pendingFields()
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
            flds = layer.pendingFields()
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
        newF.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 1)))
        newF.setAttributes(["hello", 42])

        fld1 = QgsField("fld1", QVariant.Int, "integer")

        def checkBefore():
            self.assertEqual(len(layer.pendingFields()), 2)
            # check feature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        def checkAfter():
            self.assertEqual(len(layer.pendingFields()), 3)
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
            flds = layer.pendingFields()
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
            flds = layer.pendingFields()
            # for fld in flds: print "FLD", fld.name()
            self.assertEqual(len(flds), 2)
            self.assertEqual(flds[0].name(), "fldint")
            self.assertEqual(flds[1].name(), "flddouble")
            self.assertEqual(layer.pendingAllAttributesList(), [0, 1])

            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], 123)
            self.assertEqual(attrs[1], 5.5)

        checkAfterOneDelete()

        # delete last attribute
        self.assertTrue(layer.deleteAttribute(0))

        def checkAfterTwoDeletes():
            self.assertEqual(layer.pendingAllAttributesList(), [0])
            flds = layer.pendingFields()
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
            flds = layer.pendingFields()
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
        newF.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 1)))
        newF.setAttributes(["hello", 42])

        def checkBefore():
            self.assertEqual(len(layer.pendingFields()), 2)
            # check feature
            with self.assertRaises(StopIteration):
                next(layer.getFeatures())

        def checkAfter1():
            self.assertEqual(len(layer.pendingFields()), 2)
            # check feature
            f = next(layer.getFeatures())
            attrs = f.attributes()
            self.assertEqual(len(attrs), 2)
            self.assertEqual(attrs[0], "hello")
            self.assertEqual(attrs[1], 42)

        def checkAfter2():
            self.assertEqual(len(layer.pendingFields()), 1)
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

    def test_fields(self):
        layer = createLayerWithOnePoint()

        flds = layer.pendingFields()
        self.assertEqual(flds.indexFromName("fldint"), 1)
        self.assertEqual(flds.indexFromName("fldXXX"), -1)

    def test_getFeatures(self):

        layer = createLayerWithOnePoint()

        f = QgsFeature()
        fi = layer.getFeatures()
        self.assertTrue(fi.nextFeature(f))
        self.assertTrue(f.isValid())
        self.assertEqual(f.id(), 1)
        self.assertEqual(f.geometry().asPoint(), QgsPoint(100, 200))
        self.assertEqual(f["fldtxt"], "test")
        self.assertEqual(f["fldint"], 123)

        self.assertFalse(fi.nextFeature(f))

    def test_join(self):

        joinLayer = createJoinLayer()
        joinLayer2 = createJoinLayer()
        QgsMapLayerRegistry.instance().addMapLayers([joinLayer, joinLayer2])

        layer = createLayerWithOnePoint()

        join = QgsVectorJoinInfo()
        join.targetFieldName = "fldint"
        join.joinLayerId = joinLayer.id()
        join.joinFieldName = "y"
        join.memoryCache = True

        layer.addJoin(join)

        join2 = QgsVectorJoinInfo()
        join2.targetFieldName = "fldint"
        join2.joinLayerId = joinLayer2.id()
        join2.joinFieldName = "y"
        join2.memoryCache = True
        join2.prefix = "custom-prefix_"

        layer.addJoin(join2)

        flds = layer.pendingFields()
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
        #self.assertFalse(layer.deleteFeature(333))

        # CHANGE GEOMETRY

        self.assertFalse(layer.changeGeometry(
            -333, QgsGeometry.fromPoint(QgsPoint(1, 1))))

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

        cnt = layer.pendingFields().count()

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

        self.assertEqual(layer.pendingFields().count(), cnt)

    def test_ExpressionFieldEllipsoidLengthCalculation(self):
        #create a temporary layer
        temp_layer = QgsVectorLayer("LineString?crs=epsg:3111&field=pk:int", "vl", "memory")
        self.assertTrue(temp_layer.isValid())
        f1 = QgsFeature(temp_layer.dataProvider().fields(), 1)
        f1.setAttribute("pk", 1)
        f1.setGeometry(QgsGeometry.fromPolyline([QgsPoint(2484588, 2425722), QgsPoint(2482767, 2398853)]))
        temp_layer.dataProvider().addFeatures([f1])

        # set project CRS and ellipsoid
        srs = QgsCoordinateReferenceSystem(3111, QgsCoordinateReferenceSystem.EpsgCrsId)
        QgsProject.instance().writeEntry("SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4())
        QgsProject.instance().writeEntry("SpatialRefSys", "/ProjectCRSID", srs.srsid())
        QgsProject.instance().writeEntry("SpatialRefSys", "/ProjectCrs", srs.authid())
        QgsProject.instance().writeEntry("Measure", "/Ellipsoid", "WGS84")
        QgsProject.instance().writeEntry("Measurement", "/DistanceUnits", QgsUnitTypes.encodeUnit(QGis.Meters))

        idx = temp_layer.addExpressionField('$length', QgsField('length', QVariant.Double))  # NOQA

        # check value
        f = next(temp_layer.getFeatures())
        expected = 26932.156
        self.assertAlmostEqual(f['length'], expected, 3)

        # change project length unit, check calculation respects unit
        QgsProject.instance().writeEntry("Measurement", "/DistanceUnits", QgsUnitTypes.encodeUnit(QGis.Feet))
        f = next(temp_layer.getFeatures())
        expected = 88360.0918635
        self.assertAlmostEqual(f['length'], expected, 3)

    def test_ExpressionFieldEllipsoidAreaCalculation(self):
        #create a temporary layer
        temp_layer = QgsVectorLayer("Polygon?crs=epsg:3111&field=pk:int", "vl", "memory")
        self.assertTrue(temp_layer.isValid())
        f1 = QgsFeature(temp_layer.dataProvider().fields(), 1)
        f1.setAttribute("pk", 1)
        f1.setGeometry(QgsGeometry.fromPolygon([[QgsPoint(2484588, 2425722), QgsPoint(2482767, 2398853), QgsPoint(2520109, 2397715), QgsPoint(2520792, 2425494), QgsPoint(2484588, 2425722)]]))
        temp_layer.dataProvider().addFeatures([f1])

        # set project CRS and ellipsoid
        srs = QgsCoordinateReferenceSystem(3111, QgsCoordinateReferenceSystem.EpsgCrsId)
        QgsProject.instance().writeEntry("SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4())
        QgsProject.instance().writeEntry("SpatialRefSys", "/ProjectCRSID", srs.srsid())
        QgsProject.instance().writeEntry("SpatialRefSys", "/ProjectCrs", srs.authid())
        QgsProject.instance().writeEntry("Measure", "/Ellipsoid", "WGS84")
        QgsProject.instance().writeEntry("Measurement", "/AreaUnits", QgsUnitTypes.encodeUnit(QgsUnitTypes.SquareMeters))

        idx = temp_layer.addExpressionField('$area', QgsField('area', QVariant.Double))  # NOQA

        # check value
        f = next(temp_layer.getFeatures())
        expected = 1009089817.0
        self.assertAlmostEqual(f['area'], expected, delta=1.0)

        # change project area unit, check calculation respects unit
        QgsProject.instance().writeEntry("Measurement", "/AreaUnits", QgsUnitTypes.encodeUnit(QgsUnitTypes.SquareMiles))
        f = next(temp_layer.getFeatures())
        expected = 389.6117565069
        self.assertAlmostEqual(f['area'], expected, 3)

    def test_ExpressionFilter(self):
        layer = createLayerWithOnePoint()

        idx = layer.addExpressionField('5', QgsField('test', QVariant.LongLong))  # NOQA

        features = layer.getFeatures(QgsFeatureRequest().setFilterExpression('"test" = 6'))

        assert(len(list(features)) == 0)

        features = layer.getFeatures(QgsFeatureRequest().setFilterExpression('"test" = 5'))

        assert(len(list(features)) == 1)

    def onLayerTransparencyChanged(self, tr):
        self.transparencyTest = tr

    def test_setLayerTransparency(self):
        layer = createLayerWithOnePoint()

        self.transparencyTest = 0
        layer.layerTransparencyChanged.connect(self.onLayerTransparencyChanged)
        layer.setLayerTransparency(50)
        self.assertEqual(self.transparencyTest, 50)
        self.assertEqual(layer.layerTransparency(), 50)

    def onRendererChanged(self):
        self.rendererChanged = True

    def test_setRendererV2(self):
        layer = createLayerWithOnePoint()

        self.rendererChanged = False
        layer.rendererChanged.connect(self.onRendererChanged)

        r = QgsSingleSymbolRendererV2(QgsSymbolV2.defaultSymbol(QGis.Point))
        layer.setRendererV2(r)
        self.assertTrue(self.rendererChanged)
        self.assertEqual(layer.rendererV2(), r)

# TODO:
# - fetch rect: feat with changed geometry: 1. in rect, 2. out of rect
# - more join tests
# - import

if __name__ == '__main__':
    unittest.main()
