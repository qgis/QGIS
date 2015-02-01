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
import unittest
import sys
import qgis

from utilities import unitTestDataPath, getQgisTestApp

# support python < 2.7 via unittest2
# needed for expected failure decorator
if sys.version_info[0:2] < (2,7):
    try:
        from unittest2 import TestCase, expectedFailure
    except ImportError:
        print "You should install unittest2 to run the salt tests"
        sys.exit(0)
else:
    from unittest import TestCase, expectedFailure

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsZonalStatistics(TestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    #@expectedFailure
    def testSubstitutionMap(self):
        """Test that we can import zonal statistics was failing as of d5f6543
        """
        try:
            from qgis.analysis import QgsZonalStatistics
        except ImportError:
            self.fail('Failed to import zonal statistics python module')


if __name__ == '__main__':
    unittest.main()
