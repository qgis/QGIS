# -*- coding: utf-8 -*-
"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '28/11/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA

import os

from qgis.core import (
    QgsFeature,
    QgsVectorLayer,
    QgsProject,
    QgsRelation,
    QgsTransaction,
    QgsFeatureRequest,
    QgsVectorLayerTools,
    QgsGeometry
)

from qgis.gui import (
    QgsGui,
    QgsRelationWidgetWrapper,
    QgsAttributeEditorContext,
    QgsMapCanvas,
    QgsAdvancedDigitizingDockWidget
)

from qgis.PyQt.QtCore import QTimer
from qgis.PyQt.QtWidgets import (
    QToolButton,
    QMessageBox,
    QDialogButtonBox,
    QTableView,
    QDialog
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsRelationEditWidget(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """
        Setup the involved layers and relations for a n:m relation
        :return:
        """
        cls.mapCanvas = QgsMapCanvas()
        QgsGui.editorWidgetRegistry().initEditors(cls.mapCanvas)
        cls.dbconn = 'service=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layer
        cls.vl_books = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."books" sql=', 'books', 'postgres')
        cls.vl_authors = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."authors" sql=', 'authors', 'postgres')
        cls.vl_editors = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'fk_book,fk_author\' table="qgis_test"."editors" sql=', 'editors', 'postgres')
        cls.vl_link_books_authors = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."books_authors" sql=', 'books_authors', 'postgres')

        QgsProject.instance().addMapLayer(cls.vl_books)
        QgsProject.instance().addMapLayer(cls.vl_authors)
        QgsProject.instance().addMapLayer(cls.vl_editors)
        QgsProject.instance().addMapLayer(cls.vl_link_books_authors)

        cls.relMgr = QgsProject.instance().relationManager()

        # Our mock QgsVectorLayerTools, that allow injecting data where user input is expected
        cls.vltools = VlTools()
        cls.layers = {cls.vl_authors, cls.vl_books, cls.vl_link_books_authors}

        assert(cls.vl_authors.isValid())
        assert(cls.vl_books.isValid())
        assert(cls.vl_editors.isValid())
        assert(cls.vl_link_books_authors.isValid())

    @classmethod
    def tearDownClass(cls):
        QgsProject.instance().removeAllMapLayers()
        cls.vl_books = None
        cls.vl_authors = None
        cls.vl_editors = None
        cls.vl_link_books_authors = None
        cls.layers = None
        cls.mapCanvas = None
        cls.vltools = None
        cls.relMgr = None

    def setUp(self):
        self.rel_a = QgsRelation()
        self.rel_a.setReferencingLayer(self.vl_link_books_authors.id())
        self.rel_a.setReferencedLayer(self.vl_authors.id())
        self.rel_a.addFieldPair('fk_author', 'pk')
        self.rel_a.setId('rel_a')
        assert(self.rel_a.isValid())
        self.relMgr.addRelation(self.rel_a)

        self.rel_b = QgsRelation()
        self.rel_b.setReferencingLayer(self.vl_link_books_authors.id())
        self.rel_b.setReferencedLayer(self.vl_books.id())
        self.rel_b.addFieldPair('fk_book', 'pk')
        self.rel_b.setId('rel_b')
        assert(self.rel_b.isValid())
        self.relMgr.addRelation(self.rel_b)

        self.startTransaction()

    def tearDown(self):
        self.rollbackTransaction()
        del self.transaction
        self.relMgr.clear()

    def startTransaction(self):
        """
        Start a new transaction and set all layers into transaction mode.

        :return: None
        """
        self.transaction = QgsTransaction.create(self.layers)
        self.transaction.begin()
        for layer in self.layers:
            layer.startEditing()

    def rollbackTransaction(self):
        """
        Rollback all changes done in this transaction.
        We always rollback and never commit to have the database in a pristine
        state at the end of each test.

        :return: None
        """
        for layer in self.layers:
            layer.commitChanges()
        self.transaction.rollback()

    def test_delete_feature(self):
        """
        Check if a feature can be deleted properly
        """
        self.createWrapper(self.vl_authors, '"name"=\'Erich Gamma\'')

        self.assertEqual(self.table_view.model().rowCount(), 1)

        self.assertEqual(1, len([f for f in self.vl_books.getFeatures()]))

        fid = next(self.vl_books.getFeatures(QgsFeatureRequest().setFilterExpression('"name"=\'Design Patterns. Elements of Reusable Object-Oriented Software\''))).id()

        self.widget.featureSelectionManager().select([fid])

        btn = self.widget.findChild(QToolButton, 'mDeleteFeatureButton')

        def clickOk():
            # Click the "Delete features" button on the confirmation message
            # box
            widget = self.widget.findChild(QMessageBox)
            buttonBox = widget.findChild(QDialogButtonBox)
            deleteButton = next((b for b in buttonBox.buttons() if buttonBox.buttonRole(b) == QDialogButtonBox.AcceptRole))
            deleteButton.click()

        QTimer.singleShot(1, clickOk)
        btn.click()

        # This is the important check that the feature is deleted
        self.assertEqual(0, len([f for f in self.vl_books.getFeatures()]))

        # This is actually more checking that the database on delete action is properly set on the relation
        self.assertEqual(0, len([f for f in self.vl_link_books_authors.getFeatures()]))

        self.assertEqual(self.table_view.model().rowCount(), 0)

    def test_list(self):
        """
        Simple check if several related items are shown
        """
        wrapper = self.createWrapper(self.vl_books)  # NOQA

        self.assertEqual(self.table_view.model().rowCount(), 4)

    def test_add_feature(self):
        """
        Check if a new related feature is added
        """
        self.createWrapper(self.vl_authors, '"name"=\'Douglas Adams\'')

        self.assertEqual(self.table_view.model().rowCount(), 0)

        self.vltools.setValues([None, 'The Hitchhiker\'s Guide to the Galaxy', 'Sputnik Editions', 1961])
        btn = self.widget.findChild(QToolButton, 'mAddFeatureButton')
        btn.click()

        # Book entry has been created
        self.assertEqual(2, len([f for f in self.vl_books.getFeatures()]))

        # Link entry has been created
        self.assertEqual(5, len([f for f in self.vl_link_books_authors.getFeatures()]))

        self.assertEqual(self.table_view.model().rowCount(), 1)

    def test_link_feature(self):
        """
        Check if an existing feature can be linked
        """
        wrapper = self.createWrapper(self.vl_authors, '"name"=\'Douglas Adams\'')  # NOQA

        f = QgsFeature(self.vl_books.fields())
        f.setAttributes([self.vl_books.dataProvider().defaultValueClause(0), 'The Hitchhiker\'s Guide to the Galaxy', 'Sputnik Editions', 1961])
        self.vl_books.addFeature(f)

        btn = self.widget.findChild(QToolButton, 'mLinkFeatureButton')
        btn.click()

        dlg = self.widget.findChild(QDialog)
        dlg.setSelectedFeatures([f.id()])
        dlg.accept()

        # magically the above code selects the feature here...
        link_feature = next(self.vl_link_books_authors.getFeatures(QgsFeatureRequest().setFilterExpression('"fk_book"={}'.format(f[0]))))
        self.assertIsNotNone(link_feature[0])

        self.assertEqual(self.table_view.model().rowCount(), 1)

    def test_unlink_feature(self):
        """
        Check if a linked feature can be unlinked
        """
        wrapper = self.createWrapper(self.vl_books)   # NOQA

        # All authors are listed
        self.assertEqual(self.table_view.model().rowCount(), 4)

        it = self.vl_authors.getFeatures(
            QgsFeatureRequest().setFilterExpression('"name" IN (\'Richard Helm\', \'Ralph Johnson\')'))

        self.widget.featureSelectionManager().select([f.id() for f in it])

        self.assertEqual(2, self.widget.featureSelectionManager().selectedFeatureCount())

        btn = self.widget.findChild(QToolButton, 'mUnlinkFeatureButton')
        btn.click()

        # This is actually more checking that the database on delete action is properly set on the relation
        self.assertEqual(2, len([f for f in self.vl_link_books_authors.getFeatures()]))

        self.assertEqual(2, self.table_view.model().rowCount())

    def test_discover_relations(self):
        """
        Test the automatic discovery of relations
        """
        relations = self.relMgr.discoverRelations([], [self.vl_authors, self.vl_books, self.vl_link_books_authors])
        relations = {r.name(): r for r in relations}
        self.assertEqual({'books_authors_fk_book_fkey', 'books_authors_fk_author_fkey'}, set(relations.keys()))

        ba2b = relations['books_authors_fk_book_fkey']
        self.assertTrue(ba2b.isValid())
        self.assertEqual('books_authors', ba2b.referencingLayer().name())
        self.assertEqual('books', ba2b.referencedLayer().name())
        self.assertEqual([0], ba2b.referencingFields())
        self.assertEqual([0], ba2b.referencedFields())

        ba2a = relations['books_authors_fk_author_fkey']
        self.assertTrue(ba2a.isValid())
        self.assertEqual('books_authors', ba2a.referencingLayer().name())
        self.assertEqual('authors', ba2a.referencedLayer().name())
        self.assertEqual([1], ba2a.referencingFields())
        self.assertEqual([0], ba2a.referencedFields())

        self.assertEqual([], self.relMgr.discoverRelations([self.rel_a, self.rel_b], [self.vl_authors, self.vl_books, self.vl_link_books_authors]))
        self.assertEqual(1, len(self.relMgr.discoverRelations([], [self.vl_authors, self.vl_link_books_authors])))

        # composite keys relation
        relations = self.relMgr.discoverRelations([], [self.vl_books, self.vl_editors])
        self.assertEqual(len(relations), 1)
        relation = relations[0]
        self.assertEqual('books_fk_editor_fkey', relation.name())
        self.assertTrue(relation.isValid())
        self.assertEqual('books', relation.referencingLayer().name())
        self.assertEqual('editors', relation.referencedLayer().name())
        self.assertEqual([2, 3], relation.referencingFields())
        self.assertEqual([0, 1], relation.referencedFields())

    def test_selection(self):

        fbook = QgsFeature(self.vl_books.fields())
        fbook.setAttributes([self.vl_books.dataProvider().defaultValueClause(0), 'The Hitchhiker\'s Guide to the Galaxy', 'Sputnik Editions', 1961])
        self.vl_books.addFeature(fbook)

        flink = QgsFeature(self.vl_link_books_authors.fields())
        flink.setAttributes([fbook.id(), 5])
        self.vl_link_books_authors.addFeature(flink)

        self.createWrapper(self.vl_authors, '"name"=\'Douglas Adams\'')

        self.zoomToButton = self.widget.findChild(QToolButton, "mDeleteFeatureButton")
        self.assertTrue(self.zoomToButton)
        self.assertTrue(not self.zoomToButton.isEnabled())

        selectionMgr = self.widget.featureSelectionManager()
        self.assertTrue(selectionMgr)

        self.vl_books.select(fbook.id())
        self.assertEqual([fbook.id()], selectionMgr.selectedFeatureIds())
        self.assertTrue(self.zoomToButton.isEnabled())

        selectionMgr.deselect([fbook.id()])
        self.assertEqual([], selectionMgr.selectedFeatureIds())
        self.assertTrue(not self.zoomToButton.isEnabled())

        self.vl_books.select([1, fbook.id()])
        self.assertEqual([fbook.id()], selectionMgr.selectedFeatureIds())
        self.assertTrue(self.zoomToButton.isEnabled())

    def test_add_feature_geometry(self):
        """
        Test to add a feature with a geometry
        """
        vl_pipes = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."pipes" (geom) sql=', 'pipes', 'postgres')
        vl_leaks = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."leaks" (geom) sql=', 'leaks', 'postgres')
        vl_leaks.startEditing()

        QgsProject.instance().addMapLayer(vl_pipes)
        QgsProject.instance().addMapLayer(vl_leaks)

        self.assertEqual(vl_pipes.featureCount(), 2)
        self.assertEqual(vl_leaks.featureCount(), 3)

        rel = QgsRelation()
        rel.setReferencingLayer(vl_leaks.id())
        rel.setReferencedLayer(vl_pipes.id())
        rel.addFieldPair('pipe', 'id')
        rel.setId('rel_pipe_leak')
        self.assertTrue(rel.isValid())
        self.relMgr.addRelation(rel)

        # Mock vector layer tool to just set default value on created feature
        class DummyVlTools(QgsVectorLayerTools):

            def addFeature(self, layer, defaultValues, defaultGeometry, parentWidget=None, showModal=True, hideParent=False):
                f = QgsFeature(layer.fields())
                for idx, value in defaultValues.items():
                    f.setAttribute(idx, value)
                f.setGeometry(defaultGeometry)
                ok = layer.addFeature(f)

                return ok, f

        wrapper = QgsRelationWidgetWrapper(vl_leaks, rel)
        context = QgsAttributeEditorContext()
        vltool = DummyVlTools()
        context.setVectorLayerTools(vltool)
        context.setMapCanvas(self.mapCanvas)
        cadDockWidget = QgsAdvancedDigitizingDockWidget(self.mapCanvas)
        context.setCadDockWidget(cadDockWidget)
        wrapper.setContext(context)
        widget = wrapper.widget()
        widget.show()
        pipe = next(vl_pipes.getFeatures())
        self.assertEqual(pipe.id(), 1)
        wrapper.setFeature(pipe)
        table_view = widget.findChild(QTableView)
        self.assertEqual(table_view.model().rowCount(), 1)

        btn = widget.findChild(QToolButton, 'mAddFeatureGeometryButton')
        self.assertTrue(btn.isVisible())
        self.assertTrue(btn.isEnabled())
        btn.click()
        self.assertTrue(self.mapCanvas.mapTool())
        feature = QgsFeature(vl_leaks.fields())
        feature.setGeometry(QgsGeometry.fromWkt('POINT(0 0.8)'))
        self.mapCanvas.mapTool().digitizingCompleted.emit(feature)
        self.assertEqual(table_view.model().rowCount(), 2)
        self.assertEqual(vl_leaks.featureCount(), 4)
        request = QgsFeatureRequest()
        request.addOrderBy("id", False)

        # get new created feature
        feat = next(vl_leaks.getFeatures('"id" is NULL'))
        self.assertTrue(feat.isValid())
        self.assertTrue(feat.geometry().equals(QgsGeometry.fromWkt('POINT(0 0.8)')))

        vl_leaks.rollBack()

    def createWrapper(self, layer, filter=None):
        """
        Basic setup of a relation widget wrapper.
        Will create a new wrapper and set its feature to the one and only book
        in the table.
        It will also assign some instance variables to help

         * self.widget The created widget
         * self.table_view The table view of the widget

        :return: The created wrapper
        """
        if layer == self.vl_books:
            relation = self.rel_b
            nmrel = self.rel_a
        else:
            relation = self.rel_a
            nmrel = self.rel_b

        self.wrapper = QgsRelationWidgetWrapper(layer, relation)
        self.wrapper.setConfig({'nm-rel': nmrel.id()})
        context = QgsAttributeEditorContext()
        context.setMapCanvas(self.mapCanvas)
        context.setVectorLayerTools(self.vltools)
        self.wrapper.setContext(context)

        self.widget = self.wrapper.widget()
        self.widget.show()

        request = QgsFeatureRequest()
        if filter:
            request.setFilterExpression(filter)
        book = next(layer.getFeatures(request))
        self.wrapper.setFeature(book)

        self.table_view = self.widget.findChild(QTableView)
        return self.wrapper


class VlTools(QgsVectorLayerTools):

    """
    Mock the QgsVectorLayerTools
    Since we don't have a user on the test server to input this data for us, we can just use this.
    """

    def setValues(self, values):
        """
        Set the values for the next feature to insert
        :param values: An array of values that shall be used for the next inserted record
        :return: None
        """
        self.values = values

    def addFeature(self, layer, defaultValues, defaultGeometry, parentWidget=None, showModal=True, hideParent=False):
        """
        Overrides the addFeature method
        :param layer: vector layer
        :param defaultValues: some default values that may be provided by QGIS
        :param defaultGeometry: a default geometry that may be provided by QGIS
        :return: tuple(ok, f) where ok is if the layer added the feature and f is the added feature
        """
        values = list()
        for i, v in enumerate(self.values):
            if v:
                values.append(v)
            else:
                values.append(layer.dataProvider().defaultValueClause(i))
        f = QgsFeature(layer.fields())
        f.setAttributes(self.values)
        f.setGeometry(defaultGeometry)
        ok = layer.addFeature(f)

        return ok, f

    def startEditing(self, layer):
        pass

    def stopEditing(self, layer, allowCancel):
        pass

    def saveEdits(self, layer):
        pass


if __name__ == '__main__':
    unittest.main()
