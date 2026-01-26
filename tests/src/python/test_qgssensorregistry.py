"""QGIS Unit tests for QgsSensorRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Mathieu Pellerin"
__date__ = "19/03/2023"
__copyright__ = "Copyright 2023, The QGIS Project"


from qgis.core import (
    QgsSensorRegistry,
    QgsSensorAbstractMetadata,
    QgsTcpSocketSensor,
    QgsUdpSocketSensor,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestTcpSensorMetadata(QgsSensorAbstractMetadata):

    def __init__(self):
        super().__init__("test_tcp_sensor", "test tcp sensor")

    def createSensor(self, parent):
        return QgsTcpSocketSensor(parent)


class TestUdpSensorMetadata(QgsSensorAbstractMetadata):

    def __init__(self):
        super().__init__("test_udp_sensor", "test udp sensor")

    def createSensor(self, parent):
        return QgsUdpSocketSensor(parent)


class TestQgsSensorRegistry(QgisTestCase):

    def testRegistry(self):
        registry = QgsSensorRegistry()

        registry.addSensorType(TestTcpSensorMetadata())
        registry.addSensorType(TestUdpSensorMetadata())
        self.assertEqual(
            registry.sensorTypes(),
            {
                "test_tcp_sensor": "test tcp sensor",
                "test_udp_sensor": "test udp sensor",
            },
        )

        sensor = registry.createSensor("test_tcp_sensor")
        self.assertTrue(sensor)
        self.assertEqual(sensor.type(), "tcp_socket")

        sensor = registry.createSensor("test_udp_sensor")
        self.assertTrue(sensor)
        self.assertEqual(sensor.type(), "udp_socket")

        sensor = registry.createSensor("invalid_sensor_type")
        self.assertFalse(sensor)

        registry.removeSensorType("test_tcp_sensor")
        self.assertEqual(registry.sensorTypes(), {"test_udp_sensor": "test udp sensor"})


if __name__ == "__main__":
    unittest.main()
