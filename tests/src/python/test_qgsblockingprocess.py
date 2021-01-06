# -*- coding: utf-8 -*-

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

__author__ = 'Nyall Dawson'
__date__ = 'January 2021'
__copyright__ = '(C) 2021, Nyall Dawson'

import qgis  # NOQA

from qgis.core import (
    QgsBlockingProcess,
    QgsFeedback
)

from qgis.testing import unittest, start_app

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

app = start_app()


class TestQgsBlockingProcess(unittest.TestCase):

    def test_process_ok(self):

        def std_out(ba):
            std_out.val += ba.data().decode('UTF-8')

        std_out.val = ''

        def std_err(ba):
            std_err.val += ba.data().decode('UTF-8')

        std_err.val = ''

        p = QgsBlockingProcess('ogrinfo', ['--version'])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertEqual(p.run(f), 0)
        self.assertIn('GDAL', std_out.val)
        self.assertEqual(std_err.val, '')

    def test_process_err(self):

        def std_out(ba):
            std_out.val += ba.data().decode('UTF-8')

        std_out.val = ''

        def std_err(ba):
            std_err.val += ba.data().decode('UTF-8')

        std_err.val = ''

        p = QgsBlockingProcess('ogrinfo', [])
        p.setStdOutHandler(std_out)
        p.setStdErrHandler(std_err)

        f = QgsFeedback()
        self.assertEqual(p.run(f), 1)
        self.assertIn('Usage', std_out.val)
        self.assertIn('FAILURE', std_err.val)


if __name__ == '__main__':
    unittest.main()
