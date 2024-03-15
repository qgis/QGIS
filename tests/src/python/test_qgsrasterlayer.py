"""QGIS Unit tests for QgsRasterLayer.

From build dir, run:
ctest -R PyQgsRasterLayer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'

import filecmp
import os
from shutil import copyfile

import numpy as np
from osgeo import gdal
from qgis.PyQt.QtCore import QFileInfo, QSize, QTemporaryDir
from qgis.PyQt.QtGui import QColor, QResizeEvent
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsBilinearRasterResampler,
    QgsColorRampShader,
    QgsContrastEnhancement,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsCubicRasterResampler,
    QgsDataProvider,
    QgsExpressionContext,
    QgsExpressionContextScope,
    QgsGradientColorRamp,
    QgsHueSaturationFilter,
    QgsLayerDefinition,
    QgsLimitedRandomColorRamp,
    QgsMapLayerServerProperties,
    QgsMapSettings,
    QgsPalettedRasterRenderer,
    QgsPointXY,
    QgsProject,
    QgsProperty,
    QgsRaster,
    QgsRasterHistogram,
    QgsRasterLayer,
    QgsRasterMinMaxOrigin,
    QgsRasterPipe,
    QgsRasterShader,
    QgsRasterTransparency,
    QgsReadWriteContext,
    QgsSingleBandGrayRenderer,
    QgsSingleBandPseudoColorRenderer,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.testing.mocked import get_iface

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterLayer(QgisTestCase):

    def setUp(self):
        self.iface = get_iface()
        QgsProject.instance().removeAllMapLayers()

        self.iface.mapCanvas().viewport().resize(400, 400)
        # For some reason the resizeEvent is not delivered, fake it
        self.iface.mapCanvas().resizeEvent(QResizeEvent(QSize(400, 400), self.iface.mapCanvas().size()))

    def testIdentify(self):
        myPath = os.path.join(unitTestDataPath(), 'landsat.tif')
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        self.assertTrue(myRasterLayer.isValid())
        myPoint = QgsPointXY(786690, 3345803)
        # print 'Extents: %s' % myRasterLayer.extent().toString()
        # myResult, myRasterValues = myRasterLayer.identify(myPoint)
        # assert myResult
        myRasterValues = myRasterLayer.dataProvider().identify(myPoint, QgsRaster.IdentifyFormat.IdentifyFormatValue).results()

        self.assertGreater(len(myRasterValues), 0)

        # Get the name of the first band
        myBand = list(myRasterValues.keys())[0]
        # myExpectedName = 'Band 1
        self.assertEqual(1, myBand)

        # Convert each band value to a list of ints then to a string

        myValues = list(myRasterValues.values())
        myIntValues = []
        for myValue in myValues:
            myIntValues.append(int(myValue))
        myValues = str(myIntValues)
        myExpectedValues = '[127, 141, 112, 72, 86, 126, 156, 211, 170]'
        myMessage = f'Expected: {myValues}\nGot: {myExpectedValues}'
        self.assertEqual(myValues, myExpectedValues, myMessage)

    def testSampleIdentify(self):
        """Test that sample() and identify() return the same values, GH #44902"""

        tempdir = QTemporaryDir()
        temppath = os.path.join(tempdir.path(), 'test_sample.tif')

        def _test(qgis_data_type):
            rlayer = QgsRasterLayer(temppath, 'test_sample')
            self.assertTrue(rlayer.isValid())
            self.assertEqual(rlayer.dataProvider().dataType(1), qgis_data_type)

            x = 252290.5
            for y in [5022000.5, 5022001.5]:
                pos = QgsPointXY(x, y)
                value_sample = rlayer.dataProvider().sample(pos, 1)[0]
                value_identify = rlayer.dataProvider().identify(pos, QgsRaster.IdentifyFormat.IdentifyFormatValue).results()[1]
                # Check values for UInt32
                if qgis_data_type == Qgis.DataType.UInt32:
                    if y == 5022000.5:
                        self.assertEqual(value_sample, 4294967000.0)
                    else:
                        self.assertEqual(value_sample, 4294967293.0)
                self.assertEqual(value_sample, value_identify)
                # print(value_sample, value_identify)

        # Test GDT_UInt32
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 3, 3, 1, gdal.GDT_UInt32)
        outRaster.SetGeoTransform((252290.0, 1.0, 0.0, 5022002.0, 0.0, -1.0))
        outband = outRaster.GetRasterBand(1)
        npdata = np.array([[4294967293, 1000, 5],
                           [4294967000, 50, 4]], dtype=np.uint32)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        _test(Qgis.DataType.UInt32)

        # Test GDT_Int32
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 3, 3, 1, gdal.GDT_Int32)
        outRaster.SetGeoTransform((252290.0, 1.0, 0.0, 5022002.0, 0.0, -1.0))
        outband = outRaster.GetRasterBand(1)
        npdata = np.array([[1294967293, 1000, 5],
                           [1294967000, 50, 4]], dtype=np.int32)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        _test(Qgis.DataType.Int32)

        # Test GDT_Float32
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 3, 3, 1, gdal.GDT_Float32)
        outRaster.SetGeoTransform((252290.0, 1.0, 0.0, 5022002.0, 0.0, -1.0))
        outband = outRaster.GetRasterBand(1)
        npdata = np.array([[1294967293, 1000, 5],
                           [1294967000, 50, 4]], dtype=np.float32)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        _test(Qgis.DataType.Float32)

        # Test GDT_Float64
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 3, 3, 1, gdal.GDT_Float64)
        outRaster.SetGeoTransform((252290.0, 1.0, 0.0, 5022002.0, 0.0, -1.0))
        outband = outRaster.GetRasterBand(1)
        npdata = np.array([[1294967293, 1000, 5],
                           [1294967000, 50, 4]], dtype=np.float64)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        _test(Qgis.DataType.Float64)

        # Test GDT_Uint16
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 3, 3, 1, gdal.GDT_UInt16)
        outRaster.SetGeoTransform((252290.0, 1.0, 0.0, 5022002.0, 0.0, -1.0))
        outband = outRaster.GetRasterBand(1)
        npdata = np.array([[1294967293, 1000, 5],
                           [1294967000, 50, 4]], dtype=np.uint16)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        _test(Qgis.DataType.UInt16)

        # Test GDT_Int16
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 3, 3, 1, gdal.GDT_Int16)
        outRaster.SetGeoTransform((252290.0, 1.0, 0.0, 5022002.0, 0.0, -1.0))
        outband = outRaster.GetRasterBand(1)
        npdata = np.array([[31768, 1000, 5],
                           [12345, 50, 4]], dtype=np.int16)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        _test(Qgis.DataType.Int16)

        # Test GDT_Int32
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 3, 3, 1, gdal.GDT_Int32)
        outRaster.SetGeoTransform((252290.0, 1.0, 0.0, 5022002.0, 0.0, -1.0))
        outband = outRaster.GetRasterBand(1)
        npdata = np.array([[31768, 1000, 5],
                           [12345, 50, 4]], dtype=np.int32)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        _test(Qgis.DataType.Int32)

        # Test GDT_Byte
        driver = gdal.GetDriverByName('GTiff')
        outRaster = driver.Create(temppath, 3, 3, 1, gdal.GDT_Byte)
        outRaster.SetGeoTransform((252290.0, 1.0, 0.0, 5022002.0, 0.0, -1.0))
        outband = outRaster.GetRasterBand(1)
        npdata = np.array([[123, 255, 5],
                           [1, 50, 4]], dtype=np.byte)
        outband.WriteArray(npdata)
        outband.FlushCache()
        outRaster.FlushCache()
        del outRaster

        _test(Qgis.DataType.Byte)

    def testTransparency(self):
        myPath = os.path.join(unitTestDataPath('raster'),
                              'band1_float32_noct_epsg4326.tif')
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        self.assertTrue(myRasterLayer.isValid())

        renderer = QgsSingleBandGrayRenderer(myRasterLayer.dataProvider(), 1)
        myRasterLayer.setRenderer(renderer)
        myRasterLayer.setContrastEnhancement(
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum,
            QgsRasterMinMaxOrigin.Limits.MinMax)

        myContrastEnhancement = myRasterLayer.renderer().contrastEnhancement()
        # print ("myContrastEnhancement.minimumValue = %.17g" %
        #       myContrastEnhancement.minimumValue())
        # print ("myContrastEnhancement.maximumValue = %.17g" %
        #        myContrastEnhancement.maximumValue())

        # Unfortunately the minimum/maximum values calculated in C++ and Python
        # are slightly different (e.g. 3.3999999521443642e+38 x
        # 3.3999999521444001e+38)
        # It is not clear where the precision is lost.
        # We set the same values as C++.
        myContrastEnhancement.setMinimumValue(-3.3319999287625854e+38)
        myContrastEnhancement.setMaximumValue(3.3999999521443642e+38)
        # myType = myRasterLayer.dataProvider().dataType(1);
        # myEnhancement = QgsContrastEnhancement(myType);

        myTransparentSingleValuePixelList = []
        rasterTransparency = QgsRasterTransparency()

        myTransparentPixel1 = \
            QgsRasterTransparency.TransparentSingleValuePixel()
        myTransparentPixel1.min = -2.5840000772112106e+38
        myTransparentPixel1.max = -1.0879999684602689e+38
        myTransparentPixel1.percentTransparent = 50
        myTransparentSingleValuePixelList.append(myTransparentPixel1)

        myTransparentPixel2 = \
            QgsRasterTransparency.TransparentSingleValuePixel()
        myTransparentPixel2.min = 1.359999960575336e+37
        myTransparentPixel2.max = 9.520000231087593e+37
        myTransparentPixel2.percentTransparent = 70
        myTransparentSingleValuePixelList.append(myTransparentPixel2)

        rasterTransparency.setTransparentSingleValuePixelList(
            myTransparentSingleValuePixelList)

        rasterRenderer = myRasterLayer.renderer()
        self.assertTrue(rasterRenderer)

        rasterRenderer.setRasterTransparency(rasterTransparency)

        QgsProject.instance().addMapLayers([myRasterLayer, ])

        myMapSettings = QgsMapSettings()
        myMapSettings.setLayers([myRasterLayer])
        myMapSettings.setExtent(myRasterLayer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_transparency',
                'raster_transparency',
                myMapSettings)
        )

    def testIssue7023(self):
        """Check if converting a raster from 1.8 to 2 works."""
        myPath = os.path.join(unitTestDataPath('raster'),
                              'raster-palette-crash2.tif')
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        self.assertTrue(myRasterLayer.isValid())
        # crash on next line
        QgsProject.instance().addMapLayers([myRasterLayer])

    def testShaderCrash(self):
        """Check if we assign a shader and then reassign it no crash occurs."""
        myPath = os.path.join(unitTestDataPath('raster'),
                              'band1_float32_noct_epsg4326.tif')
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        self.assertTrue(myRasterLayer.isValid())

        myRasterShader = QgsRasterShader()
        myColorRampShader = QgsColorRampShader()
        myColorRampShader.setColorRampType(QgsColorRampShader.Type.Interpolated)
        myItems = []
        myItem = QgsColorRampShader.ColorRampItem(
            10, QColor('#ffff00'), 'foo')
        myItems.append(myItem)
        myItem = QgsColorRampShader.ColorRampItem(
            100, QColor('#ff00ff'), 'bar')
        myItems.append(myItem)
        myItem = QgsColorRampShader.ColorRampItem(
            1000, QColor('#00ff00'), 'kazam')
        myItems.append(myItem)
        myColorRampShader.setColorRampItemList(myItems)
        myRasterShader.setRasterShaderFunction(myColorRampShader)
        myPseudoRenderer = QgsSingleBandPseudoColorRenderer(
            myRasterLayer.dataProvider(), 1, myRasterShader)
        myRasterLayer.setRenderer(myPseudoRenderer)

        return
        # ####### works first time #############

        myRasterShader = QgsRasterShader()
        myColorRampShader = QgsColorRampShader()
        myColorRampShader.setColorRampType(QgsColorRampShader.Type.Interpolated)
        myItems = []
        myItem = QgsColorRampShader.ColorRampItem(10,
                                                  QColor('#ffff00'), 'foo')
        myItems.append(myItem)
        myItem = QgsColorRampShader.ColorRampItem(100,
                                                  QColor('#ff00ff'), 'bar')
        myItems.append(myItem)
        myItem = QgsColorRampShader.ColorRampItem(1000,
                                                  QColor('#00ff00'), 'kazam')
        myItems.append(myItem)
        myColorRampShader.setColorRampItemList(myItems)
        myRasterShader.setRasterShaderFunction(myColorRampShader)
        # ####### crash on next line (fixed now)##################
        myPseudoRenderer = QgsSingleBandPseudoColorRenderer(
            myRasterLayer.dataProvider(), 1, myRasterShader)
        myRasterLayer.setRenderer(myPseudoRenderer)

    def onRendererChanged(self):
        self.rendererChanged = True

    def test_setRenderer(self):
        myPath = os.path.join(unitTestDataPath('raster'),
                              'band1_float32_noct_epsg4326.tif')
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        layer = QgsRasterLayer(myPath, myBaseName)

        self.rendererChanged = False
        layer.rendererChanged.connect(self.onRendererChanged)

        rShader = QgsRasterShader()
        r = QgsSingleBandPseudoColorRenderer(layer.dataProvider(), 1, rShader)

        layer.setRenderer(r)
        self.assertTrue(self.rendererChanged)
        self.assertEqual(layer.renderer(), r)

    def test_server_properties(self):
        """ Test server properties. """
        raster_path = os.path.join(
            unitTestDataPath('raster'),
            'band1_float32_noct_epsg4326.tif')
        layer = QgsRasterLayer(raster_path, 'test_raster')
        self.assertIsInstance(layer.serverProperties(), QgsMapLayerServerProperties)

    def testQgsRasterMinMaxOrigin(self):

        mmo = QgsRasterMinMaxOrigin()
        mmo_default = QgsRasterMinMaxOrigin()
        self.assertEqual(mmo, mmo_default)

        mmo = QgsRasterMinMaxOrigin()
        self.assertEqual(mmo.limits(), QgsRasterMinMaxOrigin.Limits.None_)
        mmo.setLimits(QgsRasterMinMaxOrigin.Limits.CumulativeCut)
        self.assertEqual(mmo.limits(), QgsRasterMinMaxOrigin.Limits.CumulativeCut)
        self.assertNotEqual(mmo, mmo_default)

        mmo = QgsRasterMinMaxOrigin()
        self.assertEqual(mmo.extent(), QgsRasterMinMaxOrigin.Extent.WholeRaster)
        mmo.setExtent(QgsRasterMinMaxOrigin.Extent.UpdatedCanvas)
        self.assertEqual(mmo.extent(), QgsRasterMinMaxOrigin.Extent.UpdatedCanvas)
        self.assertNotEqual(mmo, mmo_default)

        mmo = QgsRasterMinMaxOrigin()
        self.assertEqual(mmo.statAccuracy(), QgsRasterMinMaxOrigin.StatAccuracy.Estimated)
        mmo.setStatAccuracy(QgsRasterMinMaxOrigin.StatAccuracy.Exact)
        self.assertEqual(mmo.statAccuracy(), QgsRasterMinMaxOrigin.StatAccuracy.Exact)
        self.assertNotEqual(mmo, mmo_default)

        mmo = QgsRasterMinMaxOrigin()
        self.assertAlmostEqual(mmo.cumulativeCutLower(), 0.02)
        mmo.setCumulativeCutLower(0.1)
        self.assertAlmostEqual(mmo.cumulativeCutLower(), 0.1)
        self.assertNotEqual(mmo, mmo_default)

        mmo = QgsRasterMinMaxOrigin()
        self.assertAlmostEqual(mmo.cumulativeCutUpper(), 0.98)
        mmo.setCumulativeCutUpper(0.9)
        self.assertAlmostEqual(mmo.cumulativeCutUpper(), 0.9)
        self.assertNotEqual(mmo, mmo_default)

        mmo = QgsRasterMinMaxOrigin()
        self.assertAlmostEqual(mmo.stdDevFactor(), 2.0)
        mmo.setStdDevFactor(2.5)
        self.assertAlmostEqual(mmo.stdDevFactor(), 2.5)
        self.assertNotEqual(mmo, mmo_default)

        mmo = QgsRasterMinMaxOrigin()
        mmo.setLimits(QgsRasterMinMaxOrigin.Limits.CumulativeCut)
        mmo.setExtent(QgsRasterMinMaxOrigin.Extent.UpdatedCanvas)
        mmo.setStatAccuracy(QgsRasterMinMaxOrigin.StatAccuracy.Exact)
        mmo.setCumulativeCutLower(0.1)
        mmo.setCumulativeCutUpper(0.9)
        mmo.setStdDevFactor(2.5)
        doc = QDomDocument()
        parentElem = doc.createElement("test")
        mmo.writeXml(doc, parentElem)
        mmoUnserialized = QgsRasterMinMaxOrigin()
        mmoUnserialized.readXml(parentElem)
        self.assertEqual(mmo, mmoUnserialized)

    def testBrightnessContrastGamma(self):
        """ test raster brightness/contrast/gamma filter"""
        path = os.path.join(unitTestDataPath(),
                            'landsat_4326.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')

        layer.brightnessFilter().setContrast(100)

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_contrast100',
                'raster_contrast100',
                ms)
        )

        layer.brightnessFilter().setContrast(-30)

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_contrast30',
                'raster_contrast30',
                ms)
        )

        layer.brightnessFilter().setContrast(0)
        layer.brightnessFilter().setBrightness(50)

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_brightness50',
                'raster_brightness50',
                ms)
        )

        layer.brightnessFilter().setBrightness(-20)

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_brightness20',
                'raster_brightness20',
                ms)
        )

        path = os.path.join(unitTestDataPath(),
                            'landsat-int16-b1.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')

        layer.brightnessFilter().setGamma(0.22)

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_gamma022',
                'raster_gamma022',
                ms)
        )

        layer.brightnessFilter().setGamma(2.22)

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_gamma222',
                'raster_gamma222',
                ms)
        )

    def testInvertColors(self):
        """ test raster invert colors filter"""
        path = os.path.join(unitTestDataPath(),
                            'landsat_4326.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')

        layer.hueSaturationFilter().setInvertColors(True)

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_invertcolors',
                'raster_invertcolors',
                ms)
        )

    def testInvertSemiOpaqueColors(self):
        """ test raster invert colors filter"""
        path = os.path.join(unitTestDataPath(),
                            'landsat_4326.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')

        layer.setOpacity(0.33)
        layer.hueSaturationFilter().setInvertColors(True)

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                'raster_invertsemiopaquecolors',
                'raster_invertsemiopaquecolors',
                ms)
        )

    def testClone(self):
        myPath = os.path.join(unitTestDataPath('raster'),
                              'band1_float32_noct_epsg4326.tif')
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        layer = QgsRasterLayer(myPath, myBaseName)

        renderer = layer.renderer().clone()
        renderer.setOpacity(33.3)
        layer.setRenderer(renderer)

        # clone layer
        clone = layer.clone()

        # generate xml from layer
        layer_doc = QDomDocument("doc")
        layer_elem = layer_doc.createElement("maplayer")
        layer.writeLayerXml(layer_elem, layer_doc, QgsReadWriteContext())

        # generate xml from clone
        clone_doc = QDomDocument("doc")
        clone_elem = clone_doc.createElement("maplayer")
        clone.writeLayerXml(clone_elem, clone_doc, QgsReadWriteContext())

        # replace id within xml of clone
        clone_id_elem = clone_elem.firstChildElement("id")
        clone_id_elem_patch = clone_doc.createElement("id")
        clone_id_elem_patch_value = clone_doc.createTextNode(layer.id())
        clone_id_elem_patch.appendChild(clone_id_elem_patch_value)
        clone_elem.replaceChild(clone_id_elem_patch, clone_id_elem)

        # update doc
        clone_doc.appendChild(clone_elem)
        layer_doc.appendChild(layer_elem)

        # compare xml documents
        self.assertEqual(layer_doc.toString(), clone_doc.toString())

    def testSetDataSource(self):
        """Test change data source"""

        temp_dir = QTemporaryDir()
        options = QgsDataProvider.ProviderOptions()
        myPath = os.path.join(unitTestDataPath('raster'),
                              'band1_float32_noct_epsg4326.tif')
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        layer = QgsRasterLayer(myPath, myBaseName)
        renderer = QgsSingleBandGrayRenderer(layer.dataProvider(), 2)

        image = layer.previewAsImage(QSize(400, 400))
        self.assertFalse(image.isNull())
        self.assertTrue(image.save(os.path.join(temp_dir.path(), 'expected.png'), "PNG"))

        layer.setDataSource(myPath.replace('4326.tif', '4326-BAD_SOURCE.tif'), 'bad_layer', 'gdal', options)
        self.assertFalse(layer.isValid())
        image = layer.previewAsImage(QSize(400, 400))
        self.assertTrue(image.isNull())

        layer.setDataSource(myPath.replace('4326-BAD_SOURCE.tif', '4326.tif'), 'bad_layer', 'gdal', options)
        self.assertTrue(layer.isValid())
        image = layer.previewAsImage(QSize(400, 400))
        self.assertFalse(image.isNull())
        self.assertTrue(image.save(os.path.join(temp_dir.path(), 'actual.png'), "PNG"))

        self.assertTrue(
            filecmp.cmp(os.path.join(temp_dir.path(), 'actual.png'), os.path.join(temp_dir.path(), 'expected.png')),
            False)

    def testWriteSld(self):
        """Test SLD generation for the XMLS fields geneerated at RasterLayer level and not to the deeper renderer level."""

        myPath = os.path.join(unitTestDataPath(), 'landsat.tif')
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        self.assertTrue(myRasterLayer.isValid())

        # do generic export with default layer values
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = root.elementsByTagName('sld:LayerFeatureConstraints')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()

        elements = element.elementsByTagName('sld:FeatureTypeConstraint')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()

        elements = root.elementsByTagName('sld:UserStyle')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()

        name = element.firstChildElement('sld:Name')
        self.assertFalse(name.isNull())
        self.assertEqual(name.text(), 'landsat')

        abstract = element.firstChildElement('sld:Abstract')
        self.assertTrue(abstract.isNull())

        title = element.firstChildElement('sld:Title')
        self.assertTrue(title.isNull())

        featureTypeStyle = element.firstChildElement('sld:FeatureTypeStyle')
        self.assertFalse(featureTypeStyle.isNull())

        rule = featureTypeStyle.firstChildElement('sld:Rule')
        self.assertFalse(rule.isNull())

        temp = rule.firstChildElement('sld:MinScaleDenominator')
        self.assertTrue(temp.isNull())

        temp = rule.firstChildElement('sld:MaxScaleDenominator')
        self.assertTrue(temp.isNull())

        rasterSymbolizer = rule.firstChildElement('sld:RasterSymbolizer')
        self.assertFalse(rule.isNull())

        vendorOptions = rasterSymbolizer.elementsByTagName('sld:VendorOption')
        self.assertEqual(vendorOptions.size(), 0)

        # set no default values and check exported sld
        myRasterLayer.setName('')
        myRasterLayer.setAbstract('fake')
        myRasterLayer.setTitle('fake')
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = root.elementsByTagName('sld:LayerFeatureConstraints')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()

        elements = element.elementsByTagName('sld:FeatureTypeConstraint')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()

        elements = root.elementsByTagName('sld:UserStyle')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()

        # no generated if empty
        name = element.firstChildElement('sld:Name')
        self.assertTrue(name.isNull())

        # generated if not empty
        abstract = element.firstChildElement('sld:Abstract')
        self.assertFalse(abstract.isNull())
        self.assertEqual(abstract.text(), 'fake')

        title = element.firstChildElement('sld:Title')
        self.assertFalse(title.isNull())
        self.assertEqual(title.text(), 'fake')

        # if setScaleBasedVisibility is true print scales
        myRasterLayer.setScaleBasedVisibility(True)
        myRasterLayer.setMaximumScale(0.0001)
        myRasterLayer.setMinimumScale(0.01)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:Rule')
        self.assertEqual(len(elements), 1)
        rule = elements.at(0).toElement()
        self.assertFalse(rule.isNull())

        temp = rule.firstChildElement('sld:MinScaleDenominator')
        self.assertFalse(temp.isNull())
        self.assertEqual(temp.text(), '0.0001')

        temp = rule.firstChildElement('sld:MaxScaleDenominator')
        self.assertFalse(temp.isNull())
        self.assertEqual(temp.text(), '0.01')

        # check non default hueSaturationFilter values
        hue = myRasterLayer.hueSaturationFilter()
        hue.setInvertColors(True)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'invertColors', '1')

        # check non default hueSaturationFilter values
        hue = myRasterLayer.hueSaturationFilter()
        hue.setGrayscaleMode(QgsHueSaturationFilter.GrayscaleMode.GrayscaleLightness)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'grayScale', 'lightness')

        hue = myRasterLayer.hueSaturationFilter()
        hue.setGrayscaleMode(QgsHueSaturationFilter.GrayscaleMode.GrayscaleLuminosity)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'grayScale', 'luminosity')

        hue = myRasterLayer.hueSaturationFilter()
        hue.setGrayscaleMode(QgsHueSaturationFilter.GrayscaleMode.GrayscaleAverage)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'grayScale', 'average')

        hue = myRasterLayer.hueSaturationFilter()
        hue.setGrayscaleMode(QgsHueSaturationFilter.GrayscaleMode.GrayscaleOff)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'grayScale', None)

        # manage colorize vendorOption tags
        hue = myRasterLayer.hueSaturationFilter()
        hue.setColorizeOn(True)
        hue.setColorizeStrength(50)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'colorizeOn', '1')
        self.assertVendorOption(element, 'colorizeRed', '255')
        self.assertVendorOption(element, 'colorizeGreen', '128')
        self.assertVendorOption(element, 'colorizeBlue', '128')
        self.assertVendorOption(element, 'colorizeStrength', '0.5')
        self.assertVendorOption(element, 'saturation', '0.498039')

        # other hue non default values, no colorize and saturation = 0
        hue = myRasterLayer.hueSaturationFilter()
        hue.setColorizeOn(False)
        hue.setSaturation(0)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'colorizeOn', None)
        self.assertVendorOption(element, 'colorizeRed', None)
        self.assertVendorOption(element, 'colorizeGreen', None)
        self.assertVendorOption(element, 'colorizeBlue', None)
        self.assertVendorOption(element, 'colorizeStrength', None)
        self.assertVendorOption(element, 'saturation', None)
        self.assertVendorOption(element, 'brightness', None)
        self.assertVendorOption(element, 'contrast', None)

        # other hue non default values, no colorize and saturation = 100
        hue = myRasterLayer.hueSaturationFilter()
        hue.setColorizeOn(False)
        hue.setSaturation(100)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'colorizeOn', None)
        self.assertVendorOption(element, 'colorizeRed', None)
        self.assertVendorOption(element, 'colorizeGreen', None)
        self.assertVendorOption(element, 'colorizeBlue', None)
        self.assertVendorOption(element, 'colorizeStrength', None)
        self.assertVendorOption(element, 'saturation', '1')
        hue.setSaturation(-100)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        self.assertVendorOption(root, 'saturation', '0')

        # brightness filter default values
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertEqual(myRasterLayer.brightnessFilter().brightness(), 0)
        self.assertEqual(myRasterLayer.brightnessFilter().contrast(), 0)
        self.assertVendorOption(element, 'brightness', None)
        self.assertVendorOption(element, 'contrast', None)

        # brightness filter no default values
        bf = myRasterLayer.brightnessFilter()
        bf.setBrightness(-255)
        bf.setContrast(-100)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'brightness', '0')
        self.assertVendorOption(element, 'contrast', '0')

        bf.setBrightness(255)
        bf.setContrast(100)
        dom, root, errorMessage = self.layerToSld(myRasterLayer)
        elements = dom.elementsByTagName('sld:RasterSymbolizer')
        self.assertEqual(len(elements), 1)
        element = elements.at(0).toElement()
        self.assertFalse(element.isNull())
        self.assertVendorOption(element, 'brightness', '1')
        self.assertVendorOption(element, 'contrast', '1')

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
            self.fail(f"found VendorOption: {name} where supposed not present")
        if expectedValue and not found:
            self.fail(f"Not found VendorOption: {name}")

    def layerToSld(self, layer, properties={}):
        dom = QDomDocument()
        root = dom.createElement("FakeRoot")
        dom.appendChild(root)
        errorMessage = ''
        layer.writeSld(root, dom, errorMessage, properties)
        return dom, root, errorMessage

    def testHistogram(self):
        """Test histogram bindings regression GH #29700"""

        l = QgsRasterLayer(unitTestDataPath('raster/landcover.img'), 'landcover')
        self.assertTrue(l.isValid())
        p = l.dataProvider()
        # Note that this is not a correct use of the API: there is no
        # need to call initHistogram(): it is called internally
        # from p.histogram()
        p.initHistogram(QgsRasterHistogram(), 1, 100)
        h = p.histogram(1)
        self.assertTrue(len(h.histogramVector), 100)
        # Check it twice because it crashed in some circumstances with the old implementation
        self.assertTrue(len(h.histogramVector), 100)

    def testInvalidLayerStyleRestoration(self):
        """
        Test that styles are correctly restored from invalid layers
        """
        source_path = os.path.join(unitTestDataPath('raster'),
                                   'band1_float32_noct_epsg4326.tif')
        # copy to temp path
        tmp_dir = QTemporaryDir()
        tmp_path = os.path.join(tmp_dir.path(), 'test_raster.tif')
        copyfile(source_path, tmp_path)

        rl = QgsRasterLayer(tmp_path, 'test_raster', 'gdal')
        self.assertTrue(rl.isValid())
        renderer = QgsSingleBandPseudoColorRenderer(rl.dataProvider(), 1)
        color_ramp = QgsGradientColorRamp(QColor(255, 255, 0), QColor(0, 0, 255))
        renderer.setClassificationMin(101)
        renderer.setClassificationMax(131)
        renderer.createShader(color_ramp)
        renderer.setOpacity(0.6)
        rl.setRenderer(renderer)
        rl.resampleFilter().setZoomedInResampler(QgsCubicRasterResampler())
        rl.resampleFilter().setZoomedOutResampler(QgsBilinearRasterResampler())

        p = QgsProject()
        p.addMapLayer(rl)
        project_path = os.path.join(tmp_dir.path(), 'test_project.qgs')
        self.assertTrue(p.write(project_path))

        # simple case, layer still exists in same path
        p2 = QgsProject()
        self.assertTrue(p2.read(project_path))

        self.assertEqual(len(p2.mapLayers()), 1)
        rl2 = list(p2.mapLayers().values())[0]
        self.assertTrue(rl2.isValid())
        self.assertEqual(rl2.name(), 'test_raster')

        self.assertIsInstance(rl2.renderer(), QgsSingleBandPseudoColorRenderer)
        self.assertEqual(rl2.renderer().classificationMin(), 101)
        self.assertEqual(rl2.renderer().classificationMax(), 131)
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color1().name(), '#ffff00')
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color2().name(), '#0000ff')
        self.assertIsInstance(rl2.resampleFilter().zoomedInResampler(), QgsCubicRasterResampler)
        self.assertIsInstance(rl2.resampleFilter().zoomedOutResampler(), QgsBilinearRasterResampler)
        self.assertEqual(rl2.renderer().opacity(), 0.6)

        # now, remove raster
        os.remove(tmp_path)
        # reload project
        p2 = QgsProject()
        self.assertTrue(p2.read(project_path))

        self.assertEqual(len(p2.mapLayers()), 1)
        rl2 = list(p2.mapLayers().values())[0]
        self.assertFalse(rl2.isValid())
        self.assertEqual(rl2.name(), 'test_raster')

        # invalid layers should still have renderer available
        self.assertIsInstance(rl2.renderer(), QgsSingleBandPseudoColorRenderer)
        self.assertEqual(rl2.renderer().classificationMin(), 101)
        self.assertEqual(rl2.renderer().classificationMax(), 131)
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color1().name(), '#ffff00')
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color2().name(), '#0000ff')
        self.assertIsInstance(rl2.resampleFilter().zoomedInResampler(), QgsCubicRasterResampler)
        self.assertIsInstance(rl2.resampleFilter().zoomedOutResampler(), QgsBilinearRasterResampler)
        self.assertEqual(rl2.renderer().opacity(), 0.6)
        # make a little change
        rl2.renderer().setOpacity(0.8)

        # now, fix path
        rl2.setDataSource(source_path, 'test_raster', 'gdal', QgsDataProvider.ProviderOptions())
        self.assertTrue(rl2.isValid())
        self.assertEqual(rl2.name(), 'test_raster')

        # at this stage, the original style should be retained...
        self.assertIsInstance(rl2.renderer(), QgsSingleBandPseudoColorRenderer)
        self.assertEqual(rl2.renderer().classificationMin(), 101)
        self.assertEqual(rl2.renderer().classificationMax(), 131)
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color1().name(), '#ffff00')
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color2().name(), '#0000ff')
        self.assertIsInstance(rl2.resampleFilter().zoomedInResampler(), QgsCubicRasterResampler)
        self.assertIsInstance(rl2.resampleFilter().zoomedOutResampler(), QgsBilinearRasterResampler)
        # the opacity change (and other renderer changes made while the layer was invalid) should be retained
        self.assertEqual(rl2.renderer().opacity(), 0.8)

        # break path
        rl2.setDataSource(tmp_path, 'test_raster', 'gdal', QgsDataProvider.ProviderOptions())
        # and restore
        rl2.setDataSource(source_path, 'test_raster', 'gdal', QgsDataProvider.ProviderOptions())
        self.assertTrue(rl2.isValid())
        self.assertEqual(rl2.name(), 'test_raster')

        # at this stage, the original style should be recreated...
        self.assertIsInstance(rl2.renderer(), QgsSingleBandPseudoColorRenderer)
        self.assertEqual(rl2.renderer().classificationMin(), 101)
        self.assertEqual(rl2.renderer().classificationMax(), 131)
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color1().name(), '#ffff00')
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color2().name(), '#0000ff')
        self.assertIsInstance(rl2.resampleFilter().zoomedInResampler(), QgsCubicRasterResampler)
        self.assertIsInstance(rl2.resampleFilter().zoomedOutResampler(), QgsBilinearRasterResampler)
        self.assertEqual(rl2.renderer().opacity(), 0.8)

        # break again
        rl2.setDataSource(tmp_path, 'test_raster', 'gdal', QgsDataProvider.ProviderOptions())

        # export via qlr, with broken path (but hopefully correct style)
        doc = QgsLayerDefinition.exportLayerDefinitionLayers([rl2], QgsReadWriteContext())
        layers = QgsLayerDefinition.loadLayerDefinitionLayers(doc, QgsReadWriteContext())
        self.assertEqual(len(layers), 1)
        rl2 = layers[0]
        self.assertFalse(rl2.isValid())

        # fix path
        rl2.setDataSource(source_path, 'test_raster', 'gdal', QgsDataProvider.ProviderOptions())
        self.assertTrue(rl2.isValid())
        self.assertEqual(rl2.name(), 'test_raster')

        # at this stage, the original style should be recreated...
        self.assertIsInstance(rl2.renderer(), QgsSingleBandPseudoColorRenderer)
        self.assertEqual(rl2.renderer().classificationMin(), 101)
        self.assertEqual(rl2.renderer().classificationMax(), 131)
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color1().name(), '#ffff00')
        self.assertEqual(rl2.renderer().shader().rasterShaderFunction().sourceColorRamp().color2().name(), '#0000ff')
        self.assertIsInstance(rl2.resampleFilter().zoomedInResampler(), QgsCubicRasterResampler)
        self.assertIsInstance(rl2.resampleFilter().zoomedOutResampler(), QgsBilinearRasterResampler)
        self.assertEqual(rl2.renderer().opacity(), 0.8)

        # another test
        rl = QgsRasterLayer(source_path, 'test_raster', 'gdal')
        self.assertTrue(rl.isValid())
        renderer = QgsSingleBandPseudoColorRenderer(rl.dataProvider(), 1)
        color_ramp = QgsGradientColorRamp(QColor(255, 255, 0), QColor(0, 0, 255))
        renderer.setClassificationMin(101)
        renderer.setClassificationMax(131)
        renderer.createShader(color_ramp)
        renderer.setOpacity(0.6)
        rl.setRenderer(renderer)
        rl.resampleFilter().setZoomedInResampler(QgsCubicRasterResampler())
        rl.resampleFilter().setZoomedOutResampler(QgsBilinearRasterResampler())

        # break path
        rl.setDataSource(tmp_path, 'test_raster', 'gdal', QgsDataProvider.ProviderOptions())

        # fix path
        rl.setDataSource(source_path, 'test_raster', 'gdal', QgsDataProvider.ProviderOptions())
        self.assertIsInstance(rl.renderer(), QgsSingleBandPseudoColorRenderer)
        self.assertEqual(rl.renderer().classificationMin(), 101)
        self.assertEqual(rl.renderer().classificationMax(), 131)
        self.assertEqual(rl.renderer().shader().rasterShaderFunction().sourceColorRamp().color1().name(), '#ffff00')
        self.assertEqual(rl.renderer().shader().rasterShaderFunction().sourceColorRamp().color2().name(), '#0000ff')
        self.assertIsInstance(rl.resampleFilter().zoomedInResampler(), QgsCubicRasterResampler)
        self.assertIsInstance(rl.resampleFilter().zoomedOutResampler(), QgsBilinearRasterResampler)
        self.assertEqual(rl.renderer().opacity(), 0.6)


class TestQgsRasterLayerTransformContext(QgisTestCase):

    def setUp(self):
        """Prepare tc"""
        super().setUp()
        self.ctx = QgsCoordinateTransformContext()
        self.ctx.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857'), 'test')
        self.rpath = os.path.join(unitTestDataPath(), 'landsat.tif')

    def testTransformContextIsSetInCtor(self):
        """Test transform context can be set from ctor"""

        rl = QgsRasterLayer(self.rpath, 'raster')
        self.assertFalse(
            rl.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

        options = QgsRasterLayer.LayerOptions(transformContext=self.ctx)
        rl = QgsRasterLayer(self.rpath, 'raster', 'gdal', options)
        self.assertTrue(
            rl.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

    def testTransformContextInheritsFromProject(self):
        """Test that when a layer is added to a project it inherits its context"""

        rl = QgsRasterLayer(self.rpath, 'raster')
        self.assertFalse(
            rl.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

        p = QgsProject()
        self.assertFalse(
            p.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))
        p.setTransformContext(self.ctx)
        self.assertTrue(
            p.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

        p.addMapLayers([rl])
        self.assertTrue(
            rl.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

    def testTransformContextIsSyncedFromProject(self):
        """Test that when a layer is synced when project context changes"""

        rl = QgsRasterLayer(self.rpath, 'raster')
        self.assertFalse(
            rl.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

        p = QgsProject()
        self.assertFalse(
            p.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))
        p.setTransformContext(self.ctx)
        self.assertTrue(
            p.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

        p.addMapLayers([rl])
        self.assertTrue(
            rl.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

        # Now change the project context
        tc2 = QgsCoordinateTransformContext()
        p.setTransformContext(tc2)
        self.assertFalse(
            p.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))
        self.assertFalse(
            rl.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))
        p.setTransformContext(self.ctx)
        self.assertTrue(
            p.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))
        self.assertTrue(
            rl.transformContext().hasTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857')))

    def test_save_restore_pipe_data_defined_settings(self):
        """
        Test that raster pipe data defined settings are correctly saved/restored along with the layer
        """
        rl = QgsRasterLayer(self.rpath, 'raster')
        rl.pipe().dataDefinedProperties().setProperty(QgsRasterPipe.Property.RendererOpacity, QgsProperty.fromExpression('100/2'))

        doc = QDomDocument()
        layer_elem = doc.createElement("maplayer")
        self.assertTrue(rl.writeLayerXml(layer_elem, doc, QgsReadWriteContext()))

        rl2 = QgsRasterLayer(self.rpath, 'raster')
        self.assertEqual(rl2.pipe().dataDefinedProperties().property(QgsRasterPipe.Property.RendererOpacity),
                         QgsProperty())

        self.assertTrue(rl2.readXml(layer_elem, QgsReadWriteContext()))
        self.assertEqual(rl2.pipe().dataDefinedProperties().property(QgsRasterPipe.Property.RendererOpacity),
                         QgsProperty.fromExpression('100/2'))

    def test_render_data_defined_opacity(self):
        path = os.path.join(unitTestDataPath('raster'),
                            'band1_float32_noct_epsg4326.tif')
        raster_layer = QgsRasterLayer(path, 'test')
        self.assertTrue(raster_layer.isValid())

        renderer = QgsSingleBandGrayRenderer(raster_layer.dataProvider(), 1)
        raster_layer.setRenderer(renderer)
        raster_layer.setContrastEnhancement(
            QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum,
            QgsRasterMinMaxOrigin.Limits.MinMax)

        raster_layer.pipe().dataDefinedProperties().setProperty(QgsRasterPipe.Property.RendererOpacity, QgsProperty.fromExpression('@layer_opacity'))

        ce = raster_layer.renderer().contrastEnhancement()
        ce.setMinimumValue(-3.3319999287625854e+38)
        ce.setMaximumValue(3.3999999521443642e+38)

        map_settings = QgsMapSettings()
        map_settings.setLayers([raster_layer])
        map_settings.setExtent(raster_layer.extent())

        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable('layer_opacity', 50)
        context.appendScope(scope)
        map_settings.setExpressionContext(context)

        self.assertTrue(
            self.render_map_settings_check(
                'raster_data_defined_opacity',
                'raster_data_defined_opacity',
                map_settings)
        )

    def test_read_xml_crash(self):
        """Check if converting a raster from 1.8 to 2 works."""
        path = os.path.join(unitTestDataPath('raster'),
                            'raster-palette-crash2.tif')
        layer = QgsRasterLayer(path, QFileInfo(path).baseName())
        context = QgsReadWriteContext()
        document = QDomDocument("style")
        map_layers_element = document.createElement("maplayers")
        map_layer_element = document.createElement("maplayer")
        layer.writeLayerXml(map_layer_element, document, context)

        num = 10
        for _ in range(num):
            layer.readLayerXml(map_layer_element, context)


if __name__ == '__main__':
    unittest.main()
