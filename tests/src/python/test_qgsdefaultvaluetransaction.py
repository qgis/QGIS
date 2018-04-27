# -*- coding: utf-8 -*-
"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '16/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.core import (
    QgsFeature,
    QgsVectorLayer,
    QgsProject,
    QgsTransaction,
    QgsFeatureRequest,
    QgsMapLayerRegistry,
)

from qgis.testing import start_app, unittest

start_app()


class TestQgsDefaultValueTransaction(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """
        Setup the involved layer
        :return:
        """
        cls.dbconn = 'service=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layer
        cls.vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."someData" sql=', 'points', 'postgres')

        QgsMapLayerRegistry.instance().addMapLayer(cls.vl)
        QgsProject.instance().setAutoTransaction(True)

        assert(cls.vl.isValid())

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_transaction_add_feature_with_default_value(self):
        """
        Check if a feature can be added programmatically with a default value in transaction mode
        """

        transaction = QgsTransaction.create([self.vl.id()])
        transaction.begin()
        self.vl.startEditing()
        
        def onError(message):
            """We should not get here. If we do, fail and say why"""
            self.assertFalse(True, message)

        self.vl.raiseError.connect(onError)

        init_fields = self.vl.fields()
        f = QgsFeature(init_fields)
        f.initAttributes(init_fields.size())
        f['name'] = 'test_transaction_add_feature_with_default_value'
        self.assertTrue(self.vl.addFeature(f))

        f = next(self.vl.getFeatures(QgsFeatureRequest().setFilterExpression('"name" = \'test_transaction_add_feature_with_default_value\'')))
        self.assertTrue(f.isValid())

        self.vl.commitChanges()
        transaction.rollback()

    def startEditing(self, layer):
        pass

    def stopEditing(self, layer, allowCancel):
        pass

    def saveEdits(self, layer):
        pass


if __name__ == '__main__':
    unittest.main()
