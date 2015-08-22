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
import glob

from utilities import (TestCase,
                       unittest)

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

from PyQt4.QtCore import qDebug

# DOCUMENTATION THRESHOLD
#
# The minimum number of undocumented public/protected member functions in QGIS api
#
# DON'T RAISE THIS THRESHOLD!!!
# (changes which lower this threshold are welcomed though!)

ACCEPTABLE_MISSING_DOCS = 4229


def elemIsDocumentableClass(elem):
    if not elem.get('kind') == 'class':
        return False

    #public or protected classes should be documented
    return elem.get('prot') in ('public', 'protected')


def memberSignature(elem):
    a = elem.find('argsstring')
    try:
        if a is not None:
            return elem.find('name').text + a.text
        else:
            return elem.find('name').text
    except:
        return None


def elemIsDocumentableMember(elem):
    if elem.get('kind') == 'variable':
        return False

    #only public or protected members should be documented
    if not elem.get('prot') in ('public', 'protected'):
        return False

    #ignore reimplemented methods
    #use two different tests, as doxygen will not detect reimplemented qt methods
    if elem.find('reimplements') is not None:
        return False
    args = elem.find('argsstring')
    if args is not None and args.text and ' override' in args.text:
        return False

    #ignore destructor
    name = elem.find('name')
    try:
        if name.text and name.text.startswith('~'):
            return False
    except:
        pass

    #ignore constructors with no arguments
    definition = elem.find('definition')
    argsstring = elem.find('argsstring')
    try:
        if definition.text == '{}::{}'.format(name.text, name.text) and argsstring.text == '()':
            return False
    except:
        pass

    #ignore certain obvious operators
    try:
        if name.text in ('operator=', 'operator=='):
            return False
    except:
        pass

    return True


def memberIsDocumented(m):
    for doc_type in ('inbodydescription', 'briefdescription', 'detaileddescription'):
        doc = m.find(doc_type)
        if doc is not None and list(doc):
            return True
    return False


def parseClassElem(e):
    documentable_members = 0
    documented_members = 0
    undocumented_members = []
    for m in e.getiterator('memberdef'):
        if elemIsDocumentableMember(m):
            documentable_members += 1
            if memberIsDocumented(m):
                documented_members += 1
            else:
                undocumented_members.append(memberSignature(m))
    return documentable_members, documented_members, undocumented_members


def parseFile(f):
    documentable_members = 0
    documented_members = 0
    try:
        for event, elem in ET.iterparse(f):
            if event == 'end' and elem.tag == 'compounddef':
                if elemIsDocumentableClass(elem):
                    members, documented, undocumented = parseClassElem(elem)
                    documentable_members += members
                    documented_members += documented
                    if documented < members:
                        print "Class {}, {}/{} members documented".format(elem.find('compoundname').text, documented, members)
                        for u in undocumented:
                            print ' Missing: {}'.format(u)
                        print "\n"
                elem.clear()
    except ET.ParseError as e:
        #sometimes Doxygen generates malformed xml (eg for < and > operators)
        line_num, col = e.position
        with open(f, 'r') as xml_file:
            for i, l in enumerate(xml_file):
                if i == line_num - 1:
                    line = l
                    break
        caret = '{:=>{}}'.format('^', col)
        print 'ParseError in {}\n{}\n{}\n{}'.format(f, e, line, caret)
    return documentable_members, documented_members


def parseDocs(path):
    documentable_members = 0
    documented_members = 0
    for f in glob.glob(os.path.join(path, '*.xml')):
        members, documented = parseFile(f)
        documentable_members += members
        documented_members += documented

    return documentable_members, documented_members


class TestQgsDocCoverage(TestCase):

    def testCoverage(self):
        print 'CTEST_FULL_OUTPUT'
        prefixPath = os.environ['QGIS_PREFIX_PATH']
        docPath = os.path.join(prefixPath, '..', 'doc', 'api', 'xml')

        documentable, documented = parseDocs(docPath)
        coverage = 100.0 * documented / documentable
        missing = documentable - documented

        print "---------------------------------"
        print "{} total documentable members".format(documentable)
        print "{} total contain valid documentation".format(documented)
        print "Total documentation coverage {}%".format(coverage)
        print "---------------------------------"
        print "{} members missing documentation, out of {} allowed".format(missing, ACCEPTABLE_MISSING_DOCS)

        assert missing <= ACCEPTABLE_MISSING_DOCS, 'FAIL: new undocumented members have been introduced, please add documentation for these members'

if __name__ == '__main__':
    unittest.main()
