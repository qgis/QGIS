# -*- coding: utf-8 -*-
"""
Tests for WFS-T provider using QGIS Server through qgis_wrapped_server.py.

This is an integration test for QGIS Desktop WFS-T provider and QGIS Server
WFS-T that check if QGIS can talk to and uderstand itself.

The test uses testdata/wfs_transactional/wfs_transactional.qgs and three
initially empty shapefiles layrs with points, lines and polygons.

All WFS-T calls are executed through the QGIS WFS data provider.

The three layers are

1. populated with WFS-T
2. checked for geometry and attributes
3. modified with WFS-T
4. checked for geometry and attributes
5. emptied with WFS-T calls to delete


From build dir, run: ctest -R PyQgsServerWFST -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Alessandro Pasotti'
__date__ = '05/15/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


import os
import sys
import subprocess
from shutil import copytree, rmtree
import tempfile
from time import sleep
from utilities import unitTestDataPath
from qgis.core import (
    QgsVectorLayer,
    QgsFeature,
    QgsGeometry,
    QgsPoint,
    QgsRectangle,
    QgsFeatureRequest,
    QgsExpression,
)
from qgis.testing import (
    start_app,
    unittest,
)

try:
    QGIS_SERVER_WFST_DEFAULT_PORT = os.environ['QGIS_SERVER_WFST_DEFAULT_PORT']
except:
    QGIS_SERVER_WFST_DEFAULT_PORT = 8081


qgis_app = start_app()


class TestWFST(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.port = QGIS_SERVER_WFST_DEFAULT_PORT
        # Create tmp folder
        cls.temp_path = tempfile.mkdtemp()
        cls.testdata_path = cls.temp_path + '/' + 'wfs_transactional' + '/'
        copytree(unitTestDataPath('wfs_transactional') + '/',
                 cls.temp_path + '/' + 'wfs_transactional')
        cls.project_path = cls.temp_path + '/' + 'wfs_transactional' + '/' + \
            'wfs_transactional.qgs'
        assert os.path.exists(cls.project_path), "Project not found: %s" % \
            cls.project_path
        # Clean env just to be sure
        env_vars = ['QUERY_STRING', 'QGIS_PROJECT_FILE']
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass
        # Clear all test layers
        for ln in ['test_point', 'test_polygon', 'test_linestring']:
            cls._clearLayer(ln)
        os.environ['QGIS_SERVER_DEFAULT_PORT'] = str(cls.port)
        server_path = os.path.dirname(os.path.realpath(__file__)) + \
            '/qgis_wrapped_server.py'
        cls.server = subprocess.Popen([sys.executable, server_path],
                                      env=os.environ)
        sleep(2)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.server.terminate()
        del cls.server
        # Clear all test layers
        for ln in ['test_point', 'test_polygon', 'test_linestring']:
            layer = cls._getLayer(ln)
            cls._clearLayer(ln)
        rmtree(cls.temp_path)

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    @classmethod
    def _clearLayer(cls, layer_name):
        """
        Delete all features from a vector layer
        """
        layer = cls._getLayer(layer_name)
        layer.startEditing()
        layer.deleteFeatures([f.id() for f in layer.getFeatures()])
        layer.commitChanges()
        assert layer.featureCount() == 0

    @classmethod
    def _getLayer(cls, layer_name):
        """
        OGR Layer factory
        """
        path = cls.testdata_path + layer_name + '.shp'
        layer = QgsVectorLayer(path, layer_name, "ogr")
        assert layer.isValid()
        return layer

    @classmethod
    def _getWFSLayer(cls, type_name, layer_name=None):
        """
        WFS layer factory
        """
        if layer_name is None:
            layer_name = 'wfs_' + type_name
        parms = {
            'srsname': 'EPSG:4326',
            'typename': type_name,
            'url': 'http://127.0.0.1:%s/?map=%s' % (cls.port,
                                                    cls.project_path),
            'version': 'auto',
            'table': '',
            #'sql': '',
        }
        uri = ' '.join([("%s='%s'" % (k, v)) for k, v in parms.items()])
        wfs_layer = QgsVectorLayer(uri, layer_name, 'WFS')
        assert wfs_layer.isValid()
        return wfs_layer

    @classmethod
    def _getFeatureByAttribute(cls, layer, attr_name, attr_value):
        """
        Find the feature and return it, raise exception if not found
        """
        request = QgsFeatureRequest(QgsExpression("%s=%s" % (attr_name,
                                                             attr_value)))
        try:
            return next(layer.dataProvider().getFeatures(request))
        except StopIteration:
            raise Exception("Wrong attributes in WFS layer %s" %
                            layer.name())

    def _checkAddFeatures(self, wfs_layer, layer, features):
        """
        Check features were added
        """
        wfs_layer.dataProvider().addFeatures(features)
        layer = self._getLayer(layer.name())
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.featureCount(), len(features))

    def _checkUpdateFeatures(self, wfs_layer, old_features, new_features):
        """
        Check features can be updated
        """
        for i in range(len(old_features)):
            f = self._getFeatureByAttribute(wfs_layer, 'id', old_features[i]['id'])
            self.assertTrue(wfs_layer.dataProvider().changeGeometryValues({f.id(): new_features[i].geometry()}))
            self.assertTrue(wfs_layer.dataProvider().changeAttributeValues({f.id(): {0: new_features[i]['id']}}))
            self.assertTrue(wfs_layer.dataProvider().changeAttributeValues({f.id(): {1: new_features[i]['name']}}))

    def _checkMatchFeatures(self, wfs_layer, features):
        """
        Check feature attributes and geometry match
        """
        for f in features:
            wf = self._getFeatureByAttribute(wfs_layer, 'id', f['id'])
            self.assertEqual(wf.geometry().exportToWkt(),
                             f.geometry().exportToWkt())
            self.assertEqual(f['name'], wf['name'])

    def _checkDeleteFeatures(self, layer, features):
        """
        Delete features
        """
        ids = []
        for f in features:
            wf = self._getFeatureByAttribute(layer, 'id', f['id'])
            ids.append(wf.id())
        self.assertTrue(layer.dataProvider().deleteFeatures(ids))

    def _testLayer(self, wfs_layer, layer, old_features, new_features):
        """
        Perform all test steps on the layer.
        """
        self.assertEqual(wfs_layer.featureCount(), 0)
        self._checkAddFeatures(wfs_layer, layer, old_features)
        self._checkMatchFeatures(wfs_layer, old_features)
        self.assertEqual(wfs_layer.dataProvider().featureCount(),
                         len(old_features))
        self._checkUpdateFeatures(wfs_layer, old_features, new_features)
        self._checkMatchFeatures(wfs_layer, new_features)
        self._checkDeleteFeatures(wfs_layer, new_features)
        self.assertEqual(wfs_layer.dataProvider().featureCount(), 0)

    def testWFSPoints(self):
        """
        Adds some points, then check and clear all
        """
        layer_name = 'test_point'
        layer = self._getLayer(layer_name)
        wfs_layer = self._getWFSLayer(layer_name)
        feat1 = QgsFeature(wfs_layer.pendingFields())
        feat1['id'] = 11
        feat1.setGeometry(QgsGeometry.fromPoint(QgsPoint(9, 45)))
        feat2 = QgsFeature(wfs_layer.pendingFields())
        feat2.setGeometry(QgsGeometry.fromPoint(QgsPoint(9.5, 45.5)))
        feat2['id'] = 12
        old_features = [feat1, feat2]
        # Change feat1
        new_feat1 = QgsFeature(wfs_layer.pendingFields())
        new_feat1['id'] = 121
        new_feat1.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 46)))
        new_features = [new_feat1, feat2]
        self._testLayer(wfs_layer, layer, old_features, new_features)

    def testWFSPointsMultipleEdits(self):
        """
        Adds some points, then check.
        Modify 2 points, then checks and clear all
        """
        layer_name = 'test_point'
        layer = self._getLayer(layer_name)
        wfs_layer = self._getWFSLayer(layer_name)
        feat1 = QgsFeature(wfs_layer.pendingFields())
        feat1['id'] = 11
        feat1['name'] = 'name 11'
        feat1.setGeometry(QgsGeometry.fromPoint(QgsPoint(9, 45)))
        feat2 = QgsFeature(wfs_layer.pendingFields())
        feat2.setGeometry(QgsGeometry.fromPoint(QgsPoint(9.5, 45.5)))
        feat2['id'] = 12
        feat2['name'] = 'name 12'
        old_features = [feat1, feat2]
        # Change feat1 and feat2
        new_feat1 = QgsFeature(wfs_layer.pendingFields())
        new_feat1['id'] = 121
        new_feat1['name'] = 'name 121'
        new_feat1.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 46)))
        new_feat2 = QgsFeature(wfs_layer.pendingFields())
        new_feat2['id'] = 122
        new_feat2['name'] = 'name 122'
        new_feat2.setGeometry(QgsGeometry.fromPoint(QgsPoint(10.5, 47)))
        new_features = [new_feat1, new_feat2]
        self._testLayer(wfs_layer, layer, old_features, new_features)

    def testWFSPolygons(self):
        """
        Adds some polygons, then check and clear all
        """
        layer_name = 'test_polygon'
        layer = self._getLayer(layer_name)
        wfs_layer = self._getWFSLayer(layer_name)
        feat1 = QgsFeature(wfs_layer.pendingFields())
        feat1['id'] = 11
        feat1['name'] = 'name 11'
        feat1.setGeometry(QgsGeometry.fromRect(QgsRectangle(QgsPoint(9, 45), QgsPoint(10, 46))))
        feat2 = QgsFeature(wfs_layer.pendingFields())
        feat2.setGeometry(QgsGeometry.fromRect(QgsRectangle(QgsPoint(9.5, 45.5), QgsPoint(10.5, 46.5))))
        feat2['id'] = 12
        feat2['name'] = 'name 12'
        old_features = [feat1, feat2]
        # Change feat1
        new_feat1 = QgsFeature(wfs_layer.pendingFields())
        new_feat1['id'] = 121
        new_feat1['name'] = 'name 121'
        new_feat1.setGeometry(QgsGeometry.fromRect(QgsRectangle(QgsPoint(10, 46), QgsPoint(11.5, 47.5))))
        new_features = [new_feat1, feat2]
        self._testLayer(wfs_layer, layer, old_features, new_features)

    def testWFSLineStrings(self):
        """
        Adds some lines, then check and clear all
        """
        layer_name = 'test_linestring'
        layer = self._getLayer(layer_name)
        wfs_layer = self._getWFSLayer(layer_name)
        feat1 = QgsFeature(wfs_layer.pendingFields())
        feat1['id'] = 11
        feat1['name'] = 'name 11'
        feat1.setGeometry(QgsGeometry.fromPolyline([QgsPoint(9, 45), QgsPoint(10, 46)]))
        feat2 = QgsFeature(wfs_layer.pendingFields())
        feat2.setGeometry(QgsGeometry.fromPolyline([QgsPoint(9.5, 45.5), QgsPoint(10.5, 46.5)]))
        feat2['id'] = 12
        feat2['name'] = 'name 12'
        old_features = [feat1, feat2]
        # Change feat1
        new_feat1 = QgsFeature(wfs_layer.pendingFields())
        new_feat1['id'] = 121
        new_feat1['name'] = 'name 121'
        new_feat1.setGeometry(QgsGeometry.fromPolyline([QgsPoint(9.8, 45.8), QgsPoint(10.8, 46.8)]))
        new_features = [new_feat1, feat2]
        self._testLayer(wfs_layer, layer, old_features, new_features)


if __name__ == '__main__':
    unittest.main()
