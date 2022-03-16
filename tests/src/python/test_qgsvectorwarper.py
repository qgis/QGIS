# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorWarper

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '01/03/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA
from qgis.analysis import (
    QgsVectorWarper,
    QgsGcpPoint,
    QgsGcpTransformerInterface
)
from qgis.core import (
    QgsVectorLayer,
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsFeatureStore,
    QgsCoordinateReferenceSystem,
    QgsProject
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsVectorWarper(unittest.TestCase):

    def testWarper(self):
        # create source layer
        source_layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                      "addfeat", "memory")
        pr = source_layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
        f3 = QgsFeature()
        f3.setAttributes(["test3", 888])
        f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
        f4 = QgsFeature()
        f4.setAttributes(["test4", -1])
        f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(400, 300)))
        f5 = QgsFeature()
        f5.setAttributes(["test5", 0])
        f5.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(0, 0)))
        self.assertTrue(pr.addFeatures([f, f2, f3, f4, f5]))
        self.assertTrue(source_layer.featureCount() == 5)

        # create sink
        sink = QgsFeatureStore()
        warper = QgsVectorWarper(QgsGcpTransformerInterface.TransformMethod.PolynomialOrder1,
                                 [
                                     QgsGcpPoint(QgsPointXY(90, 210), QgsPointXY(8, 20),
                                                 QgsCoordinateReferenceSystem('EPSG:4283'), True),
                                     QgsGcpPoint(QgsPointXY(210, 190), QgsPointXY(20.5, 20),
                                                 QgsCoordinateReferenceSystem('EPSG:4283'), True),
                                     QgsGcpPoint(QgsPointXY(350, 220), QgsPointXY(30, 21),
                                                 QgsCoordinateReferenceSystem('EPSG:4283'), True),
                                     QgsGcpPoint(QgsPointXY(390, 290), QgsPointXY(39, 28),
                                                 QgsCoordinateReferenceSystem('EPSG:4283'), True),
                                 ],
                                 QgsCoordinateReferenceSystem('EPSG:4283'))

        self.assertTrue(warper.transformFeatures(source_layer.getFeatures(),
                                                 sink,
                                                 QgsProject.instance().transformContext()))

        self.assertEqual(sink.count(), 5)
        feature_map = {f.attributes()[0]: {'geom': f.geometry().asWkt(1),
                                           'attributes': f.attributes()} for f in sink.features()}
        self.assertEqual(feature_map, {'test': {'geom': 'Point (9.4 19.7)', 'attributes': ['test', 123]},
                                       'test2': {'geom': 'Point (18 19.9)', 'attributes': ['test2', 457]},
                                       'test3': {'geom': 'Point (26.6 20.1)', 'attributes': ['test3', 888]},
                                       'test4': {'geom': 'Point (39.6 28.5)', 'attributes': ['test4', -1]},
                                       'test5': {'geom': 'Point (-7.8 3)', 'attributes': ['test5', 0]}})


if __name__ == '__main__':
    unittest.main()
