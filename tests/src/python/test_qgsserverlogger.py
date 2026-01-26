"""QGIS Unit tests for QgsServerLogger.

From build dir, run: ctest -R PyQgsServerLogger -V

"""

__author__ = "Eric Lemoine"
__date__ = "11/09/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

import os

from qgis.server import QgsServerLogger
from qgis.testing import unittest

from utilities import unitTestDataPath


class TestQgsServerLogger(unittest.TestCase):

    log_file = os.path.join(unitTestDataPath("qgis_server"), "qgis_server_test.log")

    @staticmethod
    def remove_file(filename):
        try:
            os.remove(filename)
        except FileNotFoundError:
            pass

    def setUp(self):
        self.logger = QgsServerLogger.instance()
        self.logger.setLogLevel(0)
        self.logger.setLogFile(self.log_file)
        exists = os.access(self.log_file, os.R_OK)
        self.assertTrue(exists)
        self.remove_file(self.log_file)

    def tearDown(self):
        self.remove_file(self.log_file)

    def test_logging_no_log_file(self):
        self.logger.setLogFile("")
        exists = os.access(self.log_file, os.R_OK)
        self.assertFalse(exists)

    def test_logging_log_file(self):
        self.logger.setLogFile(self.log_file)
        exists = os.access(self.log_file, os.R_OK)
        self.assertTrue(exists)

    def test_logging_log_file_stderr(self):
        self.logger.setLogFile("stderr")
        exists = os.access(self.log_file, os.R_OK)
        self.assertFalse(exists)

    def test_logging_stderr(self):
        self.logger.setLogStderr()
        exists = os.access(self.log_file, os.R_OK)
        self.assertFalse(exists)
