# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthias Kuhn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import os
import sys
import difflib
import functools

from qgis.PyQt.QtCore import QVariant
from qgis.core import QgsApplication, QgsFeatureRequest, NULL
import unittest

# Get a backup, we will patch this one later
_TestCase = unittest.TestCase
unittest.util._MAX_LENGTH = 2000


class TestCase(_TestCase):

    def assertLayersEqual(self, layer_expected, layer_result, **kwargs):
        """
        :param layer_expected: The first layer to compare
        :param layer_result: The second layer to compare
        :param request: Optional, A feature request. This can be used to specify
                        an order by clause to make sure features are compared in
                        a given sequence if they don't match by default.
        :keyword compare: A map of comparison options. e.g.
                         { fields: { a: skip, b: { precision: 2 }, geometry: { precision: 5 } }
                         { fields: { __all__: cast( str ) } }
        :keyword pk: "Primary key" type field - used to match features
        from the expected table to their corresponding features in the result table. If not specified
        features are compared by their order in the layer (e.g. first feature compared with first feature,
        etc)
        """
        self.checkLayersEqual(layer_expected, layer_result, True, **kwargs)

    def checkLayersEqual(self, layer_expected, layer_result, use_asserts=False, **kwargs):
        """
        :param layer_expected: The first layer to compare
        :param layer_result: The second layer to compare
        :param use_asserts: If true, asserts are used to test conditions, if false, asserts
        are not used and the function will only return False if the test fails
        :param request: Optional, A feature request. This can be used to specify
                        an order by clause to make sure features are compared in
                        a given sequence if they don't match by default.
        :keyword compare: A map of comparison options. e.g.
                         { fields: { a: skip, b: { precision: 2 }, geometry: { precision: 5 } }
                         { fields: { __all__: cast( str ) } }
        :keyword pk: "Primary key" type field - used to match features
        from the expected table to their corresponding features in the result table. If not specified
        features are compared by their order in the layer (e.g. first feature compared with first feature,
        etc)
        """

        try:
            request = kwargs['request']
        except KeyError:
            request = QgsFeatureRequest()

        try:
            compare = kwargs['compare']
        except KeyError:
            compare = {}

        # Compare CRS
        if 'ignore_crs_check' not in compare or not compare['ignore_crs_check']:
            if use_asserts:
                _TestCase.assertEqual(self, layer_expected.dataProvider().crs().authid(), layer_result.dataProvider().crs().authid())
            elif not layer_expected.dataProvider().crs().authid() == layer_result.dataProvider().crs().authid():
                return False

        # Compare features
        if use_asserts:
            _TestCase.assertEqual(self, layer_expected.featureCount(), layer_result.featureCount())
        elif layer_expected.featureCount() != layer_result.featureCount():
            return False

        try:
            precision = compare['geometry']['precision']
        except KeyError:
            precision = 14

        try:
            topo_equal_check = compare['geometry']['topo_equal_check']
        except KeyError:
            topo_equal_check = False

        try:
            unordered = compare['unordered']
        except KeyError:
            unordered = False

        if unordered:
            features_expected = [f for f in layer_expected.getFeatures(request)]
            for feat in layer_result.getFeatures(request):
                feat_expected_equal = None
                for feat_expected in features_expected:
                    if self.checkGeometriesEqual(feat.geometry(), feat_expected.geometry(),
                                                 feat.id(), feat_expected.id(),
                                                 False, precision, topo_equal_check) and \
                       self.checkAttributesEqual(feat, feat_expected, layer_expected.fields(), False, compare):
                        feat_expected_equal = feat_expected
                        break

                if feat_expected_equal is not None:
                    features_expected.remove(feat_expected_equal)
                else:
                    if use_asserts:
                        _TestCase.assertTrue(
                            self, False,
                            'Unexpected result feature: fid {}, geometry: {}, attributes: {}'.format(
                                feat.id(),
                                feat.geometry().constGet().asWkt(precision) if feat.geometry() else 'NULL',
                                feat.attributes())
                        )
                    else:
                        return False

            if len(features_expected) != 0:
                if use_asserts:
                    lst_missing = []
                    for feat in features_expected:
                        lst_missing.append('fid {}, geometry: {}, attributes: {}'.format(
                            feat.id(),
                            feat.geometry().constGet().asWkt(precision) if feat.geometry() else 'NULL',
                            feat.attributes())
                        )
                    _TestCase.assertTrue(self, False, 'Some expected features not found in results:\n' + '\n'.join(lst_missing))
                else:
                    return False

            return True

        def sort_by_pk_or_fid(f):
            if 'pk' in kwargs and kwargs['pk'] is not None:
                key = kwargs['pk']
                if isinstance(key, list) or isinstance(key, tuple):
                    return [f[k] for k in key]
                else:
                    return f[kwargs['pk']]
            else:
                return f.id()

        expected_features = sorted(layer_expected.getFeatures(request), key=sort_by_pk_or_fid)
        result_features = sorted(layer_result.getFeatures(request), key=sort_by_pk_or_fid)

        for feats in zip(expected_features, result_features):

            eq = self.checkGeometriesEqual(feats[0].geometry(),
                                           feats[1].geometry(),
                                           feats[0].id(),
                                           feats[1].id(),
                                           use_asserts, precision, topo_equal_check)
            if not eq and not use_asserts:
                return False

            eq = self.checkAttributesEqual(feats[0], feats[1], layer_expected.fields(), use_asserts, compare)
            if not eq and not use_asserts:
                return False

        return True

    def assertFilesEqual(self, filepath_expected, filepath_result):
        with open(filepath_expected, 'r') as file_expected:
            with open(filepath_result, 'r') as file_result:
                diff = difflib.unified_diff(
                    file_expected.readlines(),
                    file_result.readlines(),
                    fromfile='expected',
                    tofile='result',
                )
                diff = list(diff)
                self.assertEqual(0, len(diff), ''.join(diff))

    def assertGeometriesEqual(self, geom0, geom1, geom0_id='geometry 1', geom1_id='geometry 2', precision=14, topo_equal_check=False):
        self.checkGeometriesEqual(geom0, geom1, geom0_id, geom1_id, use_asserts=True, precision=precision, topo_equal_check=topo_equal_check)

    def checkGeometriesEqual(self, geom0, geom1, geom0_id, geom1_id, use_asserts=False, precision=14, topo_equal_check=False):
        """ Checks whether two geometries are the same - using either a strict check of coordinates (up to given precision)
        or by using topological equality (where e.g. a polygon with clockwise is equal to a polygon with counter-clockwise
        order of vertices)
        .. versionadded:: 3.2
        """
        if not geom0.isNull() and not geom1.isNull():
            if topo_equal_check:
                equal = geom0.isGeosEqual(geom1)
            else:
                equal = geom0.constGet().asWkt(precision) == geom1.constGet().asWkt(precision)
        elif geom0.isNull() and geom1.isNull():
            equal = True
        else:
            equal = False

        if use_asserts:
            _TestCase.assertTrue(
                self,
                equal, ''
                ' Features (Expected fid: {}, Result fid: {}) differ in geometry with method {}: \n\n'
                '  At given precision ({}):\n'
                '   Expected geometry: {}\n'
                '   Result geometry:   {}\n\n'
                '  Full precision:\n'
                '   Expected geometry : {}\n'
                '   Result geometry:   {}\n\n'.format(
                    geom0_id,
                    geom1_id,
                    'geos' if topo_equal_check else 'wkt',
                    precision,
                    geom0.constGet().asWkt(precision) if not geom0.isNull() else 'NULL',
                    geom1.constGet().asWkt(precision) if not geom1.isNull() else 'NULL',
                    geom0.constGet().asWkt() if not geom1.isNull() else 'NULL',
                    geom1.constGet().asWkt() if not geom0.isNull() else 'NULL'
                )
            )
        else:
            return equal

    def checkAttributesEqual(self, feat0, feat1, fields_expected, use_asserts, compare):
        """ Checks whether attributes of two features are the same
        .. versionadded:: 3.2
        """

        for attr_expected, field_expected in zip(feat0.attributes(), fields_expected.toList()):
            try:
                cmp = compare['fields'][field_expected.name()]
            except KeyError:
                try:
                    cmp = compare['fields']['__all__']
                except KeyError:
                    cmp = {}

            # Skip field
            if 'skip' in cmp:
                continue

            if use_asserts:
                _TestCase.assertIn(
                    self,
                    field_expected.name().lower(),
                    [name.lower() for name in feat1.fields().names()])

            attr_result = feat1[field_expected.name()]
            field_result = [fld for fld in fields_expected.toList() if fld.name() == field_expected.name()][0]

            # Cast field to a given type
            if 'cast' in cmp:
                if cmp['cast'] == 'int':
                    attr_expected = int(attr_expected) if attr_expected else None
                    attr_result = int(attr_result) if attr_result else None
                if cmp['cast'] == 'float':
                    attr_expected = float(attr_expected) if attr_expected else None
                    attr_result = float(attr_result) if attr_result else None
                if cmp['cast'] == 'str':
                    attr_expected = str(attr_expected) if attr_expected else None
                    attr_result = str(attr_result) if attr_result else None

            # Round field (only numeric so it works with __all__)
            if 'precision' in cmp and field_expected.type() in [QVariant.Int, QVariant.Double, QVariant.LongLong]:
                if not attr_expected == NULL:
                    attr_expected = round(attr_expected, cmp['precision'])
                if not attr_result == NULL:
                    attr_result = round(attr_result, cmp['precision'])

            if use_asserts:
                _TestCase.assertEqual(
                    self,
                    attr_expected,
                    attr_result,
                    'Features {}/{} differ in attributes\n\n * Field expected: {} ({})\n * result  : {} ({})\n\n * Expected: {} != Result  : {}'.format(
                        feat0.id(),
                        feat1.id(),
                        field_expected.name(),
                        field_expected.typeName(),
                        field_result.name(),
                        field_result.typeName(),
                        repr(attr_expected),
                        repr(attr_result)
                    )
                )
            elif attr_expected != attr_result:
                return False

        return True


class _UnexpectedSuccess(Exception):

    """
    The test was supposed to fail, but it didn't!
    """
    pass


def expectedFailure(*args):
    """
    Will decorate a unittest function as an expectedFailure. A function
    flagged as expectedFailure will be succeed if it raises an exception.
    If it does not raise an exception, this will throw an
    `_UnexpectedSuccess` exception.

        @expectedFailure
        def my_test(self):
            self.assertTrue(False)

    The decorator also accepts a parameter to only expect a failure under
    certain conditions.

        @expectedFailure(time.localtime().tm_year < 2002)
        def my_test(self):
            self.assertTrue(qgisIsInvented())
    """
    if hasattr(args[0], '__call__'):
        # We got a function as parameter: assume usage like
        #   @expectedFailure
        #   def testfunction():
        func = args[0]

        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            try:
                func(*args, **kwargs)
            except Exception:
                pass
            else:
                raise _UnexpectedSuccess
        return wrapper
    else:
        # We got a function as parameter: assume usage like
        #   @expectedFailure(failsOnThisPlatform)
        #   def testfunction():
        condition = args[0]

        def realExpectedFailure(func):
            @functools.wraps(func)
            def wrapper(*args, **kwargs):
                if condition:
                    try:
                        func(*args, **kwargs)
                    except Exception:
                        pass
                    else:
                        raise _UnexpectedSuccess
                else:
                    func(*args, **kwargs)
            return wrapper

        return realExpectedFailure


# Patch unittest
unittest.TestCase = TestCase
unittest.expectedFailure = expectedFailure


def start_app(cleanup=True):
    """
    Will start a QgsApplication and call all initialization code like
    registering the providers and other infrastructure. It will not load
    any plugins.

    You can always get the reference to a running app by calling `QgsApplication.instance()`.

    The initialization will only happen once, so it is safe to call this method repeatedly.

        Parameters
        ----------

        cleanup: Do cleanup on exit. Defaults to true.

        Returns
        -------
        QgsApplication

        A QgsApplication singleton
    """
    global QGISAPP

    try:
        QGISAPP
    except NameError:
        myGuiFlag = True  # All test will run qgis in gui mode

        # In python3 we need to convert to a bytes object (or should
        # QgsApplication accept a QString instead of const char* ?)
        try:
            argvb = list(map(os.fsencode, sys.argv))
        except AttributeError:
            argvb = sys.argv

        # Note: QGIS_PREFIX_PATH is evaluated in QgsApplication -
        # no need to mess with it here.
        QGISAPP = QgsApplication(argvb, myGuiFlag)

        QGISAPP.initQgis()
        print(QGISAPP.showSettings())

        def debug_log_message(message, tag, level):
            print('{}({}): {}'.format(tag, level, message))

        QgsApplication.instance().messageLog().messageReceived.connect(debug_log_message)

        if cleanup:
            import atexit

            @atexit.register
            def exitQgis():
                QGISAPP.exitQgis()

    return QGISAPP


def stop_app():
    """
    Cleans up and exits QGIS
    """
    global QGISAPP

    QGISAPP.exitQgis()
    del QGISAPP
