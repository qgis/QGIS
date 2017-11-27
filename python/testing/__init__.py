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
            if feats[0].hasGeometry():
                geom0 = feats[0].geometry().constGet().asWkt(precision)
            else:
                geom0 = None
            if feats[1].hasGeometry():
                geom1 = feats[1].geometry().constGet().asWkt(precision)
            else:
                geom1 = None
            if use_asserts:
                _TestCase.assertEqual(
                    self,
                    geom0,
                    geom1,
                    'Features {}/{} differ in geometry: \n\n {}\n\n vs \n\n {}'.format(
                        feats[0].id(),
                        feats[1].id(),
                        geom0,
                        geom1
                    )
                )
            elif geom0 != geom1:
                return False

            for attr_expected, field_expected in zip(feats[0].attributes(), layer_expected.fields().toList()):
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

                attr_result = feats[1][field_expected.name()]
                field_result = [fld for fld in layer_expected.fields().toList() if fld.name() == field_expected.name()][0]

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
                            feats[0].id(),
                            feats[1].id(),
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
