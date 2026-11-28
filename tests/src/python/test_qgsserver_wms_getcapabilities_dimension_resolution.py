"""QGIS Unit tests for QgsServer WMS GetCapabilities, dimension resolution.

From build dir, run: ctest -R PyQgsServerWMSGetCapabilitiesDimensionResolution -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Germán Carrillo"
__date__ = "26/11/2026"
__copyright__ = "Copyright 2026, The QGIS Project"

import os
import uuid

from lxml import etree as et
from qgis.core import (
    Qgis,
    QgsProject,
    QgsServerWmsDimensionProperties,
    QgsVectorLayer,
)
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase


class TestQgsServerWMSGetCapabilitiesDimensionResolution(QgsServerTestBase):
    """QGIS Server WMS Tests for GetCapabilities request for dimension resolution"""

    def test_get_capabilities_time_dimension(self):
        """Test if get capabilities return correct time dimension"""

        # Test get capabilities
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetCapabilities",
                    }.items()
                )
            ]
        )

        project = QgsProject()

        projectPath = os.path.join(
            self.temporary_path,
            "qgis_server_accesscontrol",
            "project_with_dimensions.qgs",
        )
        self.assertTrue(
            os.path.isfile(self.projectPath),
            f'Could not find project file "{projectPath}"',
        )
        self.assertTrue(
            project.read(projectPath),
            f'Could not read project file "{projectPath}"',
        )

        def get_time_dim(layer_name, dim_name):
            # Set a filename to avoid capabilities cache breaking tests
            project.setFileName(str(uuid.uuid4()))

            r, h = self._result(self._execute_request_project(qs, project))

            t = et.fromstring(r)
            ns = t.nsmap
            del ns[None]
            ns["wms"] = "http://www.opengis.net/wms"

            dims = t.xpath(
                f"//wms:Layer/wms:Name[text()='{layer_name}']/../wms:Dimension[@name='{dim_name}']",
                namespaces=ns,
            )

            return dims[0] if dims else None

        # Test original layer's time dimension
        dim_cap = get_time_dim("Datetime_dim", "time")
        self.assertEqual(
            len(dim_cap.text.split(", ")),
            10,  # List of values, no interval
        )

        # Test a copy layer with interval
        test_data_path = (
            str(self.get_test_data_path("qgis_server_accesscontrol/dimensions.gpkg"))
            + "|layername=datetime_dim"
        )
        vl1 = QgsVectorLayer(test_data_path, "test_interval", "ogr")

        dim1 = QgsServerWmsDimensionProperties.WmsDimensionInfo(
            "time",
            "datetime",
            "",
            "ISO8601",
            "",
            Qgis.WmsDimensionDefaultDisplay.MaxValue,
            None,
            dimUseInterval=True,
            dimResolutionType=Qgis.WmsDimensionResolutionType.Disabled,
            dimCustomResolution="",
        )
        self.assertTrue(vl1.serverProperties().addWmsDimension(dim1))
        project.addMapLayer(vl1)

        dim_cap = get_time_dim("test_interval", "time")
        self.assertEqual(
            dim_cap.text, "2021-05-31T10:00:00.000Z/2021-12-27T22:00:00.000Z"
        )

        # Test that no resolution is present if there is no interval
        vl2 = QgsVectorLayer(test_data_path, "test_no_resolution_if_no_interval", "ogr")
        dim2 = QgsServerWmsDimensionProperties.WmsDimensionInfo(
            "time",
            "datetime",
            "",
            "ISO8601",
            "",
            Qgis.WmsDimensionDefaultDisplay.MaxValue,
            None,
            dimUseInterval=False,
            dimResolutionType=Qgis.WmsDimensionResolutionType.Yearly,
            dimCustomResolution="",
        )
        self.assertTrue(vl2.serverProperties().addWmsDimension(dim2))
        project.addMapLayer(vl2)

        dim_cap = get_time_dim("test_no_resolution_if_no_interval", "time")
        self.assertTrue(len(dim_cap.text) > 0 and not ("/" in dim_cap.text))

        # Test a yearly resolution
        vl3 = QgsVectorLayer(test_data_path, "test_resolution_yearly", "ogr")

        dim3 = QgsServerWmsDimensionProperties.WmsDimensionInfo(
            "time",
            "date",
            "",
            "ISO8601",
            "",
            Qgis.WmsDimensionDefaultDisplay.MaxValue,
            None,
            dimUseInterval=True,
            dimResolutionType=Qgis.WmsDimensionResolutionType.Yearly,
            dimCustomResolution="",
        )
        self.assertTrue(vl3.serverProperties().addWmsDimension(dim3))
        project.addMapLayer(vl3)

        dim_cap = get_time_dim("test_resolution_yearly", "time")
        self.assertEqual(dim_cap.text, "2021-05-31/2021-12-27/P1Y")

        # Test a custom resolution
        vl4 = QgsVectorLayer(test_data_path, "test_custom_resolution", "ogr")

        dim4 = QgsServerWmsDimensionProperties.WmsDimensionInfo(
            "time",
            "datetime",
            "",
            "ISO8601",
            "",
            Qgis.WmsDimensionDefaultDisplay.MaxValue,
            None,
            dimUseInterval=True,
            dimResolutionType=Qgis.WmsDimensionResolutionType.Custom,
            dimCustomResolution="PT2H",
        )
        self.assertTrue(vl4.serverProperties().addWmsDimension(dim4))
        project.addMapLayer(vl4)

        dim_cap = get_time_dim("test_custom_resolution", "time")
        self.assertEqual(
            dim_cap.text, "2021-05-31T10:00:00.000Z/2021-12-27T22:00:00.000Z/PT2H"
        )


if __name__ == "__main__":
    unittest.main()
