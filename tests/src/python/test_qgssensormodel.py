"""QGIS Unit tests for QgsSensorModel and its related classes.

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
    QDateTime,
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
    QgsSensorModel,
    QgsTcpSocketSensor,
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

    def type(self):
        return "test_sensor"

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


class TestQgsSensorModel(QgisTestCase):

    manager = None
    model = None

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
        cls.model = QgsSensorModel(cls.manager)

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testModel(self):
        sensor1 = QgsTcpSocketSensor()
        sensor1.setName("name1")
        sensor2 = TestSensor()
        sensor2.setName("name2")

        model_row_inserted_spy = QSignalSpy(self.model.rowsInserted)

        self.manager.addSensor(sensor1)
        self.assertEqual(len(model_row_inserted_spy), 1)
        self.manager.addSensor(sensor2)
        self.assertEqual(len(model_row_inserted_spy), 2)
        self.assertEqual(self.model.rowCount(), 2)

        model_row_removed_spy = QSignalSpy(self.model.rowsRemoved)

        self.manager.removeSensor(sensor1.id())
        self.assertEqual(len(model_row_removed_spy), 1)
        self.assertEqual(self.model.rowCount(), 1)

        self.assertEqual(
            self.model.data(self.model.index(0, 0), QgsSensorModel.Role.SensorId),
            sensor2.id(),
        )
        self.assertEqual(
            self.model.data(self.model.index(0, 0), QgsSensorModel.Role.SensorName),
            sensor2.name(),
        )
        self.assertEqual(
            self.model.data(
                self.model.index(0, 0), QgsSensorModel.Role.SensorLastValue
            ),
            None,
        )
        self.assertEqual(
            self.model.data(
                self.model.index(0, 0), QgsSensorModel.Role.SensorLastTimestamp
            ),
            None,
        )
        self.assertEqual(
            self.model.data(self.model.index(0, 0), QgsSensorModel.Role.Sensor), sensor2
        )

        model_data_changed_spy = QSignalSpy(self.model.dataChanged)

        sensor2.setName("new name2")
        sensor2.connectSensor()
        sensor2.pushData("test string")
        model_data_changed_spy.wait()
        self.assertEqual(
            len(model_data_changed_spy), 4
        )  # new name, connecting + connected state, and data change signals
        self.assertEqual(
            self.model.data(self.model.index(0, 0), QgsSensorModel.Role.SensorName),
            sensor2.name(),
        )
        self.assertEqual(
            self.model.data(
                self.model.index(0, 0), QgsSensorModel.Role.SensorLastValue
            ),
            sensor2.data().lastValue,
        )
        self.assertEqual(
            self.model.data(
                self.model.index(0, 0), QgsSensorModel.Role.SensorLastTimestamp
            ),
            sensor2.data().lastTimestamp,
        )


if __name__ == "__main__":
    unittest.main()
