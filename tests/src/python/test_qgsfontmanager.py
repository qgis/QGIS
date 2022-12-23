# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFontManager

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '15/06/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os.path

import qgis  # NOQA

from qgis.core import (
    QgsFontManager,
    QgsSettings,
    QgsApplication,
    QgsTextFormat,
    QgsReadWriteContext
)
from qgis.PyQt.QtCore import QCoreApplication, QUrl
from qgis.PyQt.QtGui import QFont
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from utilities import getTestFont
import tempfile

start_app()


class TestQgsFontManager(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsFontManager.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsFontManager")
        QgsSettings().clear()
        start_app()

    def test_family_replacement(self):
        manager = QgsFontManager()
        self.assertFalse(manager.fontFamilyReplacements())
        self.assertEqual(manager.processFontFamilyName('xxx'), 'xxx')

        manager.addFontFamilyReplacement('comic sans', 'something better')
        self.assertEqual(manager.fontFamilyReplacements(), {'comic sans': 'something better'})
        self.assertEqual(manager.processFontFamilyName('xxx'), 'xxx')
        self.assertEqual(manager.processFontFamilyName('comic sans'), 'something better')
        # process font family name should be case insensitive
        self.assertEqual(manager.processFontFamilyName('Comic Sans'), 'something better')

        # make sure replacements are persisted locally
        manager2 = QgsFontManager()
        self.assertEqual(manager2.fontFamilyReplacements(), {'comic sans': 'something better'})
        self.assertEqual(manager2.processFontFamilyName('xxx'), 'xxx')
        self.assertEqual(manager2.processFontFamilyName('comic sans'), 'something better')
        self.assertEqual(manager2.processFontFamilyName('Comic Sans'), 'something better')

        manager.addFontFamilyReplacement('arial', 'something else better')
        self.assertEqual(manager.fontFamilyReplacements(), {'arial': 'something else better', 'comic sans': 'something better'})
        self.assertEqual(manager.processFontFamilyName('xxx'), 'xxx')
        self.assertEqual(manager.processFontFamilyName('comic sans'), 'something better')
        self.assertEqual(manager.processFontFamilyName('Comic Sans'), 'something better')
        self.assertEqual(manager.processFontFamilyName('arial'), 'something else better')
        self.assertEqual(manager.processFontFamilyName('arIAl'), 'something else better')

        manager2 = QgsFontManager()
        self.assertEqual(manager2.fontFamilyReplacements(), {'arial': 'something else better', 'comic sans': 'something better'})
        self.assertEqual(manager2.processFontFamilyName('xxx'), 'xxx')
        self.assertEqual(manager2.processFontFamilyName('comic sans'), 'something better')
        self.assertEqual(manager2.processFontFamilyName('Comic Sans'), 'something better')
        self.assertEqual(manager2.processFontFamilyName('arial'), 'something else better')
        self.assertEqual(manager2.processFontFamilyName('arIAl'), 'something else better')

        manager.addFontFamilyReplacement('arial', 'comic sans')
        self.assertEqual(manager.fontFamilyReplacements(), {'arial': 'comic sans', 'comic sans': 'something better'})

        self.assertEqual(manager.processFontFamilyName('xxx'), 'xxx')
        self.assertEqual(manager.processFontFamilyName('comic sans'), 'something better')
        self.assertEqual(manager.processFontFamilyName('Comic Sans'), 'something better')
        self.assertEqual(manager.processFontFamilyName('arial'), 'comic sans')
        self.assertEqual(manager.processFontFamilyName('arIAl'), 'comic sans')

        manager.addFontFamilyReplacement('arial', '')
        self.assertEqual(manager.fontFamilyReplacements(), {'comic sans': 'something better'})
        self.assertEqual(manager.processFontFamilyName('xxx'), 'xxx')
        self.assertEqual(manager.processFontFamilyName('comic sans'), 'something better')
        self.assertEqual(manager.processFontFamilyName('Comic Sans'), 'something better')
        self.assertEqual(manager.processFontFamilyName('arial'), 'arial')

        manager.setFontFamilyReplacements({'arial': 'something else better2', 'comic sans': 'something better2'})
        self.assertEqual(manager.fontFamilyReplacements(), {'arial': 'something else better2', 'comic sans': 'something better2'})
        self.assertEqual(manager.processFontFamilyName('xxx'), 'xxx')
        self.assertEqual(manager.processFontFamilyName('comic sans'), 'something better2')
        self.assertEqual(manager.processFontFamilyName('Comic Sans'), 'something better2')
        self.assertEqual(manager.processFontFamilyName('arial'), 'something else better2')
        self.assertEqual(manager.processFontFamilyName('arIAl'), 'something else better2')

        manager2 = QgsFontManager()
        self.assertEqual(manager2.fontFamilyReplacements(), {'arial': 'something else better2', 'comic sans': 'something better2'})
        self.assertEqual(manager2.processFontFamilyName('xxx'), 'xxx')
        self.assertEqual(manager2.processFontFamilyName('comic sans'), 'something better2')
        self.assertEqual(manager2.processFontFamilyName('Comic Sans'), 'something better2')
        self.assertEqual(manager2.processFontFamilyName('arial'), 'something else better2')
        self.assertEqual(manager2.processFontFamilyName('arIAl'), 'something else better2')

    def test_replacements(self):
        manager = QgsApplication.fontManager()
        format = QgsTextFormat()
        font = QFont('original family')
        format.setFont(font)

        self.assertEqual(format.font().family(), 'original family')

        doc = QDomDocument()
        context = QgsReadWriteContext()
        elem = format.writeXml(doc, context)
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t2 = QgsTextFormat()
        t2.readXml(parent, context)
        self.assertFalse(t2.fontFound())
        self.assertEqual(context.takeMessages()[0].message(), 'Font “original family” not available on system')

        # with a font replacement in place
        test_font = getTestFont()
        manager.addFontFamilyReplacement('original Family', test_font.family())

        t3 = QgsTextFormat()
        t3.readXml(parent, context)
        self.assertTrue(t3.fontFound())
        self.assertEqual(t3.font().family(), 'QGIS Vera Sans')

    def test_install_font(self):
        manager = QgsFontManager()
        with tempfile.TemporaryDirectory() as user_font_dir:
            manager.addUserFontDirectory(user_font_dir)

            spy_installed = QSignalSpy(manager.fontDownloaded)
            spy_failed = QSignalSpy(manager.fontDownloadErrorOccurred)

            manager.downloadAndInstallFont(QUrl.fromLocalFile('xxxx'))
            spy_failed.wait()
            self.assertEqual(len(spy_failed), 1)
            self.assertEqual(len(spy_installed), 0)

            manager.downloadAndInstallFont(QUrl.fromLocalFile(unitTestDataPath() + '/fascinate.ttf'))
            spy_installed.wait()
            self.assertEqual(len(spy_failed), 1)
            self.assertEqual(len(spy_installed), 1)
            self.assertEqual(spy_installed[0][0], ['Fascinate'])

            self.assertTrue(os.path.exists(os.path.join(user_font_dir, 'Fascinate')))
            self.assertEqual(manager.userFontToFamilyMap(), {os.path.join(user_font_dir, 'Fascinate'): ['Fascinate']})

            manager.removeUserFont(os.path.join(user_font_dir, 'Fascinate'))
            self.assertFalse(manager.userFontToFamilyMap())
            self.assertFalse(os.path.exists(os.path.join(user_font_dir, 'Fascinate')))

    def test_install_zipped_font(self):
        manager = QgsFontManager()
        with tempfile.TemporaryDirectory() as user_font_dir:
            manager.addUserFontDirectory(user_font_dir)

            spy_installed = QSignalSpy(manager.fontDownloaded)
            spy_failed = QSignalSpy(manager.fontDownloadErrorOccurred)

            manager.downloadAndInstallFont(QUrl.fromLocalFile('xxxx'))
            spy_failed.wait()
            self.assertEqual(len(spy_failed), 1)
            self.assertEqual(len(spy_installed), 0)

            manager.downloadAndInstallFont(QUrl.fromLocalFile(unitTestDataPath() + '/zipped_font.zip'))
            spy_installed.wait()
            self.assertEqual(len(spy_failed), 1)
            self.assertEqual(len(spy_installed), 1)
            self.assertEqual(spy_installed[0][0], ['Fresca'])
            self.assertTrue(spy_installed[0][1].startswith('Copyright (c) 2011'))

            self.assertTrue(os.path.exists(os.path.join(user_font_dir, 'Fresca-Regular.ttf')))

            self.assertEqual(manager.userFontToFamilyMap(), {os.path.join(user_font_dir, 'Fresca-Regular.ttf'): ['Fresca']})

    def test_font_download_url(self):
        manager = QgsFontManager()
        self.assertEqual(manager.urlForFontDownload('xxx'), ('', ''))
        self.assertEqual(manager.urlForFontDownload('Alegreya SC'), ('https://fonts.google.com/download?family=Alegreya+SC', 'Alegreya SC'))
        self.assertEqual(manager.urlForFontDownload('AlegreyaSC'), ('https://fonts.google.com/download?family=Alegreya+SC', 'Alegreya SC'))
        self.assertEqual(manager.urlForFontDownload('alegreya_sc'), ('https://fonts.google.com/download?family=Alegreya+SC', 'Alegreya SC'))

        self.assertEqual(manager.urlForFontDownload('Roboto'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))

        self.assertEqual(manager.urlForFontDownload('Open Sans'),
                         ('https://fonts.google.com/download?family=Open+Sans', 'Open Sans'))

        self.assertEqual(manager.urlForFontDownload('Open Sans Condensed'),
                         ('https://fonts.google.com/download?family=Open+Sans+Condensed', 'Open Sans Condensed'))

        self.assertEqual(manager.urlForFontDownload('Noto Sans'),
                         ('https://fonts.google.com/download?family=Noto+Sans', 'Noto Sans'))

        self.assertEqual(manager.urlForFontDownload('Roboto Condensed'), ('https://fonts.google.com/download?family=Roboto+Condensed', 'Roboto Condensed'))

        # variants for font names typically seen in vector tile styles
        self.assertEqual(manager.urlForFontDownload('RobotoCondensedRegular'), ('https://fonts.google.com/download?family=Roboto+Condensed', 'Roboto Condensed'))
        self.assertEqual(manager.urlForFontDownload('Roboto Condensed Regular'), ('https://fonts.google.com/download?family=Roboto+Condensed', 'Roboto Condensed'))
        self.assertEqual(manager.urlForFontDownload('Roboto_Condensed_Regular'),
                         ('https://fonts.google.com/download?family=Roboto+Condensed', 'Roboto Condensed'))

        # with style names
        self.assertEqual(manager.urlForFontDownload('Roboto Black'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Black Italic'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Bold'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Bold Italic'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Italic'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Light'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Light Italic'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Medium'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Medium Italic'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Regular'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Thin'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))
        self.assertEqual(manager.urlForFontDownload('Roboto Thin Italic'),
                         ('https://fonts.google.com/download?family=Roboto', 'Roboto'))


if __name__ == '__main__':
    unittest.main()
