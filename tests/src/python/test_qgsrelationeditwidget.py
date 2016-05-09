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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.core import (
    QgsFeature,
    QgsVectorLayer,
    QgsProject,
    QgsRelation,
    QgsMapLayerRegistry,
    QgsTransaction,
    QgsFeatureRequest
)

from qgis.gui import (
    QgsEditorWidgetRegistry,
    QgsRelationWidgetWrapper,
    QgsAttributeEditorContext,
    QgsVectorLayerTools
)

from qgis.PyQt.QtCore import QTimer
from qgis.PyQt.QtWidgets import QToolButton, QTableView, QApplication
from qgis.testing import start_app, unittest

start_app()


class TestQgsRelationEditWidget(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """
        Setup the involved layers and relations for a n:m relation
        :return:
        """
        QgsEditorWidgetRegistry.initEditors()
        cls.dbconn = u'dbname=\'qgis_test\' host=localhost port=5432 user=\'postgres\' password=\'postgres\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layer
        cls.vl_b = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."books" sql=', 'test', 'postgres')
        cls.vl_a = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."authors" sql=', 'test', 'postgres')
        cls.vl_link = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."books_authors" sql=', 'test', 'postgres')

        QgsMapLayerRegistry.instance().addMapLayer(cls.vl_b)
        QgsMapLayerRegistry.instance().addMapLayer(cls.vl_a)
        QgsMapLayerRegistry.instance().addMapLayer(cls.vl_link)

        relMgr = QgsProject.instance().relationManager()

        cls.rel_a = QgsRelation()
        cls.rel_a.setReferencingLayer(cls.vl_link.id())
        cls.rel_a.setReferencedLayer(cls.vl_a.id())
        cls.rel_a.addFieldPair('fk_author', 'pk')
        cls.rel_a.setRelationId('rel_a')
        assert(cls.rel_a.isValid())
        relMgr.addRelation(cls.rel_a)

        cls.rel_b = QgsRelation()
        cls.rel_b.setReferencingLayer(cls.vl_link.id())
        cls.rel_b.setReferencedLayer(cls.vl_b.id())
        cls.rel_b.addFieldPair('fk_book', 'pk')
        cls.rel_b.setRelationId('rel_b')
        assert(cls.rel_b.isValid())
        relMgr.addRelation(cls.rel_b)

        # Our mock QgsVectorLayerTools, that allow injecting data where user input is expected
        cls.vltools = VlTools()

        assert(cls.vl_a.isValid())
        assert(cls.vl_b.isValid())
        assert(cls.vl_link.isValid())

    def setUp(self):
        self.startTransaction()

    def tearDown(self):
        self.rollbackTransaction()

    def test_delete_feature(self):
        """
        Check if a feature can be deleted properly
        """
        self.createWrapper(self.vl_a, '"name"=\'Erich Gamma\'')

        self.assertEqual(self.table_view.model().rowCount(), 1)

        self.assertEqual(1, len([f for f in self.vl_b.getFeatures()]))

        fid = next(self.vl_b.getFeatures(QgsFeatureRequest().setFilterExpression('"name"=\'Design Patterns. Elements of Reusable Object-Oriented Software\''))).id()

        self.widget.featureSelectionManager().select([fid])

        btn = self.widget.findChild(QToolButton, 'mDeleteFeatureButton')
        btn.click()

        # This is the important check that the feature is deleted
        self.assertEqual(0, len([f for f in self.vl_b.getFeatures()]))

        # This is actually more checking that the database on delete action is properly set on the relation
        self.assertEqual(0, len([f for f in self.vl_link.getFeatures()]))

        self.assertEqual(self.table_view.model().rowCount(), 0)

    def test_list(self):
        """
        Simple check if several related items are shown
        """
        wrapper = self.createWrapper(self.vl_b)  # NOQA

        self.assertEqual(self.table_view.model().rowCount(), 4)

    @unittest.expectedFailure(os.environ['QT_VERSION'] == '4' and os.environ['TRAVIS_OS_NAME'] == 'linux') # It's probably not related to this variables at all, but that's the closest we can get to the real source of this problem at the moment...
    def test_add_feature(self):
        """
        Check if a new related feature is added
        """
        self.createWrapper(self.vl_a, '"name"=\'Douglas Adams\'')

        self.assertEqual(self.table_view.model().rowCount(), 0)

        self.vltools.setValues([None, 'The Hitchhiker\'s Guide to the Galaxy'])
        btn = self.widget.findChild(QToolButton, 'mAddFeatureButton')
        btn.click()

        # Book entry has been created
        self.assertEqual(2, len([f for f in self.vl_b.getFeatures()]))

        # Link entry has been created
        self.assertEqual(5, len([f for f in self.vl_link.getFeatures()]))

        self.assertEqual(self.table_view.model().rowCount(), 1)

    def test_link_feature(self):
        """
        Check if an existing feature can be linked
        """
        wrapper = self.createWrapper(self.vl_a, '"name"=\'Douglas Adams\'')  # NOQA

        f = QgsFeature(self.vl_b.fields())
        f.setAttributes([self.vl_b.dataProvider().defaultValue(0), 'The Hitchhiker\'s Guide to the Galaxy'])
        self.vl_b.addFeature(f)

        def choose_linked_feature():
            dlg = QApplication.activeModalWidget()
            dlg.setSelectedFeatures([f.id()])
            dlg.accept()

        btn = self.widget.findChild(QToolButton, 'mLinkFeatureButton')

        timer = QTimer()
        timer.setSingleShot(True)
        timer.setInterval(0)  # will run in the event loop as soon as it's processed when the dialog is opened
        timer.timeout.connect(choose_linked_feature)
        timer.start()

        btn.click()
        # magically the above code selects the feature here...

        link_feature = next(self.vl_link.getFeatures(QgsFeatureRequest().setFilterExpression('"fk_book"={}'.format(f[0]))))
        self.assertIsNotNone(link_feature[0])

        self.assertEqual(self.table_view.model().rowCount(), 1)

    def test_unlink_feature(self):
        """
        Check if a linked feature can be unlinked
        """
        wrapper = self.createWrapper(self.vl_b)
        wdg = wrapper.widget()  # NOQA

        # All authors are listed
        self.assertEqual(self.table_view.model().rowCount(), 4)

        it = self.vl_a.getFeatures(
            QgsFeatureRequest().setFilterExpression('"name" IN (\'Richard Helm\', \'Ralph Johnson\')'))

        self.widget.featureSelectionManager().select([f.id() for f in it])

        btn = self.widget.findChild(QToolButton, 'mUnlinkFeatureButton')
        btn.click()

        # This is actually more checking that the database on delete action is properly set on the relation
        self.assertEqual(2, len([f for f in self.vl_link.getFeatures()]))

        self.assertEqual(2, self.table_view.model().rowCount())

    def startTransaction(self):
        """
        Start a new transaction and set all layers into transaction mode.

        :return: None
        """
        lyrs = [self.vl_a, self.vl_b, self.vl_link]

        self.transaction = QgsTransaction.create([l.id() for l in lyrs])
        self.transaction.begin()
        for l in lyrs:
            l.startEditing()

    def rollbackTransaction(self):
        """
        Rollback all changes done in this transaction.
        We always rollback and never commit to have the database in a pristine
        state at the end of each test.

        :return: None
        """
        lyrs = [self.vl_a, self.vl_b, self.vl_link]
        for l in lyrs:
            l.commitChanges()
        self.transaction.rollback()

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
        if layer == self.vl_b:
            relation = self.rel_b
            nmrel = self.rel_a
        else:
            relation = self.rel_a
            nmrel = self.rel_b

        self.wrapper = QgsRelationWidgetWrapper(layer, relation)
        self.wrapper.setConfig({'nm-rel': nmrel.id()})
        context = QgsAttributeEditorContext()
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

    def addFeature(self, layer, defaultValues, defaultGeometry):
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
                values.append(layer.dataProvider().defaultValue(i))
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
