# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProfileRequest

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/03/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os

import qgis  # NOQA

from qgis.PyQt.QtCore import QTemporaryDir

from qgis.core import (
    QgsLineString,
    QgsProfileRequest,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsFlatTerrainProvider,
    QgsMeshTerrainProvider,
    QgsExpressionContext,
    QgsExpressionContextScope
)

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsProfileRequest(unittest.TestCase):

    def testBasic(self):
        req = QgsProfileRequest(QgsLineString([[1, 2], [3, 4]]))
        self.assertEqual(req.profileCurve().asWkt(), 'LineString (1 2, 3 4)')

        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857')).setTolerance(5).setStepDistance(15)
        self.assertEqual(req.crs().authid(), 'EPSG:3857')
        self.assertEqual(req.tolerance(), 5)
        self.assertEqual(req.stepDistance(), 15)

        proj_string = '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'
        transform_context = QgsCoordinateTransformContext()
        transform_context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                 QgsCoordinateReferenceSystem('EPSG:4283'), proj_string)
        req.setTransformContext(transform_context)
        self.assertEqual(req.transformContext().calculateCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                             QgsCoordinateReferenceSystem('EPSG:4283')), proj_string)

        exp_context = QgsExpressionContext()
        context_scope = QgsExpressionContextScope()
        context_scope.setVariable('test_var', 5, True)
        exp_context.appendScope(context_scope)
        req.setExpressionContext(exp_context)

        self.assertEqual(req.expressionContext().variable('test_var'), 5)

        terrain = QgsFlatTerrainProvider()
        terrain.setOffset(5)
        req.setTerrainProvider(terrain)
        self.assertEqual(req.terrainProvider().offset(), 5)

        copy = QgsProfileRequest(req)
        self.assertEqual(copy.profileCurve().asWkt(), 'LineString (1 2, 3 4)')
        self.assertEqual(copy.crs().authid(), 'EPSG:3857')
        self.assertEqual(copy.tolerance(), 5)
        self.assertEqual(copy.stepDistance(), 15)
        self.assertEqual(copy.transformContext().calculateCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                              QgsCoordinateReferenceSystem('EPSG:4283')), proj_string)
        self.assertIsInstance(copy.terrainProvider(), QgsFlatTerrainProvider)
        self.assertEqual(copy.terrainProvider().offset(), 5)
        self.assertEqual(copy.expressionContext().variable('test_var'), 5)

    def testEquality(self):
        """
        Test equality operator
        """
        req = QgsProfileRequest(None)
        req2 = QgsProfileRequest(None)
        self.assertEqual(req, req2)

        req.setProfileCurve(QgsLineString([[1, 2], [3, 4]]))
        self.assertNotEqual(req, req2)

        req2.setProfileCurve(QgsLineString([[1, 2], [3, 5]]))
        self.assertNotEqual(req, req2)

        req.setProfileCurve(None)
        self.assertNotEqual(req, req2)

        req.setProfileCurve(QgsLineString([[1, 2], [3, 5]]))
        self.assertEqual(req, req2)

        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertNotEqual(req, req2)
        req2.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(req, req2)

        proj_string = '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'
        transform_context = QgsCoordinateTransformContext()
        transform_context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                 QgsCoordinateReferenceSystem('EPSG:4283'), proj_string)

        req.setTransformContext(transform_context)
        self.assertNotEqual(req, req2)
        req2.setTransformContext(transform_context)
        self.assertEqual(req, req2)

        req.setTolerance(5)
        self.assertNotEqual(req, req2)
        req2.setTolerance(5)
        self.assertEqual(req, req2)

        req.setStepDistance(15)
        self.assertNotEqual(req, req2)
        req2.setStepDistance(15)
        self.assertEqual(req, req2)

        terrain = QgsFlatTerrainProvider()
        terrain.setOffset(5)
        req.setTerrainProvider(terrain)
        self.assertNotEqual(req, req2)

        req2.setTerrainProvider(QgsMeshTerrainProvider())
        self.assertNotEqual(req, req2)

        req.setTerrainProvider(None)
        self.assertNotEqual(req, req2)

        req.setTerrainProvider(QgsFlatTerrainProvider())
        self.assertNotEqual(req, req2)

        req.setTerrainProvider(QgsMeshTerrainProvider())
        self.assertEqual(req, req2)


if __name__ == '__main__':
    unittest.main()
