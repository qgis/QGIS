# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCoordinateTransformContext

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '11/5/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsCoordinateReferenceSystem,
                       QgsCoordinateTransformContext,
                       QgsDatumTransform,
                       QgsReadWriteContext,
                       QgsProject,
                       QgsSettings,
                       QgsProjUtils)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtCore import QCoreApplication

app = start_app()


class TestQgsCoordinateTransformContext(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestQgsCoordinateTransformContext.com")
        QCoreApplication.setApplicationName("TestQgsCoordinateTransformContext")
        QgsSettings().clear()

    @unittest.skipIf(QgsProjUtils.projVersionMajor() >= 6, 'Skipped on proj6 builds')
    def testSourceDestinationDatumTransforms(self):
        context = QgsCoordinateTransformContext()
        self.assertEqual(context.sourceDestinationDatumTransforms(), {})
        self.assertFalse(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                   QgsCoordinateReferenceSystem('EPSG:4283'), 1, 2))
        self.assertTrue(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertFalse(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4326')))
        self.assertFalse(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3113'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem(4283), 3, 4))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem(28357), 7, 8))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(7, 8)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem('EPSG:28357'), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11)})

        # invalid additions
        self.assertFalse(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(),
                                                                    QgsCoordinateReferenceSystem('EPSG:28357'), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11)})
        self.assertFalse(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                    QgsCoordinateReferenceSystem(), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11)})

        # indicate no transform required
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(28357),
                                                                   QgsCoordinateReferenceSystem(28356), -1, -1))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                          ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3111),
                                                                   QgsCoordinateReferenceSystem(28356), 17, -1))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                          ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                          ('EPSG:3111', 'EPSG:28356'): QgsDatumTransform.TransformPair(17, -1)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3113),
                                                                   QgsCoordinateReferenceSystem(28356), -1, 18))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                          ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                          ('EPSG:3111', 'EPSG:28356'): QgsDatumTransform.TransformPair(17, -1),
                          ('EPSG:3113', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, 18)})
        # remove non-existing
        context.removeCoordinateOperation(QgsCoordinateReferenceSystem(3113), QgsCoordinateReferenceSystem(3111))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                          ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                          ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                          ('EPSG:3111', 'EPSG:28356'): QgsDatumTransform.TransformPair(17, -1),
                          ('EPSG:3113', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, 18)})

        # remove existing
        context.removeCoordinateOperation(QgsCoordinateReferenceSystem(3111),
                                          QgsCoordinateReferenceSystem(4283))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                          ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                          ('EPSG:3111', 'EPSG:28356'): QgsDatumTransform.TransformPair(17, -1),
                          ('EPSG:3113', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, 18)})
        context.removeCoordinateOperation(QgsCoordinateReferenceSystem(3111),
                                          QgsCoordinateReferenceSystem(28356))
        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                          ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                          ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                          ('EPSG:3113', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, 18)})

        context.clear()
        self.assertEqual(context.sourceDestinationDatumTransforms(), {})

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Skipped on non proj6 builds')
    def testSourceDestinationDatumTransformsProj6(self):
        context = QgsCoordinateTransformContext()
        self.assertEqual(context.sourceDestinationDatumTransforms(), {})
        proj_string = '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'
        self.assertFalse(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                       QgsCoordinateReferenceSystem('EPSG:4283'), proj_string))
        self.assertTrue(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertFalse(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4326')))
        self.assertFalse(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3113'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertEqual(context.coordinateOperations(), {('EPSG:3111', 'EPSG:4283'): proj_string})
        proj_string_2 = '+proj=pipeline +step +inv +proj=utm +zone=56 +south +ellps=GRS80 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'
        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                       QgsCoordinateReferenceSystem(4283), proj_string_2))
        self.assertEqual(context.coordinateOperations(), {('EPSG:3111', 'EPSG:4283'): proj_string,
                                                          ('EPSG:28356', 'EPSG:4283'): proj_string_2})
        proj_string_3 = '+proj=pipeline +step +inv +proj=utm +zone=56 +south +ellps=GRS80 +step +proj=utm +zone=57 +south +ellps=GRS80'
        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                       QgsCoordinateReferenceSystem(28357), proj_string_3))
        self.assertEqual(context.coordinateOperations(), {('EPSG:3111', 'EPSG:4283'): proj_string,
                                                          ('EPSG:28356', 'EPSG:4283'): proj_string_2,
                                                          ('EPSG:28356', 'EPSG:28357'): proj_string_3})
        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                       QgsCoordinateReferenceSystem('EPSG:28357'),
                                                       'some other proj string'))
        self.assertEqual(context.coordinateOperations(), {('EPSG:3111', 'EPSG:4283'): proj_string,
                                                          ('EPSG:28356', 'EPSG:4283'): proj_string_2,
                                                          ('EPSG:28356', 'EPSG:28357'): 'some other proj string'})

        # invalid additions
        self.assertFalse(context.addCoordinateOperation(QgsCoordinateReferenceSystem(),
                                                        QgsCoordinateReferenceSystem('EPSG:28357'), 'bad proj'))
        self.assertEqual(context.coordinateOperations(), {('EPSG:3111', 'EPSG:4283'): proj_string,
                                                          ('EPSG:28356', 'EPSG:4283'): proj_string_2,
                                                          ('EPSG:28356', 'EPSG:28357'): 'some other proj string'})
        self.assertFalse(context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                        QgsCoordinateReferenceSystem(), 'bad proj'))
        self.assertEqual(context.coordinateOperations(), {('EPSG:3111', 'EPSG:4283'): proj_string,
                                                          ('EPSG:28356', 'EPSG:4283'): proj_string_2,
                                                          ('EPSG:28356', 'EPSG:28357'): 'some other proj string'})

        # indicate no transform required
        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem(28357),
                                                       QgsCoordinateReferenceSystem(28356), ''))
        self.assertEqual(context.coordinateOperations(), {('EPSG:3111', 'EPSG:4283'): proj_string,
                                                          ('EPSG:28356', 'EPSG:4283'): proj_string_2,
                                                          ('EPSG:28356', 'EPSG:28357'): 'some other proj string',
                                                          ('EPSG:28357', 'EPSG:28356'): ''})

        # remove non-existing
        context.removeCoordinateOperation(QgsCoordinateReferenceSystem(3113), QgsCoordinateReferenceSystem(3111))
        self.assertEqual(context.coordinateOperations(), {('EPSG:3111', 'EPSG:4283'): proj_string,
                                                          ('EPSG:28356', 'EPSG:4283'): proj_string_2,
                                                          ('EPSG:28356', 'EPSG:28357'): 'some other proj string',
                                                          ('EPSG:28357', 'EPSG:28356'): ''})

        # remove existing
        context.removeCoordinateOperation(QgsCoordinateReferenceSystem(3111),
                                          QgsCoordinateReferenceSystem(4283))
        self.assertEqual(context.coordinateOperations(), {('EPSG:28356', 'EPSG:4283'): proj_string_2,
                                                          ('EPSG:28356', 'EPSG:28357'): 'some other proj string',
                                                          ('EPSG:28357', 'EPSG:28356'): ''})
        context.removeCoordinateOperation(QgsCoordinateReferenceSystem(28356),
                                          QgsCoordinateReferenceSystem(28357))
        self.assertEqual(context.coordinateOperations(), {('EPSG:28356', 'EPSG:4283'): proj_string_2,
                                                          ('EPSG:28357', 'EPSG:28356'): ''})

        context.clear()
        self.assertEqual(context.coordinateOperations(), {})

    @unittest.skipIf(QgsProjUtils.projVersionMajor() >= 6, 'Skipped on proj6 builds')
    def testCalculateSourceDest(self):
        context = QgsCoordinateTransformContext()

        # empty context
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         QgsDatumTransform.TransformPair(-1, -1))

        # add specific source/dest pair - should take precedence
        context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                   QgsCoordinateReferenceSystem('EPSG:4283'),
                                                   3, 4)
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         QgsDatumTransform.TransformPair(3, 4))
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         QgsDatumTransform.TransformPair(-1, -1))
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                          QgsCoordinateReferenceSystem('EPSG:3111')),
                         QgsDatumTransform.TransformPair(-1, -1))
        # check that reverse transforms are automatically supported
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:4283'),
                                                          QgsCoordinateReferenceSystem('EPSG:28356')),
                         QgsDatumTransform.TransformPair(4, 3))

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Skipped on non proj6 builds')
    def testCalculateSourceDestProj6(self):
        context = QgsCoordinateTransformContext()

        # empty context
        self.assertEqual(context.calculateCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                              QgsCoordinateReferenceSystem('EPSG:4283')),
                         '')

        # add specific source/dest pair - should take precedence
        context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                       QgsCoordinateReferenceSystem('EPSG:4283'),
                                       'proj 1')
        self.assertEqual(context.calculateCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                              QgsCoordinateReferenceSystem('EPSG:4283')),
                         'proj 1')
        self.assertEqual(context.calculateCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                              QgsCoordinateReferenceSystem('EPSG:4283')),
                         '')
        self.assertEqual(context.calculateCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                              QgsCoordinateReferenceSystem('EPSG:3111')),
                         '')
        # check that reverse transforms are automatically supported
        self.assertEqual(context.calculateCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:4283'),
                                                              QgsCoordinateReferenceSystem('EPSG:28356')),
                         'proj 1')

    @unittest.skipIf(QgsProjUtils.projVersionMajor() >= 6, 'Skipped on proj6 builds')
    def testWriteReadXml(self):
        # setup a context
        context = QgsCoordinateTransformContext()

        source_id_1 = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4204),
                                                             QgsCoordinateReferenceSystem(4326))[0].sourceTransformId
        dest_id_1 = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4204),
                                                           QgsCoordinateReferenceSystem(4326))[0].destinationTransformId

        source_id_2 = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4205),
                                                             QgsCoordinateReferenceSystem(4326))[0].sourceTransformId
        dest_id_2 = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4205),
                                                           QgsCoordinateReferenceSystem(4326))[0].destinationTransformId

        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(4204),
                                                                   QgsCoordinateReferenceSystem(4326), source_id_1,
                                                                   dest_id_1))
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(4205),
                                                                   QgsCoordinateReferenceSystem(4326), source_id_2,
                                                                   dest_id_2))

        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:4204', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_1, dest_id_1),
                          ('EPSG:4205', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_2, dest_id_2)})

        # save to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        context.writeXml(elem, QgsReadWriteContext())

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        context2.readXml(elem, QgsReadWriteContext())

        # check result
        self.assertEqual(context2.sourceDestinationDatumTransforms(),
                         {('EPSG:4204', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_1, dest_id_1),
                          ('EPSG:4205', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_2, dest_id_2)})

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Skipped on non proj6 builds')
    def testWriteReadXmlProj6(self):
        # setup a context
        context = QgsCoordinateTransformContext()

        proj_1 = '+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=intl +step +proj=helmert +x=-18.944 +y=-379.364 +z=-24.063 +rx=-0.04 +ry=0.764 +rz=-6.431 +s=3.657 +convention=coordinate_frame +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'
        proj_2 = '+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=intl +step +proj=helmert +x=-150 +y=-250 +z=-1 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'

        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem(4204),
                                                       QgsCoordinateReferenceSystem(4326), proj_1))
        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem(4205),
                                                       QgsCoordinateReferenceSystem(4326), proj_2))

        self.assertEqual(context.coordinateOperations(),
                         {('EPSG:4204', 'EPSG:4326'): proj_1,
                          ('EPSG:4205', 'EPSG:4326'): proj_2})

        # save to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        context.writeXml(elem, QgsReadWriteContext())

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        context2.readXml(elem, QgsReadWriteContext())

        # check result
        self.assertEqual(context2.coordinateOperations(),
                         {('EPSG:4204', 'EPSG:4326'): proj_1,
                          ('EPSG:4205', 'EPSG:4326'): proj_2})

    @unittest.skipIf(QgsProjUtils.projVersionMajor() >= 6, 'Skipped on proj6 builds')
    def testMissingTransforms(self):
        # fudge context xml with a missing transform
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        contextElem = doc.createElement("transformContext")
        transformElem = doc.createElement("srcDest")
        transformElem.setAttribute("source", 'EPSG:4204')
        transformElem.setAttribute("dest", 'EPSG:4326')
        transformElem.setAttribute("sourceTransform", 'not valid')
        transformElem.setAttribute("destTransform", 'not valid 2')
        contextElem.appendChild(transformElem)

        elem2 = doc.createElement("test2")
        elem2.appendChild(contextElem)

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        ok, errors = context2.readXml(elem2, QgsReadWriteContext())
        self.assertFalse(ok)

        # check result
        self.assertEqual(errors, ['not valid', 'not valid 2'])

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Skipped on non proj6 builds')
    def testMissingTransformsProj6(self):
        return # TODO -- this seems impossible to determine with existing PROJ6 api
        # fudge context xml with a missing transform
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        contextElem = doc.createElement("transformContext")
        transformElem = doc.createElement("srcDest")
        transformElem.setAttribute("source", 'EPSG:4204')
        transformElem.setAttribute("dest", 'EPSG:4326')

        # fake a proj string with a grid which will NEVER exist
        fake_proj = '+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=this_is_not_a_real_grid.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'
        transformElem.setAttribute("coordinateOp", fake_proj)
        contextElem.appendChild(transformElem)

        elem2 = doc.createElement("test2")
        elem2.appendChild(contextElem)

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        ok, errors = context2.readXml(elem2, QgsReadWriteContext())
        self.assertFalse(ok)

        # check result
        self.assertEqual(errors, ['not valid'])

    @unittest.skipIf(QgsProjUtils.projVersionMajor() >= 6, 'Skipped on proj6 builds')
    def testProject(self):
        """
        Test project's transform context
        """
        project = QgsProject()
        context_changed_spy = QSignalSpy(project.transformContextChanged)
        context = project.transformContext()
        context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                   QgsCoordinateReferenceSystem('EPSG:4283'), 1, 2)
        project.setTransformContext(context)
        self.assertEqual(len(context_changed_spy), 1)
        self.assertEqual(project.transformContext().sourceDestinationDatumTransforms(),
                         {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2)})

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Skipped on non proj6 builds')
    def testProjectProj6(self):
        """
        Test project's transform context
        """
        project = QgsProject()
        context_changed_spy = QSignalSpy(project.transformContextChanged)
        context = project.transformContext()
        context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                       QgsCoordinateReferenceSystem('EPSG:4283'), 'proj')
        project.setTransformContext(context)
        self.assertEqual(len(context_changed_spy), 1)
        self.assertEqual(project.transformContext().coordinateOperations(),
                         {('EPSG:3111', 'EPSG:4283'): 'proj'})

    @unittest.skipIf(QgsProjUtils.projVersionMajor() >= 6, 'Skipped on proj6 builds')
    def testReadWriteSettings(self):
        context = QgsCoordinateTransformContext()
        context.readSettings()

        source_id_1 = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4204),
                                                             QgsCoordinateReferenceSystem(4326))[0].sourceTransformId
        dest_id_1 = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4204),
                                                           QgsCoordinateReferenceSystem(4326))[0].destinationTransformId

        source_id_2 = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4205),
                                                             QgsCoordinateReferenceSystem(4326))[0].sourceTransformId
        dest_id_2 = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4205),
                                                           QgsCoordinateReferenceSystem(4326))[0].destinationTransformId

        # should be empty
        self.assertEqual(context.sourceDestinationDatumTransforms(), {})

        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:4204'),
                                                                   QgsCoordinateReferenceSystem('EPSG:4326'),
                                                                   source_id_1, dest_id_1))
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:4205'),
                                                                   QgsCoordinateReferenceSystem(4326), source_id_2,
                                                                   dest_id_2))

        self.assertEqual(context.sourceDestinationDatumTransforms(),
                         {('EPSG:4204', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_1, dest_id_1),
                          ('EPSG:4205', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_2, dest_id_2)})

        # save to settings
        context.writeSettings()

        # restore from settings
        context2 = QgsCoordinateTransformContext()
        self.assertEqual(context2.sourceDestinationDatumTransforms(), {})
        context2.readSettings()

        # check result
        self.assertEqual(context2.sourceDestinationDatumTransforms(),
                         {('EPSG:4204', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_1, dest_id_1),
                          ('EPSG:4205', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_2, dest_id_2)})

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Skipped on non proj6 builds')
    def testReadWriteSettingsProj6(self):
        context = QgsCoordinateTransformContext()
        context.readSettings()

        proj_1 = '+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=intl +step +proj=helmert +x=-18.944 +y=-379.364 +z=-24.063 +rx=-0.04 +ry=0.764 +rz=-6.431 +s=3.657 +convention=coordinate_frame +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'
        proj_2 = '+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=intl +step +proj=helmert +x=-150 +y=-250 +z=-1 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1'

        # should be empty
        self.assertEqual(context.coordinateOperations(), {})

        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem(4204),
                                                       QgsCoordinateReferenceSystem(4326), proj_1))
        self.assertTrue(context.addCoordinateOperation(QgsCoordinateReferenceSystem(4205),
                                                       QgsCoordinateReferenceSystem(4326), proj_2))

        self.assertEqual(context.coordinateOperations(),
                         {('EPSG:4204', 'EPSG:4326'): proj_1,
                          ('EPSG:4205', 'EPSG:4326'): proj_2})

        # save to settings
        context.writeSettings()

        # restore from settings
        context2 = QgsCoordinateTransformContext()
        self.assertEqual(context2.coordinateOperations(), {})
        context2.readSettings()

        # check result
        self.assertEqual(context2.coordinateOperations(),
                         {('EPSG:4204', 'EPSG:4326'): proj_1,
                          ('EPSG:4205', 'EPSG:4326'): proj_2})

    @unittest.skipIf(QgsProjUtils.projVersionMajor() >= 6, 'Skipped on proj6 builds')
    def testEqualOperator(self):
        context1 = QgsCoordinateTransformContext()
        context2 = QgsCoordinateTransformContext()
        self.assertTrue(context1 == context2)

        context1.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                    QgsCoordinateReferenceSystem('EPSG:4283'), 1, 2)
        self.assertFalse(context1 == context2)

        context2.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                    QgsCoordinateReferenceSystem('EPSG:4283'), 1, 2)
        self.assertTrue(context1 == context2)

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Skipped on non proj6 builds')
    def testEqualOperatorProj6(self):
        context1 = QgsCoordinateTransformContext()
        context2 = QgsCoordinateTransformContext()
        self.assertTrue(context1 == context2)

        context1.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                        QgsCoordinateReferenceSystem('EPSG:4283'), 'p1')
        self.assertFalse(context1 == context2)

        context2.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:3111'),
                                        QgsCoordinateReferenceSystem('EPSG:4283'), 'p1')
        self.assertTrue(context1 == context2)


if __name__ == '__main__':
    unittest.main()
