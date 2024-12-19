"""QGIS Unit tests for QgsPalettedRasterRenderer

From build dir, run:
ctest -R PyQgsPalettedRasterRenderer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os

import numpy as np
from osgeo import gdal
from qgis.PyQt.QtCore import QFileInfo, QTemporaryDir
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsColorRampShader,
    QgsGradientColorRamp,
    QgsLimitedRandomColorRamp,
    QgsMapSettings,
    QgsPalettedRasterRenderer,
    QgsRasterLayer
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsPalettedRasterRenderer(QgisTestCase):

    def testPaletted(self):
        """ test paletted raster renderer with raster with color table"""
        path = os.path.join(unitTestDataPath('raster'),
                            'with_color_table.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')

        renderer = QgsPalettedRasterRenderer(layer.dataProvider(), 1,
                                             [QgsPalettedRasterRenderer.Class(1, QColor(0, 255, 0), 'class 2'),
                                              QgsPalettedRasterRenderer.Class(3, QColor(255, 0, 0), 'class 1')])

        self.assertEqual(renderer.nColors(), 2)
        self.assertEqual(renderer.usesBands(), [1])
        self.assertEqual(renderer.inputBand(), 1)

        # test labels
        self.assertEqual(renderer.label(1), 'class 2')
        self.assertEqual(renderer.label(3), 'class 1')
        self.assertFalse(renderer.label(101))

        # test legend symbology - should be sorted by value
        legend = renderer.legendSymbologyItems()
        self.assertEqual(legend[0][0], 'class 2')
        self.assertEqual(legend[1][0], 'class 1')
        self.assertEqual(legend[0][1].name(), '#00ff00')
        self.assertEqual(legend[1][1].name(), '#ff0000')

        # test retrieving classes
        classes = renderer.classes()
        self.assertEqual(classes[0].value, 1)
        self.assertEqual(classes[1].value, 3)
        self.assertEqual(classes[0].label, 'class 2')
        self.assertEqual(classes[1].label, 'class 1')
        self.assertEqual(classes[0].color.name(), '#00ff00')
        self.assertEqual(classes[1].color.name(), '#ff0000')

        # test set label
        # bad index
        renderer.setLabel(1212, 'bad')
        renderer.setLabel(3, 'new class')
        self.assertEqual(renderer.label(3), 'new class')

        # color ramp
        r = QgsLimitedRandomColorRamp(5)
        renderer.setSourceColorRamp(r)
        self.assertEqual(renderer.sourceColorRamp().type(), 'random')
        self.assertEqual(renderer.sourceColorRamp().count(), 5)

        # clone
        new_renderer = renderer.clone()
        classes = new_renderer.classes()
        self.assertEqual(classes[0].value, 1)
        self.assertEqual(classes[1].value, 3)
        self.assertEqual(classes[0].label, 'class 2')
        self.assertEqual(classes[1].label, 'new class')
        self.assertEqual(classes[0].color.name(), '#00ff00')
        self.assertEqual(classes[1].color.name(), '#ff0000')
        self.assertEqual(new_renderer.sourceColorRamp().type(), 'random')
        self.assertEqual(new_renderer.sourceColorRamp().count(), 5)

        # write to xml and read
        doc = QDomDocument('testdoc')
        elem = doc.createElement('qgis')
        renderer.writeXml(doc, elem)
        restored = QgsPalettedRasterRenderer.create(elem.firstChild().toElement(), layer.dataProvider())
        self.assertTrue(restored)
        self.assertEqual(restored.usesBands(), [1])
        classes = restored.classes()
        self.assertTrue(classes)
        self.assertEqual(classes[0].value, 1)
        self.assertEqual(classes[1].value, 3)
        self.assertEqual(classes[0].label, 'class 2')
        self.assertEqual(classes[1].label, 'new class')
        self.assertEqual(classes[0].color.name(), '#00ff00')
        self.assertEqual(classes[1].color.name(), '#ff0000')
        self.assertEqual(restored.sourceColorRamp().type(), 'random')
        self.assertEqual(restored.sourceColorRamp().count(), 5)

        # render test
        layer.setRenderer(renderer)
        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'paletted_renderer',
                'paletted_renderer',
                ms)
        )

    def testPalettedBandInvalidLayer(self):
        """ test paletted raster render band with a broken layer path"""
        renderer = QgsPalettedRasterRenderer(None, 2,
                                             [QgsPalettedRasterRenderer.Class(137, QColor(0, 255, 0), 'class 2'),
                                              QgsPalettedRasterRenderer.Class(138, QColor(255, 0, 0), 'class 1'),
                                              QgsPalettedRasterRenderer.Class(139, QColor(0, 0, 255), 'class 1')])

        self.assertEqual(renderer.inputBand(), 2)

        # the renderer input is broken, we don't know what bands are valid, so all should be accepted
        self.assertTrue(renderer.setInputBand(10))
        self.assertEqual(renderer.inputBand(), 10)

    def testPalettedBand(self):
        """ test paletted raster render band"""
        path = os.path.join(unitTestDataPath(),
                            'landsat_4326.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')

        renderer = QgsPalettedRasterRenderer(layer.dataProvider(), 2,
                                             [QgsPalettedRasterRenderer.Class(137, QColor(0, 255, 0), 'class 2'),
                                              QgsPalettedRasterRenderer.Class(138, QColor(255, 0, 0), 'class 1'),
                                              QgsPalettedRasterRenderer.Class(139, QColor(0, 0, 255), 'class 1')])

        self.assertEqual(renderer.inputBand(), 2)
        self.assertFalse(renderer.setInputBand(0))
        self.assertEqual(renderer.inputBand(), 2)
        self.assertFalse(renderer.setInputBand(10))
        self.assertEqual(renderer.inputBand(), 2)
        self.assertTrue(renderer.setInputBand(1))
        self.assertEqual(renderer.inputBand(), 1)
        self.assertTrue(renderer.setInputBand(2))

        layer.setRenderer(renderer)
        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'paletted_renderer_band2',
                'paletted_renderer_band2',
                ms)
        )

        renderer = QgsPalettedRasterRenderer(layer.dataProvider(), 3,
                                             [QgsPalettedRasterRenderer.Class(120, QColor(0, 255, 0), 'class 2'),
                                              QgsPalettedRasterRenderer.Class(123, QColor(255, 0, 0), 'class 1'),
                                              QgsPalettedRasterRenderer.Class(124, QColor(0, 0, 255), 'class 1')])

        layer.setRenderer(renderer)
        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'paletted_renderer_band3',
                'paletted_renderer_band3',
                ms)
        )

    def testPalettedColorTableToClassData(self):
        entries = [QgsColorRampShader.ColorRampItem(5, QColor(255, 0, 0), 'item1'),
                   QgsColorRampShader.ColorRampItem(3, QColor(0, 255, 0), 'item2'),
                   QgsColorRampShader.ColorRampItem(6, QColor(0, 0, 255), 'item3'),
                   ]
        classes = QgsPalettedRasterRenderer.colorTableToClassData(entries)
        self.assertEqual(classes[0].value, 5)
        self.assertEqual(classes[1].value, 3)
        self.assertEqual(classes[2].value, 6)
        self.assertEqual(classes[0].label, 'item1')
        self.assertEqual(classes[1].label, 'item2')
        self.assertEqual(classes[2].label, 'item3')
        self.assertEqual(classes[0].color.name(), '#ff0000')
        self.assertEqual(classes[1].color.name(), '#00ff00')
        self.assertEqual(classes[2].color.name(), '#0000ff')

        # test #13263
        path = os.path.join(unitTestDataPath('raster'),
                            'hub13263.vrt')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')
        classes = QgsPalettedRasterRenderer.colorTableToClassData(layer.dataProvider().colorTable(1))
        self.assertEqual(len(classes), 4)
        classes = QgsPalettedRasterRenderer.colorTableToClassData(layer.dataProvider().colorTable(15))
        self.assertEqual(len(classes), 256)

    def testLoadPalettedColorDataFromString(self):
        """
        Test interpreting a bunch of color data format strings
        """
        esri_clr_format = '1 255 255 0\n2 64 0 128\n3 255 32 32\n4 0 255 0\n5 0 0 255'
        esri_clr_format_win = '1 255 255 0\r\n2 64 0 128\r\n3 255 32 32\r\n4 0 255 0\r\n5 0 0 255'
        esri_clr_format_tab = '1\t255\t255\t0\n2\t64\t0\t128\n3\t255\t32\t32\n4\t0\t255\t0\n5\t0\t0\t255'
        esri_clr_spaces = '1    255    255    0\n2    64    0    128\n3    255   32    32\n4   0   255   0\n5   0   0   255'
        gdal_clr_comma = '1,255,255,0\n2,64,0,128\n3,255,32,32\n4,0,255,0\n5,0,0,255'
        gdal_clr_colon = '1:255:255:0\n2:64:0:128\n3:255:32:32\n4:0:255:0\n5:0:0:255'
        for f in [esri_clr_format,
                  esri_clr_format_win,
                  esri_clr_format_tab,
                  esri_clr_spaces,
                  gdal_clr_comma,
                  gdal_clr_colon]:
            classes = QgsPalettedRasterRenderer.classDataFromString(f)
            self.assertEqual(len(classes), 5)
            self.assertEqual(classes[0].value, 1)
            self.assertEqual(classes[0].color.name(), '#ffff00')
            self.assertEqual(classes[1].value, 2)
            self.assertEqual(classes[1].color.name(), '#400080')
            self.assertEqual(classes[2].value, 3)
            self.assertEqual(classes[2].color.name(), '#ff2020')
            self.assertEqual(classes[3].value, 4)
            self.assertEqual(classes[3].color.name(), '#00ff00')
            self.assertEqual(classes[4].value, 5)
            self.assertEqual(classes[4].color.name(), '#0000ff')

        grass_named_colors = '0 white\n1 yellow\n3 black\n6 blue\n9 magenta\n11 aqua\n13 grey\n14 gray\n15 orange\n19 brown\n21 purple\n22 violet\n24 indigo\n90 green\n180 cyan\n270 red\n'
        classes = QgsPalettedRasterRenderer.classDataFromString(grass_named_colors)
        self.assertEqual(len(classes), 16)
        self.assertEqual(classes[0].value, 0)
        self.assertEqual(classes[0].color.name(), '#ffffff')
        self.assertEqual(classes[1].value, 1)
        self.assertEqual(classes[1].color.name(), '#ffff00')
        self.assertEqual(classes[2].value, 3)
        self.assertEqual(classes[2].color.name(), '#000000')
        self.assertEqual(classes[3].value, 6)
        self.assertEqual(classes[3].color.name(), '#0000ff')
        self.assertEqual(classes[4].value, 9)
        self.assertEqual(classes[4].color.name(), '#ff00ff')
        self.assertEqual(classes[5].value, 11)
        self.assertEqual(classes[5].color.name(), '#00ffff')
        self.assertEqual(classes[6].value, 13)
        self.assertEqual(classes[6].color.name(), '#808080')
        self.assertEqual(classes[7].value, 14)
        self.assertEqual(classes[7].color.name(), '#808080')
        self.assertEqual(classes[8].value, 15)
        self.assertEqual(classes[8].color.name(), '#ffa500')
        self.assertEqual(classes[9].value, 19)
        self.assertEqual(classes[9].color.name(), '#a52a2a')
        self.assertEqual(classes[10].value, 21)
        self.assertEqual(classes[10].color.name(), '#800080')
        self.assertEqual(classes[11].value, 22)
        self.assertEqual(classes[11].color.name(), '#ee82ee')
        self.assertEqual(classes[12].value, 24)
        self.assertEqual(classes[12].color.name(), '#4b0082')
        self.assertEqual(classes[13].value, 90)
        self.assertEqual(classes[13].color.name(), '#008000')
        self.assertEqual(classes[14].value, 180)
        self.assertEqual(classes[14].color.name(), '#00ffff')
        self.assertEqual(classes[15].value, 270)
        self.assertEqual(classes[15].color.name(), '#ff0000')

        gdal_alpha = '1:255:255:0:0\n2:64:0:128:50\n3:255:32:32:122\n4:0:255:0:200\n5:0:0:255:255'
        classes = QgsPalettedRasterRenderer.classDataFromString(gdal_alpha)
        self.assertEqual(len(classes), 5)
        self.assertEqual(classes[0].value, 1)
        self.assertEqual(classes[0].color.name(), '#ffff00')
        self.assertEqual(classes[0].color.alpha(), 0)
        self.assertEqual(classes[1].value, 2)
        self.assertEqual(classes[1].color.name(), '#400080')
        self.assertEqual(classes[1].color.alpha(), 50)
        self.assertEqual(classes[2].value, 3)
        self.assertEqual(classes[2].color.name(), '#ff2020')
        self.assertEqual(classes[2].color.alpha(), 122)
        self.assertEqual(classes[3].value, 4)
        self.assertEqual(classes[3].color.name(), '#00ff00')
        self.assertEqual(classes[3].color.alpha(), 200)
        self.assertEqual(classes[4].value, 5)
        self.assertEqual(classes[4].color.name(), '#0000ff')
        self.assertEqual(classes[4].color.alpha(), 255)

        # qgis style, with labels
        qgis_style = '3 255 0 0 255 class 1\n4 0 255 0 200 class 2'
        classes = QgsPalettedRasterRenderer.classDataFromString(qgis_style)
        self.assertEqual(len(classes), 2)
        self.assertEqual(classes[0].value, 3)
        self.assertEqual(classes[0].color.name(), '#ff0000')
        self.assertEqual(classes[0].color.alpha(), 255)
        self.assertEqual(classes[0].label, 'class 1')
        self.assertEqual(classes[1].value, 4)
        self.assertEqual(classes[1].color.name(), '#00ff00')
        self.assertEqual(classes[1].color.alpha(), 200)
        self.assertEqual(classes[1].label, 'class 2')

        # some bad inputs
        bad = ''
        classes = QgsPalettedRasterRenderer.classDataFromString(bad)
        self.assertEqual(len(classes), 0)
        bad = '\n\n\n'
        classes = QgsPalettedRasterRenderer.classDataFromString(bad)
        self.assertEqual(len(classes), 0)
        bad = 'x x x x'
        classes = QgsPalettedRasterRenderer.classDataFromString(bad)
        self.assertEqual(len(classes), 0)
        bad = '1 255 0 0\n2 255 255\n3 255 0 255'
        classes = QgsPalettedRasterRenderer.classDataFromString(bad)
        self.assertEqual(len(classes), 2)
        bad = '1 255 a 0'
        classes = QgsPalettedRasterRenderer.classDataFromString(bad)
        self.assertEqual(len(classes), 1)

    def testLoadPalettedClassDataFromFile(self):
        # bad file
        classes = QgsPalettedRasterRenderer.classDataFromFile('ajdhjashjkdh kjahjkdhk')
        self.assertEqual(len(classes), 0)

        # good file!
        path = os.path.join(unitTestDataPath('raster'),
                            'test.clr')
        classes = QgsPalettedRasterRenderer.classDataFromFile(path)
        self.assertEqual(len(classes), 10)
        self.assertEqual(classes[0].value, 1)
        self.assertEqual(classes[0].color.name(), '#000000')
        self.assertEqual(classes[0].color.alpha(), 255)
        self.assertEqual(classes[1].value, 2)
        self.assertEqual(classes[1].color.name(), '#c8c8c8')
        self.assertEqual(classes[2].value, 3)
        self.assertEqual(classes[2].color.name(), '#006e00')
        self.assertEqual(classes[3].value, 4)
        self.assertEqual(classes[3].color.name(), '#6e4100')
        self.assertEqual(classes[4].value, 5)
        self.assertEqual(classes[4].color.name(), '#0000ff')
        self.assertEqual(classes[4].color.alpha(), 255)
        self.assertEqual(classes[5].value, 6)
        self.assertEqual(classes[5].color.name(), '#0059ff')
        self.assertEqual(classes[6].value, 7)
        self.assertEqual(classes[6].color.name(), '#00aeff')
        self.assertEqual(classes[7].value, 8)
        self.assertEqual(classes[7].color.name(), '#00fff6')
        self.assertEqual(classes[8].value, 9)
        self.assertEqual(classes[8].color.name(), '#eeff00')
        self.assertEqual(classes[9].value, 10)
        self.assertEqual(classes[9].color.name(), '#ffb600')

    def testPalettedClassDataToString(self):
        classes = [QgsPalettedRasterRenderer.Class(1, QColor(0, 255, 0), 'class 2'),
                   QgsPalettedRasterRenderer.Class(3, QColor(255, 0, 0), 'class 1')]
        self.assertEqual(QgsPalettedRasterRenderer.classDataToString(classes),
                         '1 0 255 0 255 class 2\n3 255 0 0 255 class 1')
        # must be sorted by value to work OK in ArcMap
        classes = [QgsPalettedRasterRenderer.Class(4, QColor(0, 255, 0), 'class 2'),
                   QgsPalettedRasterRenderer.Class(3, QColor(255, 0, 0), 'class 1')]
        self.assertEqual(QgsPalettedRasterRenderer.classDataToString(classes),
                         '3 255 0 0 255 class 1\n4 0 255 0 255 class 2')

    def testPalettedClassDataFromLayer(self):
        # no layer
        classes = QgsPalettedRasterRenderer.classDataFromRaster(None, 1)
        self.assertFalse(classes)

        # 10 class layer
        path = os.path.join(unitTestDataPath('raster'),
                            'with_color_table.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer10 = QgsRasterLayer(path, base_name)
        classes = QgsPalettedRasterRenderer.classDataFromRaster(layer10.dataProvider(), 1)
        self.assertEqual(len(classes), 10)
        self.assertEqual(classes[0].value, 1)
        self.assertEqual(classes[0].label, '1')
        self.assertEqual(classes[1].value, 2)
        self.assertEqual(classes[1].label, '2')
        self.assertEqual(classes[2].value, 3)
        self.assertEqual(classes[2].label, '3')
        self.assertEqual(classes[3].value, 4)
        self.assertEqual(classes[3].label, '4')
        self.assertEqual(classes[4].value, 5)
        self.assertEqual(classes[4].label, '5')
        self.assertEqual(classes[5].value, 6)
        self.assertEqual(classes[5].label, '6')
        self.assertEqual(classes[6].value, 7)
        self.assertEqual(classes[6].label, '7')
        self.assertEqual(classes[7].value, 8)
        self.assertEqual(classes[7].label, '8')
        self.assertEqual(classes[8].value, 9)
        self.assertEqual(classes[8].label, '9')
        self.assertEqual(classes[9].value, 10)
        self.assertEqual(classes[9].label, '10')

        # bad band
        self.assertFalse(QgsPalettedRasterRenderer.classDataFromRaster(layer10.dataProvider(), 10101010))

        # with ramp
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))
        classes = QgsPalettedRasterRenderer.classDataFromRaster(layer10.dataProvider(), 1, r)
        self.assertEqual(len(classes), 10)
        self.assertEqual(classes[0].color.name(), '#c80000')
        self.assertEqual(classes[1].color.name(), '#b21600')
        self.assertEqual(classes[2].color.name(), '#9c2c00')
        self.assertIn(classes[3].color.name(), ('#854200', '#854300'))
        self.assertEqual(classes[4].color.name(), '#6f5900')
        self.assertEqual(classes[5].color.name(), '#596f00')
        self.assertIn(classes[6].color.name(), ('#428500', '#438500'))
        self.assertEqual(classes[7].color.name(), '#2c9c00')
        self.assertEqual(classes[8].color.name(), '#16b200')
        self.assertEqual(classes[9].color.name(), '#00c800')

        # 30 class layer
        path = os.path.join(unitTestDataPath('raster'),
                            'unique_1.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer10 = QgsRasterLayer(path, base_name)
        classes = QgsPalettedRasterRenderer.classDataFromRaster(layer10.dataProvider(), 1)
        self.assertEqual(len(classes), 30)
        expected = [11, 21, 22, 24, 31, 82, 2002, 2004, 2014, 2019, 2027, 2029, 2030, 2080, 2081, 2082, 2088, 2092,
                    2097, 2098, 2099, 2105, 2108, 2110, 2114, 2118, 2126, 2152, 2184, 2220]
        self.assertEqual([c.value for c in classes], expected)

        # bad layer
        path = os.path.join(unitTestDataPath('raster'),
                            'hub13263.vrt')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        classes = QgsPalettedRasterRenderer.classDataFromRaster(layer.dataProvider(), 1)
        self.assertFalse(classes)

    def testPalettedRendererWithNegativeColorValue(self):
        """ test paletted raster renderer with negative values in color table"""

        path = os.path.join(unitTestDataPath('raster'),
                            'hub13263.vrt')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')

        renderer = QgsPalettedRasterRenderer(layer.dataProvider(), 1,
                                             [QgsPalettedRasterRenderer.Class(-1, QColor(0, 255, 0), 'class 2'),
                                              QgsPalettedRasterRenderer.Class(3, QColor(255, 0, 0), 'class 1')])

        self.assertEqual(renderer.nColors(), 2)
        self.assertEqual(renderer.usesBands(), [1])

    def testPalettedRendererWithFloats(self):
        """Tests for https://github.com/qgis/QGIS/issues/39058"""

        tempdir = QTemporaryDir()
        temppath = os.path.join(tempdir.path(), 'paletted.tif')

        # Create a float raster with unique values up to 65536 + one extra row
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 256, 256 + 1, 1, gdal.GDT_Float32)
        outband = outRaster.GetRasterBand(1)
        data = []
        for r in range(256 + 1):
            data.append(list(range(r * 256, (r + 1) * 256)))
        npdata = np.array(data, np.float32)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        layer = QgsRasterLayer(temppath, 'paletted')
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.dataProvider().dataType(1), Qgis.DataType.Float32)
        classes = QgsPalettedRasterRenderer.classDataFromRaster(layer.dataProvider(), 1)
        # Check max classes count, hardcoded in QGIS renderer
        self.assertEqual(len(classes), 65536)
        class_values = []
        for c in classes:
            class_values.append(c.value)
        self.assertEqual(sorted(class_values), list(range(65536)))


if __name__ == '__main__':
    unittest.main()
