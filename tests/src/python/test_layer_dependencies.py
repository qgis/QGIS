"""QGIS Unit tests for QgsSnappingUtils (complement to C++-based tests)

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Hugo Mercier"
__date__ = "12/07/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

import os
import tempfile

from qgis.PyQt.QtCore import QPoint, QSize
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    Qgis,
    QgsFeature,
    QgsGeometry,
    QgsLayerDefinition,
    QgsMapLayerDependency,
    QgsMapSettings,
    QgsPointXY,
    QgsProject,
    QgsRectangle,
    QgsSnappingConfig,
    QgsSnappingUtils,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.utils import spatialite_connect

# Convenience instances in case you may need them
start_app()


class TestLayerDependencies(QgisTestCase):

    def setUp(self):
        """Run before each test."""
        # create a temp SpatiaLite db with a trigger
        fo = tempfile.NamedTemporaryFile()
        fn = fo.name
        fo.close()
        self.fn = fn
        con = spatialite_connect(fn)
        cur = con.cursor()
        cur.execute("SELECT InitSpatialMetadata(1)")
        cur.execute("create table node(id integer primary key autoincrement);")
        cur.execute("select AddGeometryColumn('node', 'geom', 4326, 'POINT');")
        cur.execute(
            "create table section(id integer primary key autoincrement, node1 integer, node2 integer);"
        )
        cur.execute("select AddGeometryColumn('section', 'geom', 4326, 'LINESTRING');")
        cur.execute(
            "create trigger add_nodes after insert on section begin insert into node (geom) values (st_startpoint(NEW.geom)); insert into node (geom) values (st_endpoint(NEW.geom)); end;"
        )
        cur.execute(
            "insert into node (geom) values (geomfromtext('point(0 0)', 4326));"
        )
        cur.execute(
            "insert into node (geom) values (geomfromtext('point(1 0)', 4326));"
        )
        cur.execute("create table node2(id integer primary key autoincrement);")
        cur.execute("select AddGeometryColumn('node2', 'geom', 4326, 'POINT');")
        cur.execute(
            "create trigger add_nodes2 after insert on node begin insert into node2 (geom) values (st_translate(NEW.geom, 0.2, 0, 0)); end;"
        )
        con.commit()
        con.close()

        self.pointsLayer = QgsVectorLayer(
            f"dbname='{fn}' table=\"node\" (geom) sql=", "points", "spatialite"
        )
        assert self.pointsLayer.isValid()
        self.linesLayer = QgsVectorLayer(
            f"dbname='{fn}' table=\"section\" (geom) sql=", "lines", "spatialite"
        )
        assert self.linesLayer.isValid()
        self.pointsLayer2 = QgsVectorLayer(
            f"dbname='{fn}' table=\"node2\" (geom) sql=", "_points2", "spatialite"
        )
        assert self.pointsLayer2.isValid()
        QgsProject.instance().addMapLayers(
            [self.pointsLayer, self.linesLayer, self.pointsLayer2]
        )

        # save the project file
        fo = tempfile.NamedTemporaryFile()
        fn = fo.name
        fo.close()
        self.projectFile = fn
        QgsProject.instance().setFileName(self.projectFile)
        QgsProject.instance().write()

    def tearDown(self):
        """Run after each test."""
        QgsProject.instance().clear()
        pass

    def test_resetSnappingIndex(self):
        self.pointsLayer.setDependencies([])
        self.linesLayer.setDependencies([])
        self.pointsLayer2.setDependencies([])

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(100, 100))
        ms.setExtent(QgsRectangle(0, 0, 1, 1))
        self.assertTrue(ms.hasValidSettings())

        u = QgsSnappingUtils()
        u.setMapSettings(ms)
        cfg = u.config()
        cfg.setEnabled(True)
        cfg.setMode(Qgis.SnappingMode.AdvancedConfiguration)
        cfg.setIndividualLayerSettings(
            self.pointsLayer,
            QgsSnappingConfig.IndividualLayerSettings(
                True, Qgis.SnappingType.Vertex, 20, Qgis.MapToolUnit.Pixels, 0.0, 0.0
            ),
        )
        u.setConfig(cfg)

        m = u.snapToMap(QPoint(95, 100))
        self.assertTrue(m.isValid())
        self.assertTrue(m.hasVertex())
        self.assertEqual(m.point(), QgsPointXY(1, 0))

        f = QgsFeature(self.linesLayer.fields())
        f.setId(1)
        geom = QgsGeometry.fromWkt("LINESTRING(0 0,1 1)")
        f.setGeometry(geom)
        self.linesLayer.startEditing()
        self.linesLayer.addFeatures([f])
        self.linesLayer.commitChanges()

        l1 = len([f for f in self.pointsLayer.getFeatures()])
        self.assertEqual(l1, 4)
        m = u.snapToMap(QPoint(95, 0))
        # snapping not updated
        self.pointsLayer.setDependencies([])
        self.assertEqual(m.isValid(), False)

        # set layer dependencies
        self.pointsLayer.setDependencies([QgsMapLayerDependency(self.linesLayer.id())])
        # add another line
        f = QgsFeature(self.linesLayer.fields())
        f.setId(2)
        geom = QgsGeometry.fromWkt("LINESTRING(0 0,0.5 0.5)")
        f.setGeometry(geom)
        self.linesLayer.startEditing()
        self.linesLayer.addFeatures([f])
        self.linesLayer.commitChanges()
        # check the snapped point is OK
        m = u.snapToMap(QPoint(45, 50))
        self.assertTrue(m.isValid())
        self.assertTrue(m.hasVertex())
        self.assertEqual(m.point(), QgsPointXY(0.5, 0.5))
        self.pointsLayer.setDependencies([])

        # test chained layer dependencies A -> B -> C
        cfg.setIndividualLayerSettings(
            self.pointsLayer2,
            QgsSnappingConfig.IndividualLayerSettings(
                True, Qgis.SnappingType.Vertex, 20, Qgis.MapToolUnit.Pixels, 0.0, 0.0
            ),
        )
        u.setConfig(cfg)
        self.pointsLayer.setDependencies([QgsMapLayerDependency(self.linesLayer.id())])
        self.pointsLayer2.setDependencies(
            [QgsMapLayerDependency(self.pointsLayer.id())]
        )
        # add another line
        f = QgsFeature(self.linesLayer.fields())
        f.setId(3)
        geom = QgsGeometry.fromWkt("LINESTRING(0 0.2,0.5 0.8)")
        f.setGeometry(geom)
        self.linesLayer.startEditing()
        self.linesLayer.addFeatures([f])
        self.linesLayer.commitChanges()
        # check the second snapped point is OK
        m = u.snapToMap(QPoint(75, 100 - 80))
        self.assertTrue(m.isValid())
        self.assertTrue(m.hasVertex())
        self.assertEqual(m.point(), QgsPointXY(0.7, 0.8))
        self.pointsLayer.setDependencies([])
        self.pointsLayer2.setDependencies([])

    def test_circular_dependencies_with_2_layers(self):

        spy_points_data_changed = QSignalSpy(self.pointsLayer.dataChanged)
        spy_lines_data_changed = QSignalSpy(self.linesLayer.dataChanged)
        spy_points_repaint_requested = QSignalSpy(self.pointsLayer.repaintRequested)
        spy_lines_repaint_requested = QSignalSpy(self.linesLayer.repaintRequested)

        # only points fire dataChanged because we change its dependencies
        self.assertTrue(
            self.pointsLayer.setDependencies(
                [QgsMapLayerDependency(self.linesLayer.id())]
            )
        )
        self.assertEqual(len(spy_points_data_changed), 1)
        self.assertEqual(len(spy_lines_data_changed), 0)

        # lines fire dataChanged because we changes its dependencies
        # points fire dataChanged because it depends on line
        self.assertTrue(
            self.linesLayer.setDependencies(
                [QgsMapLayerDependency(self.pointsLayer.id())]
            )
        )
        self.assertEqual(len(spy_points_data_changed), 2)
        self.assertEqual(len(spy_lines_data_changed), 1)

        f = QgsFeature(self.pointsLayer.fields())
        f.setId(1)
        geom = QgsGeometry.fromWkt("POINT(0 0)")
        f.setGeometry(geom)
        self.pointsLayer.startEditing()

        # new point fire featureAdded so depending line fire dataChanged
        # point depends on line, so fire dataChanged
        self.pointsLayer.addFeatures([f])
        self.assertEqual(len(spy_points_data_changed), 3)
        self.assertEqual(len(spy_lines_data_changed), 2)

        # commit changes fires dataChanged because external changes could happen (provider side)
        self.pointsLayer.commitChanges(False)
        self.assertEqual(len(spy_points_data_changed), 4)
        self.assertEqual(len(spy_lines_data_changed), 3)

        # points fire dataChanged on geometryChanged
        # line depends on point, so fire dataChanged
        self.pointsLayer.changeGeometry(f.id(), QgsGeometry.fromWkt("POINT(0 2)"))
        self.assertEqual(len(spy_points_data_changed), 5)
        self.assertEqual(len(spy_lines_data_changed), 4)

        # commit changes fires dataChanged because external changes could happen (provider side)
        self.assertTrue(self.pointsLayer.commitChanges())
        self.assertEqual(len(spy_points_data_changed), 6)
        self.assertEqual(len(spy_lines_data_changed), 5)

        # repaintRequested is called on commit changes on point
        # so it is on depending line
        # (ideally only one repaintRequested signal is fired, but it's harmless to fire multiple ones)
        self.assertGreaterEqual(len(spy_lines_repaint_requested), 2)
        self.assertGreaterEqual(len(spy_points_repaint_requested), 2)

    def test_circular_dependencies_with_1_layer(self):

        # You can define a layer dependent on it self (for instance, a line
        # layer that trigger connected lines modifications when you modify
        # one line)
        spy_lines_data_changed = QSignalSpy(self.linesLayer.dataChanged)
        spy_lines_repaint_requested = QSignalSpy(self.linesLayer.repaintRequested)

        # line fire dataChanged because we change its dependencies
        self.assertTrue(
            self.linesLayer.setDependencies(
                [QgsMapLayerDependency(self.linesLayer.id())]
            )
        )
        self.assertEqual(len(spy_lines_data_changed), 1)

        f = QgsFeature(self.linesLayer.fields())
        f.setId(1)
        geom = QgsGeometry.fromWkt("LINESTRING(0 0,1 1)")
        f.setGeometry(geom)
        self.linesLayer.startEditing()

        # line fire featureAdded so depending line fire dataChanged once more
        self.linesLayer.addFeatures([f])
        self.assertEqual(len(spy_lines_data_changed), 2)

        # line fire dataChanged on commitChanges
        self.linesLayer.commitChanges(False)
        self.assertEqual(len(spy_lines_data_changed), 3)

        # repaintRequested is called only once on commit changes on line
        # (ideally only one repaintRequested signal is fired, but it's harmless to fire multiple ones)
        self.assertGreaterEqual(len(spy_lines_repaint_requested), 2)

        # line fire dataChanged on geometryChanged
        self.linesLayer.changeGeometry(
            f.id(), QgsGeometry.fromWkt("LINESTRING(0 0, 2 2)")
        )
        self.assertEqual(len(spy_lines_data_changed), 4)

        # commit changes fires dataChanged because external changes could happen (provider side)
        self.linesLayer.commitChanges()
        self.assertEqual(len(spy_lines_data_changed), 5)

    def test_layerDefinitionRewriteId(self):
        tmpfile = os.path.join(tempfile.tempdir, "test.qlr")

        ltr = QgsProject.instance().layerTreeRoot()

        self.pointsLayer.setDependencies([QgsMapLayerDependency(self.linesLayer.id())])

        QgsLayerDefinition.exportLayerDefinition(tmpfile, [ltr])

        grp = ltr.addGroup("imported")
        QgsLayerDefinition.loadLayerDefinition(tmpfile, QgsProject.instance(), grp)

        newPointsLayer = None
        newLinesLayer = None
        for l in grp.findLayers():
            if l.layerId().startswith("points"):
                newPointsLayer = l.layer()
            elif l.layerId().startswith("lines"):
                newLinesLayer = l.layer()
        self.assertIsNotNone(newPointsLayer)
        self.assertIsNotNone(newLinesLayer)
        self.assertIn(
            newLinesLayer.id(), [dep.layerId() for dep in newPointsLayer.dependencies()]
        )

        self.pointsLayer.setDependencies([])

    def test_signalConnection(self):
        # remove all layers
        QgsProject.instance().removeAllMapLayers()
        # set dependencies and add back layers
        self.pointsLayer = QgsVectorLayer(
            f"dbname='{self.fn}' table=\"node\" (geom) sql=", "points", "spatialite"
        )
        self.assertTrue(self.pointsLayer.isValid())
        self.linesLayer = QgsVectorLayer(
            f"dbname='{self.fn}' table=\"section\" (geom) sql=", "lines", "spatialite"
        )
        self.assertTrue(self.linesLayer.isValid())
        self.pointsLayer2 = QgsVectorLayer(
            f"dbname='{self.fn}' table=\"node2\" (geom) sql=", "_points2", "spatialite"
        )
        self.assertTrue(self.pointsLayer2.isValid())
        self.pointsLayer.setDependencies([QgsMapLayerDependency(self.linesLayer.id())])
        self.pointsLayer2.setDependencies(
            [QgsMapLayerDependency(self.pointsLayer.id())]
        )
        # this should update connections between layers
        QgsProject.instance().addMapLayers([self.pointsLayer])
        QgsProject.instance().addMapLayers([self.linesLayer])
        QgsProject.instance().addMapLayers([self.pointsLayer2])

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(100, 100))
        ms.setExtent(QgsRectangle(0, 0, 1, 1))
        self.assertTrue(ms.hasValidSettings())

        u = QgsSnappingUtils()
        u.setMapSettings(ms)
        cfg = u.config()
        cfg.setEnabled(True)
        cfg.setMode(Qgis.SnappingMode.AdvancedConfiguration)
        cfg.setIndividualLayerSettings(
            self.pointsLayer,
            QgsSnappingConfig.IndividualLayerSettings(
                True, Qgis.SnappingType.Vertex, 20, Qgis.MapToolUnit.Pixels, 0.0, 0.0
            ),
        )
        cfg.setIndividualLayerSettings(
            self.pointsLayer2,
            QgsSnappingConfig.IndividualLayerSettings(
                True, Qgis.SnappingType.Vertex, 20, Qgis.MapToolUnit.Pixels, 0.0, 0.0
            ),
        )
        u.setConfig(cfg)
        # add another line
        f = QgsFeature(self.linesLayer.fields())
        f.setId(4)
        geom = QgsGeometry.fromWkt("LINESTRING(0.5 0.2,0.6 0)")
        f.setGeometry(geom)
        self.linesLayer.startEditing()
        self.linesLayer.addFeatures([f])
        self.linesLayer.commitChanges()
        # check the second snapped point is OK
        m = u.snapToMap(QPoint(75, 100 - 0))
        self.assertTrue(m.isValid())
        self.assertTrue(m.hasVertex())
        self.assertEqual(m.point(), QgsPointXY(0.8, 0.0))

        self.pointsLayer.setDependencies([])
        self.pointsLayer2.setDependencies([])


if __name__ == "__main__":
    unittest.main()
