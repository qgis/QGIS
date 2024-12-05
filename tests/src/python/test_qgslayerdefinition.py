"""QGIS Unit tests for QgsLayerDefinition

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Hugo Mercier"
__date__ = "07/01/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

import os
import shutil

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import Qgis, QgsLayerDefinition, QgsProject, QgsVectorLayer
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayerDefinition(QgisTestCase):

    def testDependency(self):
        inDoc = """
        <maplayers>
        <maplayer>
          <id>layerB</id>
          <layerDependencies>
            <layer id="layerA"/>
          </layerDependencies>
        </maplayer>
        <maplayer>
          <id>layerA</id>
        </maplayer>
        </maplayers>"""
        doc = QDomDocument("testdoc")
        doc.setContent(inDoc)
        dep = QgsLayerDefinition.DependencySorter(doc)
        nodes = dep.sortedLayerNodes()
        nodeIds = dep.sortedLayerIds()
        self.assertTrue(not dep.hasCycle())
        self.assertTrue(not dep.hasMissingDependency())
        self.assertEqual(nodes[0].firstChildElement("id").text(), "layerA")
        self.assertEqual(nodes[1].firstChildElement("id").text(), "layerB")
        self.assertEqual(nodeIds[0], "layerA")
        self.assertEqual(nodeIds[1], "layerB")

    def testDependencyQgz(self):
        path = os.path.join(TEST_DATA_DIR, "embedded_groups", "project1.qgz")
        dep = QgsLayerDefinition.DependencySorter(path)
        ids = dep.sortedLayerIds()
        self.assertEqual(len(ids), 3)

    def testMissingDependency(self):
        inDoc = """
        <maplayers>
        <maplayer>
          <id>layerB</id>
          <layerDependencies>
            <layer id="layerA"/>
          </layerDependencies>
        </maplayer>
        <maplayer>
          <id>layerA</id>
          <layerDependencies>
            <layer id="layerC"/>
          </layerDependencies>
        </maplayer>
        </maplayers>"""
        doc = QDomDocument("testdoc")
        doc.setContent(inDoc)
        dep = QgsLayerDefinition.DependencySorter(doc)
        self.assertTrue(not dep.hasCycle())
        self.assertTrue(dep.hasMissingDependency())

    def testCyclicDependency(self):
        inDoc = """
        <maplayers>
        <maplayer>
          <id>layerB</id>
          <layerDependencies>
            <layer id="layerA"/>
          </layerDependencies>
        </maplayer>
        <maplayer>
          <id>layerA</id>
          <layerDependencies>
            <layer id="layerB"/>
          </layerDependencies>
        </maplayer>
        </maplayers>"""
        doc = QDomDocument("testdoc")
        doc.setContent(inDoc)
        dep = QgsLayerDefinition.DependencySorter(doc)
        self.assertTrue(dep.hasCycle())

    def testVectorAndRaster(self):
        # Load a simple QLR containing a vector layer and a raster layer.
        QgsProject.instance().removeAllMapLayers()
        layers = QgsProject.instance().mapLayers()
        self.assertEqual(len(layers), 0)

        (result, errMsg) = QgsLayerDefinition.loadLayerDefinition(
            TEST_DATA_DIR + "/vector_and_raster.qlr",
            QgsProject.instance(),
            QgsProject.instance().layerTreeRoot(),
        )
        self.assertTrue(result)

        layers = QgsProject.instance().mapLayers()
        self.assertEqual(len(layers), 2)
        QgsProject.instance().removeAllMapLayers()

    def testInvalidSource(self):
        # Load a QLR containing a vector layer with a broken path
        QgsProject.instance().removeAllMapLayers()
        layers = QgsProject.instance().mapLayers()
        self.assertEqual(len(layers), 0)

        (result, errMsg) = QgsLayerDefinition.loadLayerDefinition(
            TEST_DATA_DIR + "/invalid_source.qlr",
            QgsProject.instance(),
            QgsProject.instance().layerTreeRoot(),
        )
        self.assertTrue(result)
        self.assertFalse(errMsg)

        layers = QgsProject.instance().mapLayers()
        self.assertEqual(len(layers), 1)
        self.assertFalse(list(layers.values())[0].isValid())
        QgsProject.instance().removeAllMapLayers()

    def test_path_storage(self):
        """
        Test storage of relative/absolute paths
        """
        temp_dir = QTemporaryDir()
        gpkg_path = temp_dir.filePath("points_gpkg.gpkg")
        shutil.copy(TEST_DATA_DIR + "/points_gpkg.gpkg", gpkg_path)

        p = QgsProject()
        vl = QgsVectorLayer(gpkg_path)
        self.assertTrue(vl.isValid())
        p.addMapLayer(vl)

        # write qlr with relative paths
        ok, err = QgsLayerDefinition.exportLayerDefinition(
            temp_dir.filePath("relative.qlr"),
            [p.layerTreeRoot()],
            Qgis.FilePathType.Relative,
        )
        self.assertTrue(ok)

        with open(temp_dir.filePath("relative.qlr")) as f:
            lines = f.readlines()
        self.assertIn('source="./points_gpkg.gpkg"', "\n".join(lines))

        # write qlr with absolute paths
        ok, err = QgsLayerDefinition.exportLayerDefinition(
            temp_dir.filePath("absolute.qlr"),
            [p.layerTreeRoot()],
            Qgis.FilePathType.Absolute,
        )
        self.assertTrue(ok)

        with open(temp_dir.filePath("absolute.qlr")) as f:
            lines = f.readlines()
        self.assertIn(f'source="{gpkg_path}"', "\n".join(lines))

    def testWidgetConfig(self):

        temp = QTemporaryDir()
        temp_path = temp.path()
        temp_qlr = os.path.join(temp_path, "widget_config.qlr")

        qlr = """<!DOCTYPE qgis-layer-definition>
        <qlr>
        <layer-tree-group expanded="1" checked="Qt::Checked" name="">
            <customproperties/>
            <layer-tree-group name="group">
                <customproperties/>
                <layer-tree-layer providerKey="memory" name="OldMemory" source="NoGeometry?crs=EPSG:4326&amp;field=BA_ART_LAT:string&amp;uid={908cbaab-cabe-4bd9-9058-2b80ebf87a72}" id="OldMemory_ffa4d8a4_e5be_46a9_a0e0_fb6ee924f3cd" />
                <layer-tree-layer providerKey="memory" name="NewMemory" source="NoGeometry?crs=EPSG:4326&amp;field=BA_ART_LAT:string&amp;uid={64c9bcbf-cabe-4bd9-9058-2b80ebf87a72}" id="NewMemory_ffa4d8a4_e5be_46a9_a0e0_fb6ee924f3cd" />
                <customproperties/>
            </layer-tree-group>
        </layer-tree-group>
        <maplayers>
            <maplayer type="vector" autoRefreshTime="0" geometry="No geometry">
                <id>OldMemory_ffa4d8a4_e5be_46a9_a0e0_fb6ee924f3cd</id>
                <datasource>memory?geometry=NoGeometry&amp;field=BA_ART_LAT:string&amp;crs=EPSG:4326</datasource>
                <keywordList>
                    <value></value>
                </keywordList>
                <layername>OldMemory</layername>
                <srs>
                    <spatialrefsys>
                    <proj4>+proj=longlat +datum=WGS84 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                    </spatialrefsys>
                </srs>
                <provider encoding="UTF-8">memory</provider>
            </maplayer>

            <maplayer type="vector"geometry="No geometry">
            <id>NewMemory_ffa4d8a4_e5be_46a9_a0e0_fb6ee924f3cd</id>
            <datasource>memory?geometry=NoGeometry&amp;field=BA_ART_LAT:string&amp;crs=EPSG:4326</datasource>
            <layername>NewMemory</layername>
            <srs>
                <spatialrefsys>
                <proj4>+proj=longlat +datum=WGS84 +no_defs</proj4>
                <srsid>3452</srsid>
                <srid>4326</srid>
                <authid>EPSG:4326</authid>
                <description>WGS 84</description>
                <projectionacronym>longlat</projectionacronym>
                <ellipsoidacronym>WGS84</ellipsoidacronym>
                <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <provider encoding="UTF-8">memory</provider>
            <fieldConfiguration>
                <field name="BA_ART_LAT" configurationFlags="None">
                <editWidget type="ValueRelation">
                    <config>
                    <Option type="Map">
                        <Option value="false" name="AllowMulti" type="bool"/>
                        <Option value="false" name="AllowNull" type="bool"/>
                        <Option value="" name="Description" type="QString"/>
                        <Option value="" name="FilterExpression" type="QString"/>
                        <Option value="Wert" name="Key" type="QString"/>
                        <Option value="OldMemory_ffa4d8a4_e5be_46a9_a0e0_fb6ee924f3cd" name="Layer" type="QString"/>
                        <Option value="SL_BA_ART_LAT" name="LayerName" type="QString"/>
                        <Option value="ogr" name="LayerProviderName" type="QString"/>
                        <Option value="NoGeometry?crs=EPSG:4326&amp;field=BA_ART_LAT:string&amp;uid={908cbaab-cabe-4bd9-9058-2b80ebf87a72}" name="LayerSource" type="QString"/>
                        <Option value="1" name="NofColumns" type="int"/>
                        <Option value="false" name="OrderByValue" type="bool"/>
                        <Option value="false" name="UseCompleter" type="bool"/>
                        <Option value="Bezeichnung" name="Value" type="QString"/>
                    </Option>
                    </config>
                </editWidget>
                </field>
            </fieldConfiguration>
            </maplayer>
        </maplayers>
        </qlr>
        """

        with open(temp_qlr, "w+") as f:
            f.write(qlr)

        (result, errMsg) = QgsLayerDefinition.loadLayerDefinition(
            temp_qlr, QgsProject.instance(), QgsProject.instance().layerTreeRoot()
        )
        self.assertTrue(result)
        self.assertFalse(errMsg)

        vl = QgsProject.instance().mapLayersByName("NewMemory")[0]
        field = vl.fields().at(0)
        config = field.editorWidgetSetup().config()

        self.assertFalse(config["Description"])
        self.assertFalse(config["FilterExpression"])


if __name__ == "__main__":
    unittest.main()
