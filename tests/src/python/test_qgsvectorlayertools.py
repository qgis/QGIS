# -*- coding: utf-8 -*-
"""QGIS Unit test utils for provider tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Denis Rouzaud'
__date__ = '2016-11-07'
__copyright__ = 'Copyright 2015, The QGIS Project'

from qgis.core import QgsFeatureRequest, QgsVectorLayer, QgsProject, QgsVectorLayerTools
from qgis.testing import start_app, unittest

import os

start_app()


class SubQgsVectorLayerTools(QgsVectorLayerTools):

    def __init__(self):
        super().__init__()

    def addFeature(self, layer):
        pass

    def startEditing(self, layer):
        pass

    def stopEditing(self, layer):
        pass

    def saveEdits(self, layer):
        pass


class TestQgsVectorLayerTools(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """
        Setup the involved layers and relations for a n:m relation
        :return:
        """
        cls.dbconn = 'service=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layer
        cls.vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."someData" (geom) sql=', 'layer', 'postgres')

        QgsProject.instance().addMapLayer(cls.vl)

        cls.vltools = SubQgsVectorLayerTools()

    def testCopyMoveFeature(self):
        """ Test copy and move features"""
        rqst = QgsFeatureRequest()
        rqst.setFilterFid(4)
        self.vl.startEditing()
        (ok, rqst, msg) = self.vltools.copyMoveFeatures(self.vl, rqst, -0.1, 0.2)
        self.assertTrue(ok)
        for f in self.vl.getFeatures(rqst):
            geom = f.geometry()
            self.assertAlmostEqual(geom.asPoint().x(), -65.42)
            self.assertAlmostEqual(geom.asPoint().y(), 78.5)


if __name__ == '__main__':
    unittest.main()
