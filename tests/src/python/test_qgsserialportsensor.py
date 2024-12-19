"""QGIS Unit tests for QgsSerialPortSensor.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2023 by Mathieu Pellerin'
__date__ = '19/03/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import os
import posix

from qgis.PyQt.QtCore import QCoreApplication, QEvent, QLocale, QTemporaryDir, QIODevice, QBuffer, QByteArray
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsProject,
    QgsSerialPortSensor,
    QgsSensorManager
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSerialPortSensor(QgisTestCase):

    manager = None

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsSerialPortSensor.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsSerialPortSensor")
        QLocale.setDefault(QLocale(QLocale.Language.English))
        start_app()

        cls.manager = QgsProject.instance().sensorManager()

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testSerialPortSensor(self):
        fd1, fd2 = posix.openpty()
        fd_file = os.fdopen(fd1, "wb")

        serial_port_sensor = QgsSerialPortSensor()
        serial_port_sensor.setName('serial port sensor')
        serial_port_sensor.setPortName(os.ttyname(fd2))
        serial_port_sensor.setDelimiter(b'\n')

        serial_port_sensor_id = serial_port_sensor.id()

        self.manager.addSensor(serial_port_sensor)

        sensor_spy = QSignalSpy(serial_port_sensor.dataChanged)
        manager_spy = QSignalSpy(self.manager.sensorDataCaptured)

        serial_port_sensor.connectSensor()
        self.assertEqual(serial_port_sensor.status(), Qgis.DeviceConnectionStatus.Connected)

        QCoreApplication.processEvents()
        fd_file.write(b'test 1\nfull ')
        fd_file.flush()
        QCoreApplication.processEvents()

        # No signal should be fired as the delimiter must be captured at least once to insure a full data frame
        self.assertEqual(len(sensor_spy), 0)
        self.assertEqual(len(manager_spy), 0)

        QCoreApplication.processEvents()
        fd_file.write(b'test 2\n')
        fd_file.flush()
        QCoreApplication.processEvents()

        self.assertEqual(len(sensor_spy), 1)
        self.assertEqual(len(manager_spy), 1)
        self.assertEqual(serial_port_sensor.data().lastValue, QByteArray(b'full test 2'))
        self.assertEqual(self.manager.sensorData('serial port sensor').lastValue, b'full test 2')

        self.manager.removeSensor(serial_port_sensor_id)


if __name__ == '__main__':
    unittest.main()
