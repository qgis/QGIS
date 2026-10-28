"""QGIS Unit tests for QgsAcademicReference

From build dir, run: ctest -R PyQgsAlignmentComboBox -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import Qgis, QgsAcademicReference
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsAcademicReference(QgisTestCase):
    def test_invalid(self):
        ref = QgsAcademicReference()
        self.assertFalse(ref.isValid())
        self.assertEqual(ref.type(), Qgis.AcademicReferenceType.Unknown)

        ref.setType(Qgis.AcademicReferenceType.Book)
        self.assertTrue(ref.isValid())

    def test_getters_setters(self):
        ref = QgsAcademicReference()

        ref.setType(Qgis.AcademicReferenceType.JournalArticle)
        self.assertEqual(ref.type(), Qgis.AcademicReferenceType.JournalArticle)

        ref.setAuthors(["Smith, J.", "Doe, J."])
        self.assertEqual(ref.authors(), ["Smith, J.", "Doe, J."])

        ref.setYear(2026)
        self.assertEqual(ref.year(), 2026)

        ref.setTitle("A Test Article")
        self.assertEqual(ref.title(), "A Test Article")

        ref.setJournal("Journal of Testing")
        self.assertEqual(ref.journal(), "Journal of Testing")

        ref.setVolume("12")
        self.assertEqual(ref.volume(), "12")

        ref.setIssue("4")
        self.assertEqual(ref.issue(), "4")

        ref.setPages("100-110")
        self.assertEqual(ref.pages(), "100-110")

        ref.setPublisher("Test Press")
        self.assertEqual(ref.publisher(), "Test Press")

        ref.setUrl("https://example.com")
        self.assertEqual(ref.url(), "https://example.com")

    def test_factory_methods(self):
        ref_book = QgsAcademicReference.createBook(
            ["Smith, J."], 2020, "Test Book", "Test Publisher"
        )
        self.assertEqual(ref_book.type(), Qgis.AcademicReferenceType.Book)
        self.assertEqual(ref_book.authors(), ["Smith, J."])
        self.assertEqual(ref_book.year(), 2020)
        self.assertEqual(ref_book.title(), "Test Book")
        self.assertEqual(ref_book.publisher(), "Test Publisher")

        ref_journal = QgsAcademicReference.createJournalArticle(
            ["Smith, J.", "Doe, J."],
            2021,
            "Test Article",
            "Test Journal",
            "10",
            "2",
            "1-10",
        )
        self.assertEqual(ref_journal.type(), Qgis.AcademicReferenceType.JournalArticle)
        self.assertEqual(ref_journal.authors(), ["Smith, J.", "Doe, J."])
        self.assertEqual(ref_journal.year(), 2021)
        self.assertEqual(ref_journal.title(), "Test Article")
        self.assertEqual(ref_journal.journal(), "Test Journal")
        self.assertEqual(ref_journal.volume(), "10")
        self.assertEqual(ref_journal.issue(), "2")
        self.assertEqual(ref_journal.pages(), "1-10")

        ref_presentation = QgsAcademicReference.createPresentation(
            ["Smith, J."], 2022, "Test Talk", "Test Conference", "Conf Press", "5"
        )
        self.assertEqual(
            ref_presentation.type(), Qgis.AcademicReferenceType.Presentation
        )
        self.assertEqual(ref_presentation.authors(), ["Smith, J."])
        self.assertEqual(ref_presentation.year(), 2022)
        self.assertEqual(ref_presentation.title(), "Test Talk")
        self.assertEqual(ref_presentation.journal(), "Test Conference")
        self.assertEqual(ref_presentation.publisher(), "Conf Press")
        self.assertEqual(ref_presentation.pages(), "5")

        ref_web = QgsAcademicReference.createWebPage(
            ["Smith, J."], 2023, "Test Web Page", "https://example.org"
        )
        self.assertEqual(ref_web.type(), Qgis.AcademicReferenceType.WebPage)
        self.assertEqual(ref_web.authors(), ["Smith, J."])
        self.assertEqual(ref_web.year(), 2023)
        self.assertEqual(ref_web.title(), "Test Web Page")
        self.assertEqual(ref_web.url(), "https://example.org")

        ref_preprint = QgsAcademicReference.createPreprint(
            ["Smith, J."], 2024, "Test Paper", "arXiv", "https://arxiv.org/abs/123"
        )
        self.assertEqual(ref_preprint.type(), Qgis.AcademicReferenceType.Preprint)
        self.assertEqual(ref_preprint.authors(), ["Smith, J."])
        self.assertEqual(ref_preprint.year(), 2024)
        self.assertEqual(ref_preprint.title(), "Test Paper")
        self.assertEqual(ref_preprint.journal(), "arXiv")
        self.assertEqual(ref_preprint.url(), "https://arxiv.org/abs/123")

    def test_as_plain_text(self):
        ref1 = QgsAcademicReference.createBook(
            ["Smith, J."], 2020, "Title", "Publisher"
        )
        self.assertEqual(ref1.asPlainText(), "Smith, J. (2020). Title. Publisher.")

        ref2 = QgsAcademicReference.createBook(
            ["Smith, J.", "Doe, J."], 2020, "Title", "Publisher"
        )
        self.assertEqual(
            ref2.asPlainText(), "Smith, J. & Doe, J. (2020). Title. Publisher."
        )

        ref3 = QgsAcademicReference.createBook(
            ["Smith, J.", "Doe, J.", "Brown, C."], 2020, "Title", "Publisher"
        )
        self.assertEqual(
            ref3.asPlainText(),
            "Smith, J., Doe, J., & Brown, C. (2020). Title. Publisher.",
        )

        ref_journal = QgsAcademicReference.createJournalArticle(
            ["Smith, J."], 2020, "Title", "Journal", "20", "2", "193-213"
        )
        self.assertEqual(
            ref_journal.asPlainText(),
            "Smith, J. (2020). Title. Journal 20(2), 193-213.",
        )

        ref_web = QgsAcademicReference.createWebPage(
            ["Smith, J."], 2020, "Title", "https://example.com"
        )
        self.assertEqual(
            ref_web.asPlainText(), "Smith, J. (2020). Title. https://example.com"
        )

    def test_as_html(self):
        ref_journal = QgsAcademicReference.createJournalArticle(
            ["Smith, J."], 2020, "Title", "Journal", "20", "2", "193-213"
        )
        self.assertEqual(
            ref_journal.asHtml(),
            "Smith, J. (2020). Title. <i>Journal</i> <i>20</i>(2), 193-213.",
        )

        ref_web = QgsAcademicReference.createWebPage(
            ["Smith, J."], 2020, "Title", "https://example.com"
        )
        self.assertEqual(
            ref_web.asHtml(),
            'Smith, J. (2020). Title. <a href="https://example.com">https://example.com</a>',
        )


if __name__ == "__main__":
    unittest.main()
