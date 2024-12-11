"""QGIS Unit tests for QgsStringUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "30/08/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsStringReplacement,
    QgsStringReplacementCollection,
    QgsStringUtils,
)
from qgis.testing import unittest


class PyQgsStringReplacement(unittest.TestCase):

    def testBasic(self):
        """basic tests for QgsStringReplacement"""
        r = QgsStringReplacement("match", "replace")
        self.assertEqual(r.match(), "match")
        self.assertEqual(r.replacement(), "replace")

        r = QgsStringReplacement("match", "replace", True, True)
        self.assertTrue(r.wholeWordOnly())
        self.assertTrue(r.caseSensitive())

    def testReplace(self):
        """test applying replacements"""

        # case insensitive
        r = QgsStringReplacement("match", "replace", False, False)
        self.assertEqual(r.process("one MaTch only"), "one replace only")
        self.assertEqual(
            r.process("more then one MaTch here match two"),
            "more then one replace here replace two",
        )
        self.assertEqual(
            r.process("match start and end MaTch"), "replace start and end replace"
        )
        self.assertEqual(r.process("no hits"), "no hits")
        self.assertEqual(r.process("some exmatches here"), "some exreplacees here")
        self.assertEqual(r.process(""), "")

        # case sensitive
        r = QgsStringReplacement("match", "replace", True, False)
        self.assertEqual(r.process("one MaTch only"), "one MaTch only")
        self.assertEqual(r.process("one match only"), "one replace only")

        # whole word only, case insensitive
        r = QgsStringReplacement("match", "replace", False, True)
        self.assertEqual(r.process("some exmatches here"), "some exmatches here")
        self.assertEqual(r.process("some match here"), "some replace here")
        self.assertEqual(
            r.process("some exmatches MaTch here"), "some exmatches replace here"
        )
        self.assertEqual(
            r.process("some match maTCh here"), "some replace replace here"
        )
        self.assertEqual(r.process("some -match. here"), "some -replace. here")
        self.assertEqual(r.process("match here"), "replace here")
        self.assertEqual(r.process("some match"), "some replace")

        # whole word only, case sensitive
        r = QgsStringReplacement("match", "replace", True, True)
        self.assertEqual(r.process("some exmatches here"), "some exmatches here")
        self.assertEqual(r.process("some match here"), "some replace here")
        self.assertEqual(
            r.process("some exmatches MaTch here"), "some exmatches MaTch here"
        )
        self.assertEqual(r.process("some match maTCh here"), "some replace maTCh here")

    def testEquality(self):
        """test equality operator"""
        r1 = QgsStringReplacement("a", "b", True, True)
        r2 = QgsStringReplacement("a", "b", True, True)
        self.assertEqual(r1, r2)
        r2 = QgsStringReplacement("c", "b")
        self.assertNotEqual(r1, r2)
        r2 = QgsStringReplacement("a", "c")
        self.assertNotEqual(r1, r2)
        r2 = QgsStringReplacement("a", "b", False, True)
        self.assertNotEqual(r1, r2)
        r2 = QgsStringReplacement("c", "b", True, False)
        self.assertNotEqual(r1, r2)

    def testSaveRestore(self):
        """test saving/restoring replacement to map"""
        r1 = QgsStringReplacement("a", "b", True, True)
        props = r1.properties()
        r2 = QgsStringReplacement.fromProperties(props)
        self.assertEqual(r1, r2)
        r1 = QgsStringReplacement("a", "b", False, False)
        props = r1.properties()
        r2 = QgsStringReplacement.fromProperties(props)
        self.assertEqual(r1, r2)


class PyQgsStringReplacementCollection(unittest.TestCase):

    def testBasic(self):
        """basic QgsStringReplacementCollection tests"""
        list = [QgsStringReplacement("aa", "11"), QgsStringReplacement("bb", "22")]
        c = QgsStringReplacementCollection(list)
        self.assertEqual(c.replacements(), list)

    def testReplacements(self):
        """test replacing using collection of replacements"""
        c = QgsStringReplacementCollection()
        c.setReplacements(
            [QgsStringReplacement("aa", "11"), QgsStringReplacement("bb", "22")]
        )
        self.assertEqual(
            c.process("here aa bb is aa string bb"), "here 11 22 is 11 string 22"
        )
        self.assertEqual(c.process("no matches"), "no matches")
        self.assertEqual(c.process(""), "")

        # test replacements are done in order
        c.setReplacements(
            [QgsStringReplacement("aa", "11"), QgsStringReplacement("11", "22")]
        )
        self.assertEqual(c.process("string aa"), "string 22")
        # no replacements
        c.setReplacements([])
        self.assertEqual(c.process("string aa"), "string aa")

    def testSaveRestore(self):
        """test saving and restoring collections"""
        c = QgsStringReplacementCollection(
            [
                QgsStringReplacement("aa", "11", False, False),
                QgsStringReplacement("bb", "22", True, True),
            ]
        )
        doc = QDomDocument("testdoc")
        elem = doc.createElement("replacements")
        c.writeXml(elem, doc)
        c2 = QgsStringReplacementCollection()
        c2.readXml(elem)
        self.assertEqual(c2.replacements(), c.replacements())


class PyQgsStringUtils(unittest.TestCase):

    def testMixed(self):
        """test mixed capitalization - ie, no change!"""
        self.assertFalse(
            QgsStringUtils.capitalize(None, QgsStringUtils.Capitalization.MixedCase)
        )
        self.assertEqual(
            QgsStringUtils.capitalize("", QgsStringUtils.Capitalization.MixedCase), ""
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "testing 123", QgsStringUtils.Capitalization.MixedCase
            ),
            "testing 123",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    tESTinG 123    ", QgsStringUtils.Capitalization.MixedCase
            ),
            "    tESTinG 123    ",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    TESTING ABC", QgsStringUtils.Capitalization.MixedCase
            ),
            "    TESTING ABC",
        )

    def testUpperCase(self):
        """test uppercase"""
        self.assertFalse(
            QgsStringUtils.capitalize(None, QgsStringUtils.Capitalization.AllUppercase)
        )
        self.assertEqual(
            QgsStringUtils.capitalize("", QgsStringUtils.Capitalization.AllUppercase),
            "",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "testing 123", QgsStringUtils.Capitalization.AllUppercase
            ),
            "TESTING 123",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    tESTinG abc    ", QgsStringUtils.Capitalization.AllUppercase
            ),
            "    TESTING ABC    ",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    TESTING ABC", QgsStringUtils.Capitalization.AllUppercase
            ),
            "    TESTING ABC",
        )

    def testLowerCase(self):
        """test lowercase"""
        self.assertFalse(
            QgsStringUtils.capitalize(None, QgsStringUtils.Capitalization.AllLowercase)
        )
        self.assertEqual(
            QgsStringUtils.capitalize("", QgsStringUtils.Capitalization.AllLowercase),
            "",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "testing 123", QgsStringUtils.Capitalization.AllLowercase
            ),
            "testing 123",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    tESTinG abc    ", QgsStringUtils.Capitalization.AllLowercase
            ),
            "    testing abc    ",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    TESTING ABC", QgsStringUtils.Capitalization.AllLowercase
            ),
            "    testing abc",
        )

    def testCapitalizeFirst(self):
        """test capitalize first"""
        self.assertFalse(
            QgsStringUtils.capitalize(
                None, QgsStringUtils.Capitalization.ForceFirstLetterToCapital
            )
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "", QgsStringUtils.Capitalization.ForceFirstLetterToCapital
            ),
            "",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "testing 123", QgsStringUtils.Capitalization.ForceFirstLetterToCapital
            ),
            "Testing 123",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "testing", QgsStringUtils.Capitalization.ForceFirstLetterToCapital
            ),
            "Testing",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "Testing", QgsStringUtils.Capitalization.ForceFirstLetterToCapital
            ),
            "Testing",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "TESTING", QgsStringUtils.Capitalization.ForceFirstLetterToCapital
            ),
            "TESTING",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    tESTinG abc    ",
                QgsStringUtils.Capitalization.ForceFirstLetterToCapital,
            ),
            "    TESTinG Abc    ",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    TESTING ABC",
                QgsStringUtils.Capitalization.ForceFirstLetterToCapital,
            ),
            "    TESTING ABC",
        )
        self.assertEqual(
            QgsStringUtils.capitalize(
                "    testing abc",
                QgsStringUtils.Capitalization.ForceFirstLetterToCapital,
            ),
            "    Testing Abc",
        )

    def testfuzzyScore(self):
        self.assertEqual(QgsStringUtils.fuzzyScore("", ""), 0)
        self.assertEqual(QgsStringUtils.fuzzyScore("foo", ""), 0)
        self.assertEqual(QgsStringUtils.fuzzyScore("", "foo"), 0)
        self.assertEqual(QgsStringUtils.fuzzyScore("foo", "foo"), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore("bar", "foo"), 0)
        self.assertEqual(QgsStringUtils.fuzzyScore("FOO", "foo"), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore("foo", "FOO"), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore("   foo   ", "foo"), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore("foo", "   foo   "), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore("foo", "   foo   "), 1)
        self.assertEqual(QgsStringUtils.fuzzyScore("foo_bar", "foo bar"), 1)
        self.assertGreater(QgsStringUtils.fuzzyScore("foo bar", "foo"), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore("foo bar", "fooba"), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore("foo_bar", "ob"), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore("foo bar", "foobar"), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore("foo bar", "foo_bar"), 0)
        self.assertGreater(QgsStringUtils.fuzzyScore("foo_bar", "foo bar"), 0)
        self.assertEqual(
            QgsStringUtils.fuzzyScore("foo bar", "foobar"),
            QgsStringUtils.fuzzyScore("foo_bar", "foobar"),
        )
        self.assertEqual(
            QgsStringUtils.fuzzyScore("foo bar", "foo_bar"),
            QgsStringUtils.fuzzyScore("foo_bar", "foo_bar"),
        )
        self.assertEqual(
            QgsStringUtils.fuzzyScore("foo bar", "foo bar"),
            QgsStringUtils.fuzzyScore("foo_bar", "foo bar"),
        )
        # note the accent
        self.assertEqual(
            QgsStringUtils.fuzzyScore("foo_bér", "foober"),
            QgsStringUtils.fuzzyScore("foo_ber", "foobér"),
        )
        self.assertGreater(
            QgsStringUtils.fuzzyScore("abcd efg hig", "abcd hig"),
            QgsStringUtils.fuzzyScore("abcd efg hig", "abcd e h"),
        )
        #  full words are preferred, even though the same number of characters used
        self.assertGreater(
            QgsStringUtils.fuzzyScore("abcd efg hig", "abcd hig"),
            QgsStringUtils.fuzzyScore("abcd efg hig", "abcd e hi"),
        )

    def test_truncate_from_middle(self):
        """
        Test QgsStringUtils.truncateMiddleOfString
        """
        self.assertEqual(QgsStringUtils.truncateMiddleOfString("", 0), "")
        self.assertEqual(QgsStringUtils.truncateMiddleOfString("", 10), "")
        self.assertEqual(QgsStringUtils.truncateMiddleOfString("abcdef", 10), "abcdef")
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("abcdefghij", 10), "abcdefghij"
        )
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("abcdefghijk", 10), "abcd…ghijk"
        )
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("abcdefghijkl", 10), "abcde…ijkl"
        )
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("abcdefghijklmnop", 10), "abcde…mnop"
        )
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("this is a test", 11), "this … test"
        )
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("this is a test", 1), "…"
        )
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("this is a test", 2), "t…"
        )
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("this is a test", 3), "t…t"
        )
        self.assertEqual(
            QgsStringUtils.truncateMiddleOfString("this is a test", 0), "…"
        )

    def test_contains_by_word(self):
        """
        Test QgsStringUtils.containsByWord
        """
        self.assertTrue(QgsStringUtils.containsByWord("Hello World", "world hello"))
        self.assertTrue(QgsStringUtils.containsByWord("Hello World", "hello\tworld"))
        self.assertTrue(QgsStringUtils.containsByWord("Hello World", "hello"))
        self.assertTrue(QgsStringUtils.containsByWord("Hello World", "world"))
        self.assertFalse(QgsStringUtils.containsByWord("Hello World", "goodbye"))

        # Case insensitive (default)
        self.assertTrue(QgsStringUtils.containsByWord("Hello World", "WORLD"))
        self.assertTrue(QgsStringUtils.containsByWord("HELLO WORLD", "world"))

        # Case sensitive
        self.assertFalse(
            QgsStringUtils.containsByWord(
                "Hello World", "WORLD", Qt.CaseSensitivity.CaseSensitive
            )
        )
        self.assertTrue(
            QgsStringUtils.containsByWord(
                "Hello World", "World", Qt.CaseSensitivity.CaseSensitive
            )
        )

        # Test that parts of words can match
        self.assertTrue(
            QgsStringUtils.containsByWord("Worldmap_Winkel_II", "winkel world")
        )
        self.assertTrue(QgsStringUtils.containsByWord("HelloWorld", "hello world"))
        self.assertTrue(
            QgsStringUtils.containsByWord("SuperCalifragilistic", "super cal fragi")
        )

        # empty strings
        self.assertFalse(QgsStringUtils.containsByWord("Hello World", ""))
        self.assertFalse(QgsStringUtils.containsByWord("Hello World", " "))
        self.assertFalse(QgsStringUtils.containsByWord("", "hello"))
        self.assertFalse(QgsStringUtils.containsByWord(" ", "hello"))
        self.assertFalse(QgsStringUtils.containsByWord("", ""))
        self.assertFalse(QgsStringUtils.containsByWord(" ", ""))
        self.assertFalse(QgsStringUtils.containsByWord(" ", " "))

        self.assertTrue(QgsStringUtils.containsByWord("Hello, World!", "hello world"))
        self.assertTrue(QgsStringUtils.containsByWord("Hello-World", "hello world"))
        self.assertTrue(QgsStringUtils.containsByWord("Hello_World", "hello world"))
        self.assertTrue(QgsStringUtils.containsByWord("Hello\tWorld\n", "hello world"))

        # test multiple words in different orders
        self.assertTrue(
            QgsStringUtils.containsByWord("The Quick Brown Fox", "fox quick")
        )
        self.assertTrue(
            QgsStringUtils.containsByWord("The Quick Brown Fox", "brown the fox")
        )
        self.assertFalse(
            QgsStringUtils.containsByWord("The Quick Brown Fox", "fox quick jumping")
        )

        # test handling of unicode characters"""
        self.assertTrue(QgsStringUtils.containsByWord("École Primaire", "école"))
        self.assertTrue(QgsStringUtils.containsByWord("München Stadt", "münchen"))
        self.assertTrue(QgsStringUtils.containsByWord("北京市", "北京"))

        # test handling of various whitespace scenarios
        self.assertTrue(QgsStringUtils.containsByWord("Hello   World", "hello world"))
        self.assertTrue(QgsStringUtils.containsByWord("Hello\tWorld", "hello world"))
        self.assertTrue(QgsStringUtils.containsByWord("Hello\nWorld", "hello world"))
        self.assertTrue(
            QgsStringUtils.containsByWord("  Hello  World  ", "hello world")
        )

        # Test handling of word boundaries
        self.assertTrue(QgsStringUtils.containsByWord("HelloWorld", "hello"))
        self.assertTrue(QgsStringUtils.containsByWord("WorldHello", "hello"))
        self.assertTrue(QgsStringUtils.containsByWord("TheHelloWorld", "hello world"))


if __name__ == "__main__":
    unittest.main()
