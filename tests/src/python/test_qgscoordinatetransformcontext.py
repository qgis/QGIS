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
                       QgsReadWriteContext,
                       QgsProject)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtTest import QSignalSpy

app = start_app()


class TestQgsCoordinateTransformContext(unittest.TestCase):

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
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                   QgsCoordinateReferenceSystem('EPSG:4283'), 1, 2))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem(4283), 3, 4))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem(28357), 7, 8))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (7, 8)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                                   QgsCoordinateReferenceSystem('EPSG:28357'), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11)})

        # invalid additions
        self.assertFalse(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(),
                                                                    QgsCoordinateReferenceSystem('EPSG:28357'), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11)})
        self.assertFalse(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'),
                                                                    QgsCoordinateReferenceSystem(), 9, 11))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11)})

        # indicate no transform required
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(28357),
                                                                   QgsCoordinateReferenceSystem(28356), -1, -1))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): (-1, -1)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3111),
                                                                   QgsCoordinateReferenceSystem(28356), 17, -1))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): (-1, -1),
                                                                      ('EPSG:3111', 'EPSG:28356'): (17, -1)})
        self.assertTrue(context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3113),
                                                                   QgsCoordinateReferenceSystem(28356), -1, 18))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): (-1, -1),
                                                                      ('EPSG:3111', 'EPSG:28356'): (17, -1),
                                                                      ('EPSG:3113', 'EPSG:28356'): (-1, 18)})
        # remove non-existing
        context.removeSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3113), QgsCoordinateReferenceSystem(3111))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                      ('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): (-1, -1),
                                                                      ('EPSG:3111', 'EPSG:28356'): (17, -1),
                                                                      ('EPSG:3113', 'EPSG:28356'): (-1, 18)})

        # remove existing
        context.removeSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3111),
                                                      QgsCoordinateReferenceSystem(4283))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): (-1, -1),
                                                                      ('EPSG:3111', 'EPSG:28356'): (17, -1),
                                                                      ('EPSG:3113', 'EPSG:28356'): (-1, 18)})
        context.removeSourceDestinationDatumTransform(QgsCoordinateReferenceSystem(3111),
                                                      QgsCoordinateReferenceSystem(28356))
        self.assertEqual(context.sourceDestinationDatumTransforms(), {('EPSG:28356', 'EPSG:4283'): (3, 4),
                                                                      ('EPSG:28356', 'EPSG:28357'): (9, 11),
                                                                      ('EPSG:28357', 'EPSG:28356'): (-1, -1),
                                                                      ('EPSG:3113', 'EPSG:28356'): (-1, 18)})

        context.clear()
        self.assertEqual(context.sourceDestinationDatumTransforms(), {})

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

    def testWriteReadXml(self):
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
        context.writeXml(elem, doc, QgsReadWriteContext())

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        context2.readXml(elem, doc, QgsReadWriteContext())

        # check result
        self.assertEqual(context2.sourceDatumTransforms(), {'EPSG:3111': 1, 'EPSG:28356': 2})
        self.assertEqual(context2.destinationDatumTransforms(), {'EPSG:3113': 11, 'EPSG:28355': 12})
        self.assertEqual(context2.sourceDestinationDatumTransforms(), {('EPSG:3111', 'EPSG:4283'): (1, 2),
                                                                       ('EPSG:28356', 'EPSG:4283'): (3, 4)})

    def testProject(self):
        """
        Test project's transform context
        """
        project = QgsProject()
        context_changed_spy = QSignalSpy(project.transformContextChanged)
        context = project.transformContext()
        context.addSourceDatumTransform(QgsCoordinateReferenceSystem('EPSG:3111'), 1)
        project.setTransformContext(context)
        self.assertEqual(len(context_changed_spy), 1)
        self.assertEqual(project.transformContext().sourceDatumTransforms(), {'EPSG:3111': 1})


if __name__ == '__main__':
    unittest.main()
