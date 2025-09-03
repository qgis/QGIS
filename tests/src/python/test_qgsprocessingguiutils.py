"""QGIS Unit tests for Processing GUI utils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from processing.core.Processing import Processing, ProcessingConfig
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    QgsProject,
    QgsSettings,
    QgsLayerTreeLayer,
    QgsRasterLayer,
    QgsVectorLayer,
    QgsProcessingContext,
    QgsLayerTreeGroup,
    QgsLayerTreeModel,
)
from qgis.gui import QgsProcessingGuiUtils, QgsLayerTreeView
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsProcessingGuiUtils(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsProcessingGuiUtils.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsProcessingGuiUtils")
        QgsSettings().clear()
        Processing.initialize()

    def test_configure_layer_tree_layer(self):
        ProcessingConfig.setSettingValue(ProcessingConfig.VECTOR_FEATURE_COUNT, False)
        vl = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test",
            "memory",
        )
        self.assertTrue(vl.isValid())
        rl = QgsRasterLayer(
            self.get_test_data_path("landsat_4326.tif").as_posix(), "test"
        )
        self.assertTrue(rl.isValid())

        project = QgsProject()
        project.addMapLayer(vl)
        project.addMapLayer(rl)

        vector_layer_tree_layer = project.layerTreeRoot().findLayer(vl)
        QgsProcessingGuiUtils.configureResultLayerTreeLayer(vector_layer_tree_layer)
        self.assertFalse(vector_layer_tree_layer.customProperty("showFeatureCount"))

        raster_layer_tree_layer = project.layerTreeRoot().findLayer(rl)
        QgsProcessingGuiUtils.configureResultLayerTreeLayer(raster_layer_tree_layer)
        self.assertFalse(raster_layer_tree_layer.customProperty("showFeatureCount"))

        ProcessingConfig.setSettingValue(ProcessingConfig.VECTOR_FEATURE_COUNT, True)
        QgsProcessingGuiUtils.configureResultLayerTreeLayer(vector_layer_tree_layer)
        self.assertTrue(vector_layer_tree_layer.customProperty("showFeatureCount"))
        QgsProcessingGuiUtils.configureResultLayerTreeLayer(raster_layer_tree_layer)
        self.assertFalse(raster_layer_tree_layer.customProperty("showFeatureCount"))

    def test_layer_tree_result_group(self):
        project1 = QgsProject()
        project2 = QgsProject()

        context = QgsProcessingContext()
        context.setProject(project1)

        # no processing setting for result group
        ProcessingConfig.setSettingValue(ProcessingConfig.RESULTS_GROUP_NAME, None)

        # no layer destination group
        layer_details = QgsProcessingContext.LayerDetails(
            "name", project2, "output_name"
        )
        self.assertIsNone(
            QgsProcessingGuiUtils.layerTreeResultsGroup(layer_details, context)
        )
        # no group should be created
        self.assertFalse(project1.layerTreeRoot().children())
        self.assertFalse(project2.layerTreeRoot().children())

        layer_details = QgsProcessingContext.LayerDetails(
            "name", project2, "output_name"
        )
        layer_details.groupName = "new group"
        new_group = QgsProcessingGuiUtils.layerTreeResultsGroup(layer_details, context)
        self.assertIsInstance(new_group, QgsLayerTreeGroup)
        self.assertEqual(new_group.name(), "new group")

        # group should be created in project2, as that was specified in the LayerDetails
        self.assertFalse(project1.layerTreeRoot().children())
        self.assertEqual(project2.layerTreeRoot().children(), [new_group])

        # no project, so should use project from context
        layer_details = QgsProcessingContext.LayerDetails("name", None, "output_name")
        layer_details.groupName = "new group2"
        new_group2 = QgsProcessingGuiUtils.layerTreeResultsGroup(layer_details, context)
        self.assertIsInstance(new_group2, QgsLayerTreeGroup)
        self.assertEqual(new_group2.name(), "new group2")
        # group should be created in project1, as that's specified in the context
        self.assertEqual(project1.layerTreeRoot().children(), [new_group2])
        self.assertEqual(project2.layerTreeRoot().children(), [new_group])

        # with processing setting for result group
        ProcessingConfig.setSettingValue(
            ProcessingConfig.RESULTS_GROUP_NAME, "parent group"
        )

        layer_details = QgsProcessingContext.LayerDetails(
            "name", project2, "output_name"
        )
        parent_group1 = QgsProcessingGuiUtils.layerTreeResultsGroup(
            layer_details, context
        )
        self.assertIsInstance(parent_group1, QgsLayerTreeGroup)
        self.assertEqual(parent_group1.name(), "parent group")
        self.assertCountEqual(
            project2.layerTreeRoot().children(), [new_group, parent_group1]
        )
        self.assertFalse(parent_group1.children())
        # same group should be used again if it already exists
        layer_details = QgsProcessingContext.LayerDetails(
            "name", project2, "output_name"
        )
        self.assertEqual(
            QgsProcessingGuiUtils.layerTreeResultsGroup(layer_details, context),
            parent_group1,
        )

        # group in LayerDetails, should be a sub group of the parent group from Processing settings
        layer_details = QgsProcessingContext.LayerDetails(
            "name", project2, "output_name"
        )
        layer_details.groupName = "child group"
        child_group = QgsProcessingGuiUtils.layerTreeResultsGroup(
            layer_details, context
        )
        self.assertIsInstance(child_group, QgsLayerTreeGroup)
        self.assertEqual(child_group.name(), "child group")
        self.assertCountEqual(
            project2.layerTreeRoot().children(), [new_group, parent_group1]
        )
        self.assertEqual(parent_group1.children(), [child_group])
        # same group should be used again if it already exists
        self.assertEqual(
            QgsProcessingGuiUtils.layerTreeResultsGroup(layer_details, context),
            child_group,
        )

    def test_add_result_layers(self):
        ProcessingConfig.setSettingValue(ProcessingConfig.VECTOR_FEATURE_COUNT, False)
        QgsProject.instance().clear()
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        view = QgsLayerTreeView()
        model = QgsLayerTreeModel(QgsProject.instance().layerTreeRoot(), view)
        view.setModel(model)

        # empty project, layers should be added respecting sort order
        # "Layers with a greater sort key will be placed over layers with a lesser sort key"
        vl1 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test1",
            "memory",
        )
        details1 = QgsProcessingGuiUtils.ResultLayerDetails(vl1)
        details1.destinationProject = QgsProject.instance()
        details1.sortKey = 5

        vl2 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test2",
            "memory",
        )
        details2 = QgsProcessingGuiUtils.ResultLayerDetails(vl2)
        details2.destinationProject = QgsProject.instance()
        details2.sortKey = 1

        vl3 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test3",
            "memory",
        )
        details3 = QgsProcessingGuiUtils.ResultLayerDetails(vl3)
        details3.destinationProject = QgsProject.instance()
        details3.sortKey = 15

        QgsProcessingGuiUtils.addResultLayers(
            [details1, details2, details3], context, view
        )

        self.assertCountEqual(
            QgsProject.instance().mapLayers().values(), [vl1, vl2, vl3]
        )

        self.assertEqual(
            [node.layer() for node in QgsProject.instance().layerTreeRoot().children()],
            [vl3, vl1, vl2],
        )
        self.assertEqual(view.currentNode().layer(), vl2)

        # no layer count for these layers
        self.assertFalse(
            QgsProject.instance()
            .layerTreeRoot()
            .findLayer(vl1)
            .customProperty("showFeatureCount")
        )
        self.assertFalse(
            QgsProject.instance()
            .layerTreeRoot()
            .findLayer(vl2)
            .customProperty("showFeatureCount")
        )
        self.assertFalse(
            QgsProject.instance()
            .layerTreeRoot()
            .findLayer(vl3)
            .customProperty("showFeatureCount")
        )

        # select an active layer in tree (vl1), output layers should be placed above this
        view.setCurrentNode(QgsProject.instance().layerTreeRoot().findLayer(vl1))

        vl4 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test4",
            "memory",
        )
        details4 = QgsProcessingGuiUtils.ResultLayerDetails(vl4)
        details4.destinationProject = QgsProject.instance()
        details4.sortKey = 1
        vl5 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test5",
            "memory",
        )
        details5 = QgsProcessingGuiUtils.ResultLayerDetails(vl5)
        details5.destinationProject = QgsProject.instance()
        details5.sortKey = 2

        QgsProcessingGuiUtils.addResultLayers([details4, details5], context, view)

        self.assertCountEqual(
            QgsProject.instance().mapLayers().values(), [vl1, vl2, vl3, vl4, vl5]
        )

        self.assertEqual(
            [node.layer() for node in QgsProject.instance().layerTreeRoot().children()],
            [vl3, vl5, vl4, vl1, vl2],
        )
        self.assertEqual(view.currentNode().layer(), vl4)

        # with a group selected
        group1 = QgsProject.instance().layerTreeRoot().addGroup("group1")
        view.setCurrentNode(group1)

        vl6 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test6",
            "memory",
        )
        details6 = QgsProcessingGuiUtils.ResultLayerDetails(vl6)
        details6.destinationProject = QgsProject.instance()
        details6.sortKey = 1
        vl7 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test7",
            "memory",
        )
        details7 = QgsProcessingGuiUtils.ResultLayerDetails(vl7)
        details7.destinationProject = QgsProject.instance()
        details7.sortKey = 2

        QgsProcessingGuiUtils.addResultLayers([details6, details7], context, view)

        self.assertCountEqual(
            QgsProject.instance().mapLayers().values(),
            [vl1, vl2, vl3, vl4, vl5, vl6, vl7],
        )
        self.assertEqual(
            [
                node.layer()
                for node in QgsProject.instance().layerTreeRoot().children()
                if isinstance(node, QgsLayerTreeLayer)
            ],
            [vl3, vl5, vl4, vl1, vl2],
        )
        self.assertEqual([node.layer() for node in group1.children()], [vl7, vl6])
        self.assertEqual(view.currentNode().layer(), vl6)

        vl8 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test8",
            "memory",
        )
        details8 = QgsProcessingGuiUtils.ResultLayerDetails(vl8)
        details8.destinationProject = QgsProject.instance()
        details8.sortKey = 2
        vl9 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test9",
            "memory",
        )
        details9 = QgsProcessingGuiUtils.ResultLayerDetails(vl9)
        details9.destinationProject = QgsProject.instance()
        details9.sortKey = 1

        # if group is selected, layers should be added at top of group
        view.setCurrentNode(group1)
        QgsProcessingGuiUtils.addResultLayers([details8, details9], context, view)

        self.assertCountEqual(
            QgsProject.instance().mapLayers().values(),
            [vl1, vl2, vl3, vl4, vl5, vl6, vl7, vl8, vl9],
        )
        self.assertEqual(
            [
                node.layer()
                for node in QgsProject.instance().layerTreeRoot().children()
                if isinstance(node, QgsLayerTreeLayer)
            ],
            [vl3, vl5, vl4, vl1, vl2],
        )
        self.assertEqual(
            [node.layer() for node in group1.children()], [vl8, vl9, vl7, vl6]
        )
        self.assertEqual(view.currentNode().layer(), vl9)

        # layer as child of group selected, new layers should be added above that child
        view.setCurrentNode(QgsProject.instance().layerTreeRoot().findLayer(vl7))

        vl10 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test10",
            "memory",
        )
        details10 = QgsProcessingGuiUtils.ResultLayerDetails(vl10)
        details10.destinationProject = QgsProject.instance()
        details10.sortKey = 2
        vl11 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test11",
            "memory",
        )
        details11 = QgsProcessingGuiUtils.ResultLayerDetails(vl11)
        details11.destinationProject = QgsProject.instance()
        details11.sortKey = 1

        QgsProcessingGuiUtils.addResultLayers([details10, details11], context, view)

        self.assertCountEqual(
            QgsProject.instance().mapLayers().values(),
            [vl1, vl2, vl3, vl4, vl5, vl6, vl7, vl8, vl9, vl10, vl11],
        )
        self.assertEqual(
            [
                node.layer()
                for node in QgsProject.instance().layerTreeRoot().children()
                if isinstance(node, QgsLayerTreeLayer)
            ],
            [vl3, vl5, vl4, vl1, vl2],
        )
        self.assertEqual(
            [node.layer() for node in group1.children()],
            [vl8, vl9, vl10, vl11, vl7, vl6],
        )
        self.assertEqual(view.currentNode().layer(), vl11)

        # with explicit targetLayerTreeGroup set for some layers
        group2 = QgsProject.instance().layerTreeRoot().addGroup("group2")
        view.setCurrentNode(group1)

        vl12 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test10",
            "memory",
        )
        details12 = QgsProcessingGuiUtils.ResultLayerDetails(vl12)
        details12.destinationProject = QgsProject.instance()
        details12.sortKey = 2
        vl13 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test13",
            "memory",
        )
        details13 = QgsProcessingGuiUtils.ResultLayerDetails(vl13)
        details13.destinationProject = QgsProject.instance()
        details13.targetLayerTreeGroup = group2
        details13.sortKey = 1
        vl14 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test14",
            "memory",
        )
        details14 = QgsProcessingGuiUtils.ResultLayerDetails(vl14)
        details14.destinationProject = QgsProject.instance()
        details14.targetLayerTreeGroup = group2
        details14.sortKey = 10

        QgsProcessingGuiUtils.addResultLayers(
            [details12, details13, details14], context, view
        )

        self.assertCountEqual(
            QgsProject.instance().mapLayers().values(),
            [vl1, vl2, vl3, vl4, vl5, vl6, vl7, vl8, vl9, vl10, vl11, vl12, vl13, vl14],
        )
        self.assertEqual(
            [
                node.layer()
                for node in QgsProject.instance().layerTreeRoot().children()
                if isinstance(node, QgsLayerTreeLayer)
            ],
            [vl3, vl5, vl4, vl1, vl2],
        )
        self.assertEqual(
            [node.layer() for node in group1.children()],
            [vl12, vl8, vl9, vl10, vl11, vl7, vl6],
        )
        self.assertEqual([node.layer() for node in group2.children()], [vl14, vl13])
        self.assertEqual(view.currentNode().layer(), vl13)

        # test feature count
        ProcessingConfig.setSettingValue(ProcessingConfig.VECTOR_FEATURE_COUNT, True)

        vl15 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test15",
            "memory",
        )
        details15 = QgsProcessingGuiUtils.ResultLayerDetails(vl15)
        details15.destinationProject = QgsProject.instance()
        details15.targetLayerTreeGroup = group2
        details15.sortKey = 1
        vl16 = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)",
            "test13",
            "memory",
        )
        details16 = QgsProcessingGuiUtils.ResultLayerDetails(vl16)
        details16.destinationProject = QgsProject.instance()
        details16.targetLayerTreeGroup = group2
        details16.sortKey = 10

        QgsProcessingGuiUtils.addResultLayers([details15, details16], context, view)

        self.assertCountEqual(
            QgsProject.instance().mapLayers().values(),
            [
                vl1,
                vl2,
                vl3,
                vl4,
                vl5,
                vl6,
                vl7,
                vl8,
                vl9,
                vl10,
                vl11,
                vl12,
                vl13,
                vl14,
                vl15,
                vl16,
            ],
        )
        self.assertEqual(
            [
                node.layer()
                for node in QgsProject.instance().layerTreeRoot().children()
                if isinstance(node, QgsLayerTreeLayer)
            ],
            [vl3, vl5, vl4, vl1, vl2],
        )
        self.assertEqual(
            [node.layer() for node in group1.children()],
            [vl12, vl8, vl9, vl10, vl11, vl7, vl6],
        )
        self.assertEqual(
            [node.layer() for node in group2.children()], [vl16, vl15, vl14, vl13]
        )
        self.assertEqual(view.currentNode().layer(), vl15)

        # no layer count for these layers
        self.assertFalse(
            QgsProject.instance()
            .layerTreeRoot()
            .findLayer(vl1)
            .customProperty("showFeatureCount")
        )
        self.assertFalse(
            QgsProject.instance()
            .layerTreeRoot()
            .findLayer(vl2)
            .customProperty("showFeatureCount")
        )
        self.assertFalse(
            QgsProject.instance()
            .layerTreeRoot()
            .findLayer(vl3)
            .customProperty("showFeatureCount")
        )
        # layer count for these newly added layers
        self.assertTrue(
            QgsProject.instance()
            .layerTreeRoot()
            .findLayer(vl15)
            .customProperty("showFeatureCount")
        )
        self.assertTrue(
            QgsProject.instance()
            .layerTreeRoot()
            .findLayer(vl16)
            .customProperty("showFeatureCount")
        )


if __name__ == "__main__":
    unittest.main()
