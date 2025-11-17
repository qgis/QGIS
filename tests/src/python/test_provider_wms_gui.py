"""QGIS Unit tests for the WMS provider. GUI part

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alessandro Pasotti"
__date__ = "2025-10-25"
__copyright__ = "Copyright 2025, Alessandro Pasotti"

import hashlib
import shutil
import sys
import tempfile
from urllib.parse import quote

from qgis.PyQt.QtCore import QCoreApplication, QEventLoop, Qt
from qgis.PyQt.QtTest import QTest
from qgis.core import QgsSettings, QgsOwsConnection, QgsDataItemProviderRegistry, Qgis
from qgis.gui import QgsGui
from qgis.PyQt.QtWidgets import (
    QApplication,
    QComboBox,
    QButtonGroup,
    QDialogButtonBox,
    QLineEdit,
    QPushButton,
    QTextEdit,
    QWidget,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from test_provider_wfs_gui import find_window


class TestPyQgsWMSProviderGUI(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsWMSProviderGUI.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsWMSProviderGUI")
        QgsSettings().clear()
        start_app()

        cls.basetestpath = tempfile.mkdtemp().replace("\\", "/")

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        if cls.basetestpath is not None:
            shutil.rmtree(cls.basetestpath, True)
        super().tearDownClass()

    def get_button(self, main_dialog, text):
        buttonBox = main_dialog.findChild(QDialogButtonBox, "buttonBox")
        self.assertIsNotNone(buttonBox)
        button = None
        for button in buttonBox.buttons():
            if str(button.text()) == text:
                return button
        self.assertIsNotNone(None)
        return None

    def get_button_add(self, main_dialog):
        return self.get_button(main_dialog, "&Add")

    def wait_object_destruction(self, my_object):
        loop = QEventLoop()
        name = my_object.objectName()
        my_object.destroyed.connect(loop.quit)
        loop.exec()
        self.assertIsNone(find_window(name))
        return None

    def test(self):

        # Set the last used format in global settings
        QgsSettings().setValue("qgis/lastWmsImageEncoding", "image/tiff")

        main_dialog = (
            QgsGui.providerGuiRegistry()
            .sourceSelectProviders("wms")[0]
            .createDataSourceWidget()
        )
        main_dialog.setProperty("hideDialogs", True)

        self.assertIsNotNone(main_dialog)

        # Create new connection
        btnNew = main_dialog.findChild(QWidget, "btnNew")
        self.assertIsNotNone(btnNew)

        QTest.mouseClick(btnNew, Qt.MouseButton.LeftButton)

        new_conn = find_window("QgsNewHttpConnectionBase")
        self.assertIsNotNone(new_conn)

        txtName = new_conn.findChild(QLineEdit, "txtName")
        self.assertIsNotNone(txtName)
        txtName.setText("test_connection")

        txtUrl = new_conn.findChild(QLineEdit, "txtUrl")
        self.assertIsNotNone(txtUrl)

        endpoint = self.basetestpath + "/fake_qgis_http_endpoint"
        expected_endpoint = endpoint
        if sys.platform == "win32" and expected_endpoint[1] == ":":
            expected_endpoint = expected_endpoint[0] + expected_endpoint[2:]
        with open(
            QgisTestCase.sanitize_local_url(
                endpoint,
                "?SERVICE=WMS&REQUEST=GetCapabilities",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
<WMS_Capabilities xmlns="http://www.opengis.net/wms" xmlns:sld="http://www.opengis.net/sld" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.3.0" xsi:schemaLocation="http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/sld_capabilities.xsd">
<Service>
  <Name>WMS</Name>
  <Title>MapProxy-WMS Rasterbilder Bebauungsplaene rv auf Basiskarte grau</Title>
  <Abstract>WMS mit Rasterbilder der rechtsverbindlichen Bebauungsplaene auf grauer Basiskarte ueber MapProxy</Abstract>
    <MaxWidth>4000</MaxWidth>
    <MaxHeight>4000</MaxHeight>
</Service>
<Capability>
  <Request>
    <GetCapabilities>
      <Format>text/xml</Format>
    </GetCapabilities>
    <GetMap>
      <Format>image/jpeg</Format>
      <Format>image/webp</Format>
      <Format>image/png</Format>
      <Format>image/tiff</Format>
    </GetMap>
    <GetFeatureInfo>
      <Format>text/plain</Format>
      <Format>text/html</Format>
      <Format>text/xml</Format>
    </GetFeatureInfo>
  </Request>
  <Exception>
    <Format>XML</Format>
    <Format>INIMAGE</Format>
    <Format>BLANK</Format>
  </Exception>
  <Layer>
    <Name>bplan_stadtkarte</Name>
    <Title>Bebauungsplaene Rasterbilder auf grauer Basiskarte</Title>
    <CRS>EPSG:25832</CRS>
    <CRS>EPSG:4326</CRS>
    <EX_GeographicBoundingBox>
      <westBoundLongitude>8.471329688231897</westBoundLongitude>
      <eastBoundLongitude>8.801042621684477</eastBoundLongitude>
      <southBoundLatitude>50.01482739264088</southBoundLatitude>
      <northBoundLatitude>50.22734893698391</northBoundLatitude>
    </EX_GeographicBoundingBox>
    <BoundingBox CRS="CRS:84" minx="8.471329688231897" miny="50.01482739264088" maxx="8.801042621684477" maxy="50.22734893698391" />
    <BoundingBox CRS="EPSG:4326" minx="50.01482739264088" miny="8.471329688231897" maxx="50.22734893698391" maxy="8.801042621684477" />
    <BoundingBox CRS="EPSG:25832" minx="462290" miny="5540412" maxx="485746" maxy="5563928" />
  </Layer>
</Capability>
</WMS_Capabilities>"""
            )

        # Test detect format
        detect_btn = new_conn.findChild(QPushButton, "mWmsFormatDetectButton")
        self.assertIsNotNone(detect_btn)

        # Select JPEG as image format
        format_combo = new_conn.findChild(QComboBox, "mWmsPreferredFormatCombo")
        self.assertIsNotNone(format_combo)

        # Check that default is TIFF (from global settings)
        self.assertEqual(format_combo.currentData(), "image/tiff")

        # Set to JPEG
        format_combo.setCurrentIndex(format_combo.findData("image/jpeg"))

        url = "file://" + endpoint + quote("_SERVICE=WMS_REQUEST=GetCapabilities")
        txtUrl.setText(url)

        QTest.mouseClick(detect_btn, Qt.MouseButton.LeftButton)

        # Check that formats have been detected
        detected_formats = [
            format_combo.itemData(i) for i in range(format_combo.count())
        ]
        self.assertEqual(
            detected_formats,
            [
                "image/png",
                "image/webp",
                "image/jpeg",
                "image/tiff",
            ],
        )

        new_conn.accept()

        # Wait for object to be destroyed
        new_conn = self.wait_object_destruction(new_conn)

        # Check that default format has been saved in settings
        self.assertEqual(
            QgsSettings().value(
                "/connections/ows/items/wms/connections/items/test_connection/default-image-format"
            ),
            "image/jpeg",
        )
        self.assertEqual(
            QgsSettings().value(
                "/connections/ows/items/wms/connections/items/test_connection/available-image-formats"
            ),
            ["image/png", "image/webp", "image/jpeg", "image/tiff"],
        )

        # Try to connect
        btnConnect = main_dialog.findChild(QWidget, "btnConnect")
        self.assertIsNotNone(btnConnect)

        QTest.mouseClick(btnConnect, Qt.MouseButton.LeftButton)

        # Select first layer
        layerTree = main_dialog.findChild(QWidget, "lstLayers")
        self.assertIsNotNone(layerTree)

        buttonAdd = self.get_button_add(main_dialog)
        self.assertFalse(buttonAdd.isEnabled())

        layerTree.selectAll()

        for i in range(10):
            QApplication.processEvents()
            if buttonAdd.isEnabled():
                break
        self.assertTrue(buttonAdd.isEnabled())

        # Add layer
        self.addWmsLayer_uri = None
        self.addWmsLayer_layer_name = None
        main_dialog.addRasterLayer.connect(self.slotAddWmsLayer)
        QTest.mouseClick(buttonAdd, Qt.MouseButton.LeftButton)
        self.assertEqual(
            self.addWmsLayer_uri,
            "contextualWMSLegend=0&crs=EPSG:4326&dpiMode=7&featureCount=10&"
            + "format=image/jpeg"  # <--- actual test !
            + "&layers=bplan_stadtkarte&styles&tilePixelRatio=0&url="
            + url,
        )
        self.assertEqual(
            self.addWmsLayer_layer_name,
            "Bebauungsplaene Rasterbilder auf grauer Basiskarte",
        )

        # Now add change the format and check that it is reflected in the added layer URI
        format_group = main_dialog.findChild(QButtonGroup, "mImageFormatGroup")
        self.assertIsNotNone(format_group)
        self.assertEqual(format_group.checkedButton().text(), "JPEG")

        format_buttons = main_dialog.findChild(
            QButtonGroup, "mImageFormatGroup"
        ).buttons()
        visible_buttons = [
            btn.property("mime-type") for btn in format_buttons if not btn.isHidden()
        ]

        self.assertEqual(
            visible_buttons,
            [
                "image/png",
                "image/webp",
                "image/jpeg",
                "image/tiff",
            ],
        )

        for btn in format_buttons:
            if btn.text() == "WebP":
                btn.setChecked(True)
                break

        self.addWmsLayer_uri = None
        self.addWmsLayer_layer_name = None
        QTest.mouseClick(buttonAdd, Qt.MouseButton.LeftButton)
        self.assertEqual(
            self.addWmsLayer_uri,
            "contextualWMSLegend=0&crs=EPSG:4326&dpiMode=7&featureCount=10&"
            + "format=image/webp"  # <--- actual test !
            + "&layers=bplan_stadtkarte&styles&tilePixelRatio=0&url="
            + url,
        )

        # This should also have changed the global last used format default
        self.assertEqual(
            QgsSettings().value("qgis/lastWmsImageEncoding"),
            "image/webp",
        )

        # Test dataitems URI
        registry = QgsDataItemProviderRegistry()
        wms_provider = registry.provider("WMS")
        collection_item = wms_provider.createDataItem("", None)
        connection_item = collection_item.children()[0]
        self.assertEqual(connection_item.name(), "test_connection")
        connection_item.populate()
        while connection_item.state() == Qgis.BrowserItemState.Populating:
            QApplication.processEvents()

        layer_item = connection_item.children()[0]
        self.assertEqual(
            layer_item.name(), "Bebauungsplaene Rasterbilder auf grauer Basiskarte"
        )
        self.assertIn("format=image/jpeg", layer_item.uri())

    def slotAddWmsLayer(self, uri, layer_name):
        self.addWmsLayer_uri = uri
        self.addWmsLayer_layer_name = layer_name


if __name__ == "__main__":
    unittest.main()
