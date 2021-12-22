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

import os
import sys
try:
    from qgis.static_testing import unittest
except ImportError:
    import unittest

from termcolor import colored

from doxygen_parser import DoxygenParser
from acceptable_missing_doc import ACCEPTABLE_MISSING_DOCS, ACCEPTABLE_MISSING_ADDED_NOTE, ACCEPTABLE_MISSING_BRIEF

# TO regenerate the list:
# uncomment the lines under the `# GEN LIST` in tests/code_layout/doxygen_parser.py
# $ export PYTHONPATH=build/output/python
# $ export QGIS_PREFIX_PATH=build/output
# $ python tests/code_layout/test_qgsdoccoverage.py
# copy the output to the file:
# tests/code_layout/acceptable_missing_doc.py
# in `ACCEPTABLE_MISSING_DOCS = { <past> }`.


class TestQgsDocCoverage(unittest.TestCase):

    def testCoverage(self):
        print('CTEST_FULL_OUTPUT')
        prefixPath = os.environ['QGIS_PREFIX_PATH']
        docPath = os.path.join(prefixPath, '..', 'doc', 'api', 'xml')
        parser = DoxygenParser(docPath, ACCEPTABLE_MISSING_DOCS, ACCEPTABLE_MISSING_ADDED_NOTE, ACCEPTABLE_MISSING_BRIEF)

        coverage = 100.0 * parser.documented_members / parser.documentable_members
        missing = parser.documentable_members - parser.documented_members

        print("---------------------------------")
        print(("{} total documentable members".format(parser.documentable_members)))
        print(("{} total contain valid documentation".format(parser.documented_members)))
        print(("Total documentation coverage {}%".format(coverage)))
        print("---------------------------------")
        print(("{} members missing documentation".format(missing)))
        print("---------------------------------")
        print("Unacceptable missing documentation:")

        if parser.undocumented_members:
            for cls, props in list(parser.undocumented_members.items()):
                print(('\n\nClass {}, {}/{} members documented\n'.format(colored(cls, 'yellow'), props['documented'], props['members'])))
                for mem in props['missing_members']:
                    print((colored('  "' + mem + '"', 'yellow', attrs=['bold'])))

        if parser.noncompliant_members:
            for cls, props in list(parser.noncompliant_members.items()):
                print(('\n\nClass {}, non-compliant members found\n'.format(colored(cls, 'yellow'))))
                for p in props:
                    for mem, error in p.items():
                        print((colored('  ' + mem + ': ' + error, 'yellow', attrs=['bold'])))

        if parser.broken_links:
            for cls, props in list(parser.broken_links.items()):
                print(('\n\nClass {}, broken see also links found\n'.format(colored(cls, 'yellow'))))
                for member, links in props.items():
                    for l in links:
                        print((colored('  ' + member + ': ' + l, 'yellow', attrs=['bold'])))
        # self.assertEquals(len(parser.undocumented_string), 0, 'FAIL: new undocumented members have been introduced, please add documentation for these members')

        if parser.classes_missing_group:
            print("---------------------------------")
            print('\n')
            print((colored('{} classes have been added without Doxygen group tag ("\\ingroup"):'.format(len(parser.classes_missing_group)), 'yellow')))
            print('')
            print(('  ' + '\n  '.join([colored(cls, 'yellow', attrs=['bold']) for cls in parser.classes_missing_group])))

        if parser.classes_missing_version_added:
            print("---------------------------------")
            print('\n')
            print((colored('{} classes have been added without a version added doxygen note ("\\since QGIS x.xx"):'.format(len(parser.classes_missing_version_added)), 'yellow')))
            print('')
            print(('  ' + '\n  '.join([colored(cls, 'yellow', attrs=['bold']) for cls in parser.classes_missing_version_added])))

        if parser.classes_missing_brief:
            print("---------------------------------")
            print('\n')
            print((colored('{} classes have been added without at least a brief description:'.format(len(parser.classes_missing_brief)), 'yellow')))
            print('')
            print(('  ' + '\n  '.join([colored(cls, 'yellow', attrs=['bold']) for cls in parser.classes_missing_brief])))

        sys.stdout.flush()
        self.assertTrue(not parser.undocumented_members, 'Undocumented members found')
        self.assertTrue(not parser.classes_missing_group, 'Classes without \\group tag found')
        self.assertTrue(not parser.classes_missing_version_added, 'Classes without \\since version tag found')
        self.assertTrue(not parser.classes_missing_brief, 'Classes without \\brief description found')
        self.assertTrue(not parser.noncompliant_members, 'Non compliant members found')
        self.assertTrue(not parser.broken_links, 'Broken links found')


if __name__ == '__main__':
    unittest.main()
