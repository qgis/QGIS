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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtCore
import sys
import os
import glob
import re
import time
# import locale
import shutil
import subprocess
import tempfile

from utilities import unittest, unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsAppStartup(unittest.TestCase):

    TMP_DIR = ''

    @classmethod
    def setUpClass(cls):
        cls.TMP_DIR = tempfile.mkdtemp()
        # print 'TMP_DIR: ' + cls.TMP_DIR
        # subprocess.call(['open', cls.TMP_DIR])

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.TMP_DIR, ignore_errors=True)

    # TODO: refactor parameters to **kwargs to handle all startup combinations
    def doTestStartup(self, option='', testDir='', testFile='',
                      loadPlugins=False, customization=False,
                      timeOut=10, env=None):
        """Run QGIS with the given option. Wait for testFile to be created.
        If time runs out, fail.
        """
        myTestFile = testFile

        # from unicode to local
        if testDir:
            testDir = str(QtCore.QString(testDir).toLocal8Bit())
            if not os.path.exists(testDir):
                os.mkdir(testDir)
            myTestFile = os.path.join(testDir, testFile)

        # print 'myTestFile: ', myTestFile

        if os.path.exists(myTestFile):
            os.remove(myTestFile)

        # whether to load plugins
        plugins = '' if loadPlugins else '--noplugins'

        # whether to enable GUI customization
        customize = '' if customization else '--nocustomization'

        # environnement variables = system variables + provided 'env'
        myenv = os.environ.copy()
        if env is not None:
            myenv.update(env)

        p = subprocess.Popen(
            [QGIS_BIN, "--nologo", plugins, customize, option, testDir],
            env=myenv)

        s = 0
        ok = True
        while not os.path.exists(myTestFile):
            time.sleep(1)
            s += 1
            if s > timeOut:
                ok = False
                break

        p.terminate()
        return ok

    def testOptionsPath(self):
        subdir = 'QGIS'  # Linux
        if sys.platform[:3] == 'dar':  # Mac
            subdir = 'qgis.org'
        ini = os.path.join(subdir, 'QGIS2.ini')
        for p in ['test_opts', 'test opts', u'test_optsé€']:
            assert self.doTestStartup(option="--optionspath",
                                      testDir=os.path.join(self.TMP_DIR, p),
                                      testFile=ini,
                                      timeOut=5), "options path %s" % p

    def testConfigPath(self):
        for p in ['test_config', 'test config', u'test_configé€']:
            assert self.doTestStartup(option="--configpath",
                                      testDir=os.path.join(self.TMP_DIR, p),
                                      testFile="qgis.db",
                                      timeOut=30), "config path %s" % p

    def testPluginPath(self):
        for t in ['test_plugins', 'test plugins', u'test_pluginsé€']:

            # get a unicode test dir
            testDir = os.path.join(self.TMP_DIR, t)

            # copy from testdata
            if not os.path.exists(testDir):
                os.mkdir(testDir)
            test_plug_dir = os.path.join(TEST_DATA_DIR, 'test_plugin_path')
            for item in os.listdir(test_plug_dir):
                shutil.copytree(os.path.join(test_plug_dir, item),
                                os.path.join(testDir, item))

            # we use here a minimal plugin that writes to 'plugin_started.txt'
            # when it is started. if QGIS_PLUGINPATH is correctly parsed, this
            # plugin is executed and the file is created
            assert self.doTestStartup(
                option="--optionspath",
                testDir=testDir,
                testFile="plugin_started.txt",
                timeOut=10,
                env={'QGIS_PLUGINPATH':
                         str(QtCore.QString(testDir).toLocal8Bit())})

    def testPyQgisStartupEnvVar(self):
        # verify PYQGIS_STARTUP env variable file is run by embedded interpreter
        # create a temp python module that writes out test file
        testfile = 'pyqgis_startup.txt'
        testfilepath = os.path.join(self.TMP_DIR, testfile)
        testcode = [
            "f = open('{0}', 'w')\n".format(testfilepath),
            "f.write('This is a test')\n",
            "f.close()\n"
        ]
        testmod = os.path.join(self.TMP_DIR, 'pyqgis_startup.py')
        f = open(testmod, 'w')
        f.writelines(testcode)
        f.close()
        msg = 'Failed to create test file from executing PYQGIS_STARTUP file'
        assert self.doTestStartup(
            testFile=testfilepath,
            timeOut=10,
            env={'PYQGIS_STARTUP': testmod}), msg


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
                m = re.search('/(QGIS(_\d\.\d-dev)?)\.app', app_path)
                if m:
                    QGIS_BIN = app_path + '/Contents/MacOS/' + m.group(1)
                    found = True
                    break
            if found:
                break

    print '\nQGIS_BIN: ', QGIS_BIN
    assert QGIS_BIN, 'QGIS binary not found, skipping test suite'
    unittest.main()
