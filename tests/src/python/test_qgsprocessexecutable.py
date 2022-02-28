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
import json
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

    def run_process_stdin(self, arguments, stdin_string: str):
        call = [QGIS_PROCESS_BIN] + arguments
        print(' '.join(call))

        myenv = os.environ.copy()
        myenv["QGIS_DEBUG"] = '0'

        p = subprocess.Popen(call, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, env=myenv)
        output, err = p.communicate(input=stdin_string.encode())
        rc = p.returncode

        return rc, output.decode(), err.decode()

    def testNoArgs(self):
        rc, output, err = self.run_process([])
        self.assertIn('Available commands', output)
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)

    def testPlugins(self):
        rc, output, err = self.run_process(['plugins'])
        self.assertIn('available plugins', output.lower())
        self.assertIn('processing', output.lower())
        self.assertNotIn('metasearch', output.lower())
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)

    def testPluginsJson(self):
        rc, output, err = self.run_process(['plugins', '--json'])
        res = json.loads(output)
        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)
        self.assertIn('plugins', res)
        self.assertIn('processing', res['plugins'])
        self.assertTrue(res['plugins']['processing']['loaded'])
        self.assertEqual(rc, 0)

    def testAlgorithmList(self):
        rc, output, err = self.run_process(['list'])
        self.assertIn('available algorithms', output.lower())
        self.assertIn('native:reprojectlayer', output.lower())
        self.assertIn('gdal:translate', output.lower())
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)

    def testAlgorithmListNoPython(self):
        rc, output, err = self.run_process(['--no-python', 'list'])
        self.assertIn('available algorithms', output.lower())
        self.assertIn('native:reprojectlayer', output.lower())
        self.assertNotIn('gdal:translate', output.lower())
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)

    def testAlgorithmsListJson(self):
        rc, output, err = self.run_process(['list', '--no-python', '--json'])
        res = json.loads(output)
        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)

        self.assertIn('providers', res)
        self.assertIn('native', res['providers'])
        self.assertTrue(res['providers']['native']['is_active'])
        self.assertIn('native:buffer', res['providers']['native']['algorithms'])
        self.assertFalse(res['providers']['native']['algorithms']['native:buffer']['deprecated'])

        self.assertEqual(rc, 0)

    def testAlgorithmHelpNoAlg(self):
        rc, output, err = self.run_process(['help', '--no-python'])
        self.assertEqual(rc, 1)
        self.assertIn('algorithm id or model file not specified', err.lower())
        self.assertFalse(output)

    def testAlgorithmHelp(self):
        rc, output, err = self.run_process(['help', '--no-python', 'native:centroids'])
        self.assertIn('representing the centroid', output.lower())
        self.assertIn('argument type', output.lower())
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)

    def testAlgorithmHelpJson(self):
        rc, output, err = self.run_process(['help', '--no-python', 'native:buffer', '--json'])
        res = json.loads(output)

        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)

        self.assertFalse(res['algorithm_details']['deprecated'])
        self.assertTrue(res['provider_details']['is_active'])

        self.assertIn('OUTPUT', res['outputs'])
        self.assertEqual(res['outputs']['OUTPUT']['description'], 'Buffered')
        self.assertEqual(res['parameters']['DISSOLVE']['description'], 'Dissolve result')
        self.assertFalse(res['parameters']['DISTANCE']['is_advanced'])

        self.assertEqual(rc, 0)

    def testAlgorithmRunNoAlg(self):
        rc, output, err = self.run_process(['run', '--no-python'])
        self.assertIn('algorithm id or model file not specified', err.lower())
        self.assertFalse(output)
        self.assertEqual(rc, 1)

    def testAlgorithmRunNoArgs(self):
        rc, output, err = self.run_process(['run', '--no-python', 'native:centroids'])
        self.assertIn('the following mandatory parameters were not specified', err.lower())
        self.assertIn('inputs', output.lower())
        self.assertEqual(rc, 1)

    def testAlgorithmRunLegacy(self):
        output_file = self.TMP_DIR + '/polygon_centroid.shp'
        rc, output, err = self.run_process(['run', '--no-python', 'native:centroids', '--INPUT={}'.format(TEST_DATA_DIR + '/polys.shp'), '--OUTPUT={}'.format(output_file)])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertIn('0...10...20...30...40...50...60...70...80...90', output.lower())
        self.assertIn('results', output.lower())
        self.assertIn('OUTPUT:\t' + output_file, output)
        self.assertTrue(os.path.exists(output_file))
        self.assertEqual(rc, 0)

    def testAlgorithmRun(self):
        output_file = self.TMP_DIR + '/polygon_centroid.shp'
        rc, output, err = self.run_process(['run', '--no-python', 'native:centroids', '--', 'INPUT={}'.format(TEST_DATA_DIR + '/polys.shp'), 'OUTPUT={}'.format(output_file)])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertIn('0...10...20...30...40...50...60...70...80...90', output.lower())
        self.assertIn('results', output.lower())
        self.assertIn('OUTPUT:\t' + output_file, output)
        self.assertTrue(os.path.exists(output_file))
        self.assertEqual(rc, 0)

    def testAlgorithmRunStdIn(self):
        output_file = self.TMP_DIR + '/polygon_centroid_json.shp'

        params = {
            'inputs': {
                'INPUT': TEST_DATA_DIR + '/polys.shp',
                'OUTPUT': output_file
            }
        }

        rc, output, err = self.run_process_stdin(['run', '--no-python', 'native:centroids', '-'], json.dumps(params))
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)

        res = json.loads(output)

        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)

        self.assertEqual(res['algorithm_details']['name'], 'Centroids')
        self.assertEqual(res['inputs']['INPUT'], TEST_DATA_DIR + '/polys.shp')
        self.assertEqual(res['inputs']['OUTPUT'], output_file)
        self.assertEqual(res['results']['OUTPUT'], output_file)

        self.assertTrue(os.path.exists(output_file))
        self.assertEqual(rc, 0)

    def testAlgorithmRunStdInExtraSettings(self):
        output_file = self.TMP_DIR + '/polygon_centroid_json.shp'

        params = {
            'inputs': {
                'INPUT': TEST_DATA_DIR + '/polys.shp',
                'OUTPUT': output_file
            },
            'ellipsoid': 'EPSG:7019',
            'distance_units': 'feet',
            'area_units': 'ha',
            'project_path': TEST_DATA_DIR + '/joins.qgs'
        }

        rc, output, err = self.run_process_stdin(['run', '--no-python', 'native:centroids', '-'], json.dumps(params))

        res = json.loads(output)

        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)

        self.assertEqual(res['algorithm_details']['name'], 'Centroids')
        self.assertEqual(res['ellipsoid'], 'EPSG:7019')
        self.assertEqual(res['distance_unit'], 'feet')
        self.assertEqual(res['area_unit'], 'hectares')
        self.assertEqual(res['project_path'], TEST_DATA_DIR + '/joins.qgs')
        self.assertEqual(res['inputs']['INPUT'], TEST_DATA_DIR + '/polys.shp')
        self.assertEqual(res['inputs']['OUTPUT'], output_file)
        self.assertEqual(res['results']['OUTPUT'], output_file)

        self.assertTrue(os.path.exists(output_file))
        self.assertEqual(rc, 0)

    def testAlgorithmRunStdInExtraSettingsBadDistanceUnit(self):
        output_file = self.TMP_DIR + '/polygon_centroid_json.shp'

        params = {
            'inputs': {
                'INPUT': TEST_DATA_DIR + '/polys.shp',
                'OUTPUT': output_file
            },
            'distance_units': 'xxx',
        }

        rc, output, err = self.run_process_stdin(['run', '--no-python', 'native:centroids', '-'], json.dumps(params))
        self.assertEqual(rc, 1)
        self.assertIn('xxx is not a valid distance unit value', err)

    def testAlgorithmRunStdInExtraSettingsBadAreaUnit(self):
        output_file = self.TMP_DIR + '/polygon_centroid_json.shp'

        params = {
            'inputs': {
                'INPUT': TEST_DATA_DIR + '/polys.shp',
                'OUTPUT': output_file
            },
            'area_units': 'xxx',
        }

        rc, output, err = self.run_process_stdin(['run', '--no-python', 'native:centroids', '-'], json.dumps(params))
        self.assertEqual(rc, 1)
        self.assertIn('xxx is not a valid area unit value', err)

    def testAlgorithmRunStdInExtraSettingsBadProjectPath(self):
        output_file = self.TMP_DIR + '/polygon_centroid_json.shp'

        params = {
            'inputs': {
                'INPUT': TEST_DATA_DIR + '/polys.shp',
                'OUTPUT': output_file
            },
            'project_path': 'xxx',
        }

        rc, output, err = self.run_process_stdin(['run', '--no-python', 'native:centroids', '-'], json.dumps(params))
        self.assertEqual(rc, 1)
        self.assertIn('Could not load the QGIS project "xxx"', err)

    def testAlgorithmRunStdInMissingInputKey(self):
        output_file = self.TMP_DIR + '/polygon_centroid_json.shp'

        params = {
            'INPUT': TEST_DATA_DIR + '/polys.shp',
            'OUTPUT': output_file
        }

        rc, output, err = self.run_process_stdin(['run', '--no-python', 'native:centroids', '-'], json.dumps(params))
        self.assertEqual(rc, 1)
        self.assertIn('JSON parameters object must contain an "inputs" key.', err)

    def testAlgorithmRunStdInNoInput(self):
        rc, output, err = self.run_process_stdin(['run', '--no-python', 'native:centroids', '-'], '')
        self.assertEqual(rc, 1)
        self.assertIn('Could not parse JSON parameters', err)

    def testAlgorithmRunStdInBadInput(self):
        rc, output, err = self.run_process_stdin(['run', '--no-python', 'native:centroids', '-'], '{"not valid json"}')
        self.assertEqual(rc, 1)
        self.assertIn('Could not parse JSON parameters', err)

    def testAlgorithmRunJson(self):
        output_file = self.TMP_DIR + '/polygon_centroid2.shp'
        rc, output, err = self.run_process(['run', '--no-python', '--json', 'native:centroids', '--', 'INPUT={}'.format(TEST_DATA_DIR + '/polys.shp'), 'OUTPUT={}'.format(output_file)])
        res = json.loads(output)

        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)

        self.assertEqual(res['algorithm_details']['name'], 'Centroids')
        self.assertEqual(res['inputs']['INPUT'], TEST_DATA_DIR + '/polys.shp')
        self.assertEqual(res['inputs']['OUTPUT'], output_file)
        self.assertEqual(res['results']['OUTPUT'], output_file)

        self.assertTrue(os.path.exists(output_file))
        self.assertEqual(rc, 0)

    def testAlgorithmRunListValue(self):
        """
        Test an algorithm which requires a list of layers as a parameter value
        """
        output_file = self.TMP_DIR + '/package.gpkg'
        rc, output, err = self.run_process(['run', '--no-python', '--json', 'native:package', '--',
                                            'LAYERS={}'.format(TEST_DATA_DIR + '/polys.shp'),
                                            'LAYERS={}'.format(TEST_DATA_DIR + '/points.shp'),
                                            'LAYERS={}'.format(TEST_DATA_DIR + '/lines.shp'),
                                            'OUTPUT={}'.format(output_file)])
        res = json.loads(output)

        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)

        self.assertEqual(res['algorithm_details']['name'], 'Package layers')
        self.assertEqual(len(res['inputs']['LAYERS']), 3)
        self.assertEqual(res['inputs']['OUTPUT'], output_file)
        self.assertEqual(res['results']['OUTPUT'], output_file)
        self.assertEqual(len(res['results']['OUTPUT_LAYERS']), 3)

        self.assertTrue(os.path.exists(output_file))
        self.assertEqual(rc, 0)

    def testModelHelp(self):
        rc, output, err = self.run_process(['help', '--no-python', TEST_DATA_DIR + '/test_model.model3'])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)
        self.assertIn('model description', output.lower())
        self.assertIn('author of model', output.lower())
        self.assertIn('version 2.1', output.lower())
        self.assertIn('examples', output.lower())
        self.assertIn('this is an example of running the model', output.lower())

    def testModelRun(self):
        output_file = self.TMP_DIR + '/model_output.shp'
        rc, output, err = self.run_process(['run', '--no-python', TEST_DATA_DIR + '/test_model.model3', '--', 'FEATS={}'.format(TEST_DATA_DIR + '/polys.shp'), 'native:centroids_1:CENTROIDS={}'.format(output_file)])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)
        self.assertIn('0...10...20...30...40...50...60...70...80...90', output.lower())
        self.assertIn('results', output.lower())
        self.assertTrue(os.path.exists(output_file))

    def testModelRunStdIn(self):
        output_file = self.TMP_DIR + '/model_output_stdin.shp'

        params = {
            'inputs': {
                'FEATS': TEST_DATA_DIR + '/polys.shp',
                'native:centroids_1:CENTROIDS': output_file
            }
        }

        rc, output, err = self.run_process_stdin(['run', '--no-python', TEST_DATA_DIR + '/test_model.model3', '-'], json.dumps(params))
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)
        self.assertEqual(res['algorithm_details']['id'], 'Test model')
        self.assertTrue(os.path.exists(output_file))

    def testModelRunJson(self):
        output_file = self.TMP_DIR + '/model_output2.shp'
        rc, output, err = self.run_process(['run', TEST_DATA_DIR + '/test_model.model3', '--no-python', '--json', '--', 'FEATS={}'.format(TEST_DATA_DIR + '/polys.shp'), 'native:centroids_1:CENTROIDS={}'.format(output_file)])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)
        self.assertEqual(res['algorithm_details']['id'], 'Test model')
        self.assertTrue(os.path.exists(output_file))

    def testModelRunWithLog(self):
        output_file = self.TMP_DIR + '/model_log.log'
        rc, output, err = self.run_process(['run', '--no-python', TEST_DATA_DIR + '/test_logging_model.model3', '--', 'logfile={}'.format(output_file)])
        self.assertIn('Test logged message', err)
        self.assertEqual(rc, 0)
        self.assertIn('0...10...20...30...40...50...60...70...80...90', output.lower())
        self.assertIn('results', output.lower())
        self.assertTrue(os.path.exists(output_file))

        with open(output_file, 'rt') as f:
            lines = '\n'.join(f.readlines())

        self.assertIn('Test logged message', lines)

    def testPythonScriptHelp(self):
        rc, output, err = self.run_process(['help', TEST_DATA_DIR + '/convert_to_upper.py'])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)
        self.assertIn('converts a string to upper case', output.lower())

    def testPythonScriptRun(self):
        rc, output, err = self.run_process(['run', TEST_DATA_DIR + '/convert_to_upper.py', '--', 'INPUT=abc'])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)
        self.assertIn('Converted abc to ABC', output)
        self.assertIn('OUTPUT:\tABC', output)

    def testPythonScriptRunJson(self):
        rc, output, err = self.run_process(['run', TEST_DATA_DIR + '/convert_to_upper.py', '--json', '--', 'INPUT=abc'])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)
        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)
        self.assertEqual(res['algorithm_details']['id'], 'script:converttouppercase')
        self.assertEqual(res['results']['OUTPUT'], 'ABC')

    def testScriptRunStdIn(self):
        output_file = self.TMP_DIR + '/model_output_stdin.shp'

        params = {
            'inputs':
                {
                    'INPUT': 'abc def'
                }
        }

        rc, output, err = self.run_process_stdin(['run', TEST_DATA_DIR + '/convert_to_upper.py', '-'], json.dumps(params))
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)

        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertIn('gdal_version', res)
        self.assertIn('geos_version', res)
        self.assertIn('proj_version', res)
        self.assertIn('python_version', res)
        self.assertIn('qt_version', res)
        self.assertIn('qgis_version', res)
        self.assertEqual(res['algorithm_details']['id'], 'script:converttouppercase')
        self.assertEqual(res['results']['OUTPUT'], 'ABC DEF')

    def testPythonScriptRunNotAlgorithm(self):
        rc, output, err = self.run_process(['run', TEST_DATA_DIR + '/not_a_processing_script.py'])
        self.assertEqual(rc, 1)
        self.assertIn('is not a valid Processing script', err)

    def testPythonScriptHelpNotAlgorithm(self):
        rc, output, err = self.run_process(['help', TEST_DATA_DIR + '/not_a_processing_script.py'])
        self.assertEqual(rc, 1)
        self.assertIn('is not a valid Processing script', err)

    def testPythonScriptRunError(self):
        rc, output, err = self.run_process(['run', TEST_DATA_DIR + '/script_with_error.py'])
        self.assertEqual(rc, 1)
        self.assertIn('is not a valid Processing script', err)

    def testPythonScriptHelpError(self):
        rc, output, err = self.run_process(['help', TEST_DATA_DIR + '/script_with_error.py'])
        self.assertEqual(rc, 1)
        self.assertIn('is not a valid Processing script', err)

    def testComplexParameterNames(self):
        rc, output, err = self.run_process(['run', TEST_DATA_DIR + '/complex_names.py', '--INPUT with many complex chars.123 a=abc', '--another% complex# NaMe=def'])
        if os.environ.get('TRAVIS', '') != 'true':
            # Travis DOES have errors, due to QStandardPaths: XDG_RUNTIME_DIR not set warnings raised by Qt
            self.assertFalse(err)

        self.assertIn('OUTPUT:	abc:def', output)
        self.assertEqual(rc, 0)


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
                m = re.search(r'/(QGIS(_\d\.\d-dev)?)\.app', app_path)
                if m:
                    QGIS_PROCESS_BIN = app_path + '/Contents/MacOS/' + m.group(1)
                    found = True
                    break
            if found:
                break

    print(('\nQGIS_PROCESS_BIN: {}'.format(QGIS_PROCESS_BIN)))
    assert QGIS_PROCESS_BIN, 'qgis_process binary not found, skipping test suite'
    unittest.main()
