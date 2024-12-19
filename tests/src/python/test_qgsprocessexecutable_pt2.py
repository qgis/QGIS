"""QGIS Unit tests for qgis_process.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "05/04/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import glob
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

from qgis.testing import QgisTestCase, unittest

from utilities import unitTestDataPath

print("CTEST_FULL_OUTPUT")

TEST_DATA_DIR = unitTestDataPath()


class TestQgsProcessExecutablePt2(QgisTestCase):

    TMP_DIR = ""

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.TMP_DIR = tempfile.mkdtemp()
        # print('TMP_DIR: ' + cls.TMP_DIR)
        # subprocess.call(['open', cls.TMP_DIR])

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()
        shutil.rmtree(cls.TMP_DIR, ignore_errors=True)

    def run_process(self, arguments):
        call = [QGIS_PROCESS_BIN] + arguments
        print(" ".join(call))

        myenv = os.environ.copy()
        myenv["QGIS_DEBUG"] = "0"

        p = subprocess.Popen(
            call, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=myenv
        )
        output, err = p.communicate()
        rc = p.returncode

        return rc, output.decode(), err.decode()

    def run_process_stdin(self, arguments, stdin_string: str):
        call = [QGIS_PROCESS_BIN] + arguments
        print(" ".join(call))

        myenv = os.environ.copy()
        myenv["QGIS_DEBUG"] = "0"

        p = subprocess.Popen(
            call,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdin=subprocess.PIPE,
            env=myenv,
        )
        output, err = p.communicate(input=stdin_string.encode())
        rc = p.returncode

        return rc, output.decode(), err.decode()

    def testAlgorithmRunStdInMissingInputKey(self):
        output_file = self.TMP_DIR + "/polygon_centroid_json.shp"

        params = {"INPUT": TEST_DATA_DIR + "/polys.shp", "OUTPUT": output_file}

        rc, output, err = self.run_process_stdin(
            ["run", "--no-python", "native:centroids", "-"], json.dumps(params)
        )
        self.assertEqual(rc, 1)
        self.assertIn('JSON parameters object must contain an "inputs" key.', err)

    def testAlgorithmRunStdInNoInput(self):
        rc, output, err = self.run_process_stdin(
            ["run", "--no-python", "native:centroids", "-"], ""
        )
        self.assertEqual(rc, 1)
        self.assertIn("Could not parse JSON parameters", err)

    def testAlgorithmRunStdInBadInput(self):
        rc, output, err = self.run_process_stdin(
            ["run", "--no-python", "native:centroids", "-"], '{"not valid json"}'
        )
        self.assertEqual(rc, 1)
        self.assertIn("Could not parse JSON parameters", err)

    def testAlgorithmRunJson(self):
        output_file = self.TMP_DIR + "/polygon_centroid2.shp"
        rc, output, err = self.run_process(
            [
                "run",
                "--no-python",
                "--json",
                "native:centroids",
                "--",
                f"INPUT={TEST_DATA_DIR + '/polys.shp'}",
                f"OUTPUT={output_file}",
            ]
        )
        res = json.loads(output)

        self.assertIn("gdal_version", res)
        self.assertIn("geos_version", res)
        self.assertIn("proj_version", res)
        self.assertIn("python_version", res)
        self.assertIn("qt_version", res)
        self.assertIn("qgis_version", res)

        self.assertEqual(res["algorithm_details"]["name"], "Centroids")
        self.assertEqual(res["inputs"]["INPUT"], TEST_DATA_DIR + "/polys.shp")
        self.assertEqual(res["inputs"]["OUTPUT"], output_file)
        self.assertEqual(res["results"]["OUTPUT"], output_file)

        self.assertTrue(os.path.exists(output_file))
        self.assertEqual(rc, 0)

    def testAlgorithmRunListValue(self):
        """
        Test an algorithm which requires a list of layers as a parameter value
        """
        output_file = self.TMP_DIR + "/package.gpkg"
        rc, output, err = self.run_process(
            [
                "run",
                "--no-python",
                "--json",
                "native:package",
                "--",
                f"LAYERS={TEST_DATA_DIR + '/polys.shp'}",
                f"LAYERS={TEST_DATA_DIR + '/points.shp'}",
                f"LAYERS={TEST_DATA_DIR + '/lines.shp'}",
                f"OUTPUT={output_file}",
            ]
        )
        res = json.loads(output)

        self.assertIn("gdal_version", res)
        self.assertIn("geos_version", res)
        self.assertIn("proj_version", res)
        self.assertIn("python_version", res)
        self.assertIn("qt_version", res)
        self.assertIn("qgis_version", res)

        self.assertEqual(res["algorithm_details"]["name"], "Package layers")
        self.assertEqual(len(res["inputs"]["LAYERS"]), 3)
        self.assertEqual(res["inputs"]["OUTPUT"], output_file)
        self.assertEqual(res["results"]["OUTPUT"], output_file)
        self.assertEqual(len(res["results"]["OUTPUT_LAYERS"]), 3)

        self.assertTrue(os.path.exists(output_file))
        self.assertEqual(rc, 0)

    def testModelHelp(self):
        rc, output, err = self.run_process(
            ["help", "--no-python", TEST_DATA_DIR + "/test_model.model3"]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        self.assertIn("model description", output.lower())
        self.assertIn("author of model", output.lower())
        self.assertIn("version 2.1", output.lower())
        self.assertIn("examples", output.lower())
        self.assertIn("this is an example of running the model", output.lower())

    def testModelRun(self):
        output_file = self.TMP_DIR + "/model_output.shp"
        rc, output, err = self.run_process(
            [
                "run",
                "--no-python",
                TEST_DATA_DIR + "/test_model.model3",
                "--",
                f"FEATS={TEST_DATA_DIR + '/polys.shp'}",
                f"native:centroids_1:CENTROIDS={output_file}",
            ]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        self.assertIn("0...10...20...30...40...50...60...70...80...90", output.lower())
        self.assertIn("results", output.lower())
        self.assertTrue(os.path.exists(output_file))

    def testModelRunStdIn(self):
        output_file = self.TMP_DIR + "/model_output_stdin.shp"

        params = {
            "inputs": {
                "FEATS": TEST_DATA_DIR + "/polys.shp",
                "native:centroids_1:CENTROIDS": output_file,
            }
        }

        rc, output, err = self.run_process_stdin(
            ["run", "--no-python", TEST_DATA_DIR + "/test_model.model3", "-"],
            json.dumps(params),
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertIn("gdal_version", res)
        self.assertIn("geos_version", res)
        self.assertIn("proj_version", res)
        self.assertIn("python_version", res)
        self.assertIn("qt_version", res)
        self.assertIn("qgis_version", res)
        self.assertEqual(res["algorithm_details"]["id"], "Test model")
        self.assertTrue(os.path.exists(output_file))

    def testModelRunJson(self):
        output_file = self.TMP_DIR + "/model_output2.shp"
        rc, output, err = self.run_process(
            [
                "run",
                TEST_DATA_DIR + "/test_model.model3",
                "--no-python",
                "--json",
                "--",
                f"FEATS={TEST_DATA_DIR + '/polys.shp'}",
                f"native:centroids_1:CENTROIDS={output_file}",
            ]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertIn("gdal_version", res)
        self.assertIn("geos_version", res)
        self.assertIn("proj_version", res)
        self.assertIn("python_version", res)
        self.assertIn("qt_version", res)
        self.assertIn("qgis_version", res)
        self.assertEqual(res["algorithm_details"]["id"], "Test model")
        self.assertTrue(os.path.exists(output_file))

    def testModelRunWithLog(self):
        output_file = self.TMP_DIR + "/model_log.log"
        rc, output, err = self.run_process(
            [
                "run",
                "--no-python",
                TEST_DATA_DIR + "/test_logging_model.model3",
                "--",
                f"logfile={output_file}",
            ]
        )
        self.assertEqual(rc, 0)
        self.assertIn("0...10...20...30...40...50...60...70...80...90", output.lower())
        self.assertIn("results", output.lower())
        self.assertTrue(os.path.exists(output_file))

        with open(output_file) as f:
            lines = "\n".join(f.readlines())

        self.assertIn("Test logged message", lines)

    def testPythonScriptHelp(self):
        rc, output, err = self.run_process(
            ["help", TEST_DATA_DIR + "/convert_to_upper.py"]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        self.assertIn("converts a string to upper case", output.lower())

    def testPythonScriptRun(self):
        rc, output, err = self.run_process(
            ["run", TEST_DATA_DIR + "/convert_to_upper.py", "--", "INPUT=abc"]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        self.assertIn("Converted abc to ABC", output)
        self.assertIn("OUTPUT:\tABC", output)

    def testPythonScriptRunJson(self):
        rc, output, err = self.run_process(
            ["run", TEST_DATA_DIR + "/convert_to_upper.py", "--json", "--", "INPUT=abc"]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertIn("gdal_version", res)
        self.assertIn("geos_version", res)
        self.assertIn("proj_version", res)
        self.assertIn("python_version", res)
        self.assertIn("qt_version", res)
        self.assertIn("qgis_version", res)
        self.assertEqual(res["algorithm_details"]["id"], "script:converttouppercase")
        self.assertEqual(res["results"]["OUTPUT"], "ABC")

    def testScriptRunStdIn(self):
        output_file = self.TMP_DIR + "/model_output_stdin.shp"

        params = {"inputs": {"INPUT": "abc def"}}

        rc, output, err = self.run_process_stdin(
            ["run", TEST_DATA_DIR + "/convert_to_upper.py", "-"], json.dumps(params)
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))

        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertIn("gdal_version", res)
        self.assertIn("geos_version", res)
        self.assertIn("proj_version", res)
        self.assertIn("python_version", res)
        self.assertIn("qt_version", res)
        self.assertIn("qgis_version", res)
        self.assertEqual(res["algorithm_details"]["id"], "script:converttouppercase")
        self.assertEqual(res["results"]["OUTPUT"], "ABC DEF")

    def testPythonScriptRunNotAlgorithm(self):
        rc, output, err = self.run_process(
            ["run", TEST_DATA_DIR + "/not_a_processing_script.py"]
        )
        self.assertEqual(rc, 1)
        self.assertIn("is not a valid Processing script", err)

    def testPythonScriptHelpNotAlgorithm(self):
        rc, output, err = self.run_process(
            ["help", TEST_DATA_DIR + "/not_a_processing_script.py"]
        )
        self.assertEqual(rc, 1)
        self.assertIn("is not a valid Processing script", err)

    def testPythonScriptRunError(self):
        rc, output, err = self.run_process(
            ["run", TEST_DATA_DIR + "/script_with_error.py"]
        )
        self.assertNotEqual(rc, 0)
        self.assertIn("is not a valid Processing script", err)

    def testPythonScriptHelpError(self):
        rc, output, err = self.run_process(
            ["help", TEST_DATA_DIR + "/script_with_error.py"]
        )
        self.assertNotEqual(rc, 0)
        self.assertIn("is not a valid Processing script", err)

    def testComplexParameterNames(self):
        rc, output, err = self.run_process(
            [
                "run",
                TEST_DATA_DIR + "/complex_names.py",
                "--INPUT with many complex chars.123 a=abc",
                "--another% complex# NaMe=def",
            ]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))

        self.assertIn("OUTPUT:	abc:def", output)
        self.assertEqual(rc, 0)

    def testLoadLayer(self):
        rc, output, err = self.run_process(
            [
                "run",
                "--no-python",
                "native:raiseexception",
                "--MESSAGE=CONFIRMED",
                f"--CONDITION=layer_property(load_layer('{TEST_DATA_DIR + '/points.shp'}','ogr'),'feature_count')>10",
            ]
        )
        self.assertIn("CONFIRMED", self.strip_std_ignorable_errors(err))

        self.assertEqual(rc, 1)

    def testDynamicParameters(self):
        output_file = self.TMP_DIR + "/dynamic_out2.shp"

        rc, output, err = self.run_process(
            [
                "run",
                "native:buffer",
                "--INPUT=" + TEST_DATA_DIR + "/points.shp",
                "--OUTPUT=" + output_file,
                "--DISTANCE=field:fid",
                "--json",
            ]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))

        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertEqual(res["algorithm_details"]["id"], "native:buffer")
        self.assertEqual(res["inputs"]["DISTANCE"], "field:fid")

    def testDynamicParametersJson(self):
        output_file = self.TMP_DIR + "/dynamic_out.shp"

        params = {
            "inputs": {
                "INPUT": TEST_DATA_DIR + "/points.shp",
                "DISTANCE": {"type": "data_defined", "field": "fid"},
                "OUTPUT": output_file,
            }
        }

        rc, output, err = self.run_process_stdin(
            ["run", "native:buffer", "-"], json.dumps(params)
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))

        self.assertEqual(rc, 0)

        res = json.loads(output)
        self.assertEqual(res["algorithm_details"]["id"], "native:buffer")
        self.assertEqual(
            res["inputs"]["DISTANCE"], {"field": "fid", "type": "data_defined"}
        )

    def testStartupOptimisationsStyleLazyInitialized(self):
        """
        Ensure that the costly QgsStyle.defaultStyle() initialization is NOT
        performed by default when running qgis_process commands
        """
        rc, output, err = self.run_process(
            ["run", TEST_DATA_DIR + "/report_style_initialization_status.py"]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        self.assertIn("IS_INITIALIZED:	false", output)


if __name__ == "__main__":
    # look for qgis bin path
    QGIS_PROCESS_BIN = ""
    prefixPath = os.environ["QGIS_PREFIX_PATH"]
    # see qgsapplication.cpp:98
    for f in ["", "..", "bin"]:
        d = os.path.join(prefixPath, f)
        b = os.path.abspath(os.path.join(d, "qgis_process"))
        if os.path.exists(b):
            QGIS_PROCESS_BIN = b
            break
        b = os.path.abspath(os.path.join(d, "qgis_process.exe"))
        if os.path.exists(b):
            QGIS_PROCESS_BIN = b
            break
        if sys.platform[:3] == "dar":  # Mac
            # QGIS.app may be QGIS_x.x-dev.app for nightlies
            # internal binary will match, minus the '.app'
            found = False
            for app_path in glob.glob(d + "/QGIS*.app"):
                m = re.search(r"/(QGIS(_\d\.\d-dev)?)\.app", app_path)
                if m:
                    QGIS_PROCESS_BIN = app_path + "/Contents/MacOS/" + m.group(1)
                    found = True
                    break
            if found:
                break

    print(f"\nQGIS_PROCESS_BIN: {QGIS_PROCESS_BIN}")
    assert QGIS_PROCESS_BIN, "qgis_process binary not found, skipping test suite"
    unittest.main()
