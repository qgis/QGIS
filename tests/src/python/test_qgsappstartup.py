# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsApplication.

From build dir: ctest -R PyQgsAppStartup -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Hugo Mercier (hugo.mercier@oslandia.com)'
__date__ = '17/07/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'

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


class TestPyQgsAppStartup(unittest.TestCase):

    TMP_DIR = ''

    @classmethod
    def setUpClass(cls):
        cls.TMP_DIR = tempfile.mkdtemp()
        # print('TMP_DIR: ' + cls.TMP_DIR)
        # subprocess.call(['open', cls.TMP_DIR])

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.TMP_DIR, ignore_errors=True)

    # TODO: refactor parameters to **kwargs to handle all startup combinations
    def doTestStartup(self, option='', testDir='', testFile='',
                      loadPlugins=False, customization=False,
                      timeOut=360, env=None, additionalArguments=[]):
        """Run QGIS with the given option. Wait for testFile to be created.
        If time runs out, fail.
        """
        myTestFile = testFile

        # from unicode to local
        if testDir:
            if not os.path.exists(testDir):
                os.mkdir(testDir)
            myTestFile = os.path.join(testDir, testFile)

        if os.path.exists(myTestFile):
            os.remove(myTestFile)

        # whether to load plugins
        plugins = '' if loadPlugins else '--noplugins'

        # whether to enable GUI customization
        customize = '' if customization else '--nocustomization'

        # environment variables = system variables + provided 'env'
        myenv = os.environ.copy()
        if env is not None:
            myenv.update(env)

        call = [QGIS_BIN, "--nologo", plugins, customize, option, testDir] + additionalArguments
        p = subprocess.Popen(call, env=myenv)

        s = 0
        while not os.path.exists(myTestFile):
            p.poll()
            if p.returncode is not None:
                raise Exception('Return code: {}, Call: "{}", Env: {}'.format(p.returncode, ' '.join(call), env))
            time.sleep(1)
            s += 1
            if s > timeOut:
                raise Exception('Timed out waiting for application start, Call: "{}", Env: {}'.format(' '.join(call), env))

        with open(myTestFile, 'rt', encoding='utf-8') as res_file:
            lines = res_file.readlines()

        try:
            p.terminate()
        except OSError as e:
            if e.errno != errno.ESRCH:
                raise e

        return lines

    def testPyQgisStartupEnvVar(self):
        # verify PYQGIS_STARTUP env variable file is run by embedded interpreter
        # create a temp python module that writes out test file
        testfile = 'pyqgis_startup.txt'
        testfilepath = os.path.join(self.TMP_DIR, testfile).replace('\\', '/')
        testcode = [
            "from qgis.core import QgsApplication\n"
            "f = open('{0}', 'w')\n".format(testfilepath),
            "f.write('Platform: ' + QgsApplication.platform())\n",
            "f.close()\n"
        ]
        testmod = os.path.join(self.TMP_DIR, 'pyqgis_startup.py').replace('\\', '/')
        f = open(testmod, 'w')
        f.writelines(testcode)
        f.close()
        testfile_lines = self.doTestStartup(
            testFile=testfilepath,
            timeOut=360,
            env={'PYQGIS_STARTUP': testmod})

        # platform should be "Desktop"
        self.assertEqual(testfile_lines, ['Platform: desktop'])

    def testPyArgs(self):
        testfile = 'pyqgis_code.txt'
        testfilepath = os.path.join(self.TMP_DIR, testfile).replace('\\', '/')
        testcode = [
            "import sys\n"
            "f = open('{0}', 'a')\n".format(testfilepath),
            "for arg in sys.argv:\n"
            "  f.write(arg)\n",
            "  f.write('\\n')\n",
            "f.close()\n"
        ]
        testmod = os.path.join(self.TMP_DIR, 'pyqgis_code.py').replace('\\', '/')
        f = open(testmod, 'w')
        f.writelines(testcode)
        f.close()

        testfile_lines = self.doTestStartup(
            testFile=testfilepath,
            timeOut=10,
            additionalArguments=["--code", testmod, "--py-args", "--specialScriptArgument's", 'a "Quoted" text arg', "--"])

        self.assertEqual(testfile_lines, [testmod + '\n',
                                          "--specialScriptArgument's\n",
                                          'a "Quoted" text arg\n'])


if __name__ == '__main__':
    # look for qgis bin path
    QGIS_BIN = ''
    prefixPath = os.environ['QGIS_PREFIX_PATH']
    # see qgsapplication.cpp:98
    for f in ['', '..', 'bin']:
        d = os.path.join(prefixPath, f)
        b = os.path.abspath(os.path.join(d, 'qgis'))
        if os.path.exists(b):
            QGIS_BIN = b
            break
        b = os.path.abspath(os.path.join(d, 'qgis.exe'))
        if os.path.exists(b):
            QGIS_BIN = b
            break
        if sys.platform[:3] == 'dar':  # Mac
            # QGIS.app may be QGIS_x.x-dev.app for nightlies
            # internal binary will match, minus the '.app'
            found = False
            for app_path in glob.glob(d + '/QGIS*.app'):
                m = re.search(r'/(QGIS(_\d\.\d-dev)?)\.app', app_path)
                if m:
                    QGIS_BIN = app_path + '/Contents/MacOS/' + m.group(1)
                    found = True
                    break
            if found:
                break

    print(('\nQGIS_BIN: {}'.format(QGIS_BIN)))
    assert QGIS_BIN, 'QGIS binary not found, skipping test suite'
    unittest.main()
