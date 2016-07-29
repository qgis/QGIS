# -*- coding: utf-8 -*-
"""QGIS Unit tests for qgis_local_server.py Python test module

From build dir: ctest -R PyQgsLocalServer -V
Set the following env variables when manually running tests:
  QGIS_TEST_SUITE to run specific tests (define in __main__)
  QGIS_TEST_VERBOSE to output individual test summary
  QGIS_TEST_REPORT to open any failed image check reports in web browser

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from __future__ import print_function
from future import standard_library
standard_library.install_aliases()
__author__ = 'Larry Shaffer'
__date__ = '2014/02/16'
__copyright__ = 'Copyright 2014, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import sys
import datetime
import tempfile

if os.name == 'nt':
    print("TestQgisLocalServer currently doesn't support windows")
    sys.exit(0)

from qgis.core import (
    QgsRectangle,
    QgsCoordinateReferenceSystem,
    QgsRenderChecker
)

from qgis_local_server import getLocalServer

from qgis.testing import (
    start_app,
    unittest
)

from utilities import openInBrowserTab

start_app()
MAPSERV = getLocalServer()

QGIS_TEST_REPORT = 'QGIS_TEST_REPORT' in os.environ
TESTREPORTS = {}


class TestQgisLocalServer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # setup server controller class

        # verify controller can re-initialize processes and temp directory setup
        MAPSERV.startup()  # should recreate tempdir
        msg = 'Server processes could not be restarted'
        assert MAPSERV.processes_running(), msg

        msg = 'Temp web directory could not be recreated'
        assert os.path.exists(MAPSERV.temp_dir()), msg

        # install test project components to temporary web directory
        test_proj_dir = os.path.join(MAPSERV.config_dir(), 'test-project')
        MAPSERV.web_dir_install(os.listdir(test_proj_dir), test_proj_dir)
        msg = 'Test project could not be re-copied to temp web directory'
        res = os.path.exists(os.path.join(MAPSERV.web_dir(),
                                          'test-server.qgs'))
        assert res, msg

        # web server should be left running throughout fcgi tests

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        MAPSERV.shutdown()

    def setUp(self):
        """Run before each test."""
        # web server stays up across all tests
        MAPSERV.fcgi_server_process().start()

    def tearDown(self):
        """Run after each test."""
        # web server stays up across all tests
        MAPSERV.fcgi_server_process().stop()

    # @unittest.skip('')
    def test_convert_param_instances(self):
        params = dict()
        params['LAYERS'] = ['background', 'aoi']
        params['BBOX'] = QgsRectangle(606510, 4823130, 612510, 4827130)
        # creating crs needs QGISAPP instance to access resources/srs.db
        # WGS 84 / UTM zone 13N
        params['CRS'] = QgsCoordinateReferenceSystem(
            32613, QgsCoordinateReferenceSystem.EpsgCrsId)
        params_p = MAPSERV.process_params(params)
        # print repr(params_p)
        param_lyrs = 'LAYERS=background%2Caoi'
        param_crs = 'CRS=EPSG%3A32613'
        param_bbx = 'BBOX=606510%2C4823130%2C612510%2C4827130'
        msg = '\nParameter instances could not be converted'
        assert (param_lyrs in params_p
                and param_crs in params_p
                and param_bbx in params_p), msg

    # @unittest.skip('')
    def test_getmap(self):
        test_name = 'qgis_local_server'
        success, img_path, url = MAPSERV.get_map(self.getmap_params())
        msg = '\nLocal server get_map failed'
        assert success, msg

        chk = QgsRenderChecker()
        chk.setControlName('expected_' + test_name)
        # chk.setMapRenderer(None)
        res = chk.compareImages(test_name, 0, img_path)
        if QGIS_TEST_REPORT and not res:  # don't report ok checks
            TESTREPORTS[test_name] = chk.report()
        msg = '\nRender check failed for "{0}"'.format(test_name)
        assert res, msg

    def getmap_params(self):
        return {
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetMap',
            # 'MAP': abs path, also looks in localserver.web_dir()
            'MAP': 'test-server.qgs',
            # layer stacking order for rendering: bottom, to, top
            'LAYERS': ['background', 'aoi'],  # or 'background,aoi'
            'STYLES': ',',
            'CRS': 'EPSG:32613',  # or QgsCoordinateReferenceSystem obj
            'BBOX': '606510,4823130,612510,4827130',  # or QgsRectangle obj
            'FORMAT': 'image/png',  # or: 'image/png; mode=8bit'
            'WIDTH': '600',
            'HEIGHT': '400',
            'DPI': '72',
            'MAP_RESOLUTION': '72',
            'FORMAT_OPTIONS': 'dpi:72',
            'TRANSPARENT': 'FALSE',
            'IgnoreGetMapUrl': '1'
        }


def run_suite(module, tests):
    """This allows for a list of test names to be selectively run.
    Also, ensures unittest verbose output comes at end, after debug output"""
    loader = unittest.defaultTestLoader
    if 'QGIS_TEST_SUITE' in os.environ and tests:
        suite = loader.loadTestsFromNames(tests, module)
    else:
        suite = loader.loadTestsFromModule(module)
    verb = 2 if 'QGIS_TEST_VERBOSE' in os.environ else 0

    res = unittest.TextTestRunner(verbosity=verb).run(suite)

    if QGIS_TEST_REPORT and len(TESTREPORTS) > 0:
        teststamp = 'Local Server Test Report: ' + \
                    datetime.datetime.now().strftime('%Y-%m-%d %X')
        report = '<html><head><title>{0}</title></head><body>'.format(teststamp)
        report += '\n<h2>Failed Image Tests: {0}</h2>'.format(len(TESTREPORTS))
        for k, v in TESTREPORTS.items():
            report += '\n<h3>{0}</h3>\n{1}'.format(k, v)
        report += '</body></html>'

        tmp = tempfile.NamedTemporaryFile(suffix=".html", delete=False)
        tmp.write(report)
        tmp.close()
        openInBrowserTab('file://' + tmp.name)

    return res


if __name__ == '__main__':
    # NOTE: unless QGIS_TEST_SUITE env var is set all tests will be run
    test_suite = [
        'TestQgisLocalServer.test_getmap'
    ]
    test_res = run_suite(sys.modules[__name__], test_suite)
    sys.exit(not test_res.wasSuccessful())
