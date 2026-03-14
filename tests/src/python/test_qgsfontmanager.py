"""QGIS Unit tests for QgsFontManager

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "15/06/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

import os.path
import tempfile
import unittest

from qgis.core import (
    QgsApplication,
    QgsFontManager,
    QgsReadWriteContext,
    QgsSettings,
    QgsTextFormat,
)
from qgis.PyQt.QtCore import QCoreApplication, QUrl
from qgis.PyQt.QtGui import QFont
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import QgisTestCase, start_app
from utilities import getTestFont, unitTestDataPath

start_app()


class TestQgsFontManager(QgisTestCase):
    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsFontManager.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsFontManager")
        QgsSettings().clear()
        start_app()

    def test_family_replacement(self):
        manager = QgsFontManager()
        self.assertFalse(manager.fontFamilyReplacements())
        self.assertEqual(manager.processFontFamilyName("xxx"), "xxx")

        manager.addFontFamilyReplacement("comic sans", "something better")
        self.assertEqual(
            manager.fontFamilyReplacements(), {"comic sans": "something better"}
        )
        self.assertEqual(manager.processFontFamilyName("xxx"), "xxx")
        self.assertEqual(
            manager.processFontFamilyName("comic sans"), "something better"
        )
        # process font family name should be case insensitive
        self.assertEqual(
            manager.processFontFamilyName("Comic Sans"), "something better"
        )

        # make sure replacements are persisted locally
        manager2 = QgsFontManager()
        self.assertEqual(
            manager2.fontFamilyReplacements(), {"comic sans": "something better"}
        )
        self.assertEqual(manager2.processFontFamilyName("xxx"), "xxx")
        self.assertEqual(
            manager2.processFontFamilyName("comic sans"), "something better"
        )
        self.assertEqual(
            manager2.processFontFamilyName("Comic Sans"), "something better"
        )

        manager.addFontFamilyReplacement("arial", "something else better")
        self.assertEqual(
            manager.fontFamilyReplacements(),
            {"arial": "something else better", "comic sans": "something better"},
        )
        self.assertEqual(manager.processFontFamilyName("xxx"), "xxx")
        self.assertEqual(
            manager.processFontFamilyName("comic sans"), "something better"
        )
        self.assertEqual(
            manager.processFontFamilyName("Comic Sans"), "something better"
        )
        self.assertEqual(
            manager.processFontFamilyName("arial"), "something else better"
        )
        self.assertEqual(
            manager.processFontFamilyName("arIAl"), "something else better"
        )

        manager2 = QgsFontManager()
        self.assertEqual(
            manager2.fontFamilyReplacements(),
            {"arial": "something else better", "comic sans": "something better"},
        )
        self.assertEqual(manager2.processFontFamilyName("xxx"), "xxx")
        self.assertEqual(
            manager2.processFontFamilyName("comic sans"), "something better"
        )
        self.assertEqual(
            manager2.processFontFamilyName("Comic Sans"), "something better"
        )
        self.assertEqual(
            manager2.processFontFamilyName("arial"), "something else better"
        )
        self.assertEqual(
            manager2.processFontFamilyName("arIAl"), "something else better"
        )

        manager.addFontFamilyReplacement("arial", "comic sans")
        self.assertEqual(
            manager.fontFamilyReplacements(),
            {"arial": "comic sans", "comic sans": "something better"},
        )

        self.assertEqual(manager.processFontFamilyName("xxx"), "xxx")
        self.assertEqual(
            manager.processFontFamilyName("comic sans"), "something better"
        )
        self.assertEqual(
            manager.processFontFamilyName("Comic Sans"), "something better"
        )
        self.assertEqual(manager.processFontFamilyName("arial"), "comic sans")
        self.assertEqual(manager.processFontFamilyName("arIAl"), "comic sans")

        manager.addFontFamilyReplacement("arial", "")
        self.assertEqual(
            manager.fontFamilyReplacements(), {"comic sans": "something better"}
        )
        self.assertEqual(manager.processFontFamilyName("xxx"), "xxx")
        self.assertEqual(
            manager.processFontFamilyName("comic sans"), "something better"
        )
        self.assertEqual(
            manager.processFontFamilyName("Comic Sans"), "something better"
        )
        self.assertEqual(manager.processFontFamilyName("arial"), "arial")

        manager.setFontFamilyReplacements(
            {"arial": "something else better2", "comic sans": "something better2"}
        )
        self.assertEqual(
            manager.fontFamilyReplacements(),
            {"arial": "something else better2", "comic sans": "something better2"},
        )
        self.assertEqual(manager.processFontFamilyName("xxx"), "xxx")
        self.assertEqual(
            manager.processFontFamilyName("comic sans"), "something better2"
        )
        self.assertEqual(
            manager.processFontFamilyName("Comic Sans"), "something better2"
        )
        self.assertEqual(
            manager.processFontFamilyName("arial"), "something else better2"
        )
        self.assertEqual(
            manager.processFontFamilyName("arIAl"), "something else better2"
        )

        manager2 = QgsFontManager()
        self.assertEqual(
            manager2.fontFamilyReplacements(),
            {"arial": "something else better2", "comic sans": "something better2"},
        )
        self.assertEqual(manager2.processFontFamilyName("xxx"), "xxx")
        self.assertEqual(
            manager2.processFontFamilyName("comic sans"), "something better2"
        )
        self.assertEqual(
            manager2.processFontFamilyName("Comic Sans"), "something better2"
        )
        self.assertEqual(
            manager2.processFontFamilyName("arial"), "something else better2"
        )
        self.assertEqual(
            manager2.processFontFamilyName("arIAl"), "something else better2"
        )

    def test_replacements(self):
        manager = QgsApplication.fontManager()
        format = QgsTextFormat()
        font = QFont("original family")
        format.setFont(font)

        self.assertEqual(format.font().family(), "original family")

        doc = QDomDocument()
        context = QgsReadWriteContext()
        elem = format.writeXml(doc, context)
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t2 = QgsTextFormat()
        t2.readXml(parent, context)
        self.assertFalse(t2.fontFound())
        self.assertEqual(
            context.takeMessages()[0].message(),
            "Font “original family” not available on system",
        )

        # with a font replacement in place
        test_font = getTestFont()
        manager.addFontFamilyReplacement("original Family", test_font.family())

        t3 = QgsTextFormat()
        t3.readXml(parent, context)
        self.assertTrue(t3.fontFound())
        self.assertEqual(t3.font().family(), "QGIS Vera Sans")

    def test_normal_regular(self):
        manager = QgsFontManager()
        fonts = [
            "Ubuntu-B.ttf",
            "Ubuntu-BI.ttf",
            "Ubuntu-C.ttf",
            "Ubuntu-L.ttf",
            "Ubuntu-LI.ttf",
            "Ubuntu-M.ttf",
            "Ubuntu-MI.ttf",
            "Ubuntu-R.ttf",
            "Ubuntu-RI.ttf",
            "Ubuntu-Th.ttf",
        ]
        spy_installed = QSignalSpy(manager.fontDownloaded)
        for font in fonts:
            manager.downloadAndInstallFont(
                QUrl.fromLocalFile(unitTestDataPath() + "/font/Ubuntu/" + font)
            )
            spy_installed.wait()

        doc = QDomDocument()
        doc.setContent("""<parent><text-style allowHtml="0" blendMode="0" capitalization="0" fontFamily="Ubuntu" fontItalic="0" fontKerning="1" fontLetterSpacing="0" fontSize="39" fontSizeMapUnitScale="3x:0,0,0,0,0,0" fontSizeUnit="Point" fontStrikeout="0" fontUnderline="0" fontWeight="400" fontWordSpacing="0" forcedBold="0" forcedItalic="0" multilineHeight="1" multilineHeightUnit="Percentage" namedStyle="Normal" previewBkgrdColor="255,255,255,255,rgb:1,1,1,1" stretchFactor="100" tabStopDistance="6" tabStopDistanceMapUnitScale="3x:0,0,0,0,0,0" tabStopDistanceUnit="Percentage" textColor="0,0,0,255,rgb:0,0,0,1" textOpacity="1" textOrientation="horizontal">
          <families/>
          <text-buffer bufferBlendMode="0" bufferColor="255,255,255,255,rgb:1,1,1,1" bufferDraw="0" bufferJoinStyle="128" bufferNoFill="1" bufferOpacity="1" bufferSize="1" bufferSizeMapUnitScale="3x:0,0,0,0,0,0" bufferSizeUnits="MM"/>
          <text-mask maskEnabled="0" maskJoinStyle="128" maskOpacity="1" maskSize="1.5" maskSize2="1.5" maskSizeMapUnitScale="3x:0,0,0,0,0,0" maskSizeUnits="MM" maskType="0" maskedSymbolLayers=""/>
          <background shapeBlendMode="0" shapeBorderColor="128,128,128,255,rgb:0.5019608,0.5019608,0.5019608,1" shapeBorderWidth="0" shapeBorderWidthMapUnitScale="3x:0,0,0,0,0,0" shapeBorderWidthUnit="MM" shapeDraw="0" shapeFillColor="255,255,255,255,rgb:1,1,1,1" shapeJoinStyle="64" shapeOffsetMapUnitScale="3x:0,0,0,0,0,0" shapeOffsetUnit="MM" shapeOffsetX="0" shapeOffsetY="0" shapeOpacity="1" shapeRadiiMapUnitScale="3x:0,0,0,0,0,0" shapeRadiiUnit="MM" shapeRadiiX="0" shapeRadiiY="0" shapeRotation="0" shapeRotationType="0" shapeSVGFile="" shapeSizeMapUnitScale="3x:0,0,0,0,0,0" shapeSizeType="0" shapeSizeUnit="MM" shapeSizeX="0" shapeSizeY="0" shapeType="0">
            <symbol alpha="1" clip_to_extent="1" force_rhr="0" frame_rate="10" is_animated="0" name="markerSymbol" type="marker">
              <data_defined_properties>
                <Option type="Map">
                  <Option name="name" type="QString" value=""/>
                  <Option name="properties"/>
                  <Option name="type" type="QString" value="collection"/>
                </Option>
              </data_defined_properties>
              <layer class="SimpleMarker" enabled="1" id="" locked="0" pass="0">
                <Option type="Map">
                  <Option name="angle" type="QString" value="0"/>
                  <Option name="cap_style" type="QString" value="square"/>
                  <Option name="color" type="QString" value="125,139,143,255,rgb:0.4901961,0.5450981,0.5607843,1"/>
                  <Option name="horizontal_anchor_point" type="QString" value="1"/>
                  <Option name="joinstyle" type="QString" value="bevel"/>
                  <Option name="name" type="QString" value="circle"/>
                  <Option name="offset" type="QString" value="0,0"/>
                  <Option name="offset_map_unit_scale" type="QString" value="3x:0,0,0,0,0,0"/>
                  <Option name="offset_unit" type="QString" value="MM"/>
                  <Option name="outline_color" type="QString" value="35,35,35,255,rgb:0.1372549,0.1372549,0.1372549,1"/>
                  <Option name="outline_style" type="QString" value="solid"/>
                  <Option name="outline_width" type="QString" value="0"/>
                  <Option name="outline_width_map_unit_scale" type="QString" value="3x:0,0,0,0,0,0"/>
                  <Option name="outline_width_unit" type="QString" value="MM"/>
                  <Option name="scale_method" type="QString" value="diameter"/>
                  <Option name="size" type="QString" value="2"/>
                  <Option name="size_map_unit_scale" type="QString" value="3x:0,0,0,0,0,0"/>
                  <Option name="size_unit" type="QString" value="MM"/>
                  <Option name="vertical_anchor_point" type="QString" value="1"/>
                </Option>
                <data_defined_properties>
                  <Option type="Map">
                    <Option name="name" type="QString" value=""/>
                    <Option name="properties"/>
                    <Option name="type" type="QString" value="collection"/>
                  </Option>
                </data_defined_properties>
              </layer>
            </symbol>
            <symbol alpha="1" clip_to_extent="1" force_rhr="0" frame_rate="10" is_animated="0" name="fillSymbol" type="fill">
              <data_defined_properties>
                <Option type="Map">
                  <Option name="name" type="QString" value=""/>
                  <Option name="properties"/>
                  <Option name="type" type="QString" value="collection"/>
                </Option>
              </data_defined_properties>
              <layer class="SimpleFill" enabled="1" id="" locked="0" pass="0">
                <Option type="Map">
                  <Option name="border_width_map_unit_scale" type="QString" value="3x:0,0,0,0,0,0"/>
                  <Option name="color" type="QString" value="255,255,255,255,rgb:1,1,1,1"/>
                  <Option name="joinstyle" type="QString" value="bevel"/>
                  <Option name="offset" type="QString" value="0,0"/>
                  <Option name="offset_map_unit_scale" type="QString" value="3x:0,0,0,0,0,0"/>
                  <Option name="offset_unit" type="QString" value="MM"/>
                  <Option name="outline_color" type="QString" value="128,128,128,255,rgb:0.5019608,0.5019608,0.5019608,1"/>
                  <Option name="outline_style" type="QString" value="no"/>
                  <Option name="outline_width" type="QString" value="0"/>
                  <Option name="outline_width_unit" type="QString" value="MM"/>
                  <Option name="style" type="QString" value="solid"/>
                </Option>
                <data_defined_properties>
                  <Option type="Map">
                    <Option name="name" type="QString" value=""/>
                    <Option name="properties"/>
                    <Option name="type" type="QString" value="collection"/>
                  </Option>
                </data_defined_properties>
              </layer>
            </symbol>
          </background>
          <shadow shadowBlendMode="6" shadowColor="0,0,0,255,rgb:0,0,0,1" shadowDraw="0" shadowOffsetAngle="135" shadowOffsetDist="1" shadowOffsetGlobal="1" shadowOffsetMapUnitScale="3x:0,0,0,0,0,0" shadowOffsetUnit="MM" shadowOpacity="0.69999999999999996" shadowRadius="1.5" shadowRadiusAlphaOnly="0" shadowRadiusMapUnitScale="3x:0,0,0,0,0,0" shadowRadiusUnit="MM" shadowScale="100" shadowUnder="0"/>
          <dd_properties>
            <Option type="Map">
              <Option name="name" type="QString" value=""/>
              <Option name="properties"/>
              <Option name="type" type="QString" value="collection"/>
            </Option>
          </dd_properties>
        </text-style></parent>""")
        text_format = QgsTextFormat()
        text_format.readXml(doc.documentElement(), QgsReadWriteContext())
        self.assertEqual(text_format.toQFont().styleName(), "Normal")

    def test_install_font(self):
        manager = QgsFontManager()
        with tempfile.TemporaryDirectory() as user_font_dir:
            manager.addUserFontDirectory(user_font_dir)

            spy_installed = QSignalSpy(manager.fontDownloaded)
            spy_failed = QSignalSpy(manager.fontDownloadErrorOccurred)

            manager.downloadAndInstallFont(QUrl.fromLocalFile("xxxx"))
            spy_failed.wait()
            self.assertEqual(len(spy_failed), 1)
            self.assertEqual(len(spy_installed), 0)

            manager.downloadAndInstallFont(
                QUrl.fromLocalFile(unitTestDataPath() + "/fascinate.ttf")
            )
            spy_installed.wait()
            self.assertEqual(len(spy_failed), 1)
            self.assertEqual(len(spy_installed), 1)
            self.assertEqual(spy_installed[0][0], ["Fascinate"])

            self.assertTrue(os.path.exists(os.path.join(user_font_dir, "Fascinate")))
            self.assertEqual(
                manager.userFontToFamilyMap(),
                {os.path.join(user_font_dir, "Fascinate"): ["Fascinate"]},
            )

            manager.removeUserFont(os.path.join(user_font_dir, "Fascinate"))
            self.assertFalse(manager.userFontToFamilyMap())
            self.assertFalse(os.path.exists(os.path.join(user_font_dir, "Fascinate")))

    def test_install_zipped_font(self):
        manager = QgsFontManager()
        with tempfile.TemporaryDirectory() as user_font_dir:
            manager.addUserFontDirectory(user_font_dir)

            spy_installed = QSignalSpy(manager.fontDownloaded)
            spy_failed = QSignalSpy(manager.fontDownloadErrorOccurred)

            manager.downloadAndInstallFont(QUrl.fromLocalFile("xxxx"))
            spy_failed.wait()
            self.assertEqual(len(spy_failed), 1)
            self.assertEqual(len(spy_installed), 0)

            manager.downloadAndInstallFont(
                QUrl.fromLocalFile(unitTestDataPath() + "/zipped_font.zip")
            )
            spy_installed.wait()
            self.assertEqual(len(spy_failed), 1)
            self.assertEqual(len(spy_installed), 1)
            self.assertEqual(spy_installed[0][0], ["Fresca"])
            self.assertTrue(spy_installed[0][1].startswith("Copyright (c) 2011"))

            self.assertTrue(
                os.path.exists(os.path.join(user_font_dir, "Fresca-Regular.ttf"))
            )

            self.assertEqual(
                manager.userFontToFamilyMap(),
                {os.path.join(user_font_dir, "Fresca-Regular.ttf"): ["Fresca"]},
            )

    def test_font_download_urls(self):
        manager = QgsFontManager()
        self.assertEqual(manager.urlForFontDownload("xxx"), ("", ""))
        self.assertFalse(manager.detailsForFontDownload("xxx")[0].isValid())
        self.assertEqual(
            manager.urlForFontDownload("Alegreya SC"),
            (
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-Regular.ttf",
                "Alegreya SC",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("AlegreyaSC"),
            (
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-Regular.ttf",
                "Alegreya SC",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("alegreya_sc"),
            (
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-Regular.ttf",
                "Alegreya SC",
            ),
        )

        self.assertTrue(manager.detailsForFontDownload("Alegreya SC")[0].isValid())
        self.assertEqual(
            manager.detailsForFontDownload("Alegreya SC")[0].fontUrls(),
            [
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-Regular.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-Italic.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-Medium.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-MediumItalic.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-Bold.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-BoldItalic.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-ExtraBold.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-ExtraBoldItalic.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-Black.ttf",
                "https://github.com/google/fonts/raw/main/ofl/alegreyasc/AlegreyaSC-BlackItalic.ttf",
            ],
        )
        self.assertEqual(
            manager.detailsForFontDownload("Alegreya SC")[0].licenseUrl(),
            "https://github.com/google/fonts/raw/main/ofl/alegreyasc/OFL.txt",
        )

        self.assertEqual(
            manager.urlForFontDownload("Roboto"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )

        self.assertEqual(
            manager.urlForFontDownload("Open Sans"),
            (
                "https://github.com/google/fonts/raw/main/ofl/opensans/OpenSans%5Bwdth,wght%5D.ttf",
                "Open Sans",
            ),
        )

        # not available via github?
        # self.assertEqual(manager.urlForFontDownload('Open Sans Condensed'),
        #                 ('https://fonts.google.com/download?family=Open+Sans+Condensed', 'Open Sans Condensed'))

        self.assertEqual(
            manager.urlForFontDownload("Noto Sans"),
            (
                "https://github.com/google/fonts/raw/main/ofl/notosans/NotoSans%5Bwdth,wght%5D.ttf",
                "Noto Sans",
            ),
        )

        self.assertEqual(
            manager.urlForFontDownload("Roboto Condensed"),
            (
                "https://github.com/google/fonts/raw/main/ofl/robotocondensed/RobotoCondensed%5Bwght%5D.ttf",
                "Roboto Condensed",
            ),
        )

        # variants for font names typically seen in vector tile styles
        self.assertEqual(
            manager.urlForFontDownload("RobotoCondensedRegular"),
            (
                "https://github.com/google/fonts/raw/main/ofl/robotocondensed/RobotoCondensed%5Bwght%5D.ttf",
                "Roboto Condensed",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Condensed Regular"),
            (
                "https://github.com/google/fonts/raw/main/ofl/robotocondensed/RobotoCondensed%5Bwght%5D.ttf",
                "Roboto Condensed",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto_Condensed_Regular"),
            (
                "https://github.com/google/fonts/raw/main/ofl/robotocondensed/RobotoCondensed%5Bwght%5D.ttf",
                "Roboto Condensed",
            ),
        )

        # with style names
        self.assertEqual(
            manager.urlForFontDownload("Roboto Black"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Black Italic"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Bold"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Bold Italic"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Italic"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Light"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Light Italic"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Medium"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Medium Italic"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Regular"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Thin"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )
        self.assertEqual(
            manager.urlForFontDownload("Roboto Thin Italic"),
            (
                "https://github.com/google/fonts/raw/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf",
                "Roboto",
            ),
        )


if __name__ == "__main__":
    unittest.main()
