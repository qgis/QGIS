# -*- coding: utf-8 -*-
"""QGIS Unit tests for SIP binding coverage.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '15/10/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from utilities import (TestCase,
                       unittest,
                       printImportant,
                       DoxygenParser)

from PyQt4.QtCore import qDebug

#Import all the things!
from qgis.analysis import *
from qgis.core import *
from qgis.gui import *
from qgis.networkanalysis import *
try:
    from qgis.server import *
except:
    pass

# BINDING THRESHOLD
#
# The minimum number of unbound functions in QGIS api
#
# DON'T RAISE THIS THRESHOLD!!!
# (changes which lower this threshold are welcomed though!)

ACCEPTABLE_MISSING_CLASSES = 198
ACCEPTABLE_MISSING_MEMBERS = 530


class TestQgsSipCoverage(TestCase):

    def testCoverage(self):
        print 'CTEST_FULL_OUTPUT'
        prefixPath = os.environ['QGIS_PREFIX_PATH']
        docPath = os.path.join(prefixPath, '..', 'doc', 'api', 'xml')
        parser = DoxygenParser(docPath)

        #first look for objects without any bindings
        objects = set([m[0] for m in parser.bindable_members])
        missing_objects = []
        bound_objects = {}
        for o in objects:
            try:
                bound_objects[o] = globals()[o]
            except:
                missing_objects.append(o)

        missing_objects.sort()

        missing_count = len(missing_objects)
        present_count = len(objects) - missing_count
        coverage = 100.0 * present_count / len(objects)

        print "---------------------------------"
        printImportant("{} total bindable classes".format(len(objects)))
        printImportant("{} total have bindings".format(present_count))
        printImportant("Binding coverage by classes {}%".format(coverage))
        printImportant("---------------------------------")
        printImportant("{} classes missing bindings, out of {} allowed".format(missing_count, ACCEPTABLE_MISSING_CLASSES))
        print "---------------------------------"

        assert missing_count <= ACCEPTABLE_MISSING_CLASSES, """\n\nFAIL: new unbound classes have been introduced, please add SIP bindings for these classes
If these classes are not suitable for the Python bindings, please add the Doxygen tag
"@note not available in Python bindings" to the CLASS Doxygen comments"""

        #next check for individual members
        parser.bindable_members.sort()
        missing_members = []
        for m in parser.bindable_members:
            if m[0] in bound_objects:
                obj = bound_objects[m[0]]
                if not hasattr(obj, m[1]):
                    missing_members.append('{}.{}'.format(m[0], m[1]))

        missing_members.sort()
        missing_count = len(missing_members)
        present_count = len(parser.bindable_members) - missing_count
        coverage = 100.0 * present_count / len(parser.bindable_members)

        print "---------------------------------"
        printImportant("{} total bindable members".format(len(parser.bindable_members)))
        printImportant("{} total have bindings".format(present_count))
        printImportant("Binding coverage by members {}%".format(coverage))
        printImportant("---------------------------------")
        printImportant("{} members missing bindings, out of {} allowed".format(missing_count, ACCEPTABLE_MISSING_MEMBERS))
        print "---------------------------------"
        print 'Missing classes:\n {}'.format('\n '.join(missing_objects))
        print "---------------------------------"
        print 'Missing members:\n {}'.format('\n '.join(missing_members))

        assert missing_count <= ACCEPTABLE_MISSING_MEMBERS, """\n\nFAIL: new unbound members have been introduced, please add SIP bindings for these members
If these members are not suitable for the Python bindings, please add the Doxygen tag
"@note not available in Python bindings" to the MEMBER Doxygen comments"""


if __name__ == '__main__':
    unittest.main()
