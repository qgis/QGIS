"""QGIS Unit tests for QgsMapLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "1/02/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import glob
import os
import shutil
import tempfile
from tempfile import TemporaryDirectory

from qgis.PyQt import sip
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtTest import QSignalSpy

from qgis.core import (
    Qgis,
    QgsMapLayer,
    QgsLayerMetadata,
    QgsLayerNotesUtils,
    QgsProject,
    QgsRasterLayer,
    QgsReadWriteContext,
    QgsVectorLayer,
    QgsCoordinateReferenceSystem,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsMapLayer(QgisTestCase):

    def testUniqueId(self):
        """
        Test that layers created quickly with same name get a unique ID
        """

        # make 1000 layers quickly
        layers = []
        for i in range(1000):
            layer = QgsVectorLayer(
                "Point?crs=epsg:4326&field=name:string(20)", "test", "memory"
            )
            layers.append(layer)

        # make sure all ids are unique
        ids = set()
        for l in layers:
            self.assertNotIn(l.id(), ids)
            ids.add(l.id())

    def copyLayerViaXmlReadWrite(self, source, dest):
        # write to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(source.writeLayerXml(elem, doc, QgsReadWriteContext()))
        self.assertTrue(
            dest.readLayerXml(elem, QgsReadWriteContext()), QgsProject.instance()
        )

    def testGettersSetters(self):
        # test auto refresh getters/setters
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        self.assertFalse(layer.hasAutoRefreshEnabled())
        self.assertEqual(layer.autoRefreshInterval(), 0)
        layer.setAutoRefreshInterval(5)
        self.assertFalse(layer.hasAutoRefreshEnabled())
        self.assertEqual(layer.autoRefreshInterval(), 5)
        layer.setAutoRefreshEnabled(True)
        self.assertTrue(layer.hasAutoRefreshEnabled())
        self.assertEqual(layer.autoRefreshInterval(), 5)
        layer.setAutoRefreshInterval(0)  # should disable auto refresh
        self.assertFalse(layer.hasAutoRefreshEnabled())
        self.assertEqual(layer.autoRefreshInterval(), 0)

    def test_crs(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        spy = QSignalSpy(layer.crsChanged)
        layer.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(len(spy), 1)
        self.assertFalse(layer.crs().isValid())
        layer.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(len(spy), 1)

        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(layer.crs().authid(), "EPSG:3111")
        self.assertEqual(len(spy), 2)
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(len(spy), 2)

        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertEqual(layer2.crs().authid(), "EPSG:3111")

    def test_vertical_crs(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        layer.setCrs(QgsCoordinateReferenceSystem())
        self.assertFalse(layer.verticalCrs().isValid())

        spy = QSignalSpy(layer.verticalCrsChanged)
        # not a vertical crs
        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertFalse(ok)
        self.assertEqual(err, "Specified CRS is a Projected CRS, not a Vertical CRS")
        self.assertFalse(layer.verticalCrs().isValid())

        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5703"))
        self.assertTrue(ok)
        self.assertEqual(layer.verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy), 1)
        # try overwriting with same crs, should be no new signal
        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5703"))
        self.assertTrue(ok)
        self.assertEqual(len(spy), 1)

        # check that vertical crs is saved/restored
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        spy2 = QSignalSpy(layer2.verticalCrsChanged)
        self.copyLayerViaXmlReadWrite(layer, layer2)

        self.assertEqual(layer2.verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy2), 1)
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertEqual(layer2.verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy2), 1)

        layer.setVerticalCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(len(spy), 2)
        self.assertFalse(layer.verticalCrs().isValid())

    def test_vertical_crs_with_compound_project_crs(self):
        """
        Test vertical crs logic when layer has a compound crs set
        """
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        layer.setCrs(QgsCoordinateReferenceSystem())
        self.assertFalse(layer.crs().isValid())
        self.assertFalse(layer.verticalCrs().isValid())

        spy = QSignalSpy(layer.verticalCrsChanged)
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:5500"))
        self.assertEqual(layer.crs().authid(), "EPSG:5500")
        # verticalCrs() should return the vertical part of the
        # compound CRS
        self.assertEqual(layer.verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy), 1)
        other_vert_crs = QgsCoordinateReferenceSystem("ESRI:115700")
        self.assertTrue(other_vert_crs.isValid())
        self.assertEqual(other_vert_crs.type(), Qgis.CrsType.Vertical)

        # if we explicitly set a vertical crs now, it should be ignored
        # because the main project crs is a compound crs and that takes
        # precedence
        ok, err = layer.setVerticalCrs(other_vert_crs)
        self.assertFalse(ok)
        self.assertEqual(
            err, "Layer CRS is a Compound CRS, specified Vertical CRS will be ignored"
        )
        self.assertEqual(layer.verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy), 1)
        # setting the vertical crs to the vertical component of the compound crs
        # IS permitted, even though it effectively has no impact...
        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5703"))
        self.assertTrue(ok)
        self.assertEqual(layer.verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy), 1)

        # reset horizontal crs to a non-compound crs, now the manually
        # specified vertical crs should take precedence
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(layer.verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy), 1)

        # invalid combinations
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:4979"))
        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5711"))
        self.assertFalse(ok)
        self.assertEqual(
            err,
            "Layer CRS is a Geographic 3D CRS, specified Vertical CRS will be ignored",
        )
        self.assertEqual(layer.crs3D().authid(), "EPSG:4979")

        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:4978"))
        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5711"))
        self.assertFalse(ok)
        self.assertEqual(
            err, "Layer CRS is a Geocentric CRS, specified Vertical CRS will be ignored"
        )
        self.assertEqual(layer.crs3D().authid(), "EPSG:4978")

    def test_vertical_crs_with_projected3d_project_crs(self):
        """
        Test vertical crs logic when layer has a projected 3d crs set
        """
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        layer.setCrs(QgsCoordinateReferenceSystem())
        self.assertFalse(layer.crs().isValid())
        self.assertFalse(layer.verticalCrs().isValid())

        spy = QSignalSpy(layer.verticalCrsChanged)

        projected3d_crs = QgsCoordinateReferenceSystem.fromWkt(
            'PROJCRS["NAD83(HARN) / Oregon GIC Lambert (ft)",\n'
            '    BASEGEOGCRS["NAD83(HARN)",\n'
            '        DATUM["NAD83 (High Accuracy Reference Network)",\n'
            '            ELLIPSOID["GRS 1980",6378137,298.257222101,\n'
            '                LENGTHUNIT["metre",1]]],\n'
            '        PRIMEM["Greenwich",0,\n'
            '            ANGLEUNIT["degree",0.0174532925199433]],\n'
            '        ID["EPSG",4957]],\n'
            '    CONVERSION["unnamed",\n'
            '        METHOD["Lambert Conic Conformal (2SP)",\n'
            '            ID["EPSG",9802]],\n'
            '        PARAMETER["Latitude of false origin",41.75,\n'
            '            ANGLEUNIT["degree",0.0174532925199433],\n'
            '            ID["EPSG",8821]],\n'
            '        PARAMETER["Longitude of false origin",-120.5,\n'
            '            ANGLEUNIT["degree",0.0174532925199433],\n'
            '            ID["EPSG",8822]],\n'
            '        PARAMETER["Latitude of 1st standard parallel",43,\n'
            '            ANGLEUNIT["degree",0.0174532925199433],\n'
            '            ID["EPSG",8823]],\n'
            '        PARAMETER["Latitude of 2nd standard parallel",45.5,\n'
            '            ANGLEUNIT["degree",0.0174532925199433],\n'
            '            ID["EPSG",8824]],\n'
            '        PARAMETER["Easting at false origin",1312335.958,\n'
            '            LENGTHUNIT["foot",0.3048],\n'
            '            ID["EPSG",8826]],\n'
            '        PARAMETER["Northing at false origin",0,\n'
            '            LENGTHUNIT["foot",0.3048],\n'
            '            ID["EPSG",8827]]],\n'
            "    CS[Cartesian,3],\n"
            '        AXIS["easting",east,\n'
            "            ORDER[1],\n"
            '            LENGTHUNIT["foot",0.3048]],\n'
            '        AXIS["northing",north,\n'
            "            ORDER[2],\n"
            '            LENGTHUNIT["foot",0.3048]],\n'
            '        AXIS["ellipsoidal height (h)",up,\n'
            "            ORDER[3],\n"
            '            LENGTHUNIT["foot",0.3048]]]'
        )
        self.assertTrue(projected3d_crs.isValid())
        layer.setCrs(projected3d_crs)
        self.assertEqual(layer.crs().toWkt(), projected3d_crs.toWkt())
        # layer 3d crs should be projected 3d crs
        self.assertEqual(layer.crs3D().toWkt(), projected3d_crs.toWkt())
        # verticalCrs() should return invalid crs
        self.assertFalse(layer.verticalCrs().isValid())
        self.assertEqual(len(spy), 0)
        other_vert_crs = QgsCoordinateReferenceSystem("ESRI:115700")
        self.assertTrue(other_vert_crs.isValid())
        self.assertEqual(other_vert_crs.type(), Qgis.CrsType.Vertical)

        # if we explicitly set a vertical crs now, it should be ignored
        # because the main layer crs is already 3d and that takes
        # precedence
        ok, err = layer.setVerticalCrs(other_vert_crs)
        self.assertFalse(ok)
        self.assertEqual(
            err,
            "Layer CRS is a Projected 3D CRS, specified Vertical CRS will be ignored",
        )
        self.assertFalse(layer.verticalCrs().isValid())
        self.assertEqual(len(spy), 0)
        self.assertEqual(layer.crs3D().toWkt(), projected3d_crs.toWkt())

    def test_crs_3d(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        layer.setCrs(QgsCoordinateReferenceSystem())
        self.assertFalse(layer.crs3D().isValid())

        spy = QSignalSpy(layer.crs3DChanged)

        # set layer crs to a 2d crs
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))

        self.assertEqual(layer.crs3D().authid(), "EPSG:3111")
        self.assertEqual(len(spy), 1)

        # don't change, no new signals
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(layer.crs3D().authid(), "EPSG:3111")
        self.assertEqual(len(spy), 1)

        # change 2d crs, should be new signals
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:3113"))
        self.assertEqual(layer.crs3D().authid(), "EPSG:3113")
        self.assertEqual(len(spy), 2)

        # change vertical crs:

        # not a vertical crs, no change
        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertFalse(ok)
        self.assertEqual(layer.crs3D().authid(), "EPSG:3113")
        self.assertEqual(len(spy), 2)

        # valid vertical crs
        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5703"))
        self.assertTrue(ok)
        self.assertEqual(layer.crs3D().type(), Qgis.CrsType.Compound)
        # crs3D should be a compound crs
        self.assertEqual(layer.crs3D().horizontalCrs().authid(), "EPSG:3113")
        self.assertEqual(layer.crs3D().verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy), 3)
        # try overwriting with same crs, should be no new signal
        ok, err = layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5703"))
        self.assertTrue(ok)
        self.assertEqual(len(spy), 3)

        # set 2d crs to a compound crs
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:5500"))
        self.assertEqual(layer.crs().authid(), "EPSG:5500")
        self.assertEqual(layer.crs3D().authid(), "EPSG:5500")
        self.assertEqual(len(spy), 4)

        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:5500"))
        self.assertEqual(layer.crs().authid(), "EPSG:5500")
        self.assertEqual(layer.crs3D().authid(), "EPSG:5500")
        self.assertEqual(len(spy), 4)

        # remove vertical crs, should be no change because compound crs is causing vertical crs to be ignored
        layer.setVerticalCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(layer.crs3D().authid(), "EPSG:5500")
        self.assertEqual(len(spy), 4)

        layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5703"))
        self.assertEqual(layer.crs3D().authid(), "EPSG:5500")
        self.assertEqual(len(spy), 4)

        # set crs back to 2d crs, should be new signal
        layer.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(layer.crs3D().horizontalCrs().authid(), "EPSG:3111")
        self.assertEqual(layer.crs3D().verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy), 5)

        # check that crs3D is handled correctly during save/restore
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")

        spy2 = QSignalSpy(layer2.crs3DChanged)
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertEqual(layer2.crs3D().horizontalCrs().authid(), "EPSG:3111")
        self.assertEqual(layer2.crs3D().verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy2), 1)
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertEqual(layer2.crs3D().horizontalCrs().authid(), "EPSG:3111")
        self.assertEqual(layer2.crs3D().verticalCrs().authid(), "EPSG:5703")
        self.assertEqual(len(spy2), 1)

    def testLayerNotes(self):
        """
        Test layer notes
        """
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        self.assertFalse(QgsLayerNotesUtils.layerHasNotes(layer))
        self.assertFalse(QgsLayerNotesUtils.layerNotes(layer))

        QgsLayerNotesUtils.setLayerNotes(layer, "my notes")
        self.assertTrue(QgsLayerNotesUtils.layerHasNotes(layer))
        self.assertEqual(QgsLayerNotesUtils.layerNotes(layer), "my notes")
        QgsLayerNotesUtils.setLayerNotes(layer, "my notes 2")
        self.assertEqual(QgsLayerNotesUtils.layerNotes(layer), "my notes 2")

        QgsLayerNotesUtils.removeNotes(layer)
        self.assertFalse(QgsLayerNotesUtils.layerHasNotes(layer))
        self.assertFalse(QgsLayerNotesUtils.layerNotes(layer))

    def testSaveRestoreAutoRefresh(self):
        """test saving/restoring auto refresh to xml"""
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertFalse(layer2.hasAutoRefreshEnabled())
        self.assertEqual(layer2.autoRefreshInterval(), 0)

        layer.setAutoRefreshInterval(56)
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertFalse(layer2.hasAutoRefreshEnabled())
        self.assertEqual(layer2.autoRefreshInterval(), 56)

        layer.setAutoRefreshEnabled(True)
        self.copyLayerViaXmlReadWrite(layer, layer2)
        self.assertTrue(layer2.hasAutoRefreshEnabled())
        self.assertEqual(layer2.autoRefreshInterval(), 56)

    def testReadWriteMetadata(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        m = layer.metadata()
        # Only abstract, more tests are done in test_qgslayermetadata.py
        m.setAbstract("My abstract")
        layer.setMetadata(m)
        self.assertTrue(layer.metadata().abstract(), "My abstract")
        destination = tempfile.NamedTemporaryFile(suffix=".qmd").name
        message, status = layer.saveNamedMetadata(destination)
        self.assertTrue(status, message)

        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        message, status = layer2.loadNamedMetadata(destination)
        self.assertTrue(status)
        self.assertTrue(layer2.metadata().abstract(), "My abstract")

    def testSaveNamedStyle(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        dir = QTemporaryDir()
        dir_path = dir.path()
        style_path = os.path.join(dir_path, "my.qml")
        _, result = layer.saveNamedStyle(style_path)
        self.assertTrue(result)
        self.assertTrue(os.path.exists(style_path))

    def testStyleUri(self):
        # shapefile
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "layer", "ogr"
        )
        uri = layer.styleURI()
        self.assertEqual(uri, os.path.join(TEST_DATA_DIR, "points.qml"))

        # geopackage without and with layername
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "provider", "bug_17795.gpkg"), "layer", "ogr"
        )
        uri = layer.styleURI()
        self.assertEqual(uri, os.path.join(TEST_DATA_DIR, "provider", "bug_17795.qml"))

        layer = QgsVectorLayer(
            f"{os.path.join(TEST_DATA_DIR, 'provider', 'bug_17795.gpkg')}|layername=bug_17795",
            "layer",
            "ogr",
        )
        uri = layer.styleURI()
        self.assertEqual(uri, os.path.join(TEST_DATA_DIR, "provider", "bug_17795.qml"))

        # delimited text
        uri = f"file://{os.path.join(TEST_DATA_DIR, 'delimitedtext', 'test.csv')}?type=csv&detectTypes=yes&geomType=none"
        layer = QgsVectorLayer(uri, "layer", "delimitedtext")
        uri = layer.styleURI()
        self.assertEqual(uri, os.path.join(TEST_DATA_DIR, "delimitedtext", "test.qml"))

    def testIsTemporary(self):
        # test if a layer is correctly marked as temporary
        dir = QTemporaryDir()
        dir_path = dir.path()
        for file in glob.glob(os.path.join(TEST_DATA_DIR, "france_parts.*")):
            shutil.copy(os.path.join(TEST_DATA_DIR, file), dir_path)

        not_temp_source = os.path.join(TEST_DATA_DIR, "france_parts.*")
        temp_source = os.path.join(dir_path, "france_parts.shp")

        vl = QgsVectorLayer("invalid", "test")
        self.assertFalse(vl.isValid())
        self.assertFalse(vl.isTemporary())

        vl = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "france_parts.shp"), "test")
        self.assertTrue(vl.isValid())
        self.assertFalse(vl.isTemporary())

        vl = QgsVectorLayer(temp_source, "test")
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.isTemporary())

        # memory layers are temp
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.isTemporary())

        rl = QgsRasterLayer("invalid", "test")
        self.assertFalse(rl.isValid())
        self.assertFalse(rl.isTemporary())

        not_temp_source = os.path.join(TEST_DATA_DIR, "float1-16.tif")
        shutil.copy(not_temp_source, dir_path)
        temp_source = os.path.join(dir_path, "float1-16.tif")

        rl = QgsRasterLayer(not_temp_source, "test")
        self.assertTrue(rl.isValid())
        self.assertFalse(rl.isTemporary())

        rl = QgsRasterLayer(temp_source, "test")
        self.assertTrue(rl.isValid())
        self.assertTrue(rl.isTemporary())

    def testQgsMapLayerProject(self):
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "layer", "ogr"
        )
        self.assertIsNone(layer.project())
        project = QgsProject()
        project.addMapLayer(layer)
        self.assertEqual(layer.project(), project)
        project2 = QgsProject()
        project2.addMapLayer(layer)
        self.assertEqual(layer.project(), project2)
        project.removeMapLayer(layer)
        self.assertFalse(sip.isdeleted(layer))
        project2.removeMapLayer(layer)
        self.assertTrue(sip.isdeleted(layer))

    def testRetainLayerMetadataWhenChangingDataSource(self):
        """
        Test that we retain existing layer metadata when a layer's source is changed
        """
        vl = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "points.shp"), "layer", "ogr")

        metadata = QgsLayerMetadata()
        metadata.setRights(["original right 1", "original right 2"])
        metadata.setAbstract("original abstract")
        vl.setMetadata(metadata)

        # now change layer datasource to one which has embedded provider medata
        datasource = os.path.join(unitTestDataPath(), "gdb_metadata.gdb")
        vl.setDataSource(datasource, "test", "ogr")
        self.assertTrue(vl.isValid())

        # these settings weren't present in the original layer metadata, so should have been taken from the GDB file
        self.assertEqual(vl.metadata().identifier(), "Test")
        self.assertEqual(vl.metadata().title(), "Title")
        self.assertEqual(vl.metadata().type(), "dataset")
        self.assertEqual(vl.metadata().language(), "ENG")
        self.assertEqual(vl.metadata().keywords(), {"Search keys": ["Tags"]})
        self.assertEqual(vl.metadata().constraints()[0].type, "Limitations of use")
        self.assertEqual(
            vl.metadata().constraints()[0].constraint, "This is the use limitation"
        )
        self.assertEqual(
            vl.metadata().extent().spatialExtents()[0].bounds.xMinimum(), 1
        )
        self.assertEqual(
            vl.metadata().extent().spatialExtents()[0].bounds.xMaximum(), 2
        )
        self.assertEqual(
            vl.metadata().extent().spatialExtents()[0].bounds.yMinimum(), 3
        )
        self.assertEqual(
            vl.metadata().extent().spatialExtents()[0].bounds.yMaximum(), 4
        )

        # these setting WERE present, so must be retained
        self.assertIn("original abstract", vl.metadata().abstract())
        self.assertEqual(
            vl.metadata().rights(), ["original right 1", "original right 2"]
        )

    def testMapTips(self):
        rl = QgsRasterLayer(os.path.join(TEST_DATA_DIR, "float1-16.tif"), "test")
        self.assertFalse(rl.hasMapTips())

        rl.setMapTipTemplate("some template")
        self.assertEqual(rl.mapTipTemplate(), "some template")
        self.assertTrue(rl.hasMapTips())

        rl.setMapTipTemplate(None)
        self.assertFalse(rl.mapTipTemplate())
        self.assertFalse(rl.hasMapTips())

    def testError(self):
        """
        Test error reporting methods
        """
        vl = QgsVectorLayer("ErrorString", "test", "error")
        self.assertFalse(vl.isValid())
        self.assertEqual(vl.error().summary(), "")


if __name__ == "__main__":
    unittest.main()
