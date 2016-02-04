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

from PyQt4.QtCore import QVariant
from qgis.core import QgsApplication, QgsFeatureRequest, QgsVectorLayer
from nose2.compat import unittest

# Get a backup, we will patch this one later
_TestCase = unittest.TestCase


class TestCase(_TestCase):

    def assertLayersEqual(self, layer1, layer2, **kwargs):
        """
        :param layer1: The first layer to compare
        :param layer2: The second layer to compare
        :param request: Optional, A feature request. This can be used to specify
                        an order by clause to make sure features are compared in
                        a given sequence if they don't match by default.
        :keyword compare: A map of comparison options. e.g.
                         { fields: { a: skip, b: { precision: 2 }, geometry: { precision: 5 } }
                         { fields: { __all__: cast( str ) } }
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
        _TestCase.assertEqual(self, layer1.dataProvider().crs().authid(), layer2.dataProvider().crs().authid())

        # Compare features
        _TestCase.assertEqual(self, layer1.featureCount(), layer2.featureCount())

        try:
            precision = compare['geometry']['precision']
        except KeyError:
            precision = 17

        for feats in zip(layer1.getFeatures(request), layer2.getFeatures(request)):
            if feats[0].geometry() is not None:
                geom0 = feats[0].geometry().geometry().asWkt(precision)
            else:
                geom0 = None
            if feats[1].geometry() is not None:
                geom1 = feats[1].geometry().geometry().asWkt(precision)
            else:
                geom1 = None
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

            for attr0, attr1, field1, field2 in zip(feats[0].attributes(), feats[1].attributes(), layer1.fields().toList(), layer2.fields().toList()):
                try:
                    cmp = compare['fields'][field1.name()]
                except KeyError:
                    try:
                        cmp = compare['fields']['__all__']
                    except KeyError:
                        cmp = {}

                # Skip field
                if 'skip' in cmp:
                    continue

                # Cast field to a given type
                if 'cast' in cmp:
                    if cmp['cast'] == 'int':
                        attr0 = int(attr0) if attr0 else None
                        attr1 = int(attr1) if attr0 else None
                    if cmp['cast'] == 'float':
                        attr0 = float(attr0) if attr0 else None
                        attr1 = float(attr1) if attr0 else None
                    if cmp['cast'] == 'str':
                        attr0 = str(attr0)
                        attr1 = str(attr1)

                # Round field (only numeric so it works with __all__)
                if 'precision' in cmp and field1.type() in [QVariant.Int, QVariant.Double, QVariant.LongLong]:
                    attr0 = round(attr0, cmp['precision'])
                    attr1 = round(attr1, cmp['precision'])

                _TestCase.assertEqual(
                    self,
                    attr0,
                    attr1,
                    'Features {}/{} differ in attributes\n\n * Field1: {} ({})\n * Field2: {} ({})\n\n * {} != {}'.format(
                        feats[0].id(),
                        feats[1].id(),
                        field1.name(),
                        field1.typeName(),
                        field2.name(),
                        field2.typeName(),
                        repr(attr0),
                        repr(attr1)
                    )
                )

# Patch unittest
unittest.TestCase = TestCase


def start_app():
    """
    Will start a QgsApplication and call all initialization code like
    registering the providers and other infrastructure. It will not load
    any plugins.

    You can always get the reference to a running app by calling `QgsApplication.instance()`.

    The initialization will only happen once, so it is safe to call this method repeatedly.

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
        s = QGISAPP.showSettings()
        print(s)

    return QGISAPP


def stop_app():
    """
    Cleans up and exits QGIS
    """
    global QGISAPP

    QGISAPP.exitQgis()
    del QGISAPP
