# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMessageLog.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/06/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (Qgis,
                       QgsApplication,
                       QgsMessageLog,
                       QgsMessageLogNotifyBlocker)

from qgis.PyQt.QtTest import QSignalSpy

from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMessageLog(unittest.TestCase):

    def testSignals(self):
        local_log = QgsMessageLog()
        app_log = QgsApplication.messageLog()

        # signals should only be emitted by application log, not local log
        local_spy = QSignalSpy(local_log.messageReceived)
        local_spy_received = QSignalSpy(local_log.messageReceived[bool])
        app_spy = QSignalSpy(app_log.messageReceived)
        app_spy_received = QSignalSpy(app_log.messageReceived[bool])

        local_log.logMessage('test', 'tag', Qgis.Info, notifyUser=True)
        self.assertEqual(len(local_spy), 0)
        self.assertEqual(len(local_spy_received), 0)
        self.assertEqual(len(app_spy), 1)
        self.assertEqual(app_spy[-1], ['test', 'tag', Qgis.Info])
        # info message, so messageReceived(bool) should not be emitted
        self.assertEqual(len(app_spy_received), 0)

        local_log.logMessage('test', 'tag', Qgis.Warning, notifyUser=True)
        self.assertEqual(len(local_spy), 0)
        self.assertEqual(len(local_spy_received), 0)
        self.assertEqual(len(app_spy), 2)
        self.assertEqual(app_spy[-1], ['test', 'tag', Qgis.Warning])
        # warning message, so messageReceived(bool) should be emitted
        self.assertEqual(len(app_spy_received), 1)

        local_log.logMessage('test', 'tag', Qgis.Warning, notifyUser=False)
        self.assertEqual(len(local_spy), 0)
        self.assertEqual(len(local_spy_received), 0)
        self.assertEqual(len(app_spy), 3)
        # notifyUser was False
        self.assertEqual(len(app_spy_received), 1)

    def testBlocker(self):
        local_log = QgsMessageLog()
        app_log = QgsApplication.messageLog()

        spy = QSignalSpy(app_log.messageReceived)
        spy_received = QSignalSpy(app_log.messageReceived[bool])

        local_log.logMessage('test', 'tag', Qgis.Warning, notifyUser=True)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1], ['test', 'tag', Qgis.Warning])
        self.assertEqual(len(spy_received), 1)

        # block notifications
        b = QgsMessageLogNotifyBlocker()
        local_log.logMessage('test', 'tag', Qgis.Warning, notifyUser=True)
        self.assertEqual(len(spy), 2) # should not be blocked
        self.assertEqual(len(spy_received), 1) # should be blocked

        # another blocker
        b2 = QgsMessageLogNotifyBlocker()
        local_log.logMessage('test', 'tag', Qgis.Warning, notifyUser=True)
        self.assertEqual(len(spy), 3) # should not be blocked
        self.assertEqual(len(spy_received), 1) # should be blocked

        del b
        # still blocked because of b2
        local_log.logMessage('test', 'tag', Qgis.Warning, notifyUser=True)
        self.assertEqual(len(spy), 4) # should not be blocked
        self.assertEqual(len(spy_received), 1) # should be blocked

        del b2
        # not blocked
        local_log.logMessage('test', 'tag', Qgis.Warning, notifyUser=True)
        self.assertEqual(len(spy), 5) # should not be blocked
        self.assertEqual(len(spy_received), 2) # should not be blocked


if __name__ == '__main__':
    unittest.main()
