# -*- coding: utf-8 -*-
"""QGIS Unit tests for the WFS provider. GUI part

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-03-25'
__copyright__ = 'Copyright 2016, Even Rouault'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import hashlib
import os
import sys
import tempfile
import shutil

from qgis.PyQt.QtCore import QObject, QCoreApplication, QSettings, Qt, QEventLoop
from qgis.PyQt.QtWidgets import QApplication, QWidget, QTextEdit, QLineEdit, QDialogButtonBox, QTreeView, QComboBox
from qgis.PyQt.QtTest import QTest

from qgis.core import (
    QgsProviderRegistry
)
from qgis.testing import (start_app,
                          unittest
                          )
from providertestbase import ProviderTestCase


def sanitize(endpoint, x):
    if len(endpoint + x) > 256:
        return endpoint + hashlib.md5(x.encode()).hexdigest()
    return endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"', '_').replace("'", '_').replace(' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')


def find_window(name):
    for widget in QApplication.topLevelWidgets():
        if widget.objectName() == name:
            return widget
    return None


class TestPyQgsWFSProviderGUI(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsWFSProviderGUI.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsWFSProviderGUI")
        QSettings().clear()
        start_app()

        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QSettings().clear()
        if cls.basetestpath is not None:
            shutil.rmtree(cls.basetestpath, True)

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

    def get_button_build_query(self, main_dialog):
        return self.get_button(main_dialog, "&Build query")

    def wait_object_destruction(self, my_object):
        loop = QEventLoop()
        name = my_object.objectName()
        my_object.destroyed.connect(loop.quit)
        loop.exec_()
        self.assertIsNone(find_window(name))
        return None

    def test(self):

        # This test is quite fragile as it depends on windows manager behaviour
        # regarding focus, so not surprising it doesn't pass
        # on other platforms than Linux.
        #if 'TRAVIS_OS_NAME' in os.environ and os.environ['TRAVIS_OS_NAME'] == 'osx':
        #    return

        main_dialog = QgsProviderRegistry.instance().selectWidget("WFS")
        main_dialog.setProperty("hideDialogs", True)

        self.assertIsNotNone(main_dialog)

        # Create new connection
        btnNew = main_dialog.findChild(QWidget, "btnNew")
        self.assertIsNotNone(btnNew)
        QTest.mouseClick(btnNew, Qt.LeftButton)
        new_conn = find_window('QgsNewHttpConnectionBase')
        self.assertIsNotNone(new_conn)
        txtName = new_conn.findChild(QLineEdit, "txtName")
        self.assertIsNotNone(txtName)
        txtName.setText("test_connection")
        txtUrl = new_conn.findChild(QLineEdit, "txtUrl")
        self.assertIsNotNone(txtUrl)
        txtUrl.setText("test_url")
        new_conn.accept()

        # Wait for object to be destroyed
        new_conn = self.wait_object_destruction(new_conn)

        # Try to connect
        btnConnect = main_dialog.findChild(QWidget, "btnConnect")
        self.assertIsNotNone(btnConnect)
        QTest.mouseClick(btnConnect, Qt.LeftButton)
        # Depends on asynchronous signal
        QApplication.processEvents()
        error_box = find_window('WFSCapabilitiesErrorBox')
        self.assertIsNotNone(error_box)
        # Close error box
        error_box.accept()

        # Wait for object to be destroyed
        error_box = self.wait_object_destruction(error_box)

        # Edit connection
        btnEdit = main_dialog.findChild(QWidget, "btnEdit")
        self.assertIsNotNone(btnEdit)
        QTest.mouseClick(btnEdit, Qt.LeftButton)
        new_conn = find_window('QgsNewHttpConnectionBase',)
        self.assertIsNotNone(new_conn)
        txtName = new_conn.findChild(QLineEdit, "txtName")
        self.assertIsNotNone(txtName)
        txtName.setText("test_connection")
        txtUrl = new_conn.findChild(QLineEdit, "txtUrl")
        self.assertIsNotNone(txtUrl)

        endpoint = self.basetestpath + '/fake_qgis_http_endpoint'
        expected_endpoint = endpoint
        if sys.platform == 'win32' and expected_endpoint[1] == ':':
            expected_endpoint = expected_endpoint[0] + expected_endpoint[2:]
        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
  <fes:Filter_Capabilities>
    <fes:Spatial_Capabilities>
      <fes:GeometryOperands>
        <fes:GeometryOperand name="gml:Envelope"/>
        <fes:GeometryOperand name="gml:Point"/>
        <fes:GeometryOperand name="gml:MultiPoint"/>
        <fes:GeometryOperand name="gml:LineString"/>
        <fes:GeometryOperand name="gml:MultiLineString"/>
        <fes:GeometryOperand name="gml:Polygon"/>
        <fes:GeometryOperand name="gml:MultiPolygon"/>
        <fes:GeometryOperand name="gml:MultiGeometry"/>
      </fes:GeometryOperands>
      <fes:SpatialOperators>
        <fes:SpatialOperator name="Disjoint"/>
        <fes:SpatialOperator name="Equals"/>
        <fes:SpatialOperator name="DWithin"/>
        <fes:SpatialOperator name="Beyond"/>
        <fes:SpatialOperator name="Intersects"/>
        <fes:SpatialOperator name="Touches"/>
        <fes:SpatialOperator name="Crosses"/>
        <fes:SpatialOperator name="Within"/>
        <fes:SpatialOperator name="Contains"/>
        <fes:SpatialOperator name="Overlaps"/>
        <fes:SpatialOperator name="BBOX"/>
      </fes:SpatialOperators>
    </fes:Spatial_Capabilities>
    <fes:Functions>
      <fes:Function name="abs">
        <fes:Returns>xs:int</fes:Returns>
        <fes:Arguments>
          <fes:Argument name="param">
            <fes:Type>xs:int</fes:Type>
          </fes:Argument>
        </fes:Arguments>
      </fes:Function>
    </fes:Functions>
  </fes:Filter_Capabilities>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        txtUrl.setText("http://" + endpoint)
        new_conn.accept()

        # Wait for object to be destroyed
        new_conn = self.wait_object_destruction(new_conn)

        # Try to connect
        btnConnect = main_dialog.findChild(QWidget, "btnConnect")
        self.assertIsNotNone(btnConnect)
        QTest.mouseClick(btnConnect, Qt.LeftButton)

        buttonAdd = self.get_button_add(main_dialog)
        for i in range(10):
            QApplication.processEvents()
            if buttonAdd.isEnabled():
                break
        self.assertTrue(buttonAdd.isEnabled())

        # Add layer
        self.addWfsLayer_uri = None
        self.addWfsLayer_layer_name = None
        main_dialog.addWfsLayer.connect(self.slotAddWfsLayer)
        QTest.mouseClick(buttonAdd, Qt.LeftButton)
        self.assertEqual(self.addWfsLayer_uri, ' retrictToRequestBBOX=\'1\' srsname=\'EPSG:4326\' typename=\'my:typename\' url=\'' + "http://" + expected_endpoint + '\' version=\'auto\' table="" sql=')
        self.assertEqual(self.addWfsLayer_layer_name, 'my:typename')

        # Click on Build Query
        buttonBuildQuery = self.get_button_build_query(main_dialog)
        self.assertTrue(buttonBuildQuery.isEnabled())
        QTest.mouseClick(buttonBuildQuery, Qt.LeftButton)
        error_box = find_window('WFSFeatureTypeErrorBox')
        self.assertIsNotNone(error_box)
        # Close error box
        error_box.accept()
        # Wait for object to be destroyed
        error_box = self.wait_object_destruction(error_box)

        # Click again but with valid DescribeFeatureType

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PolygonPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))
        QTest.mouseClick(buttonBuildQuery, Qt.LeftButton)

        # Check that the combos are properly initialized
        dialog = find_window('QgsSQLComposerDialogBase')
        self.assertIsNotNone(dialog)

        mTablesCombo = dialog.findChild(QComboBox, "mTablesCombo")
        self.assertIsNotNone(mTablesCombo)
        self.assertEqual(mTablesCombo.itemText(1), 'typename (Title)')

        mColumnsCombo = dialog.findChild(QComboBox, "mColumnsCombo")
        self.assertIsNotNone(mColumnsCombo)
        self.assertEqual(mColumnsCombo.itemText(1), 'intfield (int)')
        self.assertEqual(mColumnsCombo.itemText(mColumnsCombo.count() - 2), 'geometryProperty (geometry)')
        self.assertEqual(mColumnsCombo.itemText(mColumnsCombo.count() - 1), '*')

        mFunctionsCombo = dialog.findChild(QComboBox, "mFunctionsCombo")
        self.assertIsNotNone(mFunctionsCombo)
        self.assertEqual(mFunctionsCombo.itemText(1), 'abs(param: int): int')

        mSpatialPredicatesCombo = dialog.findChild(QComboBox, "mSpatialPredicatesCombo")
        self.assertIsNotNone(mSpatialPredicatesCombo)
        self.assertEqual(mSpatialPredicatesCombo.itemText(1), 'ST_Disjoint(geometry, geometry): boolean')

        mWhereEditor = dialog.findChild(QTextEdit, "mWhereEditor")
        self.assertIsNotNone(mWhereEditor)
        mWhereEditor.setText('1 = 1')

        dialog.accept()
        # Wait for object to be destroyed
        dialog = self.wait_object_destruction(dialog)

        # Add layer
        buttonAdd = self.get_button_add(main_dialog)
        self.assertTrue(buttonAdd.isEnabled())

        self.addWfsLayer_uri = None
        self.addWfsLayer_layer_name = None
        main_dialog.addWfsLayer.connect(self.slotAddWfsLayer)
        QTest.mouseClick(buttonAdd, Qt.LeftButton)
        self.assertEqual(self.addWfsLayer_uri, ' retrictToRequestBBOX=\'1\' srsname=\'EPSG:4326\' typename=\'my:typename\' url=\'' + "http://" + expected_endpoint + '\' version=\'auto\' table="" sql=SELECT * FROM typename WHERE 1 = 1')
        self.assertEqual(self.addWfsLayer_layer_name, 'my:typename')

        #main_dialog.setProperty("hideDialogs", None)
        #main_dialog.exec_()

    def slotAddWfsLayer(self, uri, layer_name):
        self.addWfsLayer_uri = uri
        self.addWfsLayer_layer_name = layer_name

if __name__ == '__main__':
    unittest.main()
