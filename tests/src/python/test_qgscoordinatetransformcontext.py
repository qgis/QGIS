"""QGIS Unit tests for QgsCoordinateTransformContext

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "11/5/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsProject,
    QgsReadWriteContext,
    QgsSettings,
)
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestQgsCoordinateTransformContext(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestQgsCoordinateTransformContext.com")
        QCoreApplication.setApplicationName("TestQgsCoordinateTransformContext")
        QgsSettings().clear()

    def testSourceDestinationDatumTransformsProj6(self):
        context = QgsCoordinateTransformContext()
        self.assertEqual(context.coordinateOperations(), {})
        proj_string = "+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1"
        self.assertFalse(
            context.hasTransform(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            )
        )
        self.assertFalse(
            context.mustReverseCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            )
        )
        self.assertFalse(
            context.mustReverseCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
                proj_string,
            )
        )
        self.assertTrue(
            context.hasTransform(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            )
        )
        self.assertFalse(
            context.hasTransform(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
        )
        self.assertFalse(
            context.hasTransform(
                QgsCoordinateReferenceSystem("EPSG:3113"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            )
        )
        self.assertEqual(
            context.coordinateOperations(), {("EPSG:3111", "EPSG:4283"): proj_string}
        )
        self.assertTrue(
            context.mustReverseCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )
        self.assertTrue(
            context.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            )
        )
        self.assertTrue(
            context.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )

        self.assertTrue(
            context.hasTransform(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )

        self.assertEqual(
            context.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            ),
            proj_string,
        )
        self.assertFalse(
            context.mustReverseCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            )
        )
        # ideally not equal, but for now it's all we can do, and return True for mustReverseCoordinateOperation here
        self.assertEqual(
            context.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            ),
            proj_string,
        )
        self.assertTrue(
            context.mustReverseCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )
        self.assertTrue(
            context.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )

        proj_string_2 = "dummy"
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
                proj_string_2,
                False,
            )
        )
        self.assertEqual(
            context.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            ),
            proj_string_2,
        )
        self.assertFalse(
            context.mustReverseCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )
        context.removeCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:4283"),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )

        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3113"),
                proj_string_2,
                False,
            )
        )
        self.assertFalse(
            context.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:3113"),
            )
        )
        self.assertFalse(
            context.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:3113"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            )
        )

        context.removeCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:4283"),
            QgsCoordinateReferenceSystem("EPSG:3113"),
        )

        proj_string_2 = "+proj=pipeline +step +inv +proj=utm +zone=56 +south +ellps=GRS80 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1"
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:28356"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
                proj_string_2,
            )
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:3111", "EPSG:4283"): proj_string,
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
            },
        )
        proj_string_3 = "+proj=pipeline +step +inv +proj=utm +zone=56 +south +ellps=GRS80 +step +proj=utm +zone=57 +south +ellps=GRS80"
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:28356"),
                QgsCoordinateReferenceSystem("EPSG:28357"),
                proj_string_3,
            )
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:3111", "EPSG:4283"): proj_string,
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
                ("EPSG:28356", "EPSG:28357"): proj_string_3,
            },
        )
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:28356"),
                QgsCoordinateReferenceSystem("EPSG:28357"),
                "some other proj string",
            )
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:3111", "EPSG:4283"): proj_string,
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
                ("EPSG:28356", "EPSG:28357"): "some other proj string",
            },
        )

        # invalid additions
        self.assertFalse(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem(),
                QgsCoordinateReferenceSystem("EPSG:28357"),
                "bad proj",
            )
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:3111", "EPSG:4283"): proj_string,
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
                ("EPSG:28356", "EPSG:28357"): "some other proj string",
            },
        )
        self.assertFalse(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem(),
                "bad proj",
            )
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:3111", "EPSG:4283"): proj_string,
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
                ("EPSG:28356", "EPSG:28357"): "some other proj string",
            },
        )

        # indicate no transform required
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:28357"),
                QgsCoordinateReferenceSystem("EPSG:28356"),
                "",
            )
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:3111", "EPSG:4283"): proj_string,
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
                ("EPSG:28356", "EPSG:28357"): "some other proj string",
                ("EPSG:28357", "EPSG:28356"): "",
            },
        )

        # remove non-existing
        context.removeCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:3113"),
            QgsCoordinateReferenceSystem("EPSG:3111"),
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:3111", "EPSG:4283"): proj_string,
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
                ("EPSG:28356", "EPSG:28357"): "some other proj string",
                ("EPSG:28357", "EPSG:28356"): "",
            },
        )

        # remove existing
        context.removeCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:3111"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
                ("EPSG:28356", "EPSG:28357"): "some other proj string",
                ("EPSG:28357", "EPSG:28356"): "",
            },
        )
        context.removeCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:28357"),
        )
        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:28356", "EPSG:4283"): proj_string_2,
                ("EPSG:28357", "EPSG:28356"): "",
            },
        )

        context.clear()
        self.assertEqual(context.coordinateOperations(), {})

    def testCalculateSourceDestProj6(self):
        context = QgsCoordinateTransformContext()

        # empty context
        self.assertEqual(
            context.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            ),
            "",
        )

        # add specific source/dest pair - should take precedence
        context.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            "proj 1",
        )
        self.assertEqual(
            context.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:28356"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            ),
            "proj 1",
        )
        self.assertEqual(
            context.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:4283"),
            ),
            "",
        )
        self.assertEqual(
            context.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:28356"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            ),
            "",
        )
        # check that reverse transforms are automatically supported
        self.assertEqual(
            context.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4283"),
                QgsCoordinateReferenceSystem("EPSG:28356"),
            ),
            "proj 1",
        )

    def testWriteReadXmlProj6(self):
        # setup a context
        context = QgsCoordinateTransformContext()

        proj_1 = "+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=intl +step +proj=helmert +x=-18.944 +y=-379.364 +z=-24.063 +rx=-0.04 +ry=0.764 +rz=-6.431 +s=3.657 +convention=coordinate_frame +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1"
        proj_2 = "+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=intl +step +proj=helmert +x=-150 +y=-250 +z=-1 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1"
        proj_3 = "+proj=pipeline +step +proj=axisswap +order=2,1"

        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4204"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                proj_1,
                True,
            )
        )
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4205"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                proj_2,
                False,
            )
        )

        # also insert a crs with no authid available
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem.fromProj(
                    "+proj=longlat +a=6378137 +rf=298.25722356300003 +no_defs"
                ),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                proj_3,
                False,
            )
        )

        self.assertEqual(
            context.coordinateOperations(),
            {
                ("EPSG:4204", "EPSG:4326"): proj_1,
                ("EPSG:4205", "EPSG:4326"): proj_2,
                ("", "EPSG:4326"): proj_3,
            },
        )

        # save to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        context.writeXml(elem, QgsReadWriteContext())

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        context2.readXml(elem, QgsReadWriteContext())

        # check result
        self.assertEqual(
            context2.coordinateOperations(),
            {
                ("EPSG:4204", "EPSG:4326"): proj_1,
                ("EPSG:4205", "EPSG:4326"): proj_2,
                ("", "EPSG:4326"): proj_3,
            },
        )
        self.assertEqual(
            context2.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem.fromProj(
                    "+proj=longlat +a=6378137 +rf=298.25722356300003 +no_defs"
                ),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            ),
            "+proj=pipeline +step +proj=axisswap +order=2,1",
        )
        self.assertFalse(
            context2.mustReverseCoordinateOperation(
                QgsCoordinateReferenceSystem.fromProj(
                    "+proj=longlat +a=6378137 +rf=298.25722356300003 +no_defs"
                ),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
        )
        self.assertEqual(
            context2.calculateCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsCoordinateReferenceSystem.fromProj(
                    "+proj=longlat +a=6378137 +rf=298.25722356300003 +no_defs"
                ),
            ),
            "+proj=pipeline +step +proj=axisswap +order=2,1",
        )
        self.assertTrue(
            context2.mustReverseCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsCoordinateReferenceSystem.fromProj(
                    "+proj=longlat +a=6378137 +rf=298.25722356300003 +no_defs"
                ),
            )
        )
        self.assertTrue(
            context2.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4204"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
        )
        self.assertFalse(
            context2.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4205"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
        )

    def testMissingTransformsProj6(self):
        return  # TODO -- this seems impossible to determine with existing PROJ6 api
        # fudge context xml with a missing transform
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        contextElem = doc.createElement("transformContext")
        transformElem = doc.createElement("srcDest")
        transformElem.setAttribute("source", "EPSG:4204")
        transformElem.setAttribute("dest", "EPSG:4326")

        # fake a proj string with a grid which will NEVER exist
        fake_proj = "+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=this_is_not_a_real_grid.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1"
        transformElem.setAttribute("coordinateOp", fake_proj)
        contextElem.appendChild(transformElem)

        elem2 = doc.createElement("test2")
        elem2.appendChild(contextElem)

        # restore from xml
        context2 = QgsCoordinateTransformContext()
        ok, errors = context2.readXml(elem2, QgsReadWriteContext())
        self.assertFalse(ok)

        # check result
        self.assertEqual(errors, ["not valid"])

    def testProjectProj6(self):
        """
        Test project's transform context
        """
        project = QgsProject()
        context_changed_spy = QSignalSpy(project.transformContextChanged)
        context = project.transformContext()
        context.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:3111"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            "proj",
            True,
        )
        project.setTransformContext(context)
        self.assertEqual(len(context_changed_spy), 1)
        self.assertEqual(
            project.transformContext().coordinateOperations(),
            {("EPSG:3111", "EPSG:4283"): "proj"},
        )
        context2 = project.transformContext()
        context2.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:3111"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            "proj",
            False,
        )
        project.setTransformContext(context2)
        self.assertEqual(len(context_changed_spy), 2)
        self.assertEqual(project.transformContext(), context2)
        self.assertNotEqual(project.transformContext(), context)

    def testReadWriteSettingsProj6(self):
        context = QgsCoordinateTransformContext()
        context.readSettings()

        proj_1 = "+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=intl +step +proj=helmert +x=-18.944 +y=-379.364 +z=-24.063 +rx=-0.04 +ry=0.764 +rz=-6.431 +s=3.657 +convention=coordinate_frame +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1"
        proj_2 = "+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=intl +step +proj=helmert +x=-150 +y=-250 +z=-1 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1"

        # should be empty
        self.assertEqual(context.coordinateOperations(), {})

        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4204"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                proj_1,
                True,
            )
        )
        self.assertTrue(
            context.addCoordinateOperation(
                QgsCoordinateReferenceSystem("EPSG:4205"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                proj_2,
                False,
            )
        )

        self.assertEqual(
            context.coordinateOperations(),
            {("EPSG:4204", "EPSG:4326"): proj_1, ("EPSG:4205", "EPSG:4326"): proj_2},
        )
        self.assertTrue(
            context.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4204"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
        )
        self.assertFalse(
            context.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4205"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
        )

        # save to settings
        context.writeSettings()

        # restore from settings
        context2 = QgsCoordinateTransformContext()
        self.assertEqual(context2.coordinateOperations(), {})
        context2.readSettings()

        # check result
        self.assertEqual(
            context2.coordinateOperations(),
            {("EPSG:4204", "EPSG:4326"): proj_1, ("EPSG:4205", "EPSG:4326"): proj_2},
        )

        self.assertTrue(
            context2.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4204"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
        )
        self.assertFalse(
            context2.allowFallbackTransform(
                QgsCoordinateReferenceSystem("EPSG:4205"),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
        )

    def testEqualOperatorProj6(self):
        context1 = QgsCoordinateTransformContext()
        context2 = QgsCoordinateTransformContext()
        self.assertTrue(context1 == context2)

        context1.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:3111"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            "p1",
        )
        self.assertFalse(context1 == context2)

        context2.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:3111"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            "p1",
        )
        self.assertTrue(context1 == context2)

        context2.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:3111"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            "p1",
            False,
        )
        self.assertFalse(context1 == context2)


if __name__ == "__main__":
    unittest.main()
