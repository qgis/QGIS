"""
***************************************************************************
    AlgorithmsTest.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthias Kuhn'


import os
import yaml
import nose2
import shutil
import glob
import hashlib
import tempfile
import re

from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly
from numpy import nan_to_num
from copy import deepcopy

from qgis.PyQt.QtCore import QT_VERSION
from qgis.core import (Qgis,
                       QgsVectorLayer,
                       QgsRasterLayer,
                       QgsCoordinateReferenceSystem,
                       QgsFeatureRequest,
                       QgsMapLayer,
                       QgsProject,
                       QgsApplication,
                       QgsProcessingContext,
                       QgsProcessingUtils,
                       QgsProcessingFeedback)
from qgis.analysis import (QgsNativeAlgorithms)
from qgis.testing import (_UnexpectedSuccess,
                          QgisTestCase,
                          start_app)
from utilities import unitTestDataPath

import processing

gdal.UseExceptions()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


def processingTestDataPath():
    return os.path.join(os.path.dirname(__file__), 'testdata')


class AlgorithmsTest:

    def test_algorithms(self):
        """
        This is the main test function. All others will be executed based on the definitions in testdata/algorithm_tests.yaml
        """
        with open(os.path.join(processingTestDataPath(), self.definition_file())) as stream:
            algorithm_tests = yaml.load(stream, Loader=yaml.SafeLoader)

        if 'tests' in algorithm_tests and algorithm_tests['tests'] is not None:
            for idx, algtest in enumerate(algorithm_tests['tests']):
                condition = algtest.get('condition')
                if condition:
                    geos_condition = condition.get('geos')
                    if geos_condition:
                        less_than_condition = geos_condition.get('less_than')
                        if less_than_condition:
                            if Qgis.geosVersionInt() >= less_than_condition:
                                print('!!! Skipping {}, requires GEOS < {}, have version {}'.format(algtest['name'], less_than_condition, Qgis.geosVersionInt()))
                                continue
                        at_least_condition = geos_condition.get('at_least')
                        if at_least_condition:
                            if Qgis.geosVersionInt() < at_least_condition:
                                print('!!! Skipping {}, requires GEOS >= {}, have version {}'.format(algtest['name'], at_least_condition, Qgis.geosVersionInt()))
                                continue
                    gdal_condition = condition.get('gdal')
                    if gdal_condition:
                        less_than_condition = gdal_condition.get('less_than')
                        if less_than_condition:
                            if int(gdal.VersionInfo('VERSION_NUM')) >= less_than_condition:
                                print('!!! Skipping {}, requires GDAL < {}, have version {}'.format(algtest['name'], less_than_condition, gdal.VersionInfo('VERSION_NUM')))
                                continue
                        at_least_condition = gdal_condition.get('at_least')
                        if at_least_condition:
                            if int(gdal.VersionInfo('VERSION_NUM')) < at_least_condition:
                                print('!!! Skipping {}, requires GDAL >= {}, have version {}'.format(algtest['name'], at_least_condition, gdal.VersionInfo('VERSION_NUM')))
                                continue
                    qt_condition = condition.get('qt')
                    if qt_condition:
                        less_than_condition = qt_condition.get('less_than')
                        if less_than_condition:
                            if QT_VERSION >= less_than_condition:
                                print('!!! Skipping {}, requires Qt < {}, have version {}'.format(algtest['name'], less_than_condition, QT_VERSION))
                                continue
                        at_least_condition = qt_condition.get('at_least')
                        if at_least_condition:
                            if QT_VERSION < at_least_condition:
                                print('!!! Skipping {}, requires Qt >= {}, have version {}'.format(algtest['name'], at_least_condition, QT_VERSION))
                                continue
                print('About to start {} of {}: "{}"'.format(idx, len(algorithm_tests['tests']), algtest['name']))
                yield self.check_algorithm, algtest['name'], algtest

    def check_algorithm(self, name, defs):
        """
        Will run an algorithm definition and check if it generates the expected result
        :param name: The identifier name used in the test output heading
        :param defs: A python dict containing a test algorithm definition
        """
        self.vector_layer_params = {}
        QgsProject.instance().clear()

        if 'project' in defs:
            full_project_path = os.path.join(processingTestDataPath(), defs['project'])
            project_read_success = QgsProject.instance().read(full_project_path)
            self.assertTrue(project_read_success, 'Failed to load project file: ' + defs['project'])

        if 'project_crs' in defs:
            QgsProject.instance().setCrs(QgsCoordinateReferenceSystem(defs['project_crs']))
        else:
            QgsProject.instance().setCrs(QgsCoordinateReferenceSystem())

        if 'ellipsoid' in defs:
            QgsProject.instance().setEllipsoid(defs['ellipsoid'])
        else:
            QgsProject.instance().setEllipsoid('')

        params = self.load_params(defs['params'])

        print('Running alg: "{}"'.format(defs['algorithm']))
        alg = QgsApplication.processingRegistry().createAlgorithmById(defs['algorithm'])

        parameters = {}
        if isinstance(params, list):
            for param in zip(alg.parameterDefinitions(), params):
                parameters[param[0].name()] = param[1]
        else:
            for k, p in params.items():
                parameters[k] = p

        for r, p in list(defs['results'].items()):
            if 'in_place_result' not in p or not p['in_place_result']:
                parameters[r] = self.load_result_param(p)

        expectFailure = False
        if 'expectedFailure' in defs:
            exec(('\n'.join(defs['expectedFailure'][:-1])), globals(), locals())
            expectFailure = eval(defs['expectedFailure'][-1])

        if 'expectedException' in defs:
            expectFailure = True

        # ignore user setting for invalid geometry handling
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        if 'ellipsoid' in defs:
            # depending on the project settings, we can't always rely
            # on QgsProject.ellipsoid() returning the same ellipsoid as was
            # specified in the test definition. So just force ensure that the
            # context's ellipsoid is the desired one
            context.setEllipsoid(defs['ellipsoid'])

        if 'skipInvalid' in defs and defs['skipInvalid']:
            context.setInvalidGeometryCheck(QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid)

        feedback = QgsProcessingFeedback()

        print(f'Algorithm parameters are {parameters}')

        # first check that algorithm accepts the parameters we pass...
        ok, msg = alg.checkParameterValues(parameters, context)
        self.assertTrue(ok, f'Algorithm failed checkParameterValues with result {msg}')

        if expectFailure:
            try:
                results, ok = alg.run(parameters, context, feedback)
                self.check_results(results, context, parameters, defs['results'])
                if ok:
                    raise _UnexpectedSuccess
            except Exception:
                pass
        else:
            results, ok = alg.run(parameters, context, feedback)
            self.assertTrue(ok, f'params: {parameters}, results: {results}')
            self.check_results(results, context, parameters, defs['results'])

    def load_params(self, params):
        """
        Loads an array of parameters
        """
        if isinstance(params, list):
            return [self.load_param(p) for p in params]
        elif isinstance(params, dict):
            return {key: self.load_param(p, key) for key, p in params.items()}
        else:
            return params

    def load_param(self, param, id=None):
        """
        Loads a parameter. If it's not a map, the parameter will be returned as-is. If it is a map, it will process the
        parameter based on its key `type` and return the appropriate parameter to pass to the algorithm.
        """
        try:
            if param['type'] in ('vector', 'raster', 'table'):
                return self.load_layer(id, param).id()
            elif param['type'] == 'vrtlayers':
                vals = []
                for p in param['params']:
                    p['layer'] = self.load_layer(None, {'type': 'vector', 'name': p['layer']})
                    vals.append(p)
                return vals
            elif param['type'] == 'multi':
                return [self.load_param(p) for p in param['params']]
            elif param['type'] == 'file':
                return self.filepath_from_param(param)
            elif param['type'] == 'interpolation':
                prefix = processingTestDataPath()
                tmp = ''
                for r in param['name'].split('::|::'):
                    v = r.split('::~::')
                    tmp += '{}::~::{}::~::{}::~::{};'.format(os.path.join(prefix, v[0]),
                                                             v[1], v[2], v[3])
                return tmp[:-1]
        except TypeError:
            # No type specified, use whatever is there
            return param

        raise KeyError("Unknown type '{}' specified for parameter".format(param['type']))

    def load_result_param(self, param):
        """
        Loads a result parameter. Creates a temporary destination where the result should go to and returns this location
        so it can be sent to the algorithm as parameter.
        """
        if param['type'] in ['vector', 'file', 'table', 'regex']:
            outdir = tempfile.mkdtemp()
            self.cleanup_paths.append(outdir)
            if isinstance(param['name'], str):
                basename = os.path.basename(param['name'])
            else:
                basename = os.path.basename(param['name'][0])

            filepath = self.uri_path_join(outdir, basename)
            return filepath
        elif param['type'] == 'rasterhash':
            outdir = tempfile.mkdtemp()
            self.cleanup_paths.append(outdir)
            basename = 'raster.tif'
            filepath = os.path.join(outdir, basename)
            return filepath
        elif param['type'] == 'directory':
            outdir = tempfile.mkdtemp()
            return outdir

        raise KeyError("Unknown type '{}' specified for parameter".format(param['type']))

    def load_layers(self, id, param):
        layers = []
        if param['type'] in ('vector', 'table'):
            if isinstance(param['name'], str) or 'uri' in param:
                layers.append(self.load_layer(id, param))
            else:
                for n in param['name']:
                    layer_param = deepcopy(param)
                    layer_param['name'] = n
                    layers.append(self.load_layer(id, layer_param))
        else:
            layers.append(self.load_layer(id, param))
        return layers

    def load_layer(self, id, param):
        """
        Loads a layer which was specified as parameter.
        """

        filepath = self.filepath_from_param(param)

        if 'in_place' in param and param['in_place']:
            # check if alg modifies layer in place
            tmpdir = tempfile.mkdtemp()
            self.cleanup_paths.append(tmpdir)
            path, file_name = os.path.split(filepath)
            base, ext = os.path.splitext(file_name)
            for file in glob.glob(os.path.join(path, f'{base}.*')):
                shutil.copy(os.path.join(path, file), tmpdir)
            filepath = os.path.join(tmpdir, file_name)
            self.in_place_layers[id] = filepath

        if param['type'] in ('vector', 'table'):
            gmlrex = r'\.gml\b'
            if re.search(gmlrex, filepath, re.IGNORECASE):
                # ewwwww - we have to force SRS detection for GML files, otherwise they'll be loaded
                # with no srs
                filepath += '|option:FORCE_SRS_DETECTION=YES'

            if filepath in self.vector_layer_params:
                return self.vector_layer_params[filepath]

            options = QgsVectorLayer.LayerOptions()
            options.loadDefaultStyle = False
            lyr = QgsVectorLayer(filepath, param['name'], 'ogr', options)
            self.vector_layer_params[filepath] = lyr
        elif param['type'] == 'raster':
            options = QgsRasterLayer.LayerOptions()
            options.loadDefaultStyle = False
            lyr = QgsRasterLayer(filepath, param['name'], 'gdal', options)

        self.assertTrue(lyr.isValid(), f'Could not load layer "{filepath}" from param {param}')
        QgsProject.instance().addMapLayer(lyr)
        return lyr

    def filepath_from_param(self, param):
        """
        Creates a filepath from a param
        """
        prefix = processingTestDataPath()
        if 'location' in param and param['location'] == 'qgs':
            prefix = unitTestDataPath()

        if 'uri' in param:
            path = param['uri']
        else:
            path = param['name']

        if not path:
            return None

        return self.uri_path_join(prefix, path)

    def uri_path_join(self, prefix, filepath):
        if filepath.startswith('ogr:'):
            if not prefix[-1] == os.path.sep:
                prefix += os.path.sep
            filepath = re.sub(r"dbname='", f"dbname='{prefix}", filepath)
        else:
            filepath = os.path.join(prefix, filepath)

        return filepath

    def check_results(self, results, context, params, expected):
        """
        Checks if result produced by an algorithm matches with the expected specification.
        """
        for id, expected_result in expected.items():
            if expected_result['type'] in ('vector', 'table'):
                if 'compare' in expected_result and not expected_result['compare']:
                    # skipping the comparison, so just make sure output is valid
                    if isinstance(results[id], QgsMapLayer):
                        result_lyr = results[id]
                    else:
                        result_lyr = QgsProcessingUtils.mapLayerFromString(results[id], context)
                    self.assertTrue(result_lyr.isValid())
                    continue

                expected_lyrs = self.load_layers(id, expected_result)
                if 'in_place_result' in expected_result:
                    result_lyr = QgsProcessingUtils.mapLayerFromString(self.in_place_layers[id], context)
                    self.assertTrue(result_lyr.isValid(), self.in_place_layers[id])
                else:
                    try:
                        results[id]
                    except KeyError as e:
                        raise KeyError(f'Expected result {str(e)} does not exist in {list(results.keys())}')

                    if isinstance(results[id], QgsMapLayer):
                        result_lyr = results[id]
                    else:
                        string = results[id]

                        gmlrex = r'\.gml\b'
                        if re.search(gmlrex, string, re.IGNORECASE):
                            # ewwwww - we have to force SRS detection for GML files, otherwise they'll be loaded
                            # with no srs
                            string += '|option:FORCE_SRS_DETECTION=YES'

                        result_lyr = QgsProcessingUtils.mapLayerFromString(string, context)
                    self.assertTrue(result_lyr, results[id])

                compare = expected_result.get('compare', {})
                pk = expected_result.get('pk', None)

                if len(expected_lyrs) == 1:
                    self.assertLayersEqual(expected_lyrs[0], result_lyr, compare=compare, pk=pk)
                else:
                    res = False
                    for l in expected_lyrs:
                        if self.checkLayersEqual(l, result_lyr, compare=compare, pk=pk):
                            res = True
                            break
                    self.assertTrue(res, 'Could not find matching layer in expected results')

            elif 'rasterhash' == expected_result['type']:
                print(f"id:{id} result:{results[id]}")
                self.assertTrue(os.path.exists(results[id]), f'File does not exist: {results[id]}, {params}')
                dataset = gdal.Open(results[id], GA_ReadOnly)
                dataArray = nan_to_num(dataset.ReadAsArray(0))
                strhash = hashlib.sha224(dataArray.data).hexdigest()

                if not isinstance(expected_result['hash'], str):
                    self.assertIn(strhash, expected_result['hash'])
                else:
                    self.assertEqual(strhash, expected_result['hash'])
            elif 'file' == expected_result['type']:
                result_filepath = results[id]
                if isinstance(expected_result.get('name'), list):
                    # test to see if any match expected
                    for path in expected_result['name']:
                        expected_filepath = self.filepath_from_param({'name': path})
                        if self.checkFilesEqual(expected_filepath, result_filepath):
                            break
                    else:
                        expected_filepath = self.filepath_from_param({'name': expected_result['name'][0]})
                else:
                    expected_filepath = self.filepath_from_param(expected_result)

                self.assertFilesEqual(expected_filepath, result_filepath)
            elif 'directory' == expected_result['type']:
                expected_dirpath = self.filepath_from_param(expected_result)
                result_dirpath = results[id]

                self.assertDirectoriesEqual(expected_dirpath, result_dirpath)
            elif 'regex' == expected_result['type']:
                with open(results[id]) as file:
                    data = file.read()

                for rule in expected_result.get('rules', []):
                    self.assertRegex(data, rule)


class GenericAlgorithmsTest(QgisTestCase):
    """
    General (non-provider specific) algorithm tests
    """

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing
        Processing.initialize()
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        from processing.core.Processing import Processing
        Processing.deinitialize()
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testAlgorithmCompliance(self):
        for p in QgsApplication.processingRegistry().providers():
            print(f'testing provider {p.id()}')
            for a in p.algorithms():
                print(f'testing algorithm {a.id()}')
                self.check_algorithm(a)

    def check_algorithm(self, alg):
        # check that calling helpUrl() works without error
        alg.helpUrl()


if __name__ == '__main__':
    nose2.main()
