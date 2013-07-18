# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsApplication.

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

from PyQt4 import QtGui, QtCore
from qgis.core import QgsApplication
import sys
import os
import time
import locale
import shutil
import subprocess

from utilities import unittest, getQgisTestApp, unitTestDataPath

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

TEST_DATA_DIR = unitTestDataPath()

class TestPyQgsAppStartup(unittest.TestCase):

    def doTestOptionsPath(self, option, testDir, testFile, timeOut, env = {}):
        """Run QGIS with the given option. Wait for testFile to be created. If time runs out, fail.
        """
        # from unicode to local
        testDir = str(QtCore.QString( testDir ).toLocal8Bit())
        myTestFile = testDir + "/" + testFile

        if os.path.exists( myTestFile ):
            os.remove( myTestFile )

        # environnement variables = system variables + provided 'env'
        myenv = os.environ.copy()
        myenv.update( env )

        p = subprocess.Popen( [ QGIS_BIN, "--nologo", option, testDir ], env = myenv )
        
        s = 0
        ok = True
        while not os.path.exists( myTestFile ):
            time.sleep(1)
            s = s+1
            if s > timeOut:
                ok = False
                break

        p.terminate()

        # remove testDir
        shutil.rmtree( testDir, ignore_errors = True )
        return ok

    def testOptionsPath( self ):
        for p in [ 'test_config', 'test config', 'test_configé€' ]:
            assert self.doTestOptionsPath( "--optionspath", (os.getcwd() + '/' + p).decode('utf-8'), "QGIS/QGIS2.ini", 5 ), "options path %s" % p

    def testConfigPath( self ):
        for p in [ 'test_config', 'test config', 'test_configé€' ]:
            assert self.doTestOptionsPath( "--configpath", (os.getcwd() + '/' + p).decode('utf-8'), "qgis.db", 30 ), "config path %s" % p

    def testPluginPath( self ):
        for t in ['test_plugins', 'test plugins', 'test_pluginsé€' ]:
        
            # get a unicode test dir
            testDir = (os.getcwd() + '/' + t).decode('utf-8')

            # copy from testdata
            shutil.rmtree( testDir, ignore_errors = True )
            shutil.copytree( TEST_DATA_DIR + '/test_plugin_path', testDir )

            # we use here a minimal plugin that writes to 'plugin_started.txt' when it is started
            # if QGIS_PLUGINPATH is correctly parsed, this plugin is executed and the file is created
            assert self.doTestOptionsPath( "--optionspath", testDir, "plugin_started.txt", 10, { 'QGIS_PLUGINPATH' : str(QtCore.QString(testDir).toLocal8Bit()) } )


if __name__ == '__main__':

    # look for qgis bin path
    QGIS_BIN = ''
    prefixPath = os.environ['QGIS_PREFIX_PATH']
    # see qgsapplication.cpp:98
    for f in [ '', '/..', '/bin', '/../../..' ]:
        testDir = prefixPath + f
        if os.path.exists( testDir + '/qgis' ):
            QGIS_BIN = testDir + '/qgis'
            break
        if os.path.exists( testDir + '/qgis.exe' ):
            QGIS_BIN = testDir + '/qgis.exe'
            break
            
    print 'QGIS_BIN =', QGIS_BIN

    unittest.main()

