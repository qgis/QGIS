# -*- coding: utf-8 -*-
"""QGIS unit tests for QgsPalLabeling: label rendering output via QGIS Server

From build dir, run: ctest -R PyQgsPalLabelingServer -V

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

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

import qgis
import sys
import os
import glob
import shutil

from PyQt4.QtCore import QSettings, qDebug

from qgis.core import QgsProject, QgsApplication, QgsPalLabeling

from utilities import mapSettingsString

from qgis_local_server import getLocalServer

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite
from test_qgspallabeling_tests import (
    TestPointBase,
    TestLineBase,
    suiteTests
)

MAPSERV = getLocalServer()


class TestServerBase(TestQgsPalLabeling):

    _TestProj = None
    """:type: QgsProject"""
    _TestProjName = ''
    layer = None
    """:type: QgsVectorLayer"""
    params = dict()

    @classmethod
    def setUpClass(cls):
        if not cls._BaseSetup:
            TestQgsPalLabeling.setUpClass()
        MAPSERV.startup()
        MAPSERV.web_dir_install(glob.glob(cls._PalDataDir + os.sep + '*.qml'))
        MAPSERV.web_dir_install(glob.glob(cls._PalDataDir + os.sep + '*.qgs'))

        # noinspection PyArgumentList
        cls._TestProj = QgsProject.instance()
        cls._TestProjName = 'test-labeling.qgs'
        cls._TestProj.setFileName(
            os.path.join(MAPSERV.web_dir(), cls._TestProjName))

        # the blue background (set via layer style) to match renderchecker's
        TestQgsPalLabeling.loadFeatureLayer('background', True)

        settings = QSettings()
        # noinspection PyArgumentList
        cls._CacheDir = settings.value(
            "cache/directory",
            os.path.join(unicode(QgsApplication.qgisSettingsDirPath()),
                         "cache"),
            type=unicode)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        TestQgsPalLabeling.tearDownClass()
        cls.removeMapLayer(cls.layer)
        cls.layer = None
        # layers removed, save empty project file
        cls._TestProj.write()
        if "PAL_SERVER_TEMP" in os.environ:
            MAPSERV.stop_processes()
            MAPSERV.open_temp_dir()
        else:
            MAPSERV.shutdown()

    def setUp(self):
        """Run before each test."""
        # web server stays up across all tests
        # MAPSERV.fcgi_server_process().stop()
        # self.deleteCache()
        super(TestServerBase, self).setUp()
        self._TestImage = ''
        # ensure per test map settings stay encapsulated
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self._Mismatch = 0
        self._ColorTol = 0
        self._Mismatches.clear()
        self._ColorTols.clear()

    # noinspection PyPep8Naming
    def delete_cache(self):
        for item in os.listdir(self._CacheDir):
            shutil.rmtree(os.path.join(self._CacheDir, item),
                          ignore_errors=True)

    # noinspection PyPep8Naming
    def get_request_params(self):
        # TODO: support other types of servers, besides WMS
        ms = self._TestMapSettings
        osize = ms.outputSize()
        dpi = str(ms.outputDpi())
        lyrs = [str(self._MapRegistry.mapLayer(i).name()) for i in ms.layers()]
        lyrs.reverse()
        params = {
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetMap',
            'MAP': self._TestProjName,
            # layer stacking order for rendering: bottom,to,top
            'LAYERS': lyrs,  # or 'name,name'
            'STYLES': ',',
            # authid str or QgsCoordinateReferenceSystem obj
            'CRS': str(ms.destinationCrs().authid()),
            # self.aoiExtent(),
            'BBOX': str(ms.extent().toString(True).replace(' : ', ',')),
            'FORMAT': 'image/png',  # or: 'image/png; mode=8bit'
            'WIDTH': str(osize.width()),
            'HEIGHT': str(osize.height()),
            'DPI': dpi,
            'MAP_RESOLUTION': dpi,
            'FORMAT_OPTIONS': 'dpi:{0}'.format(dpi),
            'TRANSPARENT': 'FALSE',
            'IgnoreGetMapUrl': '1'
        }
        # print params
        return params

    def sync_map_settings(self):
        """
        Sync custom test QgsMapSettings to Project file
        """
        pal = QgsPalLabeling()
        pal.loadEngineSettings()
        pal.init(self._TestMapSettings)
        pal.saveEngineSettings()

    def checkTest(self, **kwargs):
        self.sync_map_settings()
        self.lyr.writeToLayer(self.layer)
        # save project file
        self._TestProj.write()
        # always restart FCGI before tests, so settings can be applied
        # MAPSERV.fcgi_server_process().start()
        # get server results
        # print self.params.__repr__()

        ms = self._MapSettings  # class settings
        settings_type = 'Class'
        if self._TestMapSettings is not None:
            ms = self._TestMapSettings  # per test settings
            settings_type = 'Test'
        if 'PAL_VERBOSE' in os.environ:
            qDebug('MapSettings type: {0}'.format(settings_type))
            qDebug(mapSettingsString(ms))

        res_m, self._TestImage, url = MAPSERV.get_map(self.get_request_params(), False)
        # print self._TestImage.__repr__()
        if 'PAL_VERBOSE' in os.environ:
            qDebug('GetMap request:\n  {0}\n'.format(url))
        self.saveControlImage(self._TestImage)
        self.assertTrue(res_m, 'Failed to retrieve/save image from test server')
        mismatch = 0
        if 'PAL_NO_MISMATCH' not in os.environ:
            # some mismatch expected
            mismatch = self._Mismatch if self._Mismatch else 20
            if self._TestGroup in self._Mismatches:
                mismatch = self._Mismatches[self._TestGroup]
        colortol = 0
        if 'PAL_NO_COLORTOL' not in os.environ:
            # some mismatch expected
            # colortol = self._ColorTol if self._ColorTol else 10
            if self._TestGroup in self._ColorTols:
                colortol = self._ColorTols[self._TestGroup]
        self.assertTrue(*self.renderCheck(mismatch=mismatch,
                                          colortol=colortol,
                                          imgpath=self._TestImage))


class TestServerBasePoint(TestServerBase):

    @classmethod
    def setUpClass(cls):
        TestServerBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')


class TestServerPoint(TestServerBasePoint, TestPointBase):

    def setUp(self):
        super(TestServerPoint, self).setUp()
        self.configTest('pal_server', 'sp')


class TestServerVsCanvasPoint(TestServerBasePoint, TestPointBase):

    def setUp(self):
        super(TestServerVsCanvasPoint, self).setUp()
        self.configTest('pal_canvas', 'sp')


class TestServerBaseLine(TestServerBase):

    @classmethod
    def setUpClass(cls):
        TestServerBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('line')


class TestServerLine(TestServerBaseLine, TestLineBase):

    def setUp(self):
        """Run before each test."""
        super(TestServerLine, self).setUp()
        self.configTest('pal_server_line', 'sp')


class TestServerVsCanvasLine(TestServerBaseLine, TestLineBase):

    def setUp(self):
        super(TestServerVsCanvasLine, self).setUp()
        self.configTest('pal_canvas_line', 'sp')

if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # SEE: test_qgspallabeling_tests.suiteTests() to define suite
    suite = (
        ['TestServerPoint.' + t for t in suiteTests()['sp_suite']] +
        ['TestServerVsCanvasPoint.' + t for t in suiteTests()['sp_vs_suite']]
    )
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
