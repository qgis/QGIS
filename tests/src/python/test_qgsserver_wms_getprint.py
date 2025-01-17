"""QGIS Unit tests for QgsServer WMS GetPrint.

From build dir, run: ctest -R PyQgsServerWMSGetPrint -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "25/05/2015"
__copyright__ = "Copyright 2015, The QGIS Project"

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ["QT_HASH_SEED"] = "1"

import urllib.parse

from qgis.PyQt.QtCore import QSize
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase


class TestQgsServerWMSGetPrint(QgsServerTestBase):
    """QGIS Server WMS Tests for GetPrint request"""

    def test_wms_getprint_basic(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country,Hello",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Basic")

        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "LAYERS": "Country,Hello",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Basic")

        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country,Hello",
                        "LAYERS": "Country,Hello",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Basic")

    def test_wms_getprint_style(self):
        # default style
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country_Labels",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        assert h.get("Content-Type").startswith("image"), r
        self._img_diff_error(r, h, "WMS_GetPrint_StyleDefault")

        # custom style
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country_Labels",
                        "map0:STYLES": "custom",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleCustom")

        # default style
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "LAYERS": "Country_Labels",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleDefault")

        # custom style
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "LAYERS": "Country_Labels",
                        "STYLES": "custom",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleCustom")

        # default style
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country_Labels",
                        "LAYERS": "Country_Labels",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleDefault")

        # custom style
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country_Labels",
                        "map0:STYLES": "custom",
                        "LAYERS": "Country_Labels",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleCustom")

    def test_wms_getprint_group(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectGroupsPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country_Diagrams,Country_Labels,Country",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r_individual, h = self._result(self._execute_request(qs))

        # test reference image
        self._img_diff_error(r_individual, h, "WMS_GetPrint_Group")

        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectGroupsPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "CountryGroup",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r_group, h = self._result(self._execute_request(qs))

        # Test group image
        self._img_diff_error(r_group, h, "WMS_GetPrint_Group")

        """ Debug check:
        f = open('grouped.png', 'wb+')
        f.write(r_group)
        f.close()
        f = open('individual.png', 'wb+')
        f.write(r_individual)
        f.close()
        #"""

        # This test is too strict, it can fail
        # self.assertEqual(r_individual, r_group, 'Individual layers query and group layers query results should be identical')

    def test_wms_getprint_legend(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4copy",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country,Hello",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Legend")

    def test_wms_getprint_srs(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-309.015,-133.011,312.179,133.949",
                        "map0:LAYERS": "Country,Hello",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_SRS")

    def test_wms_getprint_scale(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country,Hello",
                        "map0:SCALE": "36293562",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Scale")

    def test_wms_getprint_size(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country,Hello",
                        "map0:SCALE": "36293562",
                        "CRS": "EPSG:3857",
                        "HEIGHT": "100",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Size", max_size_diff=QSize(1, 1))

    def test_wms_getprint_grid(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country,Hello",
                        "map0:GRID_INTERVAL_X": "1000000",
                        "map0:GRID_INTERVAL_Y": "2000000",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Grid")

    def test_wms_getprint_rotation(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country,Hello",
                        "map0:ROTATION": "45",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Rotation")

    def test_wms_getprint_two_maps(self):
        """Test map0 and map1 apply to the correct maps"""
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4twoMaps",
                        "FORMAT": "png",
                        "map0:EXTENT": "11863620.20301065221428871,-5848927.97872077487409115,19375243.89574331790208817,138857.97204941",
                        "map0:LAYERS": "Country",
                        "map1:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map1:LAYERS": "Country,Hello",
                        "CRS": "EPSG:3857",
                        "IDTEXTBOX": "",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_TwoMaps")

    def test_wms_getprint_excluded_layout(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "excluded",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )
        r, h = self._result(self._execute_request(qs))

        self.assertIn(b"The TEMPLATE parameter is invalid", r)

    @unittest.skipIf(
        os.environ.get("QGIS_CONTINUOUS_INTEGRATION_RUN", "true"),
        "Can't rely on external resources for continuous integration",
    )
    def test_wms_getprint_external(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "map0:EXTENT": "-90,-180,90,180",
                        "map0:LAYERS": "EXTERNAL_WMS:landsat,Country",
                        "landsat:layers": "GEBCO_LATEST",
                        "landsat:dpiMode": "7",
                        "landsat:url": "https://www.gebco.net/data_and_products/gebco_web_services/web_map_service/mapserv",
                        "landsat:crs": "EPSG:4326",
                        "landsat:styles": "default",
                        "landsat:format": "image/jpeg",
                        "landsat:bbox": "-90,-180,90,180",
                        "landsat:version": "1.3.0",
                        "CRS": "EPSG:4326",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_External")

    def test_wms_getprint_highlight_empty_labels(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetPrint",
                        "TEMPLATE": "layoutA4",
                        "FORMAT": "png",
                        "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
                        "map0:LAYERS": "Country,Hello",
                        "CRS": "EPSG:3857",
                        "map0:HIGHLIGHT_GEOM": "POINT(-4000000 12215266);POINT(3271207 6832268);POINT(2360238 1035192)",
                        "map0:HIGHLIGHT_LABELSTRING": "Arctic;;Africa",
                        "map0:HIGHLIGHT_SYMBOL": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"1.1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" xmlns:se=\"http://www.opengis.net/se\"><UserStyle><se:FeatureTypeStyle><se:Rule><se:PointSymbolizer><se:Graphic><se:Mark><se:WellKnownName>circle</se:WellKnownName><se:Stroke><se:SvgParameter name=\"stroke\">%23ff0000</se:SvgParameter><se:SvgParameter name=\"stroke-opacity\">1</se:SvgParameter><se:SvgParameter name=\"stroke-width\">7.5</se:SvgParameter></se:Stroke><se:Fill><se:SvgParameter name=\"fill\">%237bdcb5</se:SvgParameter><se:SvgParameter name=\"fill-opacity\">1</se:SvgParameter></se:Fill></se:Mark><se:Size>28.4</se:Size></se:Graphic></se:PointSymbolizer></se:Rule></se:FeatureTypeStyle></UserStyle></StyledLayerDescriptor>;<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"1.1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" xmlns:se=\"http://www.opengis.net/se\"><UserStyle><se:FeatureTypeStyle><se:Rule><se:PointSymbolizer><se:Graphic><se:Mark><se:WellKnownName>circle</se:WellKnownName><se:Stroke><se:SvgParameter name=\"stroke\">%23ff0000</se:SvgParameter><se:SvgParameter name=\"stroke-opacity\">1</se:SvgParameter><se:SvgParameter name=\"stroke-width\">7.5</se:SvgParameter></se:Stroke><se:Fill><se:SvgParameter name=\"fill\">%237bdcb5</se:SvgParameter><se:SvgParameter name=\"fill-opacity\">1</se:SvgParameter></se:Fill></se:Mark><se:Size>28.4</se:Size></se:Graphic></se:PointSymbolizer></se:Rule></se:FeatureTypeStyle></UserStyle></StyledLayerDescriptor>;<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"1.1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" xmlns:se=\"http://www.opengis.net/se\"><UserStyle><se:FeatureTypeStyle><se:Rule><se:PointSymbolizer><se:Graphic><se:Mark><se:WellKnownName>circle</se:WellKnownName><se:Stroke><se:SvgParameter name=\"stroke\">%23ff0000</se:SvgParameter><se:SvgParameter name=\"stroke-opacity\">1</se:SvgParameter><se:SvgParameter name=\"stroke-width\">7.5</se:SvgParameter></se:Stroke><se:Fill><se:SvgParameter name=\"fill\">%237bdcb5</se:SvgParameter><se:SvgParameter name=\"fill-opacity\">1</se:SvgParameter></se:Fill></se:Mark><se:Size>28.4</se:Size></se:Graphic></se:PointSymbolizer></se:Rule></se:FeatureTypeStyle></UserStyle></StyledLayerDescriptor>",
                        "map0:HIGHLIGHT_LABELSIZE": "16;16;16",
                        "map0:HIGHLIGHT_LABELCOLOR": "red;red;red",
                        "map0:HIGHLIGHT_LABELBUFFERCOLOR": "white;white;white",
                        "map0:HIGHLIGHT_LABELBUFFERSIZE": "1;1;1",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Highlight_Empty_Labels")


if __name__ == "__main__":
    unittest.main()
