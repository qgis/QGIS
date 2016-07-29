# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSnappingUtils (complement to C++-based tests)

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Hugo Mercier'
__date__ = '12/07/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import os

from qgis.core import (QgsMapLayerRegistry,
                       QgsVectorLayer,
                       QgsMapSettings,
                       QgsSnappingUtils,
                       QgsPointLocator,
                       QgsTolerance,
                       QgsRectangle,
                       QgsPoint,
                       QgsFeature,
                       QgsGeometry
                       )

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QSize, QPoint

import tempfile

try:
    from pyspatialite import dbapi2 as sqlite3
except ImportError:
    print("You should install pyspatialite to run the tests")
    raise ImportError

# Convenience instances in case you may need them
start_app()


class TestQgsSnappingUtils(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        # create a temp spatialite db with a trigger
        fo = tempfile.NamedTemporaryFile()
        fn = fo.name
        fo.close()
        print fn
        con = sqlite3.connect(fn)
        cur = con.cursor()
        cur.execute("SELECT InitSpatialMetadata(1)")
        cur.execute("create table node(id integer primary key autoincrement);")
        cur.execute("select AddGeometryColumn('node', 'geom', 4326, 'POINT');")
        cur.execute("create table section(id integer primary key autoincrement, node1 integer, node2 integer);")
        cur.execute("select AddGeometryColumn('section', 'geom', 4326, 'LINESTRING');")
        cur.execute("create trigger add_nodes after insert on section begin insert into node (geom) values (st_startpoint(NEW.geom)); insert into node (geom) values (st_endpoint(NEW.geom)); end;")
        cur.execute("insert into node (geom) values (geomfromtext('point(0 0)', 4326));")
        cur.execute("insert into node (geom) values (geomfromtext('point(1 0)', 4326));")
        con.commit()
        con.close()

        cls.pointsLayer = QgsVectorLayer("dbname='%s' table=\"node\" (geom) sql=" % fn, "points", "spatialite")
        assert (cls.pointsLayer.isValid())
        cls.linesLayer = QgsVectorLayer("dbname='%s' table=\"section\" (geom) sql=" % fn, "points", "spatialite")
        assert (cls.linesLayer.isValid())
        QgsMapLayerRegistry.instance().addMapLayers([cls.pointsLayer, cls.linesLayer])

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        pass

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def test_resetSnappingIndex(self):
        ms = QgsMapSettings()
        ms.setOutputSize(QSize(100, 100))
        ms.setExtent(QgsRectangle(0, 0, 1, 1))
        self.assertTrue(ms.hasValidSettings())

        u = QgsSnappingUtils()
        u.setMapSettings(ms)
        u.setSnapToMapMode(QgsSnappingUtils.SnapAdvanced)
        layers = [QgsSnappingUtils.LayerConfig(self.pointsLayer, QgsPointLocator.Vertex, 20, QgsTolerance.Pixels)]
        u.setLayers(layers)

        m = u.snapToMap(QPoint(95, 100))
        self.assertTrue(m.isValid())
        self.assertTrue(m.hasVertex())
        self.assertEqual(m.point(), QgsPoint(1, 0))

        f = QgsFeature(self.linesLayer.fields())
        f.setFeatureId(1)
        geom = QgsGeometry.fromWkt("LINESTRING(0 0,1 1)")
        f.setGeometry(geom)
        self.linesLayer.startEditing()
        self.linesLayer.addFeatures([f])
        self.linesLayer.commitChanges()

        l1 = len([f for f in self.pointsLayer.getFeatures()])
        self.assertEqual(l1, 4)
        m = u.snapToMap(QPoint(95, 0))
        # snapping not updated
        self.assertEqual(m.isValid(), False)

        # set side effect layers
        u.setSideEffectLayers([self.linesLayer])
        # add another line
        f = QgsFeature(self.linesLayer.fields())
        f.setFeatureId(2)
        geom = QgsGeometry.fromWkt("LINESTRING(0 0,0.5 0.5)")
        f.setGeometry(geom)
        self.linesLayer.startEditing()
        self.linesLayer.addFeatures([f])
        self.linesLayer.commitChanges()

        m = u.snapToMap(QPoint(45, 50))
        self.assertTrue(m.isValid())
        self.assertTrue(m.hasVertex())
        self.assertEqual(m.point(), QgsPoint(0.5, 0.5))

if __name__ == '__main__':
    unittest.main()
