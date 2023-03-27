"""QGIS Unit tests for QgsStringUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '30/08/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsStringUtils,
                       QgsStringReplacement,
                       QgsStringReplacementCollection
                       )
from qgis.testing import unittest


class PyQgsStringReplacement(unittest.TestCase):

    def testBasic(self):
        """ basic tests for QgsStringReplacement"""
        r = QgsStringReplacement('match', 'replace')
        self.assertEqual(r.match(), 'match')
        self.assertEqual(r.replacement(), 'replace')

        r = QgsStringReplacement('match', 'replace', True, True)
        self.assertTrue(r.wholeWordOnly())
        self.assertTrue(r.caseSensitive())

    def testReplace(self):
        """ test applying replacements"""

        # case insensitive
        r = QgsStringReplacement('match', 'replace', False, False)
        self.assertEqual(r.process('one MaTch only'), 'one replace only')
        self.assertEqual(r.process('more then one MaTch here match two'), 'more then one replace here replace two')
        self.assertEqual(r.process('match start and end MaTch'), 'replace start and end replace')
        self.assertEqual(r.process('no hits'), 'no hits')
        self.assertEqual(r.process('some exmatches here'), 'some exreplacees here')
        self.assertEqual(r.process(''), '')

        # case sensitive
        r = QgsStringReplacement('match', 'replace', True, False)
        self.assertEqual(r.process('one MaTch only'), 'one MaTch only')
        self.assertEqual(r.process('one match only'), 'one replace only')

        # whole word only, case insensitive
        r = QgsStringReplacement('match', 'replace', False, True)
        self.assertEqual(r.process('some exmatches here'), 'some exmatches here')
        self.assertEqual(r.process('some match here'), 'some replace here')
        self.assertEqual(r.process('some exmatches MaTch here'), 'some exmatches replace here')
        self.assertEqual(r.process('some match maTCh here'), 'some replace replace here')
        self.assertEqual(r.process('some -match. here'), 'some -replace. here')
        self.assertEqual(r.process('match here'), 'replace here')
        self.assertEqual(r.process('some match'), 'some replace')

        # whole word only, case sensitive
        r = QgsStringReplacement('match', 'replace', True, True)
        self.assertEqual(r.process('some exmatches here'), 'some exmatches here')
        self.assertEqual(r.process('some match here'), 'some replace here')
        self.assertEqual(r.process('some exmatches MaTch here'), 'some exmatches MaTch here')
        self.assertEqual(r.process('some match maTCh here'), 'some replace maTCh here')

    def testEquality(self):
        """ test equality operator"""
        r1 = QgsStringReplacement('a', 'b', True, True)
        r2 = QgsStringReplacement('a', 'b', True, True)
        self.assertEqual(r1, r2)
        r2 = QgsStringReplacement('c', 'b')
        self.assertNotEqual(r1, r2)
        r2 = QgsStringReplacement('a', 'c')
        self.assertNotEqual(r1, r2)
        r2 = QgsStringReplacement('a', 'b', False, True)
        self.assertNotEqual(r1, r2)
        r2 = QgsStringReplacement('c', 'b', True, False)
        self.assertNotEqual(r1, r2)

    def testSaveRestore(self):
        """ test saving/restoring replacement to map"""
        r1 = QgsStringReplacement('a', 'b', True, True)
        props = r1.properties()
        r2 = QgsStringReplacement.fromProperties(props)
        self.assertEqual(r1, r2)
        r1 = QgsStringReplacement('a', 'b', False, False)
        props = r1.properties()
        r2 = QgsStringReplacement.fromProperties(props)
        self.assertEqual(r1, r2)


class PyQgsStringReplacementCollection(unittest.TestCase):

    def testBasic(self):
        """ basic QgsStringReplacementCollection tests"""
        list = [QgsStringReplacement('aa', '11'),
                QgsStringReplacement('bb', '22')]
        c = QgsStringReplacementCollection(list)
        self.assertEqual(c.replacements(), list)

    def testReplacements(self):
        """ test replacing using collection of replacements """
        c = QgsStringReplacementCollection()
        c.setReplacements([QgsStringReplacement('aa', '11'),
                           QgsStringReplacement('bb', '22')])
        self.assertEqual(c.process('here aa bb is aa string bb'), 'here 11 22 is 11 string 22')
        self.assertEqual(c.process('no matches'), 'no matches')
        self.assertEqual(c.process(''), '')

        # test replacements are done in order
        c.setReplacements([QgsStringReplacement('aa', '11'),
                           QgsStringReplacement('11', '22')])
        self.assertEqual(c.process('string aa'), 'string 22')
        # no replacements
        c.setReplacements([])
        self.assertEqual(c.process('string aa'), 'string aa')

    def testSaveRestore(self):
        """ test saving and restoring collections """
        c = QgsStringReplacementCollection([QgsStringReplacement('aa', '11', False, False),
                                            QgsStringReplacement('bb', '22', True, True)])
        doc = QDomDocument("testdoc")
        elem = doc.createElement("replacements")
        c.writeXml(elem, doc)
        c2 = QgsStringReplacementCollection()
        c2.readXml(elem)
        self.assertEqual(c2.replacements(), c.replacements())


class PyQgsStringUtils(unittest.TestCase):

    def testMixed(self):
        """ test mixed capitalization - ie, no change! """
        self.assertFalse(QgsStringUtils.capitalize(None, QgsStringUtils.MixedCase))
        self.assertEqual(QgsStringUtils.capitalize('', QgsStringUtils.MixedCase), '')
        self.assertEqual(QgsStringUtils.capitalize('testing 123', QgsStringUtils.MixedCase), 'testing 123')
        self.assertEqual(QgsStringUtils.capitalize('    tESTinG 123    ', QgsStringUtils.MixedCase), '    tESTinG 123    ')
        self.assertEqual(QgsStringUtils.capitalize('    TESTING ABC', QgsStringUtils.MixedCase), '    TESTING ABC')

    def testUpperCase(self):
        """ test uppercase """
        self.assertFalse(QgsStringUtils.capitalize(None, QgsStringUtils.AllUppercase))
        self.assertEqual(QgsStringUtils.capitalize('', QgsStringUtils.AllUppercase), '')
        self.assertEqual(QgsStringUtils.capitalize('testing 123', QgsStringUtils.AllUppercase), 'TESTING 123')
        self.assertEqual(QgsStringUtils.capitalize('    tESTinG abc    ', QgsStringUtils.AllUppercase), '    TESTING ABC    ')
        self.assertEqual(QgsStringUtils.capitalize('    TESTING ABC', QgsStringUtils.AllUppercase), '    TESTING ABC')

    def testLowerCase(self):
        """ test lowercase """
        self.assertFalse(QgsStringUtils.capitalize(None, QgsStringUtils.AllLowercase))
        self.assertEqual(QgsStringUtils.capitalize('', QgsStringUtils.AllLowercase), '')
        self.assertEqual(QgsStringUtils.capitalize('testing 123', QgsStringUtils.AllLowercase), 'testing 123')
        self.assertEqual(QgsStringUtils.capitalize('    tESTinG abc    ', QgsStringUtils.AllLowercase),
                         '    testing abc    ')
        self.assertEqual(QgsStringUtils.capitalize('    TESTING ABC', QgsStringUtils.AllLowercase), '    testing abc')

    def testCapitalizeFirst(self):
        """ test capitalize first """
        self.assertFalse(QgsStringUtils.capitalize(None, QgsStringUtils.ForceFirstLetterToCapital))
        self.assertEqual(QgsStringUtils.capitalize('', QgsStringUtils.ForceFirstLetterToCapital), '')
        self.assertEqual(QgsStringUtils.capitalize('testing 123', QgsStringUtils.ForceFirstLetterToCapital), 'Testing 123')
        self.assertEqual(QgsStringUtils.capitalize('testing', QgsStringUtils.ForceFirstLetterToCapital),
                         'Testing')
        self.assertEqual(QgsStringUtils.capitalize('Testing', QgsStringUtils.ForceFirstLetterToCapital),
                         'Testing')
        self.assertEqual(QgsStringUtils.capitalize('TESTING', QgsStringUtils.ForceFirstLetterToCapital),
                         'TESTING')
        self.assertEqual(QgsStringUtils.capitalize('    tESTinG abc    ', QgsStringUtils.ForceFirstLetterToCapital),
                         '    TESTinG Abc    ')
        self.assertEqual(QgsStringUtils.capitalize('    TESTING ABC', QgsStringUtils.ForceFirstLetterToCapital), '    TESTING ABC')
        self.assertEqual(QgsStringUtils.capitalize('    testing abc', QgsStringUtils.ForceFirstLetterToCapital),
                         '    Testing Abc')

    def testfuzzyScore(self):
        self.assertEqual(QgsStringUtils.fuzzyScore('', ''), 0)
        self.assertEqual(QgsStringUtils.fuzzyScore('foo', ''), 0)
        self.assertEqual(QgsStringUtils.fuzzyScore('', 'foo'), 0)
        self.assertEqual(QgsStringUtils.fuzzyScore('foo', 'foo'), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore('bar', 'foo'), 0)
        self.assertEqual(QgsStringUtils.fuzzyScore('FOO', 'foo'), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore('foo', 'FOO'), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore('   foo   ', 'foo'), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore('foo', '   foo   '), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore('foo', '   foo   '), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore('foo_bar', 'foo bar'), 1)
        self.assertGreater(QgsStringUtils.fuzzyScore('foo bar', 'foo'), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore('foo bar', 'fooba'), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore('foo_bar', 'ob'), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore('foo bar', 'foobar'), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore('foo bar', 'foo_bar'), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore('foo_bar', 'foo bar'), 0)
        self.assertEqual(
            QgsStringUtils.fuzzyScore('foo bar', 'foobar'),
            QgsStringUtils.fuzzyScore('foo_bar', 'foobar')
        )
        self.assertEqual(
            QgsStringUtils.fuzzyScore('foo bar', 'foo_bar'),
            QgsStringUtils.fuzzyScore('foo_bar', 'foo_bar')
        )
        self.assertEqual(
            QgsStringUtils.fuzzyScore('foo bar', 'foo bar'),
            QgsStringUtils.fuzzyScore('foo_bar', 'foo bar')
        )
        # note the accent
        self.assertEqual(
            QgsStringUtils.fuzzyScore('foo_bér', 'foober'),
            QgsStringUtils.fuzzyScore('foo_ber', 'foobér')
        )
        self.assertGreater(
            QgsStringUtils.fuzzyScore('abcd efg hig', 'abcd hig'),
            QgsStringUtils.fuzzyScore('abcd efg hig', 'abcd e h')
        )
        #  full words are preferred, even though the same number of characters used
        self.assertGreater(
            QgsStringUtils.fuzzyScore('abcd efg hig', 'abcd hig'),
            QgsStringUtils.fuzzyScore('abcd efg hig', 'abcd e hi')
        )

    def test_truncate_from_middle(self):
        """
        Test QgsStringUtils.truncateMiddleOfString
        """
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('', 0), '')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('', 10), '')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('abcdef', 10), 'abcdef')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('abcdefghij', 10), 'abcdefghij')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('abcdefghijk', 10), 'abcd…ghijk')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('abcdefghijkl', 10), 'abcde…ijkl')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('abcdefghijklmnop', 10), 'abcde…mnop')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('this is a test', 11), 'this … test')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('this is a test', 1), '…')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('this is a test', 2), 't…')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('this is a test', 3), 't…t')
        self.assertEqual(QgsStringUtils.truncateMiddleOfString('this is a test', 0), '…')


if __name__ == '__main__':
    unittest.main()
