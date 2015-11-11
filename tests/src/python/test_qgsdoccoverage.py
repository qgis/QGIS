# -*- coding: utf-8 -*-
"""QGIS Unit tests for API documentation coverage.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '01/02/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from utilities import (TestCase,
                       unittest,
                       printImportant,
                       DoxygenParser)

from PyQt4.QtCore import qDebug

# DOCUMENTATION THRESHOLD
#
# The minimum number of undocumented public/protected member functions in QGIS api
#
# DON'T RAISE THIS THRESHOLD!!!
# (changes which lower this threshold are welcomed though!)

ACCEPTABLE_MISSING_DOCS = 3924


class TestQgsDocCoverage(TestCase):

    def testCoverage(self):
        print 'CTEST_FULL_OUTPUT'
        prefixPath = os.environ['QGIS_PREFIX_PATH']
        docPath = os.path.join(prefixPath, '..', 'doc', 'api', 'xml')
        parser = DoxygenParser(docPath)

        coverage = 100.0 * parser.documented_members / parser.documentable_members
        missing = parser.documentable_members - parser.documented_members

        print "---------------------------------"
        printImportant("{} total documentable members".format(parser.documentable_members))
        printImportant("{} total contain valid documentation".format(parser.documented_members))
        printImportant("Total documentation coverage {}%".format(coverage))
        printImportant("---------------------------------")
        printImportant("{} members missing documentation, out of {} allowed".format(missing, ACCEPTABLE_MISSING_DOCS))
        print "---------------------------------"
        print parser.undocumented_string

        assert missing <= ACCEPTABLE_MISSING_DOCS, 'FAIL: new undocumented members have been introduced, please add documentation for these members'

if __name__ == '__main__':
    unittest.main()
