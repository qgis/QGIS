# -*- coding: utf-8 -*-
"""
***************************************************************************
    test_qgsrasterrenderer_createsld.py
    ---------------------
    Date                 : December 2018
    Copyright            : (C) 2018 by Luigi Pirelli
    Email                : luipir at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *less
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Luigi Pirelli'
__date__ = 'December 2018'
__copyright__ = '(C) 2018, Luigi Pirelli'

import qgis  # NOQA

import os
import random

from qgis.PyQt.QtCore import (
    Qt,
    QDir,
    QFile,
    QIODevice,
    QPointF,
    QSizeF,
    QFileInfo,
)
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QColor, QFont

from qgis.core import (
    QgsRasterLayer,
    QgsRasterRenderer,
    QgsMultiBandColorRenderer,
    QgsSingleBandGrayRenderer,
    QgsPalettedRasterRenderer,
    QgsSingleBandPseudoColorRenderer,
    QgsContrastEnhancement,
    QgsRasterMinMaxOrigin,
    Qgis,
    QgsRasterBandStats,
    QgsRasterShader,
    QgsColorRampShader,
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterRendererCreateSld(unittest.TestCase):

    """
     This class tests the creation of SLD from QGis raster layers
    """

    @classmethod
    def setUpClass(self):
        pass

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)
        myPath = os.path.join(TEST_DATA_DIR, 'landsat.tif')
        rasterFileInfo = QFileInfo(myPath)
        self.raster_layer = QgsRasterLayer(rasterFileInfo.filePath(),
                                           rasterFileInfo.completeBaseName())

    def testSingleBandPseudoColorRenderer_Interpolated(self):
        # get min and max of the band to renderer
        bandNo = 3
        stats = self.raster_layer.dataProvider().bandStatistics(bandNo, QgsRasterBandStats.Min | QgsRasterBandStats.Max)
        minValue = stats.minimumValue
        maxValue = stats.maximumValue
        # create shader for the renderer
        shader = QgsRasterShader(minValue, maxValue)
        colorRampShaderFcn = QgsColorRampShader(minValue, maxValue)
        colorRampShaderFcn.setColorRampType(QgsColorRampShader.Interpolated)
        colorRampShaderFcn.setClassificationMode(QgsColorRampShader.Continuous)
        colorRampShaderFcn.setClip(True)
        items = []
        for index in range(10):
            items.append(QgsColorRampShader.ColorRampItem(index, QColor('#{0:02d}{0:02d}{0:02d}'.format(index)), "{}".format(index)))
        colorRampShaderFcn.setColorRampItemList(items)
        shader.setRasterShaderFunction(colorRampShaderFcn)
        # create instance to test
        rasterRenderer = QgsSingleBandPseudoColorRenderer(self.raster_layer.dataProvider(), bandNo, shader)
        self.raster_layer.setRenderer(rasterRenderer)

        # do test
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        self.assertChannelBand(root, 'sld:GrayChannel', '{}'.format(bandNo))
        # check ColorMapEntry classes
        colorMap = root.elementsByTagName('sld:ColorMap')
        colorMap = colorMap.item(0).toElement()
        self.assertFalse(colorMap.isNull())
        self.assertEqual(colorMap.attribute('type'), 'ramp')
        colorMapEntries = colorMap.elementsByTagName('sld:ColorMapEntry')
        self.assertEqual(colorMapEntries.count(), 10)
        for index in range(colorMapEntries.count()):
            colorMapEntry = colorMapEntries.at(index).toElement()
            self.assertEqual(colorMapEntry.attribute('quantity'), '{}'.format(index))
            self.assertEqual(colorMapEntry.attribute('label'), '{}'.format(index))
            self.assertEqual(colorMapEntry.attribute('opacity'), '')
            self.assertEqual(colorMapEntry.attribute('color'), '#{0:02d}{0:02d}{0:02d}'.format(index))

    def testSingleBandPseudoColorRenderer_Discrete(self):
        # get min and max of the band to renderer
        bandNo = 3
        stats = self.raster_layer.dataProvider().bandStatistics(bandNo, QgsRasterBandStats.Min | QgsRasterBandStats.Max)
        minValue = stats.minimumValue
        maxValue = stats.maximumValue
        # create shader for the renderer
        shader = QgsRasterShader(minValue, maxValue)
        colorRampShaderFcn = QgsColorRampShader(minValue, maxValue)
        colorRampShaderFcn.setColorRampType(QgsColorRampShader.Discrete)
        colorRampShaderFcn.setClassificationMode(QgsColorRampShader.Continuous)
        colorRampShaderFcn.setClip(True)
        items = []
        for index in range(10):
            items.append(QgsColorRampShader.ColorRampItem(index, QColor('#{0:02d}{0:02d}{0:02d}'.format(index)), "{}".format(index)))
        colorRampShaderFcn.setColorRampItemList(items)
        shader.setRasterShaderFunction(colorRampShaderFcn)
        # create instance to test
        rasterRenderer = QgsSingleBandPseudoColorRenderer(self.raster_layer.dataProvider(), bandNo, shader)
        self.raster_layer.setRenderer(rasterRenderer)

        # do test
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        self.assertChannelBand(root, 'sld:GrayChannel', '{}'.format(bandNo))
        # check ColorMapEntry classes
        colorMap = root.elementsByTagName('sld:ColorMap')
        colorMap = colorMap.item(0).toElement()
        self.assertFalse(colorMap.isNull())
        self.assertEqual(colorMap.attribute('type'), 'intervals')
        colorMapEntries = colorMap.elementsByTagName('sld:ColorMapEntry')
        self.assertEqual(colorMapEntries.count(), 10)
        for index in range(colorMapEntries.count()):
            colorMapEntry = colorMapEntries.at(index).toElement()
            self.assertEqual(colorMapEntry.attribute('quantity'), '{}'.format(index))
            self.assertEqual(colorMapEntry.attribute('label'), '{}'.format(index))
            self.assertEqual(colorMapEntry.attribute('opacity'), '')
            self.assertEqual(colorMapEntry.attribute('color'), '#{0:02d}{0:02d}{0:02d}'.format(index))

    def testSingleBandPseudoColorRenderer_Exact(self):
        # get min and max of the band to renderer
        bandNo = 3
        stats = self.raster_layer.dataProvider().bandStatistics(bandNo, QgsRasterBandStats.Min | QgsRasterBandStats.Max)
        minValue = stats.minimumValue
        maxValue = stats.maximumValue
        # create shader for the renderer
        shader = QgsRasterShader(minValue, maxValue)
        colorRampShaderFcn = QgsColorRampShader(minValue, maxValue)
        colorRampShaderFcn.setColorRampType(QgsColorRampShader.Exact)
        colorRampShaderFcn.setClassificationMode(QgsColorRampShader.Continuous)
        colorRampShaderFcn.setClip(True)
        items = []
        for index in range(10):
            items.append(QgsColorRampShader.ColorRampItem(index, QColor('#{0:02d}{0:02d}{0:02d}'.format(index)), "{}".format(index)))
        colorRampShaderFcn.setColorRampItemList(items)
        shader.setRasterShaderFunction(colorRampShaderFcn)
        # create instance to test
        rasterRenderer = QgsSingleBandPseudoColorRenderer(self.raster_layer.dataProvider(), bandNo, shader)
        self.raster_layer.setRenderer(rasterRenderer)

        # do test
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        self.assertChannelBand(root, 'sld:GrayChannel', '{}'.format(bandNo))
        # check ColorMapEntry classes
        colorMap = root.elementsByTagName('sld:ColorMap')
        colorMap = colorMap.item(0).toElement()
        self.assertFalse(colorMap.isNull())
        self.assertEqual(colorMap.attribute('type'), 'values')
        self.assertFalse(colorMap.hasAttribute('extendend'))
        colorMapEntries = colorMap.elementsByTagName('sld:ColorMapEntry')
        self.assertEqual(colorMapEntries.count(), 10)
        for index in range(colorMapEntries.count()):
            colorMapEntry = colorMapEntries.at(index).toElement()
            self.assertEqual(colorMapEntry.attribute('quantity'), '{}'.format(index))
            self.assertEqual(colorMapEntry.attribute('label'), '{}'.format(index))
            self.assertEqual(colorMapEntry.attribute('opacity'), '')
            self.assertEqual(colorMapEntry.attribute('color'), '#{0:02d}{0:02d}{0:02d}'.format(index))

        # add check that is set ColoMap extended="true" if colormap is bigger that 255 entries
        # !NOTE! can't reuse previous shader => segmentation fault
        shader = QgsRasterShader(minValue, maxValue)
        colorRampShaderFcn = QgsColorRampShader(minValue, maxValue)
        colorRampShaderFcn.setColorRampType(QgsColorRampShader.Exact)
        colorRampShaderFcn.setClassificationMode(QgsColorRampShader.Continuous)
        colorRampShaderFcn.setClip(True)
        items = []
        for index in range(255):
            items.append(QgsColorRampShader.ColorRampItem(index, QColor.fromHsv(index, 255, 255, 255), "{}".format(index)))
        colorRampShaderFcn.setColorRampItemList(items)
        shader.setRasterShaderFunction(colorRampShaderFcn)
        # create instance to test
        rasterRenderer = QgsSingleBandPseudoColorRenderer(self.raster_layer.dataProvider(), bandNo, shader)
        # self.raster_layer.setRenderer(rasterRenderer)
        # dom, root = self.rendererToSld(self.raster_layer.renderer())
        # self.assertTrue( colorMap.hasAttribute( 'extendend' ) )
        # self.assertEqual( colorMap.attribute( 'extendend' ), 'true' )

    def testPalettedRasterRenderer(self):
        # create 10 color classes
        #classesString = '122 0 0 0 255 122\n123 1 1 1 255 123\n124 2 2 2 255 124\n125 3 3 3 255 125\n126 4 4 4 255 126\n127 5 5 5 255 127\n128 6 6 6 255 128\n129 7 7 7 255 129\n130 8 8 8 255 130'
        classesString = ''
        for index in range(10):
            classesString += '{0} {0} {0} {0} 255 {0}\n'.format(index)
        classes = QgsPalettedRasterRenderer.classDataFromString(classesString)

        rasterRenderer = QgsPalettedRasterRenderer(
            self.raster_layer.dataProvider(), 3, classes)
        self.raster_layer.setRenderer(rasterRenderer)

        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        self.assertChannelBand(root, 'sld:GrayChannel', '3')
        # check ColorMapEntry classes
        colorMap = root.elementsByTagName('sld:ColorMap')
        colorMap = colorMap.item(0).toElement()
        self.assertFalse(colorMap.isNull())
        self.assertEqual(colorMap.attribute('type'), 'values')
        self.assertFalse(colorMap.hasAttribute('extendend'))
        colorMapEntries = colorMap.elementsByTagName('sld:ColorMapEntry')
        self.assertEqual(colorMapEntries.count(), 10)
        for index in range(colorMapEntries.count()):
            colorMapEntry = colorMapEntries.at(index).toElement()
            self.assertEqual(colorMapEntry.attribute('quantity'), '{}'.format(index))
            self.assertEqual(colorMapEntry.attribute('label'), '{}'.format(index))
            self.assertEqual(colorMapEntry.attribute('opacity'), '')
            self.assertEqual(colorMapEntry.attribute('color'), '#{0:02d}{0:02d}{0:02d}'.format(index))

        # add check that is set ColoMap extended="true" if colormap is bigger that 255 entries
        classesString = ''
        values = range(255)
        for index in range(255):
            classesString += '{0} {1} {1} {1} 255 {0}\n'.format(index, random.choice(values))
        classes = QgsPalettedRasterRenderer.classDataFromString(classesString)
        rasterRenderer = QgsPalettedRasterRenderer(
            self.raster_layer.dataProvider(), 3, classes)
        self.raster_layer.setRenderer(rasterRenderer)
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        colorMap = root.elementsByTagName('sld:ColorMap')
        colorMap = colorMap.item(0).toElement()
        self.assertTrue(colorMap.hasAttribute('extended'))
        self.assertEqual(colorMap.attribute('extended'), 'true')

    def testMultiBandColorRenderer(self):
        rasterRenderer = QgsMultiBandColorRenderer(
            self.raster_layer.dataProvider(), 3, 1, 2)
        self.raster_layer.setRenderer(rasterRenderer)
        self.raster_layer.setContrastEnhancement(algorithm=QgsContrastEnhancement.StretchToMinimumMaximum,
                                                 limits=QgsRasterMinMaxOrigin.MinMax)

        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        self.assertChannelBand(root, 'sld:RedChannel', '3')
        self.assertChannelBand(root, 'sld:GreenChannel', '1')
        self.assertChannelBand(root, 'sld:BlueChannel', '2')

    def testSingleBandGrayRenderer(self):
        # check with StretchToMinimumMaximum
        rasterRenderer = QgsSingleBandGrayRenderer(self.raster_layer.dataProvider(), 3)
        self.raster_layer.setRenderer(rasterRenderer)
        self.raster_layer.setContrastEnhancement(algorithm=QgsContrastEnhancement.StretchToMinimumMaximum,
                                                 limits=QgsRasterMinMaxOrigin.MinMax)
        maximum = self.raster_layer.renderer().contrastEnhancement().maximumValue()
        minmum = self.raster_layer.renderer().contrastEnhancement().minimumValue()
        self.assertEqual(minmum, 51)
        self.assertEqual(maximum, 172)

        # check default values
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        self.assertChannelBand(root, 'sld:GrayChannel', '3')

        elements = root.elementsByTagName('sld:ContrastEnhancement')
        self.assertEqual(len(elements), 1)
        enhancement = elements.at(0).toElement()
        self.assertFalse(enhancement.isNull())

        normalize = enhancement.firstChildElement('sld:Normalize')
        self.assertFalse(normalize.isNull())
        self.assertVendorOption(normalize, 'algorithm', 'StretchToMinimumMaximum')
        self.assertVendorOption(normalize, 'minValue', '51')
        self.assertVendorOption(normalize, 'maxValue', '172')

        elements = root.elementsByTagName('sld:ColorMap')
        self.assertEqual(len(elements), 1)
        colorMap = elements.at(0).toElement()
        self.assertFalse(colorMap.isNull())

        colorMapEntries = colorMap.elementsByTagName('sld:ColorMapEntry')
        self.assertEqual(len(colorMapEntries), 2)
        clorMap1 = colorMapEntries.at(0)
        self.assertEqual(clorMap1.attributes().namedItem('color').nodeValue(), '#000000')
        self.assertEqual(clorMap1.attributes().namedItem('quantity').nodeValue(), '0')

        clorMap2 = colorMapEntries.at(1)
        self.assertEqual(clorMap2.attributes().namedItem('color').nodeValue(), '#ffffff')
        self.assertEqual(clorMap2.attributes().namedItem('quantity').nodeValue(), '255')

        # check when StretchAndClipToMinimumMaximum
        # then min/max have always to be the real one and not that set in the contrastEnhancement
        self.raster_layer.setContrastEnhancement(algorithm=QgsContrastEnhancement.StretchAndClipToMinimumMaximum,
                                                 limits=QgsRasterMinMaxOrigin.MinMax)
        minmum = self.raster_layer.renderer().contrastEnhancement().setMinimumValue(100)
        maximum = self.raster_layer.renderer().contrastEnhancement().maximumValue()
        minmum = self.raster_layer.renderer().contrastEnhancement().minimumValue()
        self.assertEqual(minmum, 100)
        self.assertEqual(maximum, 172)

        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        self.assertChannelBand(root, 'sld:GrayChannel', '3')

        elements = root.elementsByTagName('sld:ContrastEnhancement')
        self.assertEqual(len(elements), 1)
        enhancement = elements.at(0).toElement()
        self.assertFalse(enhancement.isNull())

        normalize = enhancement.firstChildElement('sld:Normalize')
        self.assertFalse(normalize.isNull())
        self.assertVendorOption(normalize, 'minValue', '51')
        self.assertVendorOption(normalize, 'maxValue', '172')

        elements = root.elementsByTagName('sld:ColorMap')
        self.assertEqual(len(elements), 1)
        colorMap = elements.at(0).toElement()
        self.assertFalse(colorMap.isNull())

        colorMapEntries = colorMap.elementsByTagName('sld:ColorMapEntry')
        self.assertEqual(len(colorMapEntries), 4)
        clorMap1 = colorMapEntries.at(0)
        self.assertEqual(clorMap1.attributes().namedItem('color').nodeValue(), '#000000')
        self.assertEqual(clorMap1.attributes().namedItem('quantity').nodeValue(), '100')
        self.assertEqual(clorMap1.attributes().namedItem('opacity').nodeValue(), '0')

        clorMap2 = colorMapEntries.at(1)
        self.assertEqual(clorMap2.attributes().namedItem('color').nodeValue(), '#000000')
        self.assertEqual(clorMap2.attributes().namedItem('quantity').nodeValue(), '100')

        clorMap3 = colorMapEntries.at(2)
        self.assertEqual(clorMap3.attributes().namedItem('color').nodeValue(), '#ffffff')
        self.assertEqual(clorMap3.attributes().namedItem('quantity').nodeValue(), '172')

        clorMap4 = colorMapEntries.at(3)
        self.assertEqual(clorMap4.attributes().namedItem('color').nodeValue(), '#ffffff')
        self.assertEqual(clorMap4.attributes().namedItem('quantity').nodeValue(), '172')
        self.assertEqual(clorMap4.attributes().namedItem('opacity').nodeValue(), '0')

        # check when ClipToMinimumMaximum
        # then min/max have always to be the real one and not that set in the contrastEnhancement
        self.raster_layer.setContrastEnhancement(algorithm=QgsContrastEnhancement.ClipToMinimumMaximum,
                                                 limits=QgsRasterMinMaxOrigin.MinMax)
        minmum = self.raster_layer.renderer().contrastEnhancement().setMinimumValue(100)
        maximum = self.raster_layer.renderer().contrastEnhancement().maximumValue()
        minmum = self.raster_layer.renderer().contrastEnhancement().minimumValue()
        self.assertEqual(minmum, 100)
        self.assertEqual(maximum, 172)

        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        self.assertChannelBand(root, 'sld:GrayChannel', '3')

        elements = root.elementsByTagName('sld:ContrastEnhancement')
        self.assertEqual(len(elements), 1)
        enhancement = elements.at(0).toElement()
        self.assertFalse(enhancement.isNull())

        normalize = enhancement.firstChildElement('sld:Normalize')
        self.assertFalse(normalize.isNull())
        self.assertVendorOption(normalize, 'minValue', '51')
        self.assertVendorOption(normalize, 'maxValue', '172')

        elements = root.elementsByTagName('sld:ColorMap')
        self.assertEqual(len(elements), 1)
        colorMap = elements.at(0).toElement()
        self.assertFalse(colorMap.isNull())

        colorMapEntries = colorMap.elementsByTagName('sld:ColorMapEntry')
        self.assertEqual(len(colorMapEntries), 4)
        clorMap1 = colorMapEntries.at(0)
        self.assertEqual(clorMap1.attributes().namedItem('color').nodeValue(), '#000000')
        self.assertEqual(clorMap1.attributes().namedItem('quantity').nodeValue(), '100')
        self.assertEqual(clorMap1.attributes().namedItem('opacity').nodeValue(), '0')

        clorMap2 = colorMapEntries.at(1)
        self.assertEqual(clorMap2.attributes().namedItem('color').nodeValue(), '#000000')
        self.assertEqual(clorMap2.attributes().namedItem('quantity').nodeValue(), '100')

        clorMap3 = colorMapEntries.at(2)
        self.assertEqual(clorMap3.attributes().namedItem('color').nodeValue(), '#ffffff')
        self.assertEqual(clorMap3.attributes().namedItem('quantity').nodeValue(), '172')

        clorMap4 = colorMapEntries.at(3)
        self.assertEqual(clorMap4.attributes().namedItem('color').nodeValue(), '#ffffff')
        self.assertEqual(clorMap4.attributes().namedItem('quantity').nodeValue(), '172')
        self.assertEqual(clorMap4.attributes().namedItem('opacity').nodeValue(), '0')

    def testRasterRenderer(self):
        class fakerenderer(QgsRasterRenderer):

            def __init__(self, interface):
                QgsRasterRenderer.__init__(self, interface, '')

        rasterRenderer = fakerenderer(self.raster_layer.dataProvider())
        self.raster_layer.setRenderer(rasterRenderer)

        # check opacity default value is not exported
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertNoOpacity(root)
        # check if opacity is not the default value
        rasterRenderer.setOpacity(1.1)
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertOpacity(root, '1.1')

        # check gamma properties from [-100:0] stretched to [0:1]
        #  and (0:100] stretche dto (1:100]
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '-100'})
        # self.assertGamma(root, '0')
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '-50'})
        # self.assertGamma(root, '0.5')
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '0'})
        # self.assertGamma(root, '1')
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '1'})
        # self.assertGamma(root, '1')
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '100'})
        # self.assertGamma(root, '100')
        # # input contrast are always integer, btw the value is managed also if it's double
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '1.1'})
        # self.assertGamma(root, '1.1')
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '1.6'})
        # self.assertGamma(root, '1.6')
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '-50.5'})
        # self.assertGamma(root, '0.495')
        # dom, root = self.rendererToSld(rasterRenderer, {'contrast': '-0.1'})
        # self.assertGamma(root, '0.999')

    def testStretchingAlgorithm(self):
        rasterRenderer = QgsMultiBandColorRenderer(
            self.raster_layer.dataProvider(), 3, 1, 2)
        self.raster_layer.setRenderer(rasterRenderer)

        # check StretchToMinimumMaximum stretching alg
        self.raster_layer.setContrastEnhancement(algorithm=QgsContrastEnhancement.StretchToMinimumMaximum,
                                                 limits=QgsRasterMinMaxOrigin.MinMax)
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertContrastEnhancement(root, 'sld:RedChannel', 'StretchToMinimumMaximum', '51', '172')
        self.assertContrastEnhancement(root, 'sld:GreenChannel', 'StretchToMinimumMaximum', '122', '130')
        self.assertContrastEnhancement(root, 'sld:BlueChannel', 'StretchToMinimumMaximum', '133', '148')

        # check StretchAndClipToMinimumMaximum stretching alg
        self.raster_layer.setContrastEnhancement(algorithm=QgsContrastEnhancement.StretchAndClipToMinimumMaximum,
                                                 limits=QgsRasterMinMaxOrigin.MinMax)
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertContrastEnhancement(root, 'sld:RedChannel', 'ClipToZero', '51', '172')
        self.assertContrastEnhancement(root, 'sld:GreenChannel', 'ClipToZero', '122', '130')
        self.assertContrastEnhancement(root, 'sld:BlueChannel', 'ClipToZero', '133', '148')

        # check ClipToMinimumMaximum stretching alg
        self.raster_layer.setContrastEnhancement(algorithm=QgsContrastEnhancement.ClipToMinimumMaximum,
                                                 limits=QgsRasterMinMaxOrigin.MinMax)
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertContrastEnhancement(root, 'sld:RedChannel', 'ClipToMinimumMaximum', '51', '172')
        self.assertContrastEnhancement(root, 'sld:GreenChannel', 'ClipToMinimumMaximum', '122', '130')
        self.assertContrastEnhancement(root, 'sld:BlueChannel', 'ClipToMinimumMaximum', '133', '148')

        # check NoEnhancement stretching alg
        self.raster_layer.setContrastEnhancement(algorithm=QgsContrastEnhancement.NoEnhancement)
        dom, root = self.rendererToSld(self.raster_layer.renderer())
        self.assertContrastEnhancement(root, 'sld:RedChannel')
        self.assertContrastEnhancement(root, 'sld:GreenChannel')
        self.assertContrastEnhancement(root, 'sld:BlueChannel')

    def assertVendorOption(self, root, name, expectedValue):
        """Set expectedValue=None to check that the vendor option is not present."""
        vendorOptions = root.elementsByTagName('sld:VendorOption')
        found = False
        for vendorOptionIndex in range(vendorOptions.count()):
            vendorOption = vendorOptions.at(vendorOptionIndex)
            self.assertEqual('sld:VendorOption', vendorOption.nodeName())
            if (vendorOption.attributes().namedItem('name').nodeValue() == name):
                found = True
                self.assertEqual(vendorOption.firstChild().nodeValue(), expectedValue)
        if (expectedValue is None) and found:
            self.fail("found VendorOption: {} where supposed not present".format(name))
        if expectedValue and not found:
            self.fail("Not found VendorOption: {}".format(name))

    def assertGamma(self, root, expectedValue, index=0):
        enhancement = root.elementsByTagName('sld:ContrastEnhancement').item(index)
        gamma = enhancement.firstChildElement('sld:GammaValue')
        self.assertEqual(expectedValue, gamma.firstChild().nodeValue())

    def assertOpacity(self, root, expectedValue, index=0):
        opacity = root.elementsByTagName('sld:Opacity').item(index)
        self.assertEqual(expectedValue, opacity.firstChild().nodeValue())

    def assertNoOpacity(self, root):
        opacities = root.elementsByTagName('sld:Opacity')
        self.assertEqual(opacities.size(), 0)

    def assertContrastEnhancement(self, root, bandTag, expectedAlg=None, expectedMin=None, expectedMax=None, index=0):
        channelSelection = root.elementsByTagName('sld:ChannelSelection').item(index)
        self.assertIsNotNone(channelSelection)
        band = channelSelection.firstChildElement(bandTag)
        # check if no enhancement alg is iset
        if (not expectedAlg):
            contrastEnhancementName = band.firstChildElement('sld:ContrastEnhancement')
            self.assertEqual('', contrastEnhancementName.firstChild().nodeName())
            return
        # check if enhancement alg is set
        contrastEnhancementName = band.firstChildElement('sld:ContrastEnhancement')
        self.assertEqual('sld:Normalize', contrastEnhancementName.firstChild().nodeName())
        normalize = contrastEnhancementName.firstChildElement('sld:Normalize')
        vendorOptions = normalize.elementsByTagName('VendorOption')
        for vendorOptionIndex in range(vendorOptions.count()):
            vendorOption = vendorOptions.at(vendorOptionIndex)
            self.assertEqual('VendorOption', vendorOption.nodeName())
            if (vendorOption.attributes().namedItem('name').nodeValue() == 'algorithm'):
                self.assertEqual(expectedAlg, vendorOption.firstChild().nodeValue())
            elif (vendorOption.attributes().namedItem('name').nodeValue() == 'minValue'):
                self.assertEqual(expectedMin, vendorOption.firstChild().nodeValue())
            elif (vendorOption.attributes().namedItem('name').nodeValue() == 'maxValue'):
                self.assertEqual(expectedMax, vendorOption.firstChild().nodeValue())
            else:
                self.fail('Unrecognised vendorOption name {}'.format(vendorOption.attributes().namedItem('name').nodeValue()))

    def assertChannelBand(self, root, bandTag, expectedValue, index=0):
        channelSelection = root.elementsByTagName('sld:ChannelSelection').item(index)
        self.assertIsNotNone(channelSelection)
        band = channelSelection.firstChildElement(bandTag)
        sourceChannelName = band.firstChildElement('sld:SourceChannelName')
        self.assertEqual(expectedValue, sourceChannelName.firstChild().nodeValue())

    def rendererToSld(self, renderer, properties={}):
        dom = QDomDocument()
        root = dom.createElement("FakeRoot")
        dom.appendChild(root)
        renderer.toSld(dom, root, properties)
        return dom, root


if __name__ == '__main__':
    unittest.main()
