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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsCoordinateReferenceSystem,
                       QgsCoordinateTransformContext,
                       QgsCoordinateTransform,
                       QgsDatumTransform,
                       QgsReadWriteContext,
                       QgsProject,
                       QgsSettings)
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
        QCoreApplication.setOrganizationDomain("TestPyQgsWFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsWFSProvider")
        QgsSettings().clear()

    @unittest.skip('ifdefed out in c++ until required')
    def testSourceDatumTransforms(self):
        context = QgsCoordinateTransformContext()
        self.assertEqual(context.sourceDatumTransforms(), {})
        self.assertTrue(context.addSourceDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'), 1))
        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1})
        self.assertTrue(context.addSourceDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'), 2))
        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 2})
        self.assertTrue(context.addSourceDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'), 3))
        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3})
        self.assertTrue(context.addSourceDatumTransform(QgsCoordinateReferenceSystem(28356), 3))
        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3})

        # invalid crs should fail
        self.assertFalse(context.addSourceDatumTransform(QgsCoordinateReferenceSystem(), 4))
        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3})

        # indicate no transform required
        self.assertTrue(context.addSourceDatumTransform(QgsCoordinateReferenceSystem(28357), -1))
        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3, 'EPSG:28357': -1})

        # removing non-existing
        context.removeSourceDatumTransform(QgsCoordinateReferenceSystem(28354))
        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3, 'EPSG:28357': -1})

        # remove existing
        context.removeSourceDatumTransform(QgsCoordinateReferenceSystem(28356))
        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28357': -1})

        context.clear()
        self.assertEqual(context.sourceDatumTransforms(), {})

    @unittest.skip('ifdefed out in c++ until required')
    def testDestDatumTransforms(self):
        context = QgsCoordinateTransformContext()
        self.assertEqual(context.destinationDatumTransforms(), {})
        self.assertTrue(context.addDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'), 1))
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3111': 1})
        self.assertTrue(context.addDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'), 2))
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 2})
        self.assertTrue(context.addDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'), 3))
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3})
        self.assertTrue(context.addDestinationDatumTransform(QgsCoordinateReferenceSystem(28356), 3))
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3})

        # invalid crs should fail
        self.assertFalse(context.addDestinationDatumTransform(QgsCoordinateReferenceSystem(), 4))
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3})

        # indicate no transform required
        self.assertTrue(context.addDestinationDatumTransform(QgsCoordinateReferenceSystem(28357), -1))
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3, 'EPSG:28357': -1})

        # removing non-existing
        context.removeSourceDatumTransform(QgsCoordinateReferenceSystem(28354))
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 3, 'EPSG:28357': -1})

        # remove existing
        context.removeDestinationDatumTransform(QgsCoordinateReferenceSystem(28356))
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28357': -1})

        context.clear()
        self.assertEqual(context.destinationDatumTransforms(), {})

    def testSourceDestinationDatumTransforms(self):
        context = QgsCoordinateTransformContext()
        self.assertEqual(context.sourceDestinationDatumTransforms(), {})
        self.assertFalse(context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                   QgsCoordinateReferenceSystem('EPSG:4283'), 1, 2))
        self.assertTrue(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertFalse(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:4326')))
        self.assertFalse(
            context.hasTransform(QgsCoordinateReferenceSystem('EPSG:3113'), QgsCoordinateReferenceSystem('EPSG:4283')))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem(4283), 3, 4))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem(28357), 7, 8))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(7, 8)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem('EPSG:28357'), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11)})

        # invalid additions
        self.assertFalse(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(),
                                                                    QgsCoordinateReferenceSystem('EPSG:28357'), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11)})
        self.assertFalse(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                    QgsCoordinateReferenceSystem(), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11)})

        # indicate no transform required
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(28357),
                                                                   QgsCoordinateReferenceSystem(28356), -1, -1))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3111),
                                                                   QgsCoordinateReferenceSystem(28356), 17, -1))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                                                                      ('EPSG:3111', 'EPSG:28356'): QgsDatumTransform.TransformPair(17, -1)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3113),
                                                                   QgsCoordinateReferenceSystem(28356), -1, 18))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                                                                      ('EPSG:3111', 'EPSG:28356'): QgsDatumTransform.TransformPair(17, -1),
                                                                      ('EPSG:3113', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, 18)})
        # remove non-existing
        context.removeSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3113), QgsCoordinateReferenceSystem(3111))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                                                                      ('EPSG:3111', 'EPSG:28356'): QgsDatumTransform.TransformPair(17, -1),
                                                                      ('EPSG:3113', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, 18)})

        # remove existing
        context.removeSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3111),
                                                      QgsCoordinateReferenceSystem(4283))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                                                                      ('EPSG:3111', 'EPSG:28356'): QgsDatumTransform.TransformPair(17, -1),
                                                                      ('EPSG:3113', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, 18)})
        context.removeSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3111),
                                                      QgsCoordinateReferenceSystem(28356))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:28356', 'EPSG:4283'): QgsDatumTransform.TransformPair(3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): QgsDatumTransform.TransformPair(9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, -1),
                                                                      ('EPSG:3113', 'EPSG:28356'): QgsDatumTransform.TransformPair(-1, 18)})

        context.clear()
        self.assertEqual(context.sourceDestinationDatumTransforms(), {})

    @unittest.skip('ifdefed out in c++ until required')
    def testCalculate(self):
        context = QgsCoordinateTransformContext()

        #empty context
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         (-1, -1))

        #add src transform
        context.addSourceDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'), 1)
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         (-1, -1))
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         (1, -1))
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:4283'),
                                                          QgsCoordinateReferenceSystem('EPSG:28356')),
                         (-1, -1))

        #add dest transform
        context.addDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:4283'), 2)
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                          QgsCoordinateReferenceSystem('EPSG:4326')),
                         (-1, -1))
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         (-1, 2))
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:4283'),
                                                          QgsCoordinateReferenceSystem('EPSG:3111')),
                         (-1, -1))

        #add specific source/dest pair - should take precedence
        context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                   QgsCoordinateReferenceSystem('EPSG:4283'),
                                                   3, 4)
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         (3, 4))
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         (-1, 2))
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                          QgsCoordinateReferenceSystem('EPSG:3111')),
                         (1, -1))

    def testCalculateSourceDest(self):
        context = QgsCoordinateTransformContext()

        #empty context
        self.assertEqual(context.calculateDatumTransforms(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                          QgsCoordinateReferenceSystem('EPSG:4283')),
                         QgsDatumTransform.TransformPair(-1, -1))

        #add specific source/dest pair - should take precedence
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

    @unittest.skip('ifdefed out in c++ until required')
    def testWriteReadXmlSingleVariant(self):
        # setup a context
        context = QgsCoordinateTransformContext()
        self.assertTrue(context.addSourceDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'), 1))
        self.assertTrue(context.addSourceDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'), 2))
        self.assertTrue(context.addDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3113'), 11))
        self.assertTrue(context.addDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28355'), 12))
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                   QgsCoordinateReferenceSystem('EPSG:4283'), 1, 2))
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem(4283), 3, 4))

        self.assertEqual(context.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 2})
        self.assertEqual(context.destinationDatumTransforms(), {'EPSG:3113': 11, 'EPSG:28355': 12})
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4)})

        # save to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        context.writeXml(elem, QgsReadWriteContext())

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        context2.readXml(elem, QgsReadWriteContext())

        # check result
        self.assertEqual(context2.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 2})
        self.assertEqual(context2.destinationDatumTransforms(), {'EPSG:3113': 11, 'EPSG:28355': 12})
        self.assertEqual(context2.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                       ('EPSG:28356', 'EPSG:4283'): (3, 4)})

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
                                                                   QgsCoordinateReferenceSystem(4326), source_id_1, dest_id_1))
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(4205),
                                                                   QgsCoordinateReferenceSystem(4326), source_id_2, dest_id_2))

        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:4204', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_1, dest_id_1),
                                                                      ('EPSG:4205', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_2, dest_id_2)})

        # save to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        context.writeXml(elem, QgsReadWriteContext())

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        context2.readXml(elem, QgsReadWriteContext())

        # check result
        self.assertEqual(context2.sourceDestinationDatumTransforms(), {('EPSG:4204', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_1, dest_id_1),
                                                                       ('EPSG:4205', 'EPSG:4326'): QgsDatumTransform.TransformPair(source_id_2, dest_id_2)})

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
        self.assertEqual(project.transformContext().sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): QgsDatumTransform.TransformPair(1, 2)})

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
                                                                   QgsCoordinateReferenceSystem('EPSG:4326'), source_id_1, dest_id_1))
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:4205'),
                                                                   QgsCoordinateReferenceSystem(4326), source_id_2, dest_id_2))

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


if __name__ == '__main__':
    unittest.main()
