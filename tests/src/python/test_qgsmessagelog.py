"""QGIS Unit tests for QgsMessageLog.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "18/06/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsMessageLog,
    QgsMessageLogNotifyBlocker,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMessageLog(QgisTestCase):

    def testSignals(self):
        app_log = QgsApplication.messageLog()

        # signals should be emitted by application log
        app_spy = QSignalSpy(app_log.messageReceived)
        app_spy_received = QSignalSpy(app_log.messageReceived[bool])

        QgsMessageLog.logMessage("test", "tag", Qgis.MessageLevel.Info, notifyUser=True)
        self.assertEqual(len(app_spy), 1)
        self.assertEqual(app_spy[-1], ["test", "tag", Qgis.MessageLevel.Info])
        # info message, so messageReceived(bool) should not be emitted
        self.assertEqual(len(app_spy_received), 0)

        QgsMessageLog.logMessage(
            "test", "tag", Qgis.MessageLevel.Warning, notifyUser=True
        )
        self.assertEqual(len(app_spy), 2)
        self.assertEqual(app_spy[-1], ["test", "tag", Qgis.MessageLevel.Warning])
        # warning message, so messageReceived(bool) should be emitted
        self.assertEqual(len(app_spy_received), 1)

        QgsMessageLog.logMessage(
            "test", "tag", Qgis.MessageLevel.Warning, notifyUser=False
        )
        self.assertEqual(len(app_spy), 3)
        # notifyUser was False
        self.assertEqual(len(app_spy_received), 1)

    def testBlocker(self):
        app_log = QgsApplication.messageLog()

        spy = QSignalSpy(app_log.messageReceived)
        spy_received = QSignalSpy(app_log.messageReceived[bool])

        QgsMessageLog.logMessage(
            "test", "tag", Qgis.MessageLevel.Warning, notifyUser=True
        )
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1], ["test", "tag", Qgis.MessageLevel.Warning])
        self.assertEqual(len(spy_received), 1)

        # block notifications
        b = QgsMessageLogNotifyBlocker()
        QgsMessageLog.logMessage(
            "test", "tag", Qgis.MessageLevel.Warning, notifyUser=True
        )
        self.assertEqual(len(spy), 2)  # should not be blocked
        self.assertEqual(len(spy_received), 1)  # should be blocked

        # another blocker
        b2 = QgsMessageLogNotifyBlocker()
        QgsMessageLog.logMessage(
            "test", "tag", Qgis.MessageLevel.Warning, notifyUser=True
        )
        self.assertEqual(len(spy), 3)  # should not be blocked
        self.assertEqual(len(spy_received), 1)  # should be blocked

        del b
        # still blocked because of b2
        QgsMessageLog.logMessage(
            "test", "tag", Qgis.MessageLevel.Warning, notifyUser=True
        )
        self.assertEqual(len(spy), 4)  # should not be blocked
        self.assertEqual(len(spy_received), 1)  # should be blocked

        del b2
        # not blocked
        QgsMessageLog.logMessage(
            "test", "tag", Qgis.MessageLevel.Warning, notifyUser=True
        )
        self.assertEqual(len(spy), 5)  # should not be blocked
        self.assertEqual(len(spy_received), 2)  # should not be blocked


if __name__ == "__main__":
    unittest.main()
