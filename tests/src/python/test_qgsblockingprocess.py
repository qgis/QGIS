"""
***************************************************************************
    test_qgsblockingprocess.py
    ---------------------
    Date                 : January 2021
    Copyright            : (C) 2021 by Nyall Dawson
    Email                : nyall dot dawson at gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "January 2021"
__copyright__ = "(C) 2021, Nyall Dawson"

import os
import tempfile

from qgis.PyQt.QtCore import QProcess
from qgis.core import QgsBlockingProcess, QgsFeedback
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

app = start_app()


class TestQgsBlockingProcess(QgisTestCase):

    def test_process_ok(self):
        def std_out(ba):
            std_out.val += ba.data().decode("UTF-8")

        std_out.val = ""

        def std_err(ba):
            std_err.val += ba.data().decode("UTF-8")

        std_err.val = ""

        p = QgsBlockingProcess("ogrinfo", ["--version"])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertEqual(p.run(f), 0)
        self.assertEqual(p.exitStatus(), QProcess.ExitStatus.NormalExit)
        self.assertIn("GDAL", std_out.val)
        self.assertEqual(QgisTestCase.strip_std_ignorable_errors(std_err.val), "")

    def test_process_err(self):
        temp_folder = tempfile.mkdtemp()

        script_file = os.path.join(temp_folder, "stderr_process.sh")
        with open(script_file, "w") as f:
            f.write(
                """#!/bin/bash
echo "This goes to stdout"
echo "This goes to stderr" >&2
exit 1"""
            )

        os.chmod(script_file, 0o775)

        def std_out(ba):
            std_out.val += ba.data().decode("UTF-8")

        std_out.val = ""

        def std_err(ba):
            std_err.val += ba.data().decode("UTF-8")

        std_err.val = ""

        p = QgsBlockingProcess("sh", [script_file])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertEqual(p.run(f), 1)
        self.assertEqual(p.exitStatus(), QProcess.ExitStatus.NormalExit)
        self.assertIn(
            "This goes to stdout", QgisTestCase.strip_std_ignorable_errors(std_out.val)
        )
        self.assertIn(
            "This goes to stderr", QgisTestCase.strip_std_ignorable_errors(std_err.val)
        )

    def test_process_crash(self):
        """
        Test a script which simulates a crash
        """
        temp_folder = tempfile.mkdtemp()

        script_file = os.path.join(temp_folder, "crash_process.sh")
        with open(script_file, "w") as f:
            f.write("kill $$")

        os.chmod(script_file, 0o775)

        def std_out(ba):
            std_out.val += ba.data().decode("UTF-8")

        std_out.val = ""

        def std_err(ba):
            std_err.val += ba.data().decode("UTF-8")

        std_err.val = ""

        p = QgsBlockingProcess("sh", [script_file])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertNotEqual(p.run(f), 0)
        self.assertEqual(p.exitStatus(), QProcess.ExitStatus.CrashExit)

    def test_process_no_file(self):
        """
        Test a script which doesn't exist
        """

        def std_out(ba):
            std_out.val += ba.data().decode("UTF-8")

        std_out.val = ""

        def std_err(ba):
            std_err.val += ba.data().decode("UTF-8")

        std_err.val = ""

        # this program definitely doesn't exist!
        p = QgsBlockingProcess("qgis_sucks", ["--version"])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertEqual(p.run(f), 1)
        self.assertEqual(p.exitStatus(), QProcess.ExitStatus.NormalExit)
        self.assertEqual(p.processError(), QProcess.ProcessError.FailedToStart)

    def test_process_env(self):
        """
        Test that process inherits system environment correctly
        """
        temp_folder = tempfile.mkdtemp()

        script_file = os.path.join(temp_folder, "process_env.sh")
        with open(script_file, "w") as f:
            f.write("echo $my_var")

        os.chmod(script_file, 0o775)

        def std_out(ba):
            std_out.val += ba.data().decode("UTF-8")

        std_out.val = ""

        def std_err(ba):
            std_err.val += ba.data().decode("UTF-8")

        std_err.val = ""

        # environment variable not set:
        p = QgsBlockingProcess("sh", [script_file])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertEqual(p.run(f), 0)
        self.assertEqual(p.exitStatus(), QProcess.ExitStatus.NormalExit)
        self.assertFalse(QgisTestCase.strip_std_ignorable_errors(std_out.val).strip())
        self.assertFalse(QgisTestCase.strip_std_ignorable_errors(std_err.val).strip())

        # set environment variable
        os.environ["my_var"] = "my test variable"
        std_out.val = ""
        std_err.val = ""
        p = QgsBlockingProcess("sh", [script_file])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertEqual(p.run(f), 0)
        self.assertEqual(p.exitStatus(), QProcess.ExitStatus.NormalExit)
        self.assertEqual(
            QgisTestCase.strip_std_ignorable_errors(std_out.val).strip(),
            "my test variable",
        )
        self.assertFalse(QgisTestCase.strip_std_ignorable_errors(std_err.val).strip())

        # test python changing path

        script_file = os.path.join(temp_folder, "process_env_path.sh")
        with open(script_file, "w") as f:
            f.write("echo $PATH")

        prev_path_val = os.getenv("PATH")
        new_path = f"/my_test/folder{os.pathsep}{prev_path_val}"
        os.environ["PATH"] = new_path

        std_out.val = ""
        std_err.val = ""
        p = QgsBlockingProcess("sh", [script_file])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertEqual(p.run(f), 0)
        self.assertEqual(p.exitStatus(), QProcess.ExitStatus.NormalExit)
        self.assertEqual(
            QgisTestCase.strip_std_ignorable_errors(std_out.val).strip(), new_path
        )
        self.assertFalse(QgisTestCase.strip_std_ignorable_errors(std_err.val).strip())


if __name__ == "__main__":
    unittest.main()
