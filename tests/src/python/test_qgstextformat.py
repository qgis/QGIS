"""QGIS Unit tests for QgsTextFormat.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Mathieu Pellerin"
__date__ = "2024-10-20"
__copyright__ = "Copyright 2024, The QGIS Project"

from qgis.PyQt.QtGui import QFont
from qgis.PyQt.QtXml import (
    QDomDocument,
    QDomElement,
)
from qgis.core import (
    QgsTextFormat,
    QgsReadWriteContext,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from utilities import getTestFont

start_app()


class PyQgsTextFormat(QgisTestCase):

    def testRestoringAndSavingMissingFont(self):
        # test that a missing font on text format load will still save with the same missing font unless manually changed
        document = QDomDocument()
        document.setContent(
            '<text-style fontFamily="__MISSING__" previewBkgrdColor="255,255,255,255,rgb:1,1,1,1" fieldName="" tabStopDistance="80" tabStopDistanceMapUnitScale="3x:0,0,0,0,0,0" useSubstitutions="0" forcedItalic="0" fontWeight="50" multilineHeightUnit="Percentage" allowHtml="0" fontKerning="1" multilineHeight="1" fontSizeUnit="Point" fontSize="11" blendMode="0" tabStopDistanceUnit="Point" isExpression="1" fontStrikeout="0" fontLetterSpacing="0" fontSizeMapUnitScale="3x:0,0,0,0,0,0" textColor="50,50,50,255,rgb:0.19607843137254902,0.19607843137254902,0.19607843137254902,1" legendString="Aa" textOrientation="horizontal" namedStyle="Regular" fontItalic="0" fontUnderline="0" forcedBold="0" capitalization="0" textOpacity="1" fontWordSpacing="0"><families/><substitutions/></text-style>'
        )

        context = QgsReadWriteContext()
        text_format = QgsTextFormat()
        text_format.readXml(document.documentElement(), context)

        self.assertFalse(text_format.fontFound())
        self.assertTrue(text_format.font().family() != "__MISSING__")

        # when writign the settings to XML, the missing font family should still be there
        element = text_format.writeXml(document, context)
        self.assertEqual(element.attribute("fontFamily"), "__MISSING__")

        font = getTestFont()
        text_format.setFont(font)

        # when writing the settings to XML, the originally missing font family should have been replaced by the new font family
        element = text_format.writeXml(document, context)
        self.assertEqual(element.attribute("fontFamily"), "QGIS Vera Sans")


if __name__ == "__main__":
    unittest.main()
