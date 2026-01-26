"""QGIS Unit tests for QgsSensorManager and its related classes.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2023 by Mathieu Pellerin"
__date__ = "19/03/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import os

from qgis.PyQt.QtCore import (
    QCoreApplication,
    QEvent,
    QLocale,
    QTemporaryDir,
    QIODevice,
    QBuffer,
)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsAbstractSensor,
    QgsExpression,
    QgsExpressionContext,
    QgsExpressionContextUtils,
    QgsIODeviceSensor,
    QgsProject,
    QgsSensorManager,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestSensor(QgsIODeviceSensor):

    buffer = None

    def __init__(self):
        super().__init__()
        self.buffer = QBuffer()
        self.initIODevice(self.buffer)

    def handleConnect(self):
        self.buffer.open(QIODevice.OpenModeFlag.ReadWrite)
        self.setStatus(Qgis.DeviceConnectionStatus.Connected)

    def handleDisconnect(self):
        self.buffer.close()

    def pushData(self, data):
        self.buffer.buffer().clear()
        self.buffer.seek(0)
        self.buffer.write(data.encode("ascii"))
        self.buffer.seek(0)


class TestQgsSensorManager(QgisTestCase):

    manager = None
    sensor = None

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsSensorManager.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsSensorManager")
        QLocale.setDefault(QLocale(QLocale.Language.English))
        start_app()

        cls.manager = QgsProject.instance().sensorManager()
        cls.sensor = TestSensor()
        cls.manager.addSensor(cls.sensor)

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        self.sensor.disconnectSensor()
        self.sensor.setName("")
        pass

    def testManagerAddRemove(self):
        sensor1 = TestSensor()
        sensor1_id = sensor1.id()

        manager_added_spy = QSignalSpy(self.manager.sensorAdded)

        self.manager.addSensor(sensor1)
        self.assertEqual(len(manager_added_spy), 1)
        self.assertTrue(self.manager.sensor(sensor1_id))

        manager_abouttoberemoved_spy = QSignalSpy(self.manager.sensorAboutToBeRemoved)
        manager_removed_spy = QSignalSpy(self.manager.sensorRemoved)

        self.assertTrue(self.manager.removeSensor(sensor1_id))
        self.assertEqual(len(manager_abouttoberemoved_spy), 1)
        self.assertEqual(len(manager_removed_spy), 1)

    def testNameAndStatus(self):
        self.assertEqual(self.sensor.name(), "")
        self.sensor.setName("test sensor")
        self.assertEqual(self.sensor.name(), "test sensor")

        self.assertEqual(self.sensor.status(), Qgis.DeviceConnectionStatus.Disconnected)
        self.sensor.connectSensor()
        self.assertEqual(self.sensor.status(), Qgis.DeviceConnectionStatus.Connected)

    def testProcessData(self):
        self.sensor.setName("test sensor")
        self.sensor.connectSensor()
        self.assertEqual(self.sensor.status(), Qgis.DeviceConnectionStatus.Connected)

        sensor_spy = QSignalSpy(self.sensor.dataChanged)
        manager_spy = QSignalSpy(self.manager.sensorDataCaptured)

        self.sensor.pushData("test string")
        manager_spy.wait()
        self.assertEqual(len(sensor_spy), 1)
        self.assertEqual(len(manager_spy), 1)
        self.assertEqual(self.sensor.data().lastValue, "test string")
        self.assertEqual(
            self.manager.sensorData("test sensor").lastValue, "test string"
        )

    def testSensorDataExpression(self):
        self.sensor.setName("test sensor")
        self.sensor.connectSensor()
        self.assertEqual(self.sensor.status(), Qgis.DeviceConnectionStatus.Connected)

        data_spy = QSignalSpy(self.sensor.dataChanged)

        self.sensor.pushData("test string 2")
        data_spy.wait()
        self.assertEqual(len(data_spy), 1)
        self.assertEqual(self.sensor.data().lastValue, "test string 2")

        expression = QgsExpression("sensor_data('test sensor')")

        context = QgsExpressionContext()
        context.appendScope(
            QgsExpressionContextUtils.projectScope(QgsProject.instance())
        )

        result = expression.evaluate(context)
        self.assertEqual(result, "test string 2")


if __name__ == "__main__":
    unittest.main()
