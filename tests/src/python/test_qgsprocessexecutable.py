# -*- coding: utf-8 -*-
"""QGIS Unit tests for qgis_process.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '05/04/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import sys
import os
import glob
import re
import time
import shutil
import subprocess
import tempfile
import errno

from qgis.testing import unittest
from utilities import unitTestDataPath

print('CTEST_FULL_OUTPUT')

TEST_DATA_DIR = unitTestDataPath()


class TestQgsProcessExecutable(unittest.TestCase):

    TMP_DIR = ''

    @classmethod
    def setUpClass(cls):
        cls.TMP_DIR = tempfile.mkdtemp()
        # print('TMP_DIR: ' + cls.TMP_DIR)
        # subprocess.call(['open', cls.TMP_DIR])

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.TMP_DIR, ignore_errors=True)

    def run_process(self, arguments):
        call = [QGIS_PROCESS_BIN] + arguments
        print(' '.join(call))

        myenv = os.environ.copy()
        myenv["QGIS_DEBUG"] = '0'

        p = subprocess.Popen(call, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=myenv)
        output, err = p.communicate()
        rc = p.returncode

        return rc, output.decode(), err.decode()

    def testNoArgs(self):
        rc, output, err = self.run_process([])
        self.assertEqual(rc, 0)
        self.assertIn('Available commands', output)
        self.assertFalse(err)

    def testPlugins(self):
        rc, output, err = self.run_process(['plugins'])
        self.assertEqual(rc, 0)
        self.assertIn('available plugins', output.lower())
        self.assertIn('processing', output.lower())
        self.assertNotIn('metasearch', output.lower())
        self.assertFalse(err)

    def testAlgorithmList(self):
        rc, output, err = self.run_process(['list'])
        self.assertEqual(rc, 0)
        self.assertIn('available algorithms', output.lower())
        self.assertIn('gdal:aspect', output.lower())
        self.assertIn('native:reprojectlayer', output.lower())
        self.assertFalse(err)

    def testAlgorithmHelpNoAlg(self):
        rc, output, err = self.run_process(['help'])
        self.assertEqual(rc, 1)
        self.assertIn('algorithm id not specified', err.lower())
        self.assertFalse(output)

    def testAlgorithmHelp(self):
        rc, output, err = self.run_process(['help', 'native:centroids'])
        self.assertEqual(rc, 0)
        self.assertIn('representing the centroid', output.lower())
        self.assertIn('argument type', output.lower())
        self.assertFalse(err)

    def testAlgorithmRunNoAlg(self):
        rc, output, err = self.run_process(['run'])
        self.assertEqual(rc, 1)
        self.assertIn('algorithm id not specified', err.lower())
        self.assertFalse(output)

    def testAlgorithmRunNoArgs(self):
        rc, output, err = self.run_process(['run', 'native:centroids'])
        self.assertEqual(rc, 1)
        self.assertIn('the following mandatory parameters were not specified', err.lower())
        self.assertIn('inputs', output.lower())

    def testAlgorithmRun(self):
        output_file = self.TMP_DIR + '/polygon_centroid.shp'
        rc, output, err = self.run_process(['run', 'native:centroids', '--INPUT={}'.format(TEST_DATA_DIR + '/polys.shp'), '--ALL_PARTS=false', '--OUTPUT={}'.format(output_file)])
        self.assertEqual(rc, 0)
        self.assertFalse(err)
        self.assertIn('0...10...20...30...40...50...60...70...80...90', output.lower())
        self.assertIn('results', output.lower())
        self.assertIn('OUTPUT:\t' + output_file, output)
        self.assertTrue(os.path.exists(output_file))


if __name__ == '__main__':
    # look for qgis bin path
    QGIS_PROCESS_BIN = ''
    prefixPath = os.environ['QGIS_PREFIX_PATH']
    # see qgsapplication.cpp:98
    for f in ['', '..', 'bin']:
        d = os.path.join(prefixPath, f)
        b = os.path.abspath(os.path.join(d, 'qgis_process'))
        if os.path.exists(b):
            QGIS_PROCESS_BIN = b
            break
        b = os.path.abspath(os.path.join(d, 'qgis_process.exe'))
        if os.path.exists(b):
            QGIS_PROCESS_BIN = b
            break
        if sys.platform[:3] == 'dar':  # Mac
            # QGIS.app may be QGIS_x.x-dev.app for nightlies
            # internal binary will match, minus the '.app'
            found = False
            for app_path in glob.glob(d + '/QGIS*.app'):
                m = re.search('/(QGIS(_\d\.\d-dev)?)\.app', app_path)
                if m:
                    QGIS_PROCESS_BIN = app_path + '/Contents/MacOS/' + m.group(1)
                    found = True
                    break
            if found:
                break

    print(('\nQGIS_PROCESS_BIN: {}'.format(QGIS_PROCESS_BIN)))
    assert QGIS_PROCESS_BIN, 'qgis_process binary not found, skipping test suite'
    unittest.main()
