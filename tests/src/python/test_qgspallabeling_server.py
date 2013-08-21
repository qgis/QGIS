# -*- coding: utf-8 -*-
"""QGIS unit tests for QgsPalLabeling: label rendering via QGIS Server

From build dir: ctest -R PyQgsPalLabelingServer -V
Set the following env variables when manually running tests:
  PAL_SUITE to run specific tests (define in __main__)
  PAL_VERBOSE to output individual test summary
  PAL_CONTROL_IMAGE to trigger building of new control images
  PAL_REPORT to open any failed image check reports in web browser

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Larry Shaffer'
__date__ = '07/12/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sys
import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from utilities import (
    unittest,
    expectedFailure,
)

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite
from test_qgspallabeling_tests import TestPointBase
from qgis_local_server import (QgisLocalServerConfig,
                               ServerConfigNotAccessibleError)

SERVER = None
TESTPROJDIR = ''
TESTPROJPATH = ''
try:
    SERVER = \
        QgisLocalServerConfig(str(QgsApplication.qgisSettingsDirPath()), True)
    TESTPROJDIR = SERVER.projectDir()
    TESTPROJPATH = os.path.join(TESTPROJDIR, 'pal_test.qgs')
except ServerConfigNotAccessibleError, e:
    print e


def skipUnlessHasServer():  # skip test class decorator
    if SERVER is not None:
        return lambda func: func
    return unittest.skip('\nConfigured local QGIS Server is not accessible\n\n')


@skipUnlessHasServer()
class TestServerPoint(TestQgsPalLabeling, TestPointBase):

    @classmethod
    def setUpClass(cls):
        TestQgsPalLabeling.setUpClass()
        cls.setUpServerProjectAndDir(TESTPROJPATH, TESTPROJDIR)
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('background')
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')
        cls.checkmismatch = 1000
        cls.checkgroup = ''

    def setUp(self):
        """Run before each test."""
        self.configTest('pal_server', 'sp')
        self.lyr = self.defaultSettings()
        self.params = self.defaultWmsParams(TESTPROJPATH, 'point')
        self._TestImage = ''

    def tearDown(self):
        """Run after each test."""
        pass

    def checkTest(self, **kwargs):
        self.lyr.writeToLayer(self.layer)
        # save project file
        self._TestProj.write()
        # get server results
        res, self._TestImage = SERVER.getMap(self.params, False)
        self.saveContolImage(self._TestImage)
        self.assertTrue(res, 'Failed to retrieve/save image from test server')
        # gp = kwargs['grpprefix'] if 'grpprefix' in kwargs else ''
        self.assertTrue(*self.renderCheck(mismatch=self.checkmismatch,
                                          imgpath=self._TestImage,
                                          grpprefix=self.checkgroup))


@skipUnlessHasServer()
class TestServerVsCanvasPoint(TestServerPoint):

    @classmethod
    def setUpClass(cls):
        TestServerPoint.setUpClass()
        cls.checkgroup = 'pal_canvas'


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # ex: 'TestGroup(Point|Line|Curved|Polygon|Feature).test_method'
    suite = [
        'TestServerVsCanvasPoint.test_text_size_map_unit'
    ]
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
