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
# The minimum coverage of public/protected member functions in QGIS api
#
# DON'T LOWER THIS THRESHOLD UNLESS MEMBERS HAVE BEEN REMOVED FROM THE API
# (changes which raise this threshold are welcomed though!)

ACCEPTABLE_COVERAGE = 54.6134


def elemIsDocumentableClass(elem):
    if not elem.get('kind') == 'class':
        return False

    #public or protected classes should be documented
    return elem.get('prot') in ('public','protected')

def elemIsDocumentableMember(elem):
    if elem.get('kind') == 'variable':
        return False

    #only public or protected members should be documented
    if not elem.get('prot') in ('public','protected'):
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
    if name is not None and name.text and name.text.startswith('~'):
        return False

    return True


def memberIsDocumented(m):
    for doc_type in ('inbodydescription','briefdescription','detaileddescription'):
        doc = m.find(doc_type)
        if doc is not None and list(doc):
            return True
    return False

def parseClassElem(e):
    documentable_members = 0
    documented_members = 0
    for m in e.getiterator('memberdef'):
        if elemIsDocumentableMember(m):
            documentable_members += 1
            if memberIsDocumented(m):
                documented_members += 1
    return documentable_members, documented_members

def parseFile(f):
    documentable_members = 0
    documented_members = 0
    for event, elem in ET.iterparse(f):
        if event == 'end' and elem.tag == 'compounddef':
            if elemIsDocumentableClass(elem):
                members, documented = parseClassElem(elem)
                documentable_members += members
                documented_members += documented
            elem.clear()
    return documentable_members, documented_members


def parseDocs(path):
    documentable_members = 0
    documented_members = 0
    for f in glob.glob(os.path.join(path,'*.xml')):
        members, documented = parseFile( f )
        documentable_members += members
        documented_members += documented

    return 100.0 * documented_members / documentable_members

class TestQgsDocCoverage(TestCase):

    def testCoverage(self):
        prefixPath = os.environ['QGIS_PREFIX_PATH']
        docPath = os.path.join(prefixPath, '..', 'doc', 'api', 'xml' )

        coverage = parseDocs(docPath)
        print "Documentation coverage {}".format(coverage)

        assert coverage >= ACCEPTABLE_COVERAGE, 'Minimum coverage: %f\nActual coverage: %f\n' % (ACCEPTABLE_COVERAGE, coverage)


if __name__ == '__main__':
    unittest.main()
