"""QGIS Unit tests for QgsInputControllerManager

From build dir, run: ctest -R QgsArcGisRestUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2022 by Nyall Dawson"
__date__ = "14/07/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, Qt, QTime, QTimeZone, QVariant
from qgis.gui import (
    QgsInputControllerManager,
    QgsAbstract2DMapController,
    QgsAbstract3DMapController,
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class Dummy2dController(QgsAbstract2DMapController):

    def deviceId(self):
        return "dummy2d"

    def clone(self):
        return Dummy2dController()


class Dummy3dController(QgsAbstract3DMapController):

    def deviceId(self):
        return "dummy3d"

    def clone(self):
        return Dummy3dController()


class TestQgsInputController(unittest.TestCase):

    def test_registration(self):
        manager = QgsInputControllerManager()
        self.assertFalse(manager.available2DMapControllers())
        self.assertFalse(manager.available3DMapControllers())

        manager.register2DMapController(Dummy2dController())
        self.assertEqual(manager.available2DMapControllers(), ["dummy2d"])
        self.assertFalse(manager.available3DMapControllers())

        new_controller = manager.create2DMapController("dummy2d")
        self.assertIsInstance(new_controller, Dummy2dController)
        self.assertEqual(new_controller.deviceId(), "dummy2d")
        self.assertIsNone(manager.create2DMapController("nope"))

        manager.register3DMapController(Dummy3dController())
        self.assertEqual(manager.available2DMapControllers(), ["dummy2d"])
        self.assertEqual(manager.available3DMapControllers(), ["dummy3d"])

        new_controller = manager.create3DMapController("dummy3d")
        self.assertIsInstance(new_controller, Dummy3dController)
        self.assertEqual(new_controller.deviceId(), "dummy3d")
        self.assertIsNone(manager.create3DMapController("nope"))


if __name__ == "__main__":
    unittest.main()
