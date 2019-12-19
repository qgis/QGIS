# -*- coding: utf-8 -*-
"""QGIS Unit tests for TestQgsCoordinateOperationWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '19/12/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import os

from qgis.core import (
    QgsProjUtils,
    QgsRectangle,
    QgsCoordinateReferenceSystem,
    QgsVectorLayer,
    QgsProject,
    QgsFeature,
    QgsGeometry)
from qgis.gui import QgsCoordinateOperationWidget

from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsCoordinateOperationWidget(unittest.TestCase):

    def testGettersSetters(self):
        """ test widget getters/setters """
        w = QgsCoordinateOperationWidget()
        self.assertFalse(w.sourceCrs().isValid())
        self.assertFalse(w.destinationCrs().isValid())
        self.assertFalse(w.hasSelection())
        self.assertFalse(w.availableOperations())

        w.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:28355'))
        self.assertEqual(w.sourceCrs().authid(), 'EPSG:28355')
        self.assertFalse(w.destinationCrs().isValid())

        w.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:7855'))
        self.assertEqual(w.sourceCrs().authid(), 'EPSG:28355')
        self.assertEqual(w.destinationCrs().authid(), 'EPSG:7855')

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Not a proj6 build')
    def testOperations(self):
        w = QgsCoordinateOperationWidget()
        self.assertFalse(w.hasSelection())
        spy = QSignalSpy(w.operationChanged)
        w.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:28355'))
        self.assertEqual(len(spy), 0)
        w.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:7855'))
        self.assertEqual(len(spy), 1)
        self.assertTrue(w.hasSelection())
        self.assertEqual(len(w.availableOperations()), 3)

        self.assertEqual(w.defaultOperation().proj, '+proj=pipeline +step +inv +proj=utm +zone=55 +south +ellps=GRS80 +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=utm +zone=55 +south +ellps=GRS80')
        self.assertEqual(w.selectedOperation().proj,
                         '+proj=pipeline +step +inv +proj=utm +zone=55 +south +ellps=GRS80 +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=utm +zone=55 +south +ellps=GRS80')
        self.assertTrue(w.selectedOperation().isAvailable)

        op = QgsCoordinateOperationWidget.OperationDetails()
        op.proj = '+proj=pipeline +step +inv +proj=utm +zone=55 +south +ellps=GRS80 +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=utm +zone=55 +south +ellps=GRS80'
        w.setSelectedOperation(op)
        self.assertEqual(w.selectedOperation().proj,
                         '+proj=pipeline +step +inv +proj=utm +zone=55 +south +ellps=GRS80 +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=utm +zone=55 +south +ellps=GRS80')
        self.assertEqual(len(spy), 2)
        w.setSelectedOperation(op)
        self.assertEqual(w.selectedOperation().proj,
                         '+proj=pipeline +step +inv +proj=utm +zone=55 +south +ellps=GRS80 +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=utm +zone=55 +south +ellps=GRS80')
        self.assertEqual(len(spy), 2)

        op.proj = '+proj=pipeline +step +inv +proj=utm +zone=55 +south +ellps=GRS80 +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=utm +zone=55 +south +ellps=GRS80'
        w.setSelectedOperation(op)
        self.assertEqual(w.selectedOperation().proj,
                         '+proj=pipeline +step +inv +proj=utm +zone=55 +south +ellps=GRS80 +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=utm +zone=55 +south +ellps=GRS80')
        self.assertEqual(len(spy), 3)


if __name__ == '__main__':
    unittest.main()
