"""
***************************************************************************
    ScriptUtilsTest
    ---------------------
    Date                 : February 2019
    Copyright            : (C) 2019 by Luigi Pirelli
    Email                : luipir at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Luigi Pirelli"
__date__ = "February 2019"
__copyright__ = "(C) 2019, Luigi Pirelli"

import os
import shutil
import tempfile

from qgis.core import NULL, QgsApplication
import unittest
from qgis.testing import start_app, QgisTestCase

from processing.script import ScriptUtils

testDataPath = os.path.join(os.path.dirname(__file__), "testdata")

start_app()


class ScriptUtilsTest(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testResetScriptFolder(self):
        # if folder exist
        defaultScriptFolder = ScriptUtils.defaultScriptsFolder()
        folder = ScriptUtils.resetScriptFolder(defaultScriptFolder)
        self.assertEqual(folder, defaultScriptFolder)
        folder = ScriptUtils.resetScriptFolder(".")
        self.assertEqual(folder, ".")
        # if folder does not exist and not absolute
        folder = ScriptUtils.resetScriptFolder("fake")
        self.assertEqual(folder, None)
        # if absolute but not relative to QgsApplication.qgisSettingsDirPath()
        folder = os.path.join(tempfile.gettempdir(), "fakePath")
        newFolder = ScriptUtils.resetScriptFolder(folder)
        self.assertEqual(newFolder, folder)

        # if absolute profile but poiting somewhere
        # reset the path as pointing to profile into the current settings
        folder = QgsApplication.qgisSettingsDirPath()

        # modify default profile changing absolute path pointing somewhere
        paths = folder.split(os.sep)
        paths[0] = "/"
        paths[1] = "fakelocation"
        folder = os.path.join(*paths)

        folder = ScriptUtils.resetScriptFolder(folder)
        self.assertEqual(folder, QgsApplication.qgisSettingsDirPath())


if __name__ == "__main__":
    unittest.main()
