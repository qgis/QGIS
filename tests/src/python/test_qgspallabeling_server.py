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
import glob
import shutil
import tempfile
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
                               ServerConfigNotAccessibleError,
                               ServerSpawnNotAccessibleError)

SERVER = None
SPAWN = False
TESTPROJDIR = ''
TESTPROJPATH = ''

# TODO [LS]: attempt to spawn local server; add function in qgis_local_server.py

try:
    # first try to connect to locally spawned server
    # http://127.0.0.1:8448/qgis_mapserv.fcgi (see qgis_local_server.py)
    SERVER = QgisLocalServerConfig(
        tempfile.mkdtemp(),
        chkcapa=True, spawn=True)
    SPAWN = True
    print '\n------------ Using SPAWNED local test server ------------\n'
except ServerSpawnNotAccessibleError, e:
    SERVER = None
    # print e
    # TODO [LS]: add some error output if server is spawned, but not working
    pass  # may have no local spawn fcgi setup

if SERVER is None:
    try:
        # next try to connect to configured local server
        SERVER = QgisLocalServerConfig(
            str(QgsApplication.qgisSettingsDirPath()),
            chkcapa=True, spawn=False)
    except ServerConfigNotAccessibleError, e:
        SERVER = None
        print e

if SERVER is not None:
    TESTPROJDIR = SERVER.projectDir()
    TESTPROJPATH = os.path.join(TESTPROJDIR, 'pal_test.qgs')


def skipUnlessHasServer():  # skip test class decorator
    if SERVER is not None:
        return lambda func: func
    return unittest.skip('\nConfigured local QGIS Server is not accessible\n\n')


class TestServerBase(TestQgsPalLabeling):

    _TestProj = None
    """:type: QgsProject"""
    _TestProjSetup = False

    @classmethod
    def setUpClass(cls):
        TestQgsPalLabeling.setUpClass()

        cls._TestProj = QgsProject.instance()
        cls._TestProj.setFileName(str(TESTPROJPATH).strip())
        if not cls._TestProjSetup:
            try:
                shutil.copy(cls._PalFeaturesDb, TESTPROJDIR)
                for qml in glob.glob(cls._PalDataDir + os.sep + '*.qml'):
                    shutil.copy(qml, TESTPROJDIR)
            except IOError, e:
                raise IOError(str(e) +
                              '\nCould not set up test server directory')
            cls._TestProjSetup = True

        # the blue background (set via layer style) to match renderchecker's
        cls._BkgrdLayer = TestQgsPalLabeling.loadFeatureLayer('background')
        cls._CheckMismatch = 200  # default for server tests; mismatch expected
        cls._CheckGroup = ''  # default '' will check against server control

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        TestQgsPalLabeling.tearDownClass()
        # layers removed, save empty project file
        cls._TestProj.write()

    def defaultWmsParams(self, layername):
        return {
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetMap',
            'MAP': str(TESTPROJPATH).strip(),
            # layer stacking order for rendering: bottom,to,top
            'LAYERS': ['background', str(layername).strip()],  # or 'name,name'
            'STYLES': ',',
            # authid str or QgsCoordinateReferenceSystem obj
            'CRS': 'EPSG:32613',  # self._CRS
            'BBOX': '606510,4823130,612510,4827130',  # self.aoiExtent(),
            'FORMAT': 'image/png',  # or: 'image/png; mode=8bit'
            'WIDTH': '600',
            'HEIGHT': '400',
            'DPI': '72',
            'MAP_RESOLUTION': '72',
            'FORMAT_OPTIONS': 'dpi:72',
            'TRANSPARENT': 'FALSE',
            'IgnoreGetMapUrl': '1'
        }


@skipUnlessHasServer()
class TestServerPoint(TestServerBase, TestPointBase):

    @classmethod
    def setUpClass(cls):
        TestServerBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')

    def setUp(self):
        """Run before each test."""
        self.configTest('pal_server', 'sp')
        self.lyr = self.defaultSettings()
        self.params = self.defaultWmsParams('point')
        self._TestImage = ''

    def tearDown(self):
        """Run after each test."""
        pass

    def checkTest(self, **kwargs):
        self.lyr.writeToLayer(self.layer)
        # save project file
        self._TestProj.write()
        # get server results
        # print self.params.__repr__()
        res, self._TestImage = SERVER.getMap(self.params, False)
        # print self._TestImage.__repr__()
        self.saveContolImage(self._TestImage)
        self.assertTrue(res, 'Failed to retrieve/save image from test server')
        # gp = kwargs['grpprefix'] if 'grpprefix' in kwargs else ''
        self.assertTrue(*self.renderCheck(mismatch=self._CheckMismatch,
                                          imgpath=self._TestImage,
                                          grpprefix=self._CheckGroup))


@skipUnlessHasServer()
class TestServerVsCanvasPoint(TestServerPoint):

    @classmethod
    def setUpClass(cls):
        TestServerPoint.setUpClass()
        cls._CheckGroup = 'pal_canvas'


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # ex: 'TestGroup(Point|Line|Curved|Polygon|Feature).test_method'
    suite = [
        'TestServerVsCanvasPoint.test_text_size_map_unit'
    ]
    res = runSuite(sys.modules[__name__], suite)
    # if SPAWN:
    #     os.remove(TESTPROJDIR)  # remove temp directory (why does this error?)
    sys.exit(not res.wasSuccessful())
