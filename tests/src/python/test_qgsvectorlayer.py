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

import os
import qgis
from PyQt4.QtCore import QVariant, QObject, SIGNAL
from PyQt4.QtGui import QPainter

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
                       QgsSingleSymbolRendererV2)
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest,
                       #expectedFailure
                      )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

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
    f.setGeometry(QgsGeometry.fromPoint(QgsPoint(100,200)))
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
    f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(1,1)))
    f2 = QgsFeature()
    f2.setAttributes(["bar", 456, 654])
    f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(2,2)))
    assert pr.addFeatures([f1, f2])
    assert joinLayer.pendingFeatureCount() == 2
    return joinLayer

def dumpFeature(f):
    print "--- FEATURE DUMP ---"
    print "valid: %d   | id: %d" % (f.isValid(), f.id())
    geom = f.geometry()
    if geom:
        print "geometry wkb: %d" % geom.wkbType()
    else:
        print "no geometry"
    print "attrs: %s" % str(f.attributes())

def formatAttributes(attrs):
    return repr([ unicode(a) for a in attrs ])

def dumpEditBuffer(layer):
    editBuffer = layer.editBuffer()
    if not editBuffer:
        print "NO EDITING!"
        return
    print "ADDED:"
    for fid,f in editBuffer.addedFeatures().iteritems():
        print "%d: %s | %s" % (
            f.id(), formatAttributes(f.attributes()),
            f.geometry().exportToWkt())
    print "CHANGED GEOM:"
    for fid, geom in editBuffer.changedGeometries().iteritems():
        print "%d | %s" % (f.id(), f.geometry().exportToWkt())

class TestQgsVectorLayer(TestCase):

    def test_FeatureCount(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        myCount = myLayer.featureCount()
        myExpectedCount = 6
        myMessage = '\nExpected: %s\nGot: %s' % (myCount, myExpectedCount)
        assert myCount == myExpectedCount, myMessage

    # ADD FEATURE

    def test_AddFeature(self):
        layer = createEmptyLayer()
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(1,2)))

        def checkAfter():
            assert layer.pendingFeatureCount() == 1

            # check select+nextFeature
            f = layer.getFeatures().next()
            assert f.geometry().asPoint() == QgsPoint(1,2)

            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest(f.id())).next()
            assert f2.geometry().asPoint() == QgsPoint(1,2)

        def checkBefore():
            assert layer.pendingFeatureCount() == 0

            # check select+nextFeature
            self.assertRaises(StopIteration, layer.getFeatures().next)

        checkBefore()

        # try to add feature without editing mode
        assert not layer.addFeature(feat)

        # add feature
        layer.startEditing()
        assert layer.addFeature(feat)

        checkAfter()
        assert layer.dataProvider().featureCount() == 0

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        assert layer.commitChanges()

        checkAfter()
        assert layer.dataProvider().featureCount() == 1

    #DELETE FEATURE

    def test_DeleteFeature(self):
        layer = createLayerWithOnePoint()
        fid = 1

        def checkAfter():
            assert layer.pendingFeatureCount() == 0

            # check select+nextFeature
            self.assertRaises(StopIteration, layer.getFeatures().next)

            # check feature at id
            self.assertRaises(StopIteration,
                              layer.getFeatures(QgsFeatureRequest(fid)).next)

        def checkBefore():
            assert layer.pendingFeatureCount() == 1

            # check select+nextFeature
            fi = layer.getFeatures()
            f = fi.next()
            assert f.geometry().asPoint() == QgsPoint(100,200)
            self.assertRaises(StopIteration, fi.next)

            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest(fid)).next()

        checkBefore()

        # try to delete feature without editing mode
        assert not layer.deleteFeature(fid)

        # delete feature
        layer.startEditing()
        assert layer.deleteFeature(fid)

        checkAfter()

        # make sure calling it twice does not work
        assert not layer.deleteFeature(fid)

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        assert layer.dataProvider().featureCount() == 1

        assert layer.commitChanges()

        checkAfter()
        assert layer.dataProvider().featureCount() == 0

    def test_DeleteFeatureAfterAddFeature(self):

        layer = createEmptyLayer()
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(1,2)))

        def checkBefore():
            assert layer.pendingFeatureCount() == 0

            # check select+nextFeature
            self.assertRaises(StopIteration, layer.getFeatures().next)

        def checkAfter1():
            assert layer.pendingFeatureCount() == 1

        def checkAfter2():
            checkBefore() # should be the same state: no features

        checkBefore()

        # add feature
        layer.startEditing()
        assert layer.addFeature(feat)
        checkAfter1()
        fid = feat.id()
        assert layer.deleteFeature(fid)
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

        assert layer.commitChanges()
        checkAfter2()

        assert layer.dataProvider().featureCount() == 0

    #CHANGE ATTRIBUTE

    def test_ChangeAttribute(self):
        layer = createLayerWithOnePoint()
        fid = 1
        f = QgsFeature()

        def checkAfter():
            # check select+nextFeature
            fi = layer.getFeatures()
            f = fi.next()
            assert f[0] == "good"

            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert f2[0] == "good"

        def checkBefore():
            # check select+nextFeature
            f = layer.getFeatures().next()
            assert f[0] == "test"

        checkBefore()

        # try to change attribute without editing mode
        assert not layer.changeAttributeValue(fid, 0, "good")

        # change attribute
        layer.startEditing()
        assert layer.changeAttributeValue(fid, 0, "good")

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        assert layer.commitChanges()
        checkAfter()


    def test_ChangeAttributeAfterAddFeature(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().deleteFeatures([1]) # no need for this feature

        newF = QgsFeature()
        newF.setGeometry( QgsGeometry.fromPoint(QgsPoint(1,1)) )
        newF.setAttributes( ["hello", 42] )

        def checkAfter():
            assert len(layer.pendingFields()) == 2
            # check feature
            fi = layer.getFeatures()
            f = fi.next()
            attrs = f.attributes()
            assert len(attrs) == 2
            assert attrs[0] == "hello"
            assert attrs[1] == 12

            self.assertRaises(StopIteration, fi.next)

            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert f2[0] == "hello"
            assert f2[1] == 12

        def checkBefore():
            # check feature
            self.assertRaises(StopIteration, layer.getFeatures().next)

        checkBefore()

        layer.startEditing()
        layer.beginEditCommand("AddFeature + ChangeAttribute")
        assert layer.addFeature(newF)
        assert layer.changeAttributeValue(newF.id(), 1, 12)
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        assert layer.commitChanges()
        checkAfter()

        #print "COMMIT ERRORS:"
        #for item in list(layer.commitErrors()): print item

    # CHANGE GEOMETRY

    def test_ChangeGeometry(self):
        layer = createLayerWithOnePoint()
        fid = 1

        def checkAfter():
            # check select+nextFeature
            f = layer.getFeatures().next()
            assert f.geometry().asPoint() == QgsPoint(300,400)
            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert f2.geometry().asPoint() == QgsPoint(300,400)

        def checkBefore():
            # check select+nextFeature
            f = layer.getFeatures().next()
            assert f.geometry().asPoint() == QgsPoint(100,200)

        # try to change geometry without editing mode
        assert not layer.changeGeometry(fid, QgsGeometry.fromPoint(QgsPoint(300,400)))

        checkBefore()

        # change geometry
        layer.startEditing()
        layer.beginEditCommand("ChangeGeometry")
        assert layer.changeGeometry(fid, QgsGeometry.fromPoint(QgsPoint(300,400)))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        assert layer.commitChanges()
        checkAfter()

    def test_ChangeGeometryAfterChangeAttribute(self):
        layer = createLayerWithOnePoint()
        fid = 1

        def checkAfter():
            # check select+nextFeature
            f = layer.getFeatures().next()
            assert f.geometry().asPoint() == QgsPoint(300,400)
            assert f[0] == "changed"
            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert f2.geometry().asPoint() == QgsPoint(300,400)
            assert f2[0] == "changed"

        def checkBefore():
            # check select+nextFeature
            f = layer.getFeatures().next()
            assert f.geometry().asPoint() == QgsPoint(100,200)
            assert f[0] == "test"

        checkBefore()

        # change geometry
        layer.startEditing()
        layer.beginEditCommand("ChangeGeometry + ChangeAttribute")
        assert layer.changeAttributeValue(fid, 0, "changed")
        assert layer.changeGeometry(fid, QgsGeometry.fromPoint(QgsPoint(300,400)))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        assert layer.commitChanges()
        checkAfter()


    def test_ChangeGeometryAfterAddFeature(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().deleteFeatures([1]) # no need for this feature

        newF = QgsFeature()
        newF.setGeometry( QgsGeometry.fromPoint(QgsPoint(1,1)) )
        newF.setAttributes(["hello", 42])

        def checkAfter():
            assert len(layer.pendingFields()) == 2
            # check feature
            f = layer.getFeatures().next()
            assert f.geometry().asPoint() == QgsPoint(2,2)
            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert f2.geometry().asPoint() == QgsPoint(2,2)

        def checkBefore():
            # check feature
            self.assertRaises(StopIteration, layer.getFeatures().next)

        checkBefore()

        layer.startEditing()
        layer.beginEditCommand("AddFeature+ChangeGeometry")
        assert layer.addFeature(newF)
        assert layer.changeGeometry(newF.id(), QgsGeometry.fromPoint(QgsPoint(2,2)))
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        assert layer.commitChanges()
        checkAfter()

        #print "COMMIT ERRORS:"
        #for item in list(layer.commitErrors()): print item

    # ADD ATTRIBUTE

    def test_AddAttribute(self):
        layer = createLayerWithOnePoint()
        fld1 = QgsField("fld1", QVariant.Int, "integer")
        #fld2 = QgsField("fld2", QVariant.Int, "integer")

        def checkBefore():
            # check fields
            flds = layer.pendingFields()
            assert len(flds) == 2
            assert flds[0].name() == "fldtxt"
            assert flds[1].name() == "fldint"

            # check feature
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 2
            assert attrs[0] == "test"
            assert attrs[1] == 123

        def checkAfter():
            # check fields
            flds = layer.pendingFields()
            assert len(flds) == 3
            assert flds[0].name() == "fldtxt"
            assert flds[1].name() == "fldint"
            assert flds[2].name() == "fld1"

            # check feature
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 3
            assert attrs[0] == "test"
            assert attrs[1] == 123
            assert attrs[2] is None

            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert f2[0] == "test"
            assert f2[1] == 123
            assert f2[2] is None

        #for nt in layer.dataProvider().nativeTypes():
        #    print (nt.mTypeDesc, nt.mTypeName, nt.mType, nt.mMinLen,
        #          nt.mMaxLen, nt.mMinPrec, nt.mMaxPrec)
        assert layer.dataProvider().supportedType(fld1)

        # without editing mode
        assert not layer.addAttribute(fld1)

        layer.startEditing()

        checkBefore()

        assert layer.addAttribute(fld1)
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
        layer.dataProvider().deleteFeatures([1]) # no need for this feature

        newF = QgsFeature()
        newF.setGeometry( QgsGeometry.fromPoint(QgsPoint(1,1)) )
        newF.setAttributes(["hello", 42])

        fld1 = QgsField("fld1", QVariant.Int, "integer")

        def checkBefore():
            assert len(layer.pendingFields()) == 2
            # check feature
            self.assertRaises(StopIteration, layer.getFeatures().next)

        def checkAfter():
            assert len(layer.pendingFields()) == 3
            # check feature
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 3
            assert attrs[0] == "hello"
            assert attrs[1] == 42
            assert attrs[2] is None
            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert f2[0] == "hello"
            assert f2[1] == 42
            assert f2[2] is None

        layer.startEditing()

        checkBefore()

        layer.beginEditCommand("AddFeature + AddAttribute")
        assert layer.addFeature(newF)
        assert layer.addAttribute(fld1)
        layer.endEditCommand()

        checkAfter()

        # now try undo/redo
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfter()

        layer.commitChanges()
        checkAfter()

        #print "COMMIT ERRORS:"
        #for item in list(layer.commitErrors()): print item

    def test_AddAttributeAfterChangeValue(self):
        pass  # not interesting to test...?

    def test_AddAttributeAfterDeleteAttribute(self):
        pass # maybe it would be good to test

    # DELETE ATTRIBUTE

    def test_DeleteAttribute(self):
        layer = createLayerWithOnePoint()
        layer.dataProvider().addAttributes(
            [QgsField("flddouble", QVariant.Double, "double")] )
        layer.dataProvider().changeAttributeValues(
            {1:{2: 5.5}})

        # without editing mode
        assert not layer.deleteAttribute(0)

        def checkBefore():
            flds = layer.pendingFields()
            assert len(flds) == 3
            assert flds[0].name() == "fldtxt"
            assert flds[1].name() == "fldint"
            assert flds[2].name() == "flddouble"

            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 3
            assert attrs[0] == "test"
            assert attrs[1] == 123
            assert attrs[2] == 5.5

        layer.startEditing()

        checkBefore()

        assert layer.deleteAttribute(0)

        def checkAfterOneDelete():
            flds = layer.pendingFields()
            #for fld in flds: print "FLD", fld.name()
            assert len(flds) == 2
            assert flds[0].name() == "fldint"
            assert flds[1].name() == "flddouble"
            assert layer.pendingAllAttributesList() == [0,1]

            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 2
            assert attrs[0] == 123
            assert attrs[1] == 5.5

        checkAfterOneDelete()

        # delete last attribute
        assert layer.deleteAttribute(0)

        def checkAfterTwoDeletes():
            assert layer.pendingAllAttributesList() == [0]
            flds = layer.pendingFields()
            #for fld in flds: print "FLD", fld.name()
            assert len(flds) == 1
            assert flds[0].name() == "flddouble"

            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 1
            assert attrs[0] == 5.5
            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert len(f2.attributes()) == 1
            assert f2[0] == 5.5

        checkAfterTwoDeletes()
        layer.undoStack().undo()
        checkAfterOneDelete()
        layer.undoStack().undo()
        checkBefore()
        layer.undoStack().redo()
        checkAfterOneDelete()
        layer.undoStack().redo()
        checkAfterTwoDeletes()

        assert layer.commitChanges()  # COMMIT!
        checkAfterTwoDeletes()

    def test_DeleteAttributeAfterAddAttribute(self):
        layer = createLayerWithOnePoint()
        fld1 = QgsField("fld1", QVariant.Int, "integer")

        def checkAfter():  # layer should be unchanged
            flds = layer.pendingFields()
            assert len(flds) == 2
            assert flds[0].name() == "fldtxt"
            assert flds[1].name() == "fldint"

            # check feature
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 2
            assert attrs[0] == "test"
            assert attrs[1] == 123
            # check feature at id
            f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
            assert len(f2.attributes()) == 2
            assert f2[0] == "test"
            assert f2[1] == 123

        checkAfter()

        layer.startEditing()

        layer.beginEditCommand("AddAttribute + DeleteAttribute")
        assert layer.addAttribute(fld1)
        assert layer.deleteAttribute(2)
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
        layer.dataProvider().deleteFeatures([1]) # no need for this feature

        newF = QgsFeature()
        newF.setGeometry( QgsGeometry.fromPoint(QgsPoint(1,1)) )
        newF.setAttributes(["hello", 42])

        def checkBefore():
            assert len(layer.pendingFields()) == 2
            # check feature
            self.assertRaises(StopIteration, layer.getFeatures().next)

        def checkAfter1():
            assert len(layer.pendingFields()) == 2
            # check feature
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 2
            assert attrs[0] == "hello"
            assert attrs[1] == 42

        def checkAfter2():
            assert len(layer.pendingFields()) == 1
            # check feature
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 1
            assert attrs[0] == 42

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
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 2
            assert attrs[0] == "test"
            assert attrs[1] == 123

        def checkAfter1():
            # check feature
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 2
            assert attrs[0] == "changed"
            assert attrs[1] == 123

        def checkAfter2():
            # check feature
            f = layer.getFeatures().next()
            attrs = f.attributes()
            assert len(attrs) == 1
            assert attrs[0] == 123

        layer.startEditing()

        checkBefore()

        assert layer.changeAttributeValue(1, 0, "changed")
        checkAfter1()
        assert layer.deleteAttribute(0)
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
        assert flds.indexFromName("fldint") == 1
        assert flds.indexFromName("fldXXX") == -1

    def test_getFeatures(self):

        layer = createLayerWithOnePoint()

        f = QgsFeature()
        fi = layer.getFeatures()
        assert fi.nextFeature(f) == True
        assert f.isValid() == True
        assert f.id() == 1
        assert f.geometry().asPoint() == QgsPoint(100,200)
        assert f["fldtxt"] == "test"
        assert f["fldint"] == 123

        assert fi.nextFeature(f) == False

    def test_join(self):

        joinLayer = createJoinLayer()
        joinLayer2 = createJoinLayer()
        QgsMapLayerRegistry.instance().addMapLayers([joinLayer,joinLayer2])

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
        assert len(flds) == 6
        assert flds[2].name() == "joinlayer_x"
        assert flds[3].name() == "joinlayer_z"
        assert flds[4].name() == "custom-prefix_x"
        assert flds[5].name() == "custom-prefix_z"
        assert flds.fieldOrigin(0) == QgsFields.OriginProvider
        assert flds.fieldOrigin(2) == QgsFields.OriginJoin
        assert flds.fieldOrigin(3) == QgsFields.OriginJoin
        assert flds.fieldOriginIndex(0) == 0
        assert flds.fieldOriginIndex(2) == 0
        assert flds.fieldOriginIndex(3) == 2

        f = QgsFeature()
        fi = layer.getFeatures()
        assert fi.nextFeature(f) == True
        attrs = f.attributes()
        assert len(attrs) == 6
        assert attrs[0] == "test"
        assert attrs[1] == 123
        assert attrs[2] == "foo"
        assert attrs[3] == 321
        assert fi.nextFeature(f) == False

        f2 = layer.getFeatures(QgsFeatureRequest( f.id() )).next()
        assert len(f2.attributes()) == 6
        assert f2[2] == "foo"
        assert f2[3] == 321

    def test_InvalidOperations(self):
        layer = createLayerWithOnePoint()

        layer.startEditing()

        # ADD FEATURE

        newF1 = QgsFeature()
        assert not layer.addFeature(newF1)  # need attributes like the layer has

        # DELETE FEATURE

        assert not layer.deleteFeature(-333)
        # we do not check for existence of the feature id if it's
        # not newly added feature
        #assert not layer.deleteFeature(333)

        # CHANGE GEOMETRY

        assert not layer.changeGeometry(
            -333, QgsGeometry.fromPoint(QgsPoint(1, 1)))

        # CHANGE VALUE

        assert not layer.changeAttributeValue(-333, 0, 1)
        assert not layer.changeAttributeValue(1, -1, 1)

        # ADD ATTRIBUTE

        assert not layer.addAttribute(QgsField())

        # DELETE ATTRIBUTE

        assert not layer.deleteAttribute(-1)

    def onBlendModeChanged( self, mode ):
        self.blendModeTest = mode

    def test_setBlendMode( self ):
        layer = createLayerWithOnePoint()

        self.blendModeTest = 0
        QObject.connect( layer, SIGNAL( "blendModeChanged( const QPainter::CompositionMode )" ),
                         self.onBlendModeChanged )
        layer.setBlendMode( QPainter.CompositionMode_Screen )

        assert self.blendModeTest == QPainter.CompositionMode_Screen
        assert layer.blendMode() == QPainter.CompositionMode_Screen

    def test_setFeatureBlendMode( self ):
        layer = createLayerWithOnePoint()

        self.blendModeTest = 0
        QObject.connect( layer, SIGNAL( "featureBlendModeChanged( const QPainter::CompositionMode )" ),
                         self.onBlendModeChanged )
        layer.setFeatureBlendMode( QPainter.CompositionMode_Screen )

        assert self.blendModeTest == QPainter.CompositionMode_Screen
        assert layer.featureBlendMode() == QPainter.CompositionMode_Screen

    def onLayerTransparencyChanged( self, tr ):
        self.transparencyTest = tr

    def test_setLayerTransparency( self ):
        layer = createLayerWithOnePoint()

        self.transparencyTest = 0
        QObject.connect( layer, SIGNAL( "layerTransparencyChanged( int )" ),
                         self.onLayerTransparencyChanged )
        layer.setLayerTransparency( 50 )
        assert self.transparencyTest == 50
        assert layer.layerTransparency() == 50

    def onRendererChanged( self ):
        self.rendererChanged = True
    def test_setRendererV2( self ):
        layer = createLayerWithOnePoint()

        self.rendererChanged = False
        QObject.connect( layer, SIGNAL( "rendererChanged()" ),
                         self.onRendererChanged )

        r = QgsSingleSymbolRendererV2( QgsSymbolV2.defaultSymbol( QGis.Point ) )
        layer.setRendererV2( r )
        assert self.rendererChanged == True
        assert layer.rendererV2() == r

# TODO:
# - fetch rect: feat with changed geometry: 1. in rect, 2. out of rect
# - more join tests
# - import

if __name__ == '__main__':
    unittest.main()
