# -*- coding: utf-8 -*-
'''
test_analysis.py
                     --------------------------------------
               Date                 : September 2012
               Copyright            : (C) 2012 by Tim Sutton
               email                : tim@linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
'''

import qgis  # NOQA

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsZonalStatistics(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    #@unittest.expectedFailure
    def testSubstitutionMap(self):
        """Test that we can import zonal statistics was failing as of d5f6543
        """
        try:
            from qgis.analysis import QgsZonalStatistics  # NOQA
        except ImportError:
            self.fail('Failed to import zonal statistics python module')


if __name__ == '__main__':
    unittest.main()
