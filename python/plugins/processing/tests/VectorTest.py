# -*- coding: utf-8 -*-

"""
***************************************************************************
    VectorTest.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Sandro Santilli
    Email                : strk at kbt dot io
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Sandro Santilli'
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Sandro Santilli'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.testing import start_app, unittest
from processing.tools import vector

import os.path
import errno
import shutil

dataFolder = os.path.join(os.path.dirname(__file__), '../../../../tests/testdata/')
tmpBaseFolder = os.path.join(os.sep, 'tmp', 'qgis_test', str(os.getpid()))


def mkDirP(path):
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise


start_app()


class VectorTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        mkDirP(tmpBaseFolder)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(tmpBaseFolder)
        pass

    def test_ogrLayerName(self):
        tmpdir = os.path.join(tmpBaseFolder, 'ogrLayerName')
        os.mkdir(tmpdir)

        def linkTestfile(f, t):
            os.link(os.path.join(dataFolder, f), os.path.join(tmpdir, t))

        linkTestfile('wkt_data.csv', 'b.csv')
        name = vector.ogrLayerName(tmpdir)
        self.assertEqual(name, 'b')
        linkTestfile('geom_data.csv', 'a.csv')
        name = vector.ogrLayerName(tmpdir)
        self.assertEqual(name, 'a') # alphabetically ordered

        name = vector.ogrLayerName(tmpdir + '|layerid=0')
        self.assertEqual(name, 'a')
        name = vector.ogrLayerName(tmpdir + '|layerid=1')
        self.assertEqual(name, 'b')

        name = vector.ogrLayerName(tmpdir + '|layerid=2')
        self.assertEqual(name, 'invalid-layerid')

        name = vector.ogrLayerName(tmpdir + '|layername=f')
        self.assertEqual(name, 'f') # layername takes precedence

        name = vector.ogrLayerName(tmpdir + '|layerid=0|layername=f2')
        self.assertEqual(name, 'f2') # layername takes precedence

        name = vector.ogrLayerName(tmpdir + '|layername=f2|layerid=0')
        self.assertEqual(name, 'f2') # layername takes precedence


if __name__ == '__main__':
    unittest.main()
