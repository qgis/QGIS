# -*- coding: utf-8 -*-
"""QGIS Unit tests for API documentation coverage.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '01/02/2015'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from qgis.testing import unittest

from utilities import printImportant, DoxygenParser
from acceptable_missing_doc import ACCEPTABLE_MISSING_DOCS, ACCEPTABLE_MISSING_ADDED_NOTE

# TO regenerate the list:
# uncomment the lines under the `# GEN LIST`
# $ export PYTHONPATH=build/output/python
# $ export QGIS_PREFIX_PATH=build/output
# $ python tests/src/python/test_qgsdoccoverage.py
# copy the output to the file:
# tests/src/python/acceptable_missing_doc.py
# in `ACCEPTABLE_MISSING_DOCS = { <past> }`.


class TestQgsDocCoverage(unittest.TestCase):

    def testCoverage(self):
        print('CTEST_FULL_OUTPUT')
        prefixPath = os.environ['QGIS_PREFIX_PATH']
        docPath = os.path.join(prefixPath, '..', 'doc', 'api', 'xml')
        parser = DoxygenParser(docPath, ACCEPTABLE_MISSING_DOCS, ACCEPTABLE_MISSING_ADDED_NOTE)

        coverage = 100.0 * parser.documented_members / parser.documentable_members
        missing = parser.documentable_members - parser.documented_members

        print("---------------------------------")
        printImportant("{} total documentable members".format(parser.documentable_members))
        printImportant("{} total contain valid documentation".format(parser.documented_members))
        printImportant("Total documentation coverage {}%".format(coverage))
        printImportant("---------------------------------")
        printImportant("{} members missing documentation".format(missing))
        print("---------------------------------")
        print("Unacceptable missing documentation:")
        print(parser.undocumented_string)

        assert len(parser.undocumented_string) == 0, 'FAIL: new undocumented members have been introduced, please add documentation for these members'

        self.assertTrue(len(parser.classes_missing_group) == 0, 'FAIL: {} classes have been added without Doxygen group tags ("\ingroup"):\n{}'.format(len(parser.classes_missing_group), '\n'.join(parser.classes_missing_group)))

        self.assertTrue(len(parser.classes_missing_version_added) == 0, 'FAIL: {} classes have been added without a version added doxygen note ("@note added in QGIS x.xx"):\n{}'.format(len(parser.classes_missing_version_added), '\n'.join(parser.classes_missing_version_added)))


if __name__ == '__main__':
    unittest.main()
