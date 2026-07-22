"""QGIS Unit tests for qgis_process profile options.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "thc1006"
__date__ = "22/07/2026"
__copyright__ = "Copyright 2026, The QGIS Project"

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

# A minimal profile-local Processing script. Dropped into a profile's scripts
# folder, it must only be discoverable while that profile is the active one.
CANARY_SCRIPT = """
from qgis.core import QgsProcessingAlgorithm, QgsProcessingOutputString


class ProfileIsolationCanary(QgsProcessingAlgorithm):
    def createInstance(self):
        return ProfileIsolationCanary()

    def name(self):
        return "profileisolationcanary"

    def displayName(self):
        return "Profile isolation canary"

    def initAlgorithm(self, config=None):
        self.addOutput(QgsProcessingOutputString("OUTPUT", "Output"))

    def processAlgorithm(self, parameters, context, feedback):
        return {"OUTPUT": "ok"}
"""


class TestQgsProcessExecutablePt3(QgisTestCase):
    TMP_DIR = ""

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.TMP_DIR = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()
        shutil.rmtree(cls.TMP_DIR, ignore_errors=True)

    def run_process(self, arguments, environment=None):
        call = [QGIS_PROCESS_BIN] + arguments
        print(" ".join(call))

        myenv = os.environ.copy()
        myenv["QGIS_DEBUG"] = "0"
        if environment:
            myenv.update(environment)

        p = subprocess.Popen(
            call, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=myenv
        )
        output, err = p.communicate()
        rc = p.returncode

        return rc, output.decode(), err.decode()

    def make_profiles_root(self):
        """Returns a fresh, isolated profiles root under the class temp dir."""
        return tempfile.mkdtemp(dir=self.TMP_DIR)

    def add_canary_script(self, profiles_root, profile_name):
        """Writes the canary script into a profile's scripts folder."""
        scripts_dir = os.path.join(
            profiles_root, "profiles", profile_name, "processing", "scripts"
        )
        os.makedirs(scripts_dir, exist_ok=True)
        with open(os.path.join(scripts_dir, "profile_canary.py"), "w") as f:
            f.write(CANARY_SCRIPT)

    def testHelpDocumentsProfileOptions(self):
        rc, output, err = self.run_process(["--help"])
        self.assertEqual(rc, 0)
        self.assertIn("--profile name", output)
        self.assertIn("--profiles-path", output)
        self.assertIn("-S", output)

    def testProfileSelectsProfileLocalScript(self):
        # A script that only exists in profile "alpha" must be listed when alpha
        # is selected and absent under a different profile in the same root.
        root = self.make_profiles_root()
        self.add_canary_script(root, "alpha")

        rc, output, err = self.run_process(
            ["--profiles-path", root, "--profile", "alpha", "list"]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        self.assertIn("profileisolationcanary", output.lower())

        rc, output, err = self.run_process(
            ["--profiles-path", root, "--profile", "beta", "list"]
        )
        self.assertEqual(rc, 0)
        self.assertNotIn("profileisolationcanary", output.lower())

    def testProfilesPathIsolation(self):
        # The same profile name under two different roots must not share state.
        root_a = self.make_profiles_root()
        root_b = self.make_profiles_root()
        self.add_canary_script(root_a, "alpha")

        rc, output, err = self.run_process(
            ["--profiles-path", root_a, "--profile", "alpha", "list"]
        )
        self.assertEqual(rc, 0)
        self.assertIn("profileisolationcanary", output.lower())

        rc, output, err = self.run_process(
            ["--profiles-path", root_b, "--profile", "alpha", "list"]
        )
        self.assertEqual(rc, 0)
        self.assertNotIn("profileisolationcanary", output.lower())

    def testMissingProfileValue(self):
        rc, output, err = self.run_process(["--profile"])
        self.assertEqual(rc, 1)
        self.assertIn("--profile", err)

    def testMissingProfilesPathValue(self):
        rc, output, err = self.run_process(["--profiles-path"])
        self.assertEqual(rc, 1)
        self.assertIn("--profiles-path", err)

        rc, output, err = self.run_process(["-S"])
        self.assertEqual(rc, 1)
        self.assertIn("--profiles-path", err)

    def testProfileJsonCompatibility(self):
        root = self.make_profiles_root()
        rc, output, err = self.run_process(
            ["--profiles-path", root, "--profile", "alpha", "list", "--json"]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        res = json.loads(output)
        self.assertIn("providers", res)
        self.assertIn("native", res["providers"])

    def testProfileOptionsRespectArgumentSeparator(self):
        # Global options are parsed before "--"; the parameters after it must be
        # left untouched, so the algorithm still runs with the selected profile.
        root = self.make_profiles_root()
        output_file = os.path.join(self.TMP_DIR, "boundary_centroid.shp")
        rc, output, err = self.run_process(
            [
                "run",
                "--no-python",
                "native:centroids",
                "--profiles-path",
                root,
                "--profile",
                "alpha",
                "--",
                f"INPUT={TEST_DATA_DIR + '/polys.shp'}",
                f"OUTPUT={output_file}",
            ]
        )
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        self.assertTrue(os.path.exists(output_file))
        self.assertTrue(os.path.isdir(os.path.join(root, "profiles", "alpha")))

    def testProfilesPathWithSpaces(self):
        root = self.make_profiles_root()
        spaced = os.path.join(root, "profiles root with spaces")
        os.makedirs(spaced, exist_ok=True)
        rc, output, err = self.run_process(
            ["--profiles-path", spaced, "--profile", "alpha", "list", "--json"]
        )
        self.assertEqual(rc, 0)
        res = json.loads(output)
        self.assertIn("providers", res)
        self.assertTrue(os.path.isdir(os.path.join(spaced, "profiles", "alpha")))

    def testVersionAndHelpAfterProfileOptions(self):
        # A profile option with a valid value in front of --version/--help must
        # not swallow the fast path, and an informational query must not create
        # a profile. The environment is redirected to a temporary root so the
        # real user profile can never be touched.
        root = self.make_profiles_root()
        sandbox = {"QGIS_CUSTOM_CONFIG_PATH": root}
        for opts in (
            ["--profile", "x"],
            ["--profiles-path", root],
            ["-S", root],
        ):
            rc, output, err = self.run_process(opts + ["--version"], sandbox)
            self.assertEqual(rc, 0)
            self.assertIn("QGIS", output)

            rc, output, err = self.run_process(opts + ["--help"], sandbox)
            self.assertEqual(rc, 0)
            self.assertIn("--profile name", output)

        self.assertFalse(os.path.isdir(os.path.join(root, "profiles")))

    def testShortProfilesPathOptionSelectsProfile(self):
        # -S is the short form of --profiles-path and must work as a real option,
        # not only be recognised when its value is missing.
        root = self.make_profiles_root()
        self.add_canary_script(root, "alpha")
        rc, output, err = self.run_process(["-S", root, "--profile", "alpha", "list"])
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 0)
        self.assertIn("profileisolationcanary", output.lower())
        self.assertTrue(os.path.isdir(os.path.join(root, "profiles", "alpha")))

    def testProfileUsesCustomConfigPath(self):
        # With no --profiles-path, QGIS_CUSTOM_CONFIG_PATH provides the root.
        root = self.make_profiles_root()
        self.add_canary_script(root, "alpha")
        rc, output, err = self.run_process(
            ["--profile", "alpha", "list"], {"QGIS_CUSTOM_CONFIG_PATH": root}
        )
        self.assertEqual(rc, 0)
        self.assertIn("profileisolationcanary", output.lower())

    def testProfilesPathOverridesCustomConfigPath(self):
        # An explicit --profiles-path takes precedence over the environment.
        env_root = self.make_profiles_root()
        cli_root = self.make_profiles_root()
        self.add_canary_script(env_root, "alpha")
        rc, output, err = self.run_process(
            ["--profiles-path", cli_root, "--profile", "alpha", "list"],
            {"QGIS_CUSTOM_CONFIG_PATH": env_root},
        )
        self.assertEqual(rc, 0)
        # the script lives under the environment root, which must be ignored...
        self.assertNotIn("profileisolationcanary", output.lower())
        # ...and the profile must have been created under the command line root
        self.assertTrue(os.path.isdir(os.path.join(cli_root, "profiles", "alpha")))

    def testProfilesPathUsesConfiguredDefaultProfile(self):
        # With --profiles-path but no --profile, the default profile recorded in
        # profiles.ini under that root is used. Writing it explicitly keeps the
        # test independent of any ambient default profile setting.
        root = self.make_profiles_root()
        profiles_dir = os.path.join(root, "profiles")
        os.makedirs(profiles_dir, exist_ok=True)
        with open(os.path.join(profiles_dir, "profiles.ini"), "w") as ini:
            ini.write("[core]\ndefaultProfile=chosen\n")
        self.add_canary_script(root, "chosen")

        rc, output, err = self.run_process(["--profiles-path", root, "list"])
        self.assertEqual(rc, 0)
        self.assertIn("profileisolationcanary", output.lower())

    def testMissingValueBeforeAnotherOption(self):
        # A forgotten value must not silently consume the next option/separator.
        for arguments in (
            ["--profile", "--json", "list"],
            ["--profile", "--"],
            ["--profile", ""],
            ["--profiles-path", "--json", "list"],
            ["--profiles-path", ""],
            ["-S", "--profile"],
        ):
            rc, output, err = self.run_process(arguments)
            self.assertEqual(rc, 1, msg=f"expected failure for {arguments}")
            self.assertIn("requires", err.lower())

    def testProfilesPathPointingAtFileFails(self):
        # A profiles path that cannot hold a profiles folder must fail cleanly,
        # reporting the reason, instead of continuing with a half-created profile.
        blocker = os.path.join(self.TMP_DIR, "not_a_directory")
        with open(blocker, "w") as f:
            f.write("")
        rc, output, err = self.run_process(
            ["--profiles-path", blocker, "--profile", "alpha", "list"]
        )
        # exactly 1, so that a crash is not mistaken for a clean failure
        self.assertEqual(rc, 1)
        self.assertIn("could not create profile", err.lower())

    def testMalformedProfileOptionIsAnErrorEvenBeforeVersion(self):
        # A malformed profile option is always an error, also when --version or
        # --help follows: the fast path must not paper over the bad argument.
        for arguments in (
            ["--profile", "--version"],
            ["--profiles-path", "--version"],
            ["--profile", "--help"],
            ["-S", "-v"],
        ):
            rc, output, err = self.run_process(arguments)
            self.assertEqual(rc, 1, msg=f"{arguments}: rc={rc} err={err}")
            self.assertIn("requires", err.lower())

    def testProfileOptionAfterSeparatorIsNotConsumed(self):
        # A --profile token after "--" is an algorithm parameter, not a profile
        # switch, so it must not create or select that profile.
        root = self.make_profiles_root()
        output_file = os.path.join(self.TMP_DIR, "boundary_negative.shp")
        rc, output, err = self.run_process(
            [
                "run",
                "--no-python",
                "native:centroids",
                "--profiles-path",
                root,
                "--",
                f"INPUT={TEST_DATA_DIR + '/polys.shp'}",
                f"OUTPUT={output_file}",
                "--profile",
                "ghost",
            ]
        )
        self.assertEqual(rc, 0)
        self.assertTrue(os.path.exists(output_file))
        self.assertFalse(os.path.isdir(os.path.join(root, "profiles", "ghost")))
        self.assertTrue(os.path.isdir(os.path.join(root, "profiles", "default")))

    def testProfilePluginStateIsolation(self):
        # Enabling a plugin writes into the active profile's settings; a second
        # profile under the same root must keep its own state. This is the
        # per-profile isolation #44783 is about.
        root = self.make_profiles_root()

        def grass_enabled(profile):
            rc, output, err = self.run_process(
                ["--profiles-path", root, "--profile", profile, "plugins"]
            )
            self.assertEqual(rc, 0)
            if "grassprovider" not in output.lower():
                self.skipTest("grassprovider plugin not available in this build")
            return "* grassprovider" in output.lower()

        baseline = grass_enabled("beta")
        action = "disable" if baseline else "enable"
        rc, output, err = self.run_process(
            [
                "--profiles-path",
                root,
                "--profile",
                "alpha",
                "plugins",
                action,
                "grassprovider",
            ]
        )
        self.assertEqual(rc, 0)
        self.assertNotEqual(grass_enabled("alpha"), baseline)
        self.assertEqual(grass_enabled("beta"), baseline)


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
