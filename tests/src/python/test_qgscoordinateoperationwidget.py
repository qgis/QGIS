"""QGIS Unit tests for TestQgsCoordinateOperationWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "19/12/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

import os

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsDatumTransform,
)
from qgis.gui import QgsCoordinateOperationWidget
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsCoordinateOperationWidget(QgisTestCase):

    def testGettersSetters(self):
        """test widget getters/setters"""
        w = QgsCoordinateOperationWidget()
        self.assertFalse(w.sourceCrs().isValid())
        self.assertFalse(w.destinationCrs().isValid())
        self.assertFalse(w.hasSelection())
        self.assertFalse(w.availableOperations())

        w.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:28355"))
        self.assertEqual(w.sourceCrs().authid(), "EPSG:28355")
        self.assertFalse(w.destinationCrs().isValid())

        w.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:7855"))
        self.assertEqual(w.sourceCrs().authid(), "EPSG:28355")
        self.assertEqual(w.destinationCrs().authid(), "EPSG:7855")

    def testOperations(self):
        w = QgsCoordinateOperationWidget()
        self.assertFalse(w.hasSelection())
        spy = QSignalSpy(w.operationChanged)
        w.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:26745"))
        self.assertEqual(len(spy), 0)
        w.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        self.assertEqual(len(spy), 1)
        self.assertTrue(w.hasSelection())
        self.assertGreaterEqual(len(w.availableOperations()), 3)

        available_operations = QgsDatumTransform.operations(
            w.sourceCrs(), w.destinationCrs()
        )
        for op in available_operations:
            if op.isAvailable:
                default_proj = op.proj
                break
        else:
            self.assertTrue(False, "No operations available")

        self.assertEqual(w.defaultOperation().proj, default_proj)
        self.assertEqual(w.selectedOperation().proj, default_proj)
        self.assertTrue(w.selectedOperation().isAvailable)

        op = QgsCoordinateOperationWidget.OperationDetails()
        op.proj = "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"
        op.allowFallback = True
        w.setSelectedOperation(op)
        self.assertEqual(
            w.selectedOperation().proj,
            "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        self.assertTrue(w.selectedOperation().allowFallback)
        self.assertEqual(len(spy), 2)
        w.setSelectedOperation(op)
        self.assertEqual(
            w.selectedOperation().proj,
            "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        self.assertTrue(w.selectedOperation().allowFallback)
        self.assertEqual(len(spy), 2)

        op.proj = "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"
        op.allowFallback = False
        w.setSelectedOperation(op)
        self.assertEqual(
            w.selectedOperation().proj,
            "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        self.assertFalse(w.selectedOperation().allowFallback)
        self.assertEqual(len(spy), 3)

        op.allowFallback = True
        w.setSelectedOperation(op)
        self.assertEqual(
            w.selectedOperation().proj,
            "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        self.assertTrue(w.selectedOperation().allowFallback)
        self.assertEqual(len(spy), 4)

        context = QgsCoordinateTransformContext()
        op.proj = "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"
        w.setSelectedOperation(op)
        w.setSelectedOperationUsingContext(context)
        # should go to default, because there's nothing in the context matching these crs
        self.assertEqual(w.selectedOperation().proj, default_proj)
        self.assertEqual(len(spy), 6)

        # put something in the context
        context.addCoordinateOperation(
            w.sourceCrs(),
            w.destinationCrs(),
            "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        w.setSelectedOperationUsingContext(context)
        self.assertEqual(
            w.selectedOperation().proj,
            "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        self.assertTrue(w.selectedOperation().allowFallback)
        self.assertEqual(len(spy), 7)

        context.addCoordinateOperation(
            w.sourceCrs(),
            w.destinationCrs(),
            "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
            False,
        )
        w.setSelectedOperationUsingContext(context)
        self.assertEqual(
            w.selectedOperation().proj,
            "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84",
        )
        self.assertFalse(w.selectedOperation().allowFallback)
        self.assertEqual(len(spy), 8)

    @unittest.skipIf(
        os.environ.get("QGIS_CONTINUOUS_INTEGRATION_RUN", "true"),
        "Depends on local environment and grid presence",
    )
    def testOperationsCruftyProj(self):
        w = QgsCoordinateOperationWidget()
        self.assertFalse(w.hasSelection())
        spy = QSignalSpy(w.operationChanged)
        w.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:4283"))
        self.assertEqual(len(spy), 0)
        w.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:7844"))
        self.assertEqual(len(spy), 1)
        self.assertTrue(w.hasSelection())
        self.assertEqual(len(w.availableOperations()), 2)

        self.assertEqual(
            QgsDatumTransform.datumTransformToProj(
                w.defaultOperation().sourceTransformId
            ),
            "+nadgrids=GDA94_GDA2020_conformal_and_distortion.gsb",
        )
        self.assertEqual(w.defaultOperation().destinationTransformId, -1)
        self.assertEqual(
            QgsDatumTransform.datumTransformToProj(
                w.selectedOperation().sourceTransformId
            ),
            "+nadgrids=GDA94_GDA2020_conformal_and_distortion.gsb",
        )
        self.assertEqual(w.selectedOperation().destinationTransformId, -1)

        self.assertEqual(
            QgsDatumTransform.datumTransformToProj(
                w.availableOperations()[1].sourceTransformId
            ),
            "+nadgrids=GDA94_GDA2020_conformal.gsb",
        )
        self.assertEqual(w.availableOperations()[1].destinationTransformId, -1)

        op = QgsCoordinateOperationWidget.OperationDetails()
        op.sourceTransformId = w.availableOperations()[1].sourceTransformId
        w.setSelectedOperation(op)
        self.assertEqual(
            QgsDatumTransform.datumTransformToProj(
                w.selectedOperation().sourceTransformId
            ),
            "+nadgrids=GDA94_GDA2020_conformal.gsb",
        )
        self.assertEqual(len(spy), 2)
        w.setSelectedOperation(op)
        self.assertEqual(
            QgsDatumTransform.datumTransformToProj(
                w.selectedOperation().sourceTransformId
            ),
            "+nadgrids=GDA94_GDA2020_conformal.gsb",
        )
        self.assertEqual(len(spy), 2)

        op.sourceTransformId = w.availableOperations()[0].sourceTransformId
        op.destinationTransformId = -1
        w.setSelectedOperation(op)
        self.assertEqual(
            QgsDatumTransform.datumTransformToProj(
                w.selectedOperation().sourceTransformId
            ),
            "+nadgrids=GDA94_GDA2020_conformal_and_distortion.gsb",
        )
        self.assertEqual(len(spy), 3)

        op.destinationTransformId = w.availableOperations()[1].sourceTransformId
        op.sourceTransformId = -1
        w.setSelectedOperation(op)
        self.assertEqual(
            QgsDatumTransform.datumTransformToProj(
                w.selectedOperation().sourceTransformId
            ),
            "+nadgrids=GDA94_GDA2020_conformal.gsb",
        )
        self.assertEqual(len(spy), 4)

        op.destinationTransformId = w.availableOperations()[0].sourceTransformId
        op.sourceTransformId = -1
        w.setSelectedOperation(op)
        self.assertEqual(
            QgsDatumTransform.datumTransformToProj(
                w.selectedOperation().sourceTransformId
            ),
            "+nadgrids=GDA94_GDA2020_conformal_and_distortion.gsb",
        )
        self.assertEqual(len(spy), 5)


if __name__ == "__main__":
    unittest.main()
