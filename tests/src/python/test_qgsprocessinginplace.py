# -*- coding: utf-8 -*-
"""QGIS Unit tests for Processing In-Place algorithms.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2018-09'
__copyright__ = 'Copyright 2018, The QGIS Project'

import re
import os
from qgis.PyQt.QtCore import QCoreApplication, QVariant, QTemporaryDir
from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsSettings,
    QgsApplication,
    QgsMemoryProviderUtils,
    QgsWkbTypes,
    QgsField,
    QgsFields,
    QgsProcessingFeatureSourceDefinition,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsCoordinateReferenceSystem,
    QgsProject,
    QgsProcessingException,
    QgsVectorLayer,
    QgsFeatureSink,
    QgsProperty
)
from processing.core.Processing import Processing
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools import dataobjects
from processing.gui.AlgorithmExecutor import execute_in_place_run
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgis.PyQt.QtTest import QSignalSpy
from qgis.analysis import QgsNativeAlgorithms
from qgis.core import QgsVectorLayerUtils, QgsFeatureRequest
import shutil

start_app()


class ConsoleFeedBack(QgsProcessingFeedback):

    def reportError(self, error, fatalError=False):
        print(error)


base_types = ['Point', 'LineString', 'Polygon']


def _add_multi(base):
    return base + ['Multi' + _b for _b in base]


def _add_z(base):
    return base + [_b + 'Z' for _b in base]


def _add_m(base):
    return base + [_b + 'M' for _b in base]


def _all_true():
    types = base_types
    types = _add_multi(types)
    types = _add_z(types)
    types = _add_m(types)
    types.append('NoGeometry')
    return {t: True for t in types}


def _all_false():
    return {t: False for t in _all_true().keys()}


class TestQgsProcessingInPlace(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsProcessingInPlace.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsProcessingInPlace")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()
        fields = QgsFields()
        fields.append(QgsField('int_f', QVariant.Int))
        cls.vl = QgsMemoryProviderUtils.createMemoryLayer(
            'mylayer', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:4326'))

        f1 = QgsFeature(cls.vl.fields())
        f1['int_f'] = 1
        f1.setGeometry(QgsGeometry.fromWkt('Point(9 45)'))
        f2 = QgsFeature(cls.vl.fields())
        f2['int_f'] = 2
        f2.setGeometry(QgsGeometry.fromWkt('Point(9.5 45.6)'))
        cls.vl.dataProvider().addFeatures([f1, f2])

        assert cls.vl.isValid()
        assert cls.vl.featureCount() == 2

        # Multipolygon layer

        cls.multipoly_vl = QgsMemoryProviderUtils.createMemoryLayer(
            'mymultiplayer', fields, QgsWkbTypes.MultiPolygon, QgsCoordinateReferenceSystem('EPSG:4326'))

        f3 = QgsFeature(cls.multipoly_vl.fields())
        f3.setGeometry(QgsGeometry.fromWkt(
            'MultiPolygon (((2.81856297539240419 41.98170998812887689, 2.81874467773035464 41.98167537995160359, 2.81879535908157752 41.98154066615795443, 2.81866433873670452 41.98144056064155905, 2.81848263699778379 41.98147516865246587, 2.81843195500470811 41.98160988234612034, 2.81856297539240419 41.98170998812887689)),((2.81898589063455907 41.9815711567298635, 2.81892080450418803 41.9816030048432367, 2.81884192631866437 41.98143737613141724, 2.8190679469505846 41.98142270931093378, 2.81898589063455907 41.9815711567298635)))'))
        f4 = QgsFeature(cls.multipoly_vl.fields())
        f4.setGeometry(QgsGeometry.fromWkt(
            'MultiPolygon (((2.81823679385631332 41.98133290154246566, 2.81830770255185703 41.98123540208609228, 2.81825871989355159 41.98112524362621656, 2.81813882853970243 41.98111258462271422, 2.81806791984415872 41.98121008407908761, 2.81811690250246416 41.98132024253896333, 2.81823679385631332 41.98133290154246566)),((2.81835835162010895 41.98123286963267731, 2.8183127674586852 41.98108725356146209, 2.8184520523963692 41.98115436357689134, 2.81835835162010895 41.98123286963267731)))'))
        cls.multipoly_vl.dataProvider().addFeatures([f3, f4])

        assert cls.multipoly_vl.isValid()
        assert cls.multipoly_vl.featureCount() == 2

        QgsProject.instance().addMapLayers([cls.vl, cls.multipoly_vl])

    def _make_layer(self, layer_wkb_name):
        fields = QgsFields()
        wkb_type = getattr(QgsWkbTypes, layer_wkb_name)
        fields.append(QgsField('int_f', QVariant.Int))
        layer = QgsMemoryProviderUtils.createMemoryLayer(
            '%s_layer' % layer_wkb_name, fields, wkb_type, QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.wkbType(), wkb_type)
        return layer

    def _support_inplace_edit_tester(self, alg_name, expected):

        alg = self.registry.createAlgorithmById(alg_name)
        for layer_wkb_name, supported in expected.items():
            layer = self._make_layer(layer_wkb_name)
            # print("Checking %s ( %s ) : %s" % (alg_name, layer_wkb_name, supported))
            self.assertEqual(alg.supportInPlaceEdit(layer), supported,
                             "Expected: %s - %s = supported: %s" % (alg_name, layer_wkb_name, supported))

    def test_support_in_place_edit(self):

        ALL = _all_true()
        GEOMETRY_ONLY = {t: t != 'NoGeometry' for t in _all_true().keys()}
        NONE = _all_false()
        LINESTRING_ONLY = {t: t.find('LineString') >= 0 for t in _all_true().keys()}
        Z_ONLY = {t: t.find('Z') > 0 for t in _all_true().keys()}
        M_ONLY = {t: t.rfind('M') > 0 for t in _all_true().keys()}
        NOT_M = {t: t.rfind('M') < 1 and t != 'NoGeometry' for t in _all_true().keys()}
        POLYGON_ONLY = {t: t.find('Polygon') for t in _all_true().keys()}
        POLYGON_ONLY_NOT_M_NOT_Z = {t: t in ('Polygon', 'MultiPolygon') for t in _all_true().keys()}
        MULTI_ONLY = {t: t.find('Multi') == 0 for t in _all_true().keys()}
        SINGLE_ONLY = {t: t.find('Multi') == -1 for t in _all_true().keys()}
        LINESTRING_AND_POLYGON_ONLY = {t: (t.find('LineString') >= 0 or t.find('Polygon') >= 0) for t in
                                       _all_true().keys()}
        LINESTRING_AND_POLYGON_ONLY_NOT_M = {
            t: (t.rfind('M') < 1 and (t.find('LineString') >= 0 or t.find('Polygon') >= 0)) for t in _all_true().keys()}
        LINESTRING_AND_POLYGON_ONLY_NOT_M_NOT_Z = {
            t: (t.rfind('M') < 1 and t.find('Z') == -1 and (t.find('LineString') >= 0 or t.find('Polygon') >= 0)) for t
            in _all_true().keys()}

        self._support_inplace_edit_tester('native:smoothgeometry', LINESTRING_AND_POLYGON_ONLY)
        self._support_inplace_edit_tester('native:arrayoffsetlines', LINESTRING_ONLY)
        self._support_inplace_edit_tester('native:arraytranslatedfeatures', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:reprojectlayer', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('qgis:densifygeometries', LINESTRING_AND_POLYGON_ONLY)
        self._support_inplace_edit_tester('qgis:densifygeometriesgivenaninterval', LINESTRING_AND_POLYGON_ONLY)
        self._support_inplace_edit_tester('native:setzfromraster', Z_ONLY)
        self._support_inplace_edit_tester('native:explodelines', LINESTRING_ONLY)
        self._support_inplace_edit_tester('native:extendlines', LINESTRING_ONLY)
        self._support_inplace_edit_tester('native:fixgeometries', NOT_M)
        self._support_inplace_edit_tester('native:minimumenclosingcircle', POLYGON_ONLY_NOT_M_NOT_Z)
        self._support_inplace_edit_tester('native:multiringconstantbuffer', POLYGON_ONLY_NOT_M_NOT_Z)
        self._support_inplace_edit_tester('native:orientedminimumboundingbox', POLYGON_ONLY_NOT_M_NOT_Z)
        self._support_inplace_edit_tester('qgis:orthogonalize', LINESTRING_AND_POLYGON_ONLY)
        self._support_inplace_edit_tester('native:removeduplicatevertices', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:rotatefeatures', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:segmentizebymaxangle', NONE)
        self._support_inplace_edit_tester('native:segmentizebymaxdistance', NONE)
        self._support_inplace_edit_tester('native:setmfromraster', M_ONLY)
        self._support_inplace_edit_tester('native:simplifygeometries', LINESTRING_AND_POLYGON_ONLY)
        self._support_inplace_edit_tester('native:snappointstogrid', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:multiparttosingleparts', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:promotetomulti', MULTI_ONLY)
        self._support_inplace_edit_tester('native:subdivide', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:translategeometry', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:swapxy', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('qgis:linestopolygons', NONE)
        self._support_inplace_edit_tester('qgis:polygonstolines', NONE)
        self._support_inplace_edit_tester('native:boundary', NONE)
        self._support_inplace_edit_tester('native:clip', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:difference', GEOMETRY_ONLY)
        self._support_inplace_edit_tester('native:dropgeometries', ALL)
        self._support_inplace_edit_tester('native:splitwithlines', LINESTRING_AND_POLYGON_ONLY)
        self._support_inplace_edit_tester('native:splitlinesbylength', LINESTRING_ONLY)
        self._support_inplace_edit_tester('native:buffer', POLYGON_ONLY_NOT_M_NOT_Z)
        self._support_inplace_edit_tester('native:antimeridiansplit', LINESTRING_ONLY)
        self._support_inplace_edit_tester('native:affinetransform', GEOMETRY_ONLY)

    def _make_compatible_tester(self, feature_wkt, layer_wkb_name, attrs=[1]):
        layer = self._make_layer(layer_wkb_name)
        layer.startEditing()

        f = QgsFeature(layer.fields())
        f.setAttributes(attrs)
        f.setGeometry(QgsGeometry.fromWkt(feature_wkt))
        self.assertTrue(f.isValid())

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        # Fix it!
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f], layer)

        for new_f in new_features:
            self.assertEqual(new_f.geometry().wkbType(), layer.wkbType())

        self.assertTrue(layer.addFeatures(new_features), "Fail: %s - %s - %s" % (feature_wkt, attrs, layer_wkb_name))
        return layer, new_features

    def test_QgsVectorLayerUtilsmakeFeaturesCompatible(self):
        """Test fixer function"""
        # Test failure
        self._make_compatible_tester('LineString (1 1, 2 2, 3 3)', 'Point')
        self._make_compatible_tester('LineString (1 1, 2 2, 3 3)', 'Polygon')
        self._make_compatible_tester('Polygon((1 1, 2 2, 1 2, 1 1))', 'Point')
        self._make_compatible_tester('Polygon((1 1, 2 2, 1 2, 1 1))', 'LineString')

        self._make_compatible_tester('Point(1 1)', 'Point')
        self._make_compatible_tester('Point(1 1)', 'Point', [1, 'nope'])
        self._make_compatible_tester('Point z (1 1 3)', 'Point')
        self._make_compatible_tester('Point z (1 1 3)', 'PointZ')

        # Adding Z back
        l, f = self._make_compatible_tester('Point (1 1)', 'PointZ')
        self.assertEqual(f[0].geometry().constGet().z(), 0)

        # Adding M back
        l, f = self._make_compatible_tester('Point (1 1)', 'PointM')
        self.assertEqual(f[0].geometry().constGet().m(), 0)

        self._make_compatible_tester('Point m (1 1 3)', 'Point')
        self._make_compatible_tester('Point(1 3)', 'MultiPoint')
        self._make_compatible_tester('MultiPoint((1 3), (2 2))', 'MultiPoint')

        self._make_compatible_tester('Polygon((1 1, 2 2, 3 3, 1 1))', 'Polygon')
        self._make_compatible_tester('Polygon((1 1, 2 2, 3 3, 1 1))', 'Polygon', [1, 'nope'])
        self._make_compatible_tester('Polygon z ((1 1 1, 2 2 2, 3 3 3, 1 1 1))', 'Polygon')
        self._make_compatible_tester('Polygon z ((1 1 1, 2 2 2, 3 3 3, 1 1 1))', 'PolygonZ')

        # Adding Z back
        l, f = self._make_compatible_tester('Polygon ((1 1, 2 2, 3 3, 1 1))', 'PolygonZ')
        g = f[0].geometry()
        g2 = g.constGet()
        for v in g2.vertices():
            self.assertEqual(v.z(), 0)

        # Adding M back
        l, f = self._make_compatible_tester('Polygon ((1 1, 2 2, 3 3, 1 1))', 'PolygonM')
        g = f[0].geometry()
        g2 = g.constGet()
        for v in g2.vertices():
            self.assertEqual(v.m(), 0)

        self._make_compatible_tester('Polygon m ((1 1 1, 2 2 2, 3 3 3, 1 1 1))', 'Polygon')
        self._make_compatible_tester('Polygon m ((1 1 1, 2 2 2, 3 3 3, 1 1 1))', 'PolygonM')
        self._make_compatible_tester('Polygon((1 1, 2 2, 3 3, 1 1))', 'MultiPolygon')
        self._make_compatible_tester('MultiPolygon(((1 1, 2 2, 3 3, 1 1)), ((1 1, 2 2, 3 3, 1 1)))', 'MultiPolygon')

        self._make_compatible_tester('LineString(1 1, 2 2, 3 3, 1 1)', 'LineString')
        self._make_compatible_tester('LineString(1 1, 2 2, 3 3, 1 1)', 'LineString', [1, 'nope'])
        self._make_compatible_tester('LineString z (1 1 1, 2 2 2, 3 3 3, 1 1 1)', 'LineString')
        self._make_compatible_tester('LineString z (1 1 1, 2 2 2, 3 3 3, 1 1 1)', 'LineStringZ')
        self._make_compatible_tester('LineString m (1 1 1, 2 2 2, 3 3 3, 1 1 1)', 'LineString')
        self._make_compatible_tester('LineString m (1 1 1, 2 2 2, 3 3 3, 1 1 1)', 'LineStringM')

        # Adding Z back
        l, f = self._make_compatible_tester('LineString (1 1, 2 2, 3 3, 1 1)', 'LineStringZ')
        g = f[0].geometry()
        g2 = g.constGet()
        for v in g2.vertices():
            self.assertEqual(v.z(), 0)

        # Adding M back
        l, f = self._make_compatible_tester('LineString (1 1, 2 2, 3 3, 1 1)', 'LineStringM')
        g = f[0].geometry()
        g2 = g.constGet()
        for v in g2.vertices():
            self.assertEqual(v.m(), 0)

        self._make_compatible_tester('LineString(1 1, 2 2, 3 3, 1 1)', 'MultiLineString')
        self._make_compatible_tester('MultiLineString((1 1, 2 2, 3 3, 1 1), (1 1, 2 2, 3 3, 1 1))', 'MultiLineString')

        # Test Multi -> Single
        l, f = self._make_compatible_tester('MultiLineString((1 1, 2 2, 3 3, 1 1), (10 1, 20 2, 30 3, 10 1))',
                                            'LineString')
        self.assertEqual(len(f), 2)
        self.assertEqual(f[0].geometry().asWkt(), 'LineString (1 1, 2 2, 3 3, 1 1)')
        self.assertEqual(f[1].geometry().asWkt(), 'LineString (10 1, 20 2, 30 3, 10 1)')

        # line -> points
        l, f = self._make_compatible_tester('LineString (1 1, 2 2, 3 3)', 'Point')
        self.assertEqual(len(f), 3)
        self.assertEqual(f[0].geometry().asWkt(), 'Point (1 1)')
        self.assertEqual(f[1].geometry().asWkt(), 'Point (2 2)')
        self.assertEqual(f[2].geometry().asWkt(), 'Point (3 3)')

        l, f = self._make_compatible_tester('LineString (1 1, 2 2, 3 3)', 'MultiPoint')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'MultiPoint ((1 1),(2 2),(3 3))')

        l, f = self._make_compatible_tester('MultiLineString ((1 1, 2 2),(4 4, 3 3))', 'Point')
        self.assertEqual(len(f), 4)
        self.assertEqual(f[0].geometry().asWkt(), 'Point (1 1)')
        self.assertEqual(f[1].geometry().asWkt(), 'Point (2 2)')
        self.assertEqual(f[2].geometry().asWkt(), 'Point (4 4)')
        self.assertEqual(f[3].geometry().asWkt(), 'Point (3 3)')

        l, f = self._make_compatible_tester('MultiLineString ((1 1, 2 2),(4 4, 3 3))', 'MultiPoint')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'MultiPoint ((1 1),(2 2),(4 4),(3 3))')

        # line -> polygon
        l, f = self._make_compatible_tester('LineString (1 1, 1 2, 2 2)', 'Polygon')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'Polygon ((1 1, 1 2, 2 2, 1 1))')

        l, f = self._make_compatible_tester('LineString (1 1, 1 2, 2 2)', 'MultiPolygon')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'MultiPolygon (((1 1, 1 2, 2 2, 1 1)))')

        l, f = self._make_compatible_tester('MultiLineString ((1 1, 1 2, 2 2, 1 1),(3 3, 4 3, 4 4))', 'Polygon')
        self.assertEqual(len(f), 2)
        self.assertEqual(f[0].geometry().asWkt(), 'Polygon ((1 1, 1 2, 2 2, 1 1))')
        self.assertEqual(f[1].geometry().asWkt(), 'Polygon ((3 3, 4 3, 4 4, 3 3))')

        l, f = self._make_compatible_tester('MultiLineString ((1 1, 1 2, 2 2, 1 1),(3 3, 4 3, 4 4))', 'MultiPolygon')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))')

        l, f = self._make_compatible_tester('CircularString (1 1, 1 2, 2 2, 2 0, 1 1)', 'CurvePolygon')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'CurvePolygon (CircularString (1 1, 1 2, 2 2, 2 0, 1 1))')

        l, f = self._make_compatible_tester('CircularString (1 1, 1 2, 2 2, 2 0, 1 1)', 'Polygon')
        self.assertEqual(len(f), 1)
        self.assertTrue(f[0].geometry().asWkt(2).startswith('Polygon ((1 1, 0.99 1.01, 0.98 1.02'))

        # polygon -> points
        l, f = self._make_compatible_tester('Polygon ((1 1, 1 2, 2 2, 1 1))', 'Point')
        self.assertEqual(len(f), 3)
        self.assertEqual(f[0].geometry().asWkt(), 'Point (1 1)')
        self.assertEqual(f[1].geometry().asWkt(), 'Point (1 2)')
        self.assertEqual(f[2].geometry().asWkt(), 'Point (2 2)')

        l, f = self._make_compatible_tester('Polygon ((1 1, 1 2, 2 2, 1 1))', 'MultiPoint')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'MultiPoint ((1 1),(1 2),(2 2))')

        l, f = self._make_compatible_tester('MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))', 'Point')
        self.assertEqual(len(f), 6)
        self.assertEqual(f[0].geometry().asWkt(), 'Point (1 1)')
        self.assertEqual(f[1].geometry().asWkt(), 'Point (1 2)')
        self.assertEqual(f[2].geometry().asWkt(), 'Point (2 2)')
        self.assertEqual(f[3].geometry().asWkt(), 'Point (3 3)')
        self.assertEqual(f[4].geometry().asWkt(), 'Point (4 3)')
        self.assertEqual(f[5].geometry().asWkt(), 'Point (4 4)')

        l, f = self._make_compatible_tester('MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))',
                                            'MultiPoint')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'MultiPoint ((1 1),(1 2),(2 2),(3 3),(4 3),(4 4))')

        # polygon -> lines
        l, f = self._make_compatible_tester('Polygon ((1 1, 1 2, 2 2, 1 1))', 'LineString')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'LineString (1 1, 1 2, 2 2, 1 1)')

        l, f = self._make_compatible_tester('Polygon ((1 1, 1 2, 2 2, 1 1))', 'MultiLineString')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'MultiLineString ((1 1, 1 2, 2 2, 1 1))')

        l, f = self._make_compatible_tester('MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))',
                                            'LineString')
        self.assertEqual(len(f), 2)
        self.assertEqual(f[0].geometry().asWkt(), 'LineString (1 1, 1 2, 2 2, 1 1)')
        self.assertEqual(f[1].geometry().asWkt(), 'LineString (3 3, 4 3, 4 4, 3 3)')

        l, f = self._make_compatible_tester('MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))',
                                            'MultiLineString')
        self.assertEqual(len(f), 1)
        self.assertEqual(f[0].geometry().asWkt(), 'MultiLineString ((1 1, 1 2, 2 2, 1 1),(3 3, 4 3, 4 4, 3 3))')

    def test_make_features_compatible_attributes(self):
        """Test corner cases for attributes"""

        # Test feature without attributes
        fields = QgsFields()
        fields.append(QgsField('int_f', QVariant.Int))
        fields.append(QgsField('str_f', QVariant.String))
        layer = QgsMemoryProviderUtils.createMemoryLayer(
            'mkfca_layer', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertTrue(layer.isValid())
        f1 = QgsFeature(layer.fields())
        f1['int_f'] = 1
        f1['str_f'] = 'str'
        f1.setGeometry(QgsGeometry.fromWkt('Point(9 45)'))
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], layer)
        self.assertEqual(new_features[0].attributes(), f1.attributes())
        self.assertTrue(new_features[0].geometry().asWkt(), f1.geometry().asWkt())

        # Test pad with 0 with fields
        f1.setAttributes([])
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], layer)
        self.assertEqual(len(new_features[0].attributes()), 2)
        self.assertEqual(new_features[0].attributes()[0], QVariant())
        self.assertEqual(new_features[0].attributes()[1], QVariant())

        # Test pad with 0 without fields
        f1 = QgsFeature()
        f1.setGeometry(QgsGeometry.fromWkt('Point(9 45)'))
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], layer)
        self.assertEqual(len(new_features[0].attributes()), 2)
        self.assertEqual(new_features[0].attributes()[0], QVariant())
        self.assertEqual(new_features[0].attributes()[1], QVariant())

        # Test drop extra attrs
        f1 = QgsFeature(layer.fields())
        f1.setAttributes([1, 'foo', 'extra'])
        f1.setGeometry(QgsGeometry.fromWkt('Point(9 45)'))
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], layer)
        self.assertEqual(len(new_features[0].attributes()), 2)
        self.assertEqual(new_features[0].attributes()[0], 1)
        self.assertEqual(new_features[0].attributes()[1], 'foo')

    def test_make_features_compatible_different_field_length(self):
        """Test regression #21497"""
        fields = QgsFields()
        fields.append(QgsField('int_f1', QVariant.Int))
        f1 = QgsFeature(fields)
        f1.setAttributes([12345])
        f1.setGeometry(QgsGeometry.fromWkt('Point(9 45)'))

        fields = QgsFields()
        fields.append(QgsField('int_f2', QVariant.Int))
        fields.append(QgsField('int_f1', QVariant.Int))
        vl2 = QgsMemoryProviderUtils.createMemoryLayer(
            'mymultiplayer', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:4326'))
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], vl2)
        self.assertEqual(new_features[0].attributes(), [None, 12345])

        f1.setGeometry(QgsGeometry.fromWkt('MultiPoint((9 45))'))
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], vl2)
        self.assertEqual(new_features[0].attributes(), [None, 12345])

    def test_make_features_compatible_geometry(self):
        """Test corner cases for geometries"""

        # Make a feature with no geometry
        layer = self._make_layer('Point')
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.startEditing())
        f1 = QgsFeature(layer.fields())
        f1.setAttributes([1])

        # Check that it is accepted on a Point layer
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], layer)
        self.assertEqual(len(new_features), 1)
        self.assertEqual(new_features[0].geometry().asWkt(), '')

        # Make a geometry-less layer
        nogeom_layer = QgsMemoryProviderUtils.createMemoryLayer(
            'nogeom_layer', layer.fields(), QgsWkbTypes.NoGeometry, QgsCoordinateReferenceSystem('EPSG:4326'))
        # Check that a geometry-less feature is accepted
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], nogeom_layer)
        self.assertEqual(len(new_features), 1)
        self.assertEqual(new_features[0].geometry().asWkt(), '')

        # Make a geometry-less layer
        nogeom_layer = QgsMemoryProviderUtils.createMemoryLayer(
            'nogeom_layer', layer.fields(), QgsWkbTypes.NoGeometry, QgsCoordinateReferenceSystem('EPSG:4326'))
        # Check that a Point feature is accepted but geometry was dropped
        f1.setGeometry(QgsGeometry.fromWkt('Point(9 45)'))
        new_features = QgsVectorLayerUtils.makeFeaturesCompatible([f1], nogeom_layer)
        self.assertEqual(len(new_features), 1)
        self.assertEqual(new_features[0].geometry().asWkt(), '')

    def _alg_tester(self, alg_name, input_layer, parameters, invalid_geometry_policy=QgsFeatureRequest.GeometryNoCheck, retain_selection=False):

        alg = self.registry.createAlgorithmById(alg_name)

        self.assertIsNotNone(alg)
        parameters['INPUT'] = input_layer
        parameters['OUTPUT'] = 'memory:'

        old_features = [f for f in input_layer.getFeatures()]

        if not retain_selection:
            input_layer.selectByIds([old_features[0].id()])
            # Check selected
            self.assertEqual(input_layer.selectedFeatureIds(), [old_features[0].id()], alg_name)

        context = QgsProcessingContext()
        context.setInvalidGeometryCheck(invalid_geometry_policy)
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        input_layer.rollBack()
        ok = False
        ok, _ = execute_in_place_run(
            alg, parameters, context=context, feedback=feedback, raise_exceptions=True)
        new_features = [f for f in input_layer.getFeatures()]

        # Check ret values
        self.assertTrue(ok, alg_name)

        # Check geometry types (drop Z or M)
        self.assertEqual(new_features[0].geometry().wkbType(), old_features[0].geometry().wkbType())

        return old_features, new_features

    def test_execute_in_place_run(self):
        """Test the execution in place"""

        self.vl.rollBack()

        old_features, new_features = self._alg_tester(
            'native:translategeometry',
            self.vl,
            {
                'DELTA_X': 1.1,
                'DELTA_Y': 1.1,
            }
        )

        # First feature was selected and modified
        self.assertEqual(new_features[0].id(), old_features[0].id())
        self.assertAlmostEqual(new_features[0].geometry().asPoint().x(), old_features[0].geometry().asPoint().x() + 1.1,
                               delta=0.01)
        self.assertAlmostEqual(new_features[0].geometry().asPoint().y(), old_features[0].geometry().asPoint().y() + 1.1,
                               delta=0.01)

        # Second feature was not selected and not modified
        self.assertEqual(new_features[1].id(), old_features[1].id())
        self.assertEqual(new_features[1].geometry().asPoint().x(), old_features[1].geometry().asPoint().x())
        self.assertEqual(new_features[1].geometry().asPoint().y(), old_features[1].geometry().asPoint().y())

        # Check selected
        self.assertEqual(self.vl.selectedFeatureIds(), [old_features[0].id()])

        # Check that if the only change is Z or M then we should fail
        with self.assertRaises(QgsProcessingException) as cm:
            self._alg_tester(
                'native:translategeometry',
                self.vl,
                {
                    'DELTA_Z': 1.1,
                }
            )
        self.vl.rollBack()

        # Check that if the only change is Z or M then we should fail
        with self.assertRaises(QgsProcessingException) as cm:
            self._alg_tester(
                'native:translategeometry',
                self.vl,
                {
                    'DELTA_M': 1.1,
                }
            )
        self.vl.rollBack()

        old_features, new_features = self._alg_tester(
            'native:translategeometry',
            self.vl,
            {
                'DELTA_X': 1.1,
                'DELTA_Z': 1.1,
            }
        )

    def test_select_all_features(self):
        """Check that if there is no selection, the alg will run on all features"""

        self.vl.rollBack()
        self.vl.removeSelection()
        old_count = self.vl.featureCount()

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        alg = self.registry.createAlgorithmById('native:translategeometry')

        self.assertIsNotNone(alg)

        parameters = {
            'DELTA_X': 1.1,
            'DELTA_Y': 1.1,
        }
        parameters['INPUT'] = self.vl
        parameters['OUTPUT'] = 'memory:'

        old_features = [f for f in self.vl.getFeatures()]

        ok, _ = execute_in_place_run(
            alg, parameters, context=context, feedback=feedback, raise_exceptions=True)
        new_features = [f for f in self.vl.getFeatures()]

        self.assertEqual(len(new_features), old_count)

        # Check all are selected
        self.assertEqual(len(self.vl.selectedFeatureIds()), old_count)

    def test_multi_to_single(self):
        """Check that the geometry type is still multi after the alg is run"""

        old_features, new_features = self._alg_tester(
            'native:multiparttosingleparts',
            self.multipoly_vl,
            {
            }
        )

        self.assertEqual(len(new_features), 3)

        # Check selected
        self.assertEqual(len(self.multipoly_vl.selectedFeatureIds()), 2)

    def test_arraytranslatedfeatures(self):
        """Check that this runs correctly and additional attributes are dropped"""

        old_count = self.vl.featureCount()

        old_features, new_features = self._alg_tester(
            'native:arraytranslatedfeatures',
            self.vl,
            {
                'COUNT': 2,
                'DELTA_X': 1.1,
                'DELTA_Z': 1.1,
            }
        )

        self.assertEqual(len(new_features), old_count + 2)

        # Check selected
        self.assertEqual(len(self.vl.selectedFeatureIds()), 3)

    def test_reprojectlayer(self):
        """Check that this runs correctly"""

        old_count = self.vl.featureCount()

        old_features, new_features = self._alg_tester(
            'native:reprojectlayer',
            self.vl,
            {
                'TARGET_CRS': 'EPSG:3857',
            }
        )

        g = [f.geometry() for f in new_features][0]
        self.assertAlmostEqual(g.constGet().x(), 1001875.4, 1)
        self.assertAlmostEqual(g.constGet().y(), 5621521.5, 1)

        # Check selected
        self.assertEqual(self.vl.selectedFeatureIds(), [1])

    def test_snappointstogrid(self):
        """Check that this runs correctly"""

        polygon_layer = self._make_layer('Polygon')
        f1 = QgsFeature(polygon_layer.fields())
        f1.setAttributes([1])
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((1.2 1.2, 1.2 2.2, 2.2 2.2, 2.2 1.2, 1.2 1.2))'))
        f2 = QgsFeature(polygon_layer.fields())
        f2.setAttributes([2])
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((1.1 1.1, 1.1 2.1, 2.1 2.1, 2.1 1.1, 1.1 1.1))'))
        self.assertTrue(f2.isValid())
        self.assertTrue(polygon_layer.startEditing())
        self.assertTrue(polygon_layer.addFeatures([f1, f2]))
        self.assertEqual(polygon_layer.featureCount(), 2)
        polygon_layer.commitChanges()
        self.assertEqual(polygon_layer.featureCount(), 2)
        QgsProject.instance().addMapLayers([polygon_layer])

        polygon_layer.selectByIds([next(polygon_layer.getFeatures()).id()])
        self.assertEqual(polygon_layer.selectedFeatureCount(), 1)

        old_features, new_features = self._alg_tester(
            'native:snappointstogrid',
            polygon_layer,
            {
                'HSPACING': 0.5,
                'VSPACING': 0.5,
            }
        )

        g = [f.geometry() for f in new_features][0]
        self.assertEqual(g.asWkt(), 'Polygon ((1 1, 1 2, 2 2, 2 1, 1 1))')
        # Check selected
        self.assertEqual(polygon_layer.selectedFeatureIds(), [1])

    def test_clip(self):

        mask_layer = QgsMemoryProviderUtils.createMemoryLayer(
            'mask_layer', self.vl.fields(), QgsWkbTypes.Polygon, QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertTrue(mask_layer.isValid())
        self.assertTrue(mask_layer.startEditing())
        f = QgsFeature(mask_layer.fields())
        f.setAttributes([1])
        f.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))'))
        self.assertTrue(f.isValid())
        f2 = QgsFeature(mask_layer.fields())
        f2.setAttributes([1])
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((1.1 1.1, 1.1 2.1, 2.1 2.1, 2.1 1.1, 1.1 1.1))'))
        self.assertTrue(f2.isValid())
        self.assertTrue(mask_layer.addFeatures([f, f2]))
        mask_layer.commitChanges()
        mask_layer.rollBack()

        clip_layer = QgsMemoryProviderUtils.createMemoryLayer(
            'clip_layer', self.vl.fields(), QgsWkbTypes.LineString, QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertTrue(clip_layer.isValid())
        self.assertTrue(clip_layer.startEditing())
        f = QgsFeature(clip_layer.fields())
        f.setAttributes([1])
        f.setGeometry(QgsGeometry.fromWkt('LINESTRING(-1 -1, 3 3)'))
        self.assertTrue(f.isValid())
        self.assertTrue(clip_layer.addFeatures([f]))
        self.assertEqual(clip_layer.featureCount(), 1)
        clip_layer.commitChanges()
        clip_layer.selectAll()
        clip_layer.rollBack()

        QgsProject.instance().addMapLayers([clip_layer, mask_layer])

        old_features, new_features = self._alg_tester(
            'native:clip',
            clip_layer,
            {
                'OVERLAY': mask_layer.id(),
            }
        )

        self.assertEqual(len(new_features), 2)
        self.assertEqual(new_features[0].geometry().asWkt(), 'LineString (0 0, 1 1)')
        self.assertEqual(new_features[0].attributes(), [1])

    def test_fix_geometries(self):

        polygon_layer = self._make_layer('Polygon')
        self.assertTrue(polygon_layer.startEditing())
        f1 = QgsFeature(polygon_layer.fields())
        f1.setAttributes([1])
        # Flake!
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON ((0 0, 2 2, 0 2, 2 0, 0 0))'))
        self.assertTrue(f1.isValid())
        f2 = QgsFeature(polygon_layer.fields())
        f2.setAttributes([1])
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((1.1 1.1, 1.1 2.1, 2.1 2.1, 2.1 1.1, 1.1 1.1))'))
        self.assertTrue(f2.isValid())
        self.assertTrue(polygon_layer.addFeatures([f1, f2]))
        polygon_layer.commitChanges()
        polygon_layer.rollBack()
        self.assertEqual(polygon_layer.featureCount(), 2)

        QgsProject.instance().addMapLayers([polygon_layer])

        old_features, new_features = self._alg_tester(
            'native:fixgeometries',
            polygon_layer,
            {
            },
            QgsFeatureRequest.GeometrySkipInvalid
        )
        self.assertEqual(polygon_layer.featureCount(), 3)
        wkt1, wkt2, wkt3 = [f.geometry().asWkt() for f in new_features]
        self.assertEqual(wkt1, 'Polygon ((0 0, 1 1, 2 0, 0 0))')
        self.assertEqual(wkt2, 'Polygon ((1 1, 0 2, 2 2, 1 1))')
        self.assertEqual(re.sub(r'0000\d+', '', wkt3), 'Polygon ((1.1 1.1, 1.1 2.1, 2.1 2.1, 2.1 1.1, 1.1 1.1))')

        # Test with Z (interpolated)
        polygonz_layer = self._make_layer('PolygonZ')
        self.assertTrue(polygonz_layer.startEditing())

        f3 = QgsFeature(polygonz_layer.fields())
        f3.setAttributes([1])
        f3.setGeometry(QgsGeometry.fromWkt('POLYGON Z((0 0 1, 2 2 1, 0 2 3, 2 0 4, 0 0 1))'))
        self.assertTrue(f3.isValid())
        self.assertTrue(polygonz_layer.addFeatures([f3]))
        polygonz_layer.commitChanges()
        polygonz_layer.rollBack()
        self.assertEqual(polygonz_layer.featureCount(), 1)

        QgsProject.instance().addMapLayers([polygonz_layer])

        old_features, new_features = self._alg_tester(
            'native:fixgeometries',
            polygonz_layer,
            {
            }
        )
        self.assertEqual(polygonz_layer.featureCount(), 2)
        wkt1, wkt2 = [f.geometry().asWkt() for f in new_features]
        self.assertEqual(wkt1, 'PolygonZ ((0 0 1, 1 1 2.25, 2 0 4, 0 0 1))')
        self.assertEqual(wkt2, 'PolygonZ ((1 1 2.25, 0 2 3, 2 2 1, 1 1 2.25))')

    def _test_difference_on_invalid_geometries(self, geom_option):
        polygon_layer = self._make_layer('Polygon')
        self.assertTrue(polygon_layer.startEditing())
        f = QgsFeature(polygon_layer.fields())
        f.setAttributes([1])
        # Flake!
        f.setGeometry(QgsGeometry.fromWkt('Polygon ((0 0, 2 2, 0 2, 2 0, 0 0))'))
        self.assertTrue(f.isValid())
        self.assertTrue(polygon_layer.addFeatures([f]))
        polygon_layer.commitChanges()
        polygon_layer.rollBack()
        self.assertEqual(polygon_layer.featureCount(), 1)

        overlay_layer = self._make_layer('Polygon')
        self.assertTrue(overlay_layer.startEditing())
        f = QgsFeature(overlay_layer.fields())
        f.setAttributes([1])
        f.setGeometry(QgsGeometry.fromWkt('Polygon ((0 0, 2 0, 2 2, 0 2, 0 0))'))
        self.assertTrue(f.isValid())
        self.assertTrue(overlay_layer.addFeatures([f]))
        overlay_layer.commitChanges()
        overlay_layer.rollBack()
        self.assertEqual(overlay_layer.featureCount(), 1)

        QgsProject.instance().addMapLayers([polygon_layer, overlay_layer])

        old_features = [f for f in polygon_layer.getFeatures()]

        # 'Ignore features with invalid geometries' = 1
        ProcessingConfig.setSettingValue(ProcessingConfig.FILTER_INVALID_GEOMETRIES, geom_option)

        feedback = ConsoleFeedBack()
        context = dataobjects.createContext(feedback)
        context.setProject(QgsProject.instance())

        alg = self.registry.createAlgorithmById('native:difference')
        self.assertIsNotNone(alg)

        parameters = {
            'OVERLAY': overlay_layer,
            'INPUT': polygon_layer,
            'OUTPUT': ':memory',
        }

        old_features = [f for f in polygon_layer.getFeatures()]

        self.assertTrue(polygon_layer.startEditing())
        polygon_layer.selectAll()
        ok, _ = execute_in_place_run(
            alg, parameters, context=context, feedback=feedback, raise_exceptions=True)

        new_features = [f for f in polygon_layer.getFeatures()]

        return old_features, new_features

    def test_difference_on_invalid_geometries(self):
        """Test #20147 difference deletes invalid geometries"""

        old_features, new_features = self._test_difference_on_invalid_geometries(1)
        self.assertEqual(len(new_features), 1)
        old_features, new_features = self._test_difference_on_invalid_geometries(0)
        self.assertEqual(len(new_features), 1)
        old_features, new_features = self._test_difference_on_invalid_geometries(2)
        self.assertEqual(len(new_features), 1)

    def test_unique_constraints(self):
        """Test issue #31634"""
        temp_dir = QTemporaryDir()
        temp_path = temp_dir.path()
        gpkg_name = 'bug_31634_Multi_to_Singleparts_FID.gpkg'
        gpkg_path = os.path.join(temp_path, gpkg_name)
        shutil.copyfile(os.path.join(unitTestDataPath(), gpkg_name), gpkg_path)

        gpkg_layer = QgsVectorLayer(gpkg_path + '|layername=Multi_to_Singleparts_FID_bug', 'lyr', 'ogr')
        self.assertTrue(gpkg_layer.isValid())
        QgsProject.instance().addMapLayers([gpkg_layer])

        # Test that makeFeaturesCompatible set to NULL unique constraint fields
        feature = next(gpkg_layer.getFeatures())

        feedback = ConsoleFeedBack()
        context = dataobjects.createContext(feedback)
        context.setProject(QgsProject.instance())

        alg = self.registry.createAlgorithmById('native:multiparttosingleparts')
        self.assertIsNotNone(alg)

        parameters = {
            'INPUT': gpkg_layer,
            'OUTPUT': ':memory',
        }

        ok, _ = execute_in_place_run(
            alg, parameters, context=context, feedback=feedback, raise_exceptions=True)

        pks = set()
        for f in gpkg_layer.getFeatures():
            pks.add(f.attribute(0))

        self.assertTrue(gpkg_layer.commitChanges())

    def test_regenerate_fid(self):
        """Test RegeneratePrimaryKey flag"""

        temp_dir = QTemporaryDir()
        temp_path = temp_dir.path()
        gpkg_name = 'bug_31634_Multi_to_Singleparts_FID.gpkg'
        gpkg_path = os.path.join(temp_path, gpkg_name)
        shutil.copyfile(os.path.join(unitTestDataPath(), gpkg_name), gpkg_path)

        gpkg_layer = QgsVectorLayer(gpkg_path + '|layername=Multi_to_Singleparts_FID_bug', 'lyr', 'ogr')
        self.assertTrue(gpkg_layer.isValid())

        f = next(gpkg_layer.getFeatures())
        self.assertEqual(f['fid'], 1)
        res = QgsVectorLayerUtils.makeFeatureCompatible(f, gpkg_layer)
        self.assertEqual([ff['fid'] for ff in res], [1])

        # if RegeneratePrimaryKey set then we should discard fid field
        res = QgsVectorLayerUtils.makeFeatureCompatible(f, gpkg_layer, QgsFeatureSink.RegeneratePrimaryKey)
        self.assertEqual([ff['fid'] for ff in res], [None])

    def test_datadefinedvalue(self):
        """Check that data defined parameters work correctly"""

        polygon_layer = self._make_layer('Polygon')
        f1 = QgsFeature(polygon_layer.fields())
        f1.setAttributes([1])
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((1.2 1.2, 1.2 2.2, 2.2 2.2, 2.2 1.2, 1.2 1.2))'))
        f2 = QgsFeature(polygon_layer.fields())
        f2.setAttributes([2])
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((1.1 1.1, 1.1 2.1, 2.1 2.1, 2.1 1.1, 1.1 1.1))'))
        self.assertTrue(f2.isValid())
        self.assertTrue(polygon_layer.startEditing())
        self.assertTrue(polygon_layer.addFeatures([f1, f2]))
        self.assertEqual(polygon_layer.featureCount(), 2)
        polygon_layer.commitChanges()
        self.assertEqual(polygon_layer.featureCount(), 2)
        QgsProject.instance().addMapLayers([polygon_layer])

        polygon_layer.selectAll()
        self.assertEqual(polygon_layer.selectedFeatureCount(), 2)

        old_features, new_features = self._alg_tester(
            'native:densifygeometries',
            polygon_layer,
            {
                'VERTICES': QgsProperty.fromField('int_f'),
            }, retain_selection=True
        )

        geometries = [f.geometry() for f in new_features]
        self.assertEqual(geometries[0].asWkt(2), 'Polygon ((1.2 1.2, 1.2 1.7, 1.2 2.2, 1.7 2.2, 2.2 2.2, 2.2 1.7, 2.2 1.2, 1.7 1.2, 1.2 1.2))')
        self.assertEqual(geometries[1].asWkt(2), 'Polygon ((1.1 1.1, 1.1 1.43, 1.1 1.77, 1.1 2.1, 1.43 2.1, 1.77 2.1, 2.1 2.1, 2.1 1.77, 2.1 1.43, 2.1 1.1, 1.77 1.1, 1.43 1.1, 1.1 1.1))')
        # Check selected
        self.assertCountEqual(polygon_layer.selectedFeatureIds(), [1, 2])


if __name__ == '__main__':
    unittest.main()
