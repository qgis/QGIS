"""QGIS Unit tests for QgsCoordinateTransform.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2012 by Tim Sutton"
__date__ = "20/08/2012"
__copyright__ = "Copyright 2012, The QGIS Project"


from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCoordinateTransformContext,
    QgsProject,
    QgsRectangle,
    QgsPointXY,
    QgsProjUtils,
    QgsCsException,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCoordinateTransform(QgisTestCase):

    def testTransformBoundingBox(self):
        """Test that we can transform a rectangular bbox from utm56s to LonLat"""
        myExtent = QgsRectangle(242270, 6043737, 246330, 6045897)
        myGeoCrs = QgsCoordinateReferenceSystem("EPSG:4326")
        myUtmCrs = QgsCoordinateReferenceSystem("EPSG:32756")
        myXForm = QgsCoordinateTransform(myUtmCrs, myGeoCrs, QgsProject.instance())
        myProjectedExtent = myXForm.transformBoundingBox(myExtent)
        myExpectedExtent = (
            "150.1509239873580270,-35.7176936443908772 : "
            "150.1964384662953194,-35.6971885216629090"
        )
        myExpectedValues = [
            150.1509239873580270,
            -35.7176936443908772,
            150.1964384662953194,
            -35.6971885216629090,
        ]
        myMessage = "Expected:\n{}\nGot:\n{}\n".format(
            myExpectedExtent,
            myProjectedExtent.toString(),
        )

        self.assertAlmostEqual(
            myExpectedValues[0], myProjectedExtent.xMinimum(), msg=myMessage
        )
        self.assertAlmostEqual(
            myExpectedValues[1], myProjectedExtent.yMinimum(), msg=myMessage
        )
        self.assertAlmostEqual(
            myExpectedValues[2], myProjectedExtent.xMaximum(), msg=myMessage
        )
        self.assertAlmostEqual(
            myExpectedValues[3], myProjectedExtent.yMaximum(), msg=myMessage
        )

    def testTransformBoundingBoxSizeOverflowProtection(self):
        """Test transform bounding box size overflow protection (github issue #32302)"""
        extent = QgsRectangle(
            -176.0454709164556562,
            89.9999999999998153,
            180.0000000000000000,
            90.0000000000000000,
        )
        transform = d = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4236"),
            QgsCoordinateReferenceSystem("EPSG:3031"),
            QgsProject.instance(),
        )
        # this test checks that the line below doesn't assert and crash
        transformedExtent = transform.transformBoundingBox(extent)

    def testTransformQgsRectangle_Regression17600(self):
        """Test that rectangle transform is in the bindings"""
        myExtent = QgsRectangle(-1797107, 4392148, 6025926, 6616304)
        myGeoCrs = QgsCoordinateReferenceSystem("EPSG:4326")
        myUtmCrs = QgsCoordinateReferenceSystem("EPSG:3857")
        myXForm = QgsCoordinateTransform(myUtmCrs, myGeoCrs, QgsProject.instance())
        myTransformedExtent = myXForm.transform(myExtent)
        myTransformedExtentForward = myXForm.transform(
            myExtent, QgsCoordinateTransform.TransformDirection.ForwardTransform
        )
        self.assertAlmostEqual(
            myTransformedExtentForward.xMaximum(), myTransformedExtent.xMaximum()
        )
        self.assertAlmostEqual(
            myTransformedExtentForward.xMinimum(), myTransformedExtent.xMinimum()
        )
        self.assertAlmostEqual(
            myTransformedExtentForward.yMaximum(), myTransformedExtent.yMaximum()
        )
        self.assertAlmostEqual(
            myTransformedExtentForward.yMinimum(), myTransformedExtent.yMinimum()
        )
        self.assertAlmostEqual(myTransformedExtentForward.xMaximum(), 54.13181426773211)
        self.assertAlmostEqual(
            myTransformedExtentForward.xMinimum(), -16.14368685298181
        )
        self.assertAlmostEqual(
            myTransformedExtentForward.yMaximum(), 50.971783118386895
        )
        self.assertAlmostEqual(myTransformedExtentForward.yMinimum(), 36.66235970825241)
        myTransformedExtentReverse = myXForm.transform(
            myTransformedExtent,
            QgsCoordinateTransform.TransformDirection.ReverseTransform,
        )
        self.assertAlmostEqual(
            myTransformedExtentReverse.xMaximum(), myExtent.xMaximum()
        )
        self.assertAlmostEqual(
            myTransformedExtentReverse.xMinimum(), myExtent.xMinimum()
        )
        self.assertAlmostEqual(
            myTransformedExtentReverse.yMaximum(), myExtent.yMaximum()
        )
        self.assertAlmostEqual(
            myTransformedExtentReverse.yMinimum(), myExtent.yMinimum()
        )

    def test_transform_bounding_box_5514(self):
        """
        Testing data from https://github.com/qgis/QGIS/issues/60807
        """
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateReferenceSystem("EPSG:5514"),
            QgsCoordinateTransformContext(),
        )
        res = transform.transformBoundingBox(
            QgsRectangle(
                -787309.78412999748,
                -1242399.5252388765,
                -692024.8894337106,
                -1172964.7912488377,
            ),
            Qgis.TransformDirection.Reverse,
        )
        self.assertAlmostEqual(res.xMinimum(), 1565349, -2)
        self.assertAlmostEqual(res.yMinimum(), 6149667, -2)
        self.assertAlmostEqual(res.xMaximum(), 1721768, -2)
        self.assertAlmostEqual(res.yMaximum(), 6272889, -2)

        res = transform.transformBoundingBox(
            QgsRectangle(
                -787309.78412999748,
                -1172964.7912488377,
                -692024.8894337106,
                -1103530.057258799,
            ),
            Qgis.TransformDirection.Reverse,
        )
        self.assertAlmostEqual(res.xMinimum(), 1550304, -2)
        self.assertAlmostEqual(res.yMinimum(), 6253654, -2)
        self.assertAlmostEqual(res.xMaximum(), 1708837, -2)
        self.assertAlmostEqual(res.yMaximum(), 6378640, -2)

        res = transform.transformBoundingBox(
            QgsRectangle(
                1565349.531718306,
                6149667.9437137973,
                1721768.6637274709,
                6272889.0581493312,
            ),
            Qgis.TransformDirection.Forward,
        )
        self.assertAlmostEqual(res.xMinimum(), -796998, -2)
        self.assertAlmostEqual(res.yMinimum(), -1254766, -2)
        self.assertAlmostEqual(res.xMaximum(), -683588, -2)
        self.assertAlmostEqual(res.yMaximum(), -1160466, -2)

        res = transform.transformBoundingBox(
            QgsRectangle(
                1550304.7179158437,
                6253654.2054561656,
                1708837.6382380251,
                6378640.2628159299,
            ),
            Qgis.TransformDirection.Forward,
        )
        self.assertAlmostEqual(res.xMinimum(), -797124, -2)
        self.assertAlmostEqual(res.yMinimum(), -1185490, -2)
        self.assertAlmostEqual(res.xMaximum(), -683481, -2)
        self.assertAlmostEqual(res.yMaximum(), -1090873, -2)

        res = transform.transformBoundingBox(
            QgsRectangle(
                -882594.67882628436,
                -1172964.7912488377,
                -787309.78412999748,
                -1103530.057258799,
            ),
            Qgis.TransformDirection.Reverse,
        )
        self.assertAlmostEqual(res.xMinimum(), 1405770, -2)
        self.assertAlmostEqual(res.yMinimum(), 6232013, -2)
        self.assertAlmostEqual(res.xMaximum(), 1565349, -2)
        self.assertAlmostEqual(res.yMaximum(), 6358910, -2)

        res = transform.transformBoundingBox(
            QgsRectangle(
                1405770.2540319383,
                6232013.9054610878,
                1565349.531718306,
                6358910.5078411084,
            ),
            Qgis.TransformDirection.Forward,
        )
        self.assertAlmostEqual(res.xMinimum(), -893558, -2)
        self.assertAlmostEqual(res.yMinimum(), -1187060, -2)
        self.assertAlmostEqual(res.xMaximum(), -777618, -2)
        self.assertAlmostEqual(res.yMaximum(), -1089288, -2)

    def test_transform_bounding_box_grid(self):
        """
        This test assumes the ca_nrc_NA83SCRS.tif grid is available on the system!
        """
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4269"),
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateTransformContext(),
        )
        res = transform.transformBoundingBox(
            QgsRectangle(
                -123.65020876249999,
                45.987175336410544,
                -101.22289073749998,
                62.961980263589439,
            )
        )
        self.assertAlmostEqual(res.xMinimum(), -13764678, -2)
        self.assertAlmostEqual(res.yMinimum(), 5778294, -2)
        self.assertAlmostEqual(res.xMaximum(), -11268080, -2)
        self.assertAlmostEqual(res.yMaximum(), 9090934, -2)

        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4269"),
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateTransformContext(),
        )
        # force use of grid shift operation. This will fail as the source rect is outside of the grid bounds, but we should silently
        # fall back to the non-grid operation
        transform.setCoordinateOperation(
            "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=ca_nrc_NA83SCRS.tif +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"
        )
        res = transform.transformBoundingBox(
            QgsRectangle(
                -123.65020876249999,
                45.987175336410544,
                -101.22289073749998,
                62.961980263589439,
            )
        )
        self.assertAlmostEqual(res.xMinimum(), -13764678, -2)
        self.assertAlmostEqual(res.yMinimum(), 5778294, -2)
        self.assertAlmostEqual(res.xMaximum(), -11268080, -2)
        self.assertAlmostEqual(res.yMaximum(), 9090934, -2)

    def testContextProj6(self):
        """
        Various tests to ensure that datum transforms are correctly set respecting context
        """
        context = QgsCoordinateTransformContext()
        context.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            "proj",
        )

        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:28354"),
            QgsCoordinateReferenceSystem("EPSG:28353"),
            context,
        )
        self.assertEqual(
            list(transform.context().coordinateOperations().keys()),
            [("EPSG:28356", "EPSG:4283")],
        )
        # should be no coordinate operation
        self.assertEqual(transform.coordinateOperation(), "")
        # should default to allowing fallback transforms
        self.assertTrue(transform.allowFallbackTransforms())

        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            context,
        )
        self.assertTrue(transform.allowFallbackTransforms())
        context.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            "proj",
            False,
        )
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            context,
        )
        self.assertFalse(transform.allowFallbackTransforms())

        # matching source
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:28353"),
            context,
        )
        self.assertEqual(transform.coordinateOperation(), "")
        # matching dest
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:28354"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            context,
        )
        self.assertEqual(transform.coordinateOperation(), "")
        # matching src/dest pair
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:4283"),
            context,
        )
        self.assertEqual(transform.coordinateOperation(), "proj")

        # test manual overwriting
        transform.setCoordinateOperation("proj2")
        self.assertEqual(transform.coordinateOperation(), "proj2")
        transform.setAllowFallbackTransforms(False)
        self.assertFalse(transform.allowFallbackTransforms())
        transform.setAllowFallbackTransforms(True)
        self.assertTrue(transform.allowFallbackTransforms())

        # test that auto operation setting occurs when updating src/dest crs
        transform.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:28356"))
        self.assertEqual(transform.coordinateOperation(), "proj")
        transform.setCoordinateOperation("proj2")

        transform.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4283"))
        self.assertEqual(transform.coordinateOperation(), "proj")
        transform.setCoordinateOperation("proj2")

        # delayed context set
        transform = QgsCoordinateTransform()
        self.assertEqual(transform.coordinateOperation(), "")
        transform.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:28356"))
        transform.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4283"))
        self.assertEqual(transform.coordinateOperation(), "")
        transform.setContext(context)
        self.assertEqual(transform.coordinateOperation(), "proj")
        self.assertEqual(
            list(transform.context().coordinateOperations().keys()),
            [("EPSG:28356", "EPSG:4283")],
        )

    def testProjectContextProj6(self):
        """
        Test creating transform using convenience constructor which takes project reference
        """
        p = QgsProject()
        context = p.transformContext()
        context.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:3111"),
            "proj",
        )
        p.setTransformContext(context)

        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:28356"),
            QgsCoordinateReferenceSystem("EPSG:3111"),
            p,
        )
        self.assertEqual(transform.coordinateOperation(), "proj")

    def testTransformBoundingBoxFullWorldToWebMercator(self):
        extent = QgsRectangle(-180, -90, 180, 90)
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsProject.instance(),
        )
        transformedExtent = transform.transformBoundingBox(extent)
        self.assertAlmostEqual(transformedExtent.xMinimum(), -20037508.343, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.yMinimum(), -44927335.427, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.xMaximum(), 20037508.343, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.yMaximum(), 44927335.427, delta=1e-3)

    def test_has_vertical_component(self):
        transform = QgsCoordinateTransform()
        self.assertFalse(transform.hasVerticalComponent())

        # 2d to 2d
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateTransformContext(),
        )
        self.assertFalse(transform.hasVerticalComponent())

        # 2d to 3d
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateReferenceSystem("EPSG:7843"),
            QgsCoordinateTransformContext(),
        )
        self.assertFalse(transform.hasVerticalComponent())

        # 2d to 3d
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:32660"),
            QgsCoordinateReferenceSystem("EPSG:4979"),
            QgsCoordinateTransformContext(),
        )
        self.assertFalse(transform.hasVerticalComponent())
        transformedExtent = transform.transformBoundingBox(
            QgsRectangle(6e5, 0, 10e5, 1e6)
        )
        self.assertAlmostEqual(transformedExtent.xMinimum(), -178.456, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.yMinimum(), 0, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.xMaximum(), 177.899, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.yMaximum(), 9.045, delta=1e-3)

        # 3d to 2d
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:7843"),
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateTransformContext(),
        )
        self.assertFalse(transform.hasVerticalComponent())

        # 3d to 3d
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:7843"),
            QgsCoordinateReferenceSystem.createCompoundCrs(
                QgsCoordinateReferenceSystem("EPSG:7844"),
                QgsCoordinateReferenceSystem("EPSG:9458"),
            )[0],
            QgsCoordinateTransformContext(),
        )
        self.assertTrue(transform.hasVerticalComponent())

        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem.createCompoundCrs(
                QgsCoordinateReferenceSystem("EPSG:7844"),
                QgsCoordinateReferenceSystem("EPSG:5711"),
            )[0],
            QgsCoordinateReferenceSystem("EPSG:7843"),
            QgsCoordinateTransformContext(),
        )
        self.assertTrue(transform.hasVerticalComponent())

    def test_cs_exception(self):
        ct = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsProject.instance(),
        )
        point = QgsPointXY(-7603859, -7324441)
        with self.assertRaises(QgsCsException) as e:
            ct.transform(point)
        self.assertEqual(
            str(e.exception),
            "Forward transform (EPSG:4326 to EPSG:3857) of (-7603859.000000, -7324441.000000) Error: Invalid coordinate",
        )

        # reverse transform
        ct = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsProject.instance(),
        )
        point = QgsPointXY(-7603859, -7324441)
        with self.assertRaises(QgsCsException) as e:
            ct.transform(point, Qgis.TransformDirection.Reverse)
        self.assertEqual(
            str(e.exception),
            "Inverse transform (EPSG:4326 to EPSG:3857) of (-7603859.000000, -7324441.000000) Error: Invalid coordinate",
        )

    def testTransformBoundingBoxFromPROJPipeline(self):

        context = QgsCoordinateTransformContext()
        context.addCoordinateOperation(
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateReferenceSystem("EPSG:3844"),
            "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step "
            + "+proj=push +v_3 +step +proj=cart +ellps=WGS84 +step +inv "
            + "+proj=helmert +x=2.329 +y=-147.042 +z=-92.08 +rx=0.309 +ry=-0.325 "
            + "+rz=-0.497 +s=5.69 +convention=coordinate_frame +step +inv +proj=cart "
            + "+ellps=krass +step +proj=pop +v_3 +step +proj=sterea +lat_0=46 "
            + "+lon_0=25 +k=0.99975 +x_0=500000 +y_0=500000 +ellps=krass",
        )

        extent = QgsRectangle(4e5, 4e5, 4.5e5, 4.5e5)
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:3844"),
            QgsCoordinateReferenceSystem("EPSG:4326"),
            context,
        )
        transformedExtent = transform.transformBoundingBox(extent)
        self.assertAlmostEqual(transformedExtent.xMinimum(), 23.7177083, delta=1e-6)
        self.assertAlmostEqual(transformedExtent.yMinimum(), 45.092624, delta=1e-6)
        self.assertAlmostEqual(transformedExtent.xMaximum(), 24.363125, delta=1e-6)
        self.assertAlmostEqual(transformedExtent.yMaximum(), 45.547942, delta=1e-6)

    @unittest.skipIf(
        [QgsProjUtils.projVersionMajor(), QgsProjUtils.projVersionMinor()] < [9, 6],
        "Spilhaus support added in Proj 9.6",
    )
    def testTransformBoundingBoxWGS84ToSpilhaus(self):

        context = QgsCoordinateTransformContext()
        extent = QgsRectangle(-180, -90, 180, 90)
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateReferenceSystem("ESRI:54099"),
            context,
        )
        transformedExtent = transform.transformBoundingBox(extent)
        self.assertLess(transformedExtent.xMinimum(), -16000000)
        self.assertLess(transformedExtent.yMinimum(), -16000000)
        self.assertGreater(transformedExtent.xMaximum(), 16000000)
        self.assertGreater(transformedExtent.yMaximum(), 16000000)


if __name__ == "__main__":
    unittest.main()
