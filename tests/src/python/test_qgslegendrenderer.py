# coding=utf-8
""""Test QgsLegendRenderer JSON export

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

Run with ctest -V -R PyQgsLegendRenderer

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2020-04-29'
__copyright__ = 'Copyright 2020, ItOpen'

import os

from qgis.core import (
    QgsProject,
    QgsLegendModel,
    QgsLegendSettings,
    QgsLegendRenderer,
    QgsRenderContext,
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsLegendRenderer(unittest.TestCase):

    def test_json_export(self):

        project = QgsProject()
        self.assertTrue(project.read(os.path.join(unitTestDataPath('qgis_server'), 'test_project.qgs')))
        model = QgsLegendModel(project.layerTreeRoot())
        ctx = QgsRenderContext()
        settings = QgsLegendSettings()
        renderer = QgsLegendRenderer(model, settings)
        nodes = renderer.exportLegendToJson(ctx)['nodes'].toVariant()
        self.assertEqual(len(nodes), 7)
        self.assertEqual(nodes[0]['type'], 'layer')
        self.assertEqual(nodes[0]['title'], 'testlayer')


if __name__ == '__main__':
    unittest.main()
