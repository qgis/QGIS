# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithmTests.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthias Kuhn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import AlgorithmsTestBase
from processing.algs.gdal.OgrToPostGis import OgrToPostGis
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.algs.gdal.AssignProjection import AssignProjection
from processing.algs.gdal.ClipRasterByExtent import ClipRasterByExtent
from processing.algs.gdal.ClipRasterByMask import ClipRasterByMask
from processing.algs.gdal.gdal2tiles import gdal2tiles
from processing.algs.gdal.gdalcalc import gdalcalc
from processing.algs.gdal.gdaltindex import gdaltindex
from processing.algs.gdal.contour import contour
from processing.algs.gdal.gdalinfo import gdalinfo
from processing.algs.gdal.GridAverage import GridAverage
from processing.algs.gdal.GridDataMetrics import GridDataMetrics
from processing.algs.gdal.GridInverseDistance import GridInverseDistance
from processing.algs.gdal.GridInverseDistanceNearestNeighbor import GridInverseDistanceNearestNeighbor
from processing.algs.gdal.GridLinear import GridLinear
from processing.algs.gdal.GridNearestNeighbor import GridNearestNeighbor
from processing.algs.gdal.buildvrt import buildvrt
from processing.algs.gdal.hillshade import hillshade
from processing.algs.gdal.ogr2ogr import ogr2ogr
from processing.algs.gdal.ogrinfo import ogrinfo
from processing.algs.gdal.proximity import proximity
from processing.algs.gdal.rasterize import rasterize
from processing.algs.gdal.retile import retile
from processing.algs.gdal.translate import translate
from processing.algs.gdal.warp import warp
from processing.algs.gdal.fillnodata import fillnodata
from processing.algs.gdal.rearrange_bands import rearrange_bands

from qgis.core import (QgsProcessingContext,
                       QgsProcessingFeedback,
                       QgsCoordinateReferenceSystem,
                       QgsApplication,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsProject,
                       QgsVectorLayer,
                       QgsRectangle,
                       QgsProcessingException,
                       QgsProcessingFeatureSourceDefinition)
import nose2
import os
import shutil
import tempfile

from qgis.testing import (
    start_app,
    unittest
)

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')


class TestGdalAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing
        Processing.initialize()
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        return 'gdal_algorithm_tests.yaml'

    def testCommandName(self):
        # Test that algorithms report a valid commandName
        p = QgsApplication.processingRegistry().providerById('gdal')
        for a in p.algorithms():
            self.assertTrue(a.commandName(), 'Algorithm {} has no commandName!'.format(a.id()))

    def testCommandNameInTags(self):
        # Test that algorithms commandName is present in provided tags
        p = QgsApplication.processingRegistry().providerById('gdal')
        for a in p.algorithms():
            self.assertTrue(a.commandName() in a.tags(), 'Algorithm {} commandName not found in tags!'.format(a.id()))

    def testNoParameters(self):
        # Test that algorithms throw QgsProcessingExceptions and not base Python
        # exceptions when no parameters specified
        p = QgsApplication.processingRegistry().providerById('gdal')
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        for a in p.algorithms():
            try:
                a.getConsoleCommands({}, context, feedback)
            except QgsProcessingException:
                pass

    def testGetOgrCompatibleSourceFromMemoryLayer(self):
        # create a memory layer and add to project and context
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "testmem", "memory")
        self.assertTrue(layer.isValid())
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        self.assertTrue(pr.addFeatures([f, f2]))
        self.assertEqual(layer.featureCount(), 2)
        QgsProject.instance().addMapLayer(layer)
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        alg = QgsApplication.processingRegistry().createAlgorithmById('gdal:buffervectors')
        self.assertIsNotNone(alg)
        parameters = {'INPUT': 'testmem'}
        feedback = QgsProcessingFeedback()
        # check that memory layer is automatically saved out to shape when required by GDAL algorithms
        ogr_data_path, ogr_layer_name = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback,
                                                                   executing=True)
        self.assertTrue(ogr_data_path)
        self.assertTrue(ogr_data_path.endswith('.shp'))
        self.assertTrue(os.path.exists(ogr_data_path))
        self.assertTrue(ogr_layer_name)

        # make sure that layer has correct features
        res = QgsVectorLayer(ogr_data_path, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 2)

        # with memory layers - if not executing layer source should be ignored and replaced
        # with a dummy path, because:
        # - it has no meaning for the gdal command outside of QGIS, memory layers don't exist!
        # - we don't want to force an export of the whole memory layer to a temp file just to show the command preview
        # this might be very slow!
        ogr_data_path, ogr_layer_name = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback,
                                                                   executing=False)
        self.assertEqual(ogr_data_path, 'path_to_data_file')
        self.assertEqual(ogr_layer_name, 'layer_name')

        QgsProject.instance().removeMapLayer(layer)

    def testGetOgrCompatibleSourceFromOgrLayer(self):
        p = QgsProject()
        source = os.path.join(testDataPath, 'points.gml')
        vl = QgsVectorLayer(source)
        self.assertTrue(vl.isValid())
        p.addMapLayer(vl)

        context = QgsProcessingContext()
        context.setProject(p)
        feedback = QgsProcessingFeedback()

        alg = ogr2ogr()
        alg.initAlgorithm()
        path, layer = alg.getOgrCompatibleSource('INPUT', {'INPUT': vl.id()}, context, feedback, True)
        self.assertEqual(path, source)
        path, layer = alg.getOgrCompatibleSource('INPUT', {'INPUT': vl.id()}, context, feedback, False)
        self.assertEqual(path, source)

        # with selected features only - if not executing, the 'selected features only' setting
        # should be ignored (because it has no meaning for the gdal command outside of QGIS!)
        parameters = {'INPUT': QgsProcessingFeatureSourceDefinition(vl.id(), True)}
        path, layer = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback, False)
        self.assertEqual(path, source)

    def testGetOgrCompatibleSourceFromFeatureSource(self):
        # create a memory layer and add to project and context
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "testmem", "memory")
        self.assertTrue(layer.isValid())
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        self.assertTrue(pr.addFeatures([f, f2]))
        self.assertEqual(layer.featureCount(), 2)
        # select first feature
        layer.selectByIds([next(layer.getFeatures()).id()])
        self.assertEqual(len(layer.selectedFeatureIds()), 1)
        QgsProject.instance().addMapLayer(layer)
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        alg = QgsApplication.processingRegistry().createAlgorithmById('gdal:buffervectors')
        self.assertIsNotNone(alg)
        parameters = {'INPUT': QgsProcessingFeatureSourceDefinition('testmem', True)}
        feedback = QgsProcessingFeedback()
        # check that memory layer is automatically saved out to shape when required by GDAL algorithms
        ogr_data_path, ogr_layer_name = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback,
                                                                   executing=True)
        self.assertTrue(ogr_data_path)
        self.assertTrue(ogr_data_path.endswith('.shp'))
        self.assertTrue(os.path.exists(ogr_data_path))
        self.assertTrue(ogr_layer_name)

        # make sure that layer has only selected feature
        res = QgsVectorLayer(ogr_data_path, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 1)

        QgsProject.instance().removeMapLayer(layer)

    def testOgrLayerNameExtraction(self):
        outdir = tempfile.mkdtemp()
        self.cleanup_paths.append(outdir)

        def _copyFile(dst):
            shutil.copyfile(os.path.join(testDataPath, 'custom', 'grass7', 'weighted.csv'), dst)

        # OGR provider - single layer
        _copyFile(os.path.join(outdir, 'a.csv'))
        name = GdalUtils.ogrLayerName(outdir)
        self.assertEqual(name, 'a')

        # OGR provider - multiple layers
        _copyFile(os.path.join(outdir, 'b.csv'))
        name1 = GdalUtils.ogrLayerName(outdir + '|layerid=0')
        name2 = GdalUtils.ogrLayerName(outdir + '|layerid=1')
        self.assertEqual(sorted([name1, name2]), ['a', 'b'])

        name = GdalUtils.ogrLayerName(outdir + '|layerid=2')
        self.assertIsNone(name)

        # OGR provider - layername takes precedence
        name = GdalUtils.ogrLayerName(outdir + '|layername=f')
        self.assertEqual(name, 'f')

        name = GdalUtils.ogrLayerName(outdir + '|layerid=0|layername=f')
        self.assertEqual(name, 'f')

        name = GdalUtils.ogrLayerName(outdir + '|layername=f|layerid=0')
        self.assertEqual(name, 'f')

        # SQLiite provider
        name = GdalUtils.ogrLayerName('dbname=\'/tmp/x.sqlite\' table="t" (geometry) sql=')
        self.assertEqual(name, 't')

        # PostgreSQL provider
        name = GdalUtils.ogrLayerName(
            'port=5493 sslmode=disable key=\'edge_id\' srid=0 type=LineString table="city_data"."edge" (geom) sql=')
        self.assertEqual(name, 'city_data.edge')

    def testOgrConnectionStringAndFormat(self):
        context = QgsProcessingContext()
        output, outputFormat = GdalUtils.ogrConnectionStringAndFormat('d:/test/test.shp', context)
        self.assertEqual(output, 'd:/test/test.shp')
        self.assertEqual(outputFormat, '"ESRI Shapefile"')
        output, outputFormat = GdalUtils.ogrConnectionStringAndFormat('d:/test/test.mif', context)
        self.assertEqual(output, 'd:/test/test.mif')
        self.assertEqual(outputFormat, '"MapInfo File"')

    def testCrsConversion(self):
        self.assertFalse(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem()))
        self.assertEqual(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem('EPSG:3111')), 'EPSG:3111')
        self.assertEqual(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem('POSTGIS:3111')), 'EPSG:3111')
        self.assertEqual(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem(
            'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')),
            'EPSG:20936')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromProj4(
            '+proj=utm +zone=36 +south +a=600000 +b=70000 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')
        self.assertTrue(crs.isValid())
        self.assertEqual(GdalUtils.gdal_crs_string(crs),
                         '+proj=utm +zone=36 +south +a=600000 +b=70000 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')

    def testAssignProjection(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = AssignProjection()
        alg.initAlgorithm()

        # with target srs
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'CRS': 'EPSG:3111'}, context, feedback),
            ['gdal_edit.py',
             '-a_srs EPSG:3111 ' +
             source])

        # with target using proj string
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'CRS': custom_crs}, context, feedback),
            ['gdal_edit.py',
             '-a_srs EPSG:20936 ' +
             source])

        # with target using custom projection
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'CRS': custom_crs}, context, feedback),
            ['gdal_edit.py',
             '-a_srs "+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs" ' +
             source])

        # with non-EPSG crs code
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'CRS': 'POSTGIS:3111'}, context, feedback),
            ['gdal_edit.py',
             '-a_srs EPSG:3111 ' +
             source])

    def testGdalTranslate(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        translate_alg = translate()
        translate_alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with None NODATA value
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'NODATA': None,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'NODATA': 9999,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_nodata 9999.0 ' +
             '-of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'NODATA': 0,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_nodata 0.0 ' +
             '-of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value and custom data type
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'NODATA': 0,
                                              'DATA_TYPE': 6,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_nodata 0.0 ' +
             '-ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with target srs
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'TARGET_CRS': 'EPSG:3111',
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_srs EPSG:3111 ' +
             '-of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with target using proj string
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'TARGET_CRS': custom_crs,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_srs EPSG:20936 ' +
             '-of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with target using custom projection
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'TARGET_CRS': custom_crs,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_srs "+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs" ' +
             '-of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with non-EPSG crs code
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'TARGET_CRS': 'POSTGIS:3111',
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_srs EPSG:3111 ' +
             '-of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with copy subdatasets
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'COPY_SUBDATASETS': True,
                                              'OUTPUT': 'd:/temp/check.tif'}, context, feedback),
            ['gdal_translate',
             '-sds ' +
             '-of GTiff ' +
             source + ' ' +
             'd:/temp/check.tif'])

    def testClipRasterByExtent(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = ClipRasterByExtent()
        alg.initAlgorithm()
        extent = QgsRectangle(1, 2, 3, 4)

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'EXTENT': extent,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-projwin 0.0 0.0 0.0 0.0 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'EXTENT': extent,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-projwin 0.0 0.0 0.0 0.0 -a_nodata 9999.0 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'EXTENT': extent,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-projwin 0.0 0.0 0.0 0.0 -a_nodata 0.0 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value and custom data type
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'EXTENT': extent,
                                    'NODATA': 0,
                                    'DATA_TYPE': 6,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-projwin 0.0 0.0 0.0 0.0 -a_nodata 0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testClipRasterByMask(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        mask = os.path.join(testDataPath, 'polys.gml')
        alg = ClipRasterByMask()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MASK': mask,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-of JPEG -cutline ' +
             mask + ' -crop_to_cutline ' + source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MASK': mask,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-of JPEG -cutline ' +
             mask + ' -crop_to_cutline -dstnodata 9999.0 ' + source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MASK': mask,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-of JPEG -cutline ' +
             mask + ' -crop_to_cutline -dstnodata 0.0 ' + source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value and custom data type
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MASK': mask,
                                    'NODATA': 0,
                                    'DATA_TYPE': 6,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-ot Float32 -of JPEG -cutline ' +
             mask + ' -crop_to_cutline -dstnodata 0.0 ' + source + ' ' +
             'd:/temp/check.jpg'])

    def testContour(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = contour()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'FIELD_NAME': 'elev',
                                    'INTERVAL': 5,
                                    'OUTPUT': 'd:/temp/check.shp'}, context, feedback),
            ['gdal_contour',
             '-b 1 -a elev -i 5.0 -f "ESRI Shapefile" ' +
             source + ' ' +
             'd:/temp/check.shp'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'FIELD_NAME': 'elev',
                                    'INTERVAL': 5,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.shp'}, context, feedback),
            ['gdal_contour',
             '-b 1 -a elev -i 5.0 -snodata 9999.0 -f "ESRI Shapefile" ' +
             source + ' ' +
             'd:/temp/check.shp'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'FIELD_NAME': 'elev',
                                    'INTERVAL': 5,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.gpkg'}, context, feedback),
            ['gdal_contour',
             '-b 1 -a elev -i 5.0 -snodata 0.0 -f "GPKG" ' +
             source + ' ' +
             'd:/temp/check.gpkg'])
        # fixed level contours
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'FIELD_NAME': 'elev',
                                    'INTERVAL': 0,
                                    'OPTIONS': '-fl 100 125 150 200'
                                    'OUTPUT': 'd:/temp/check.shp'}, context, feedback),
            ['gdal_contour',
             '-b 1 -a elev -i 0.0 -fl 100 125 150 200 -f "ESRI Shapefile" ' +
             source + ' ' +
             'd:/temp/check.shp'])

    def testGdal2Tiles(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = gdal2tiles()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/'}, context, feedback),
            ['gdal2tiles.py',
             '-p mercator -w all -r average ' +
             source + ' ' +
             'd:/temp/'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': -9999,
                                    'OUTPUT': 'd:/temp/'}, context, feedback),
            ['gdal2tiles.py',
             '-p mercator -w all -r average -a -9999.0 ' +
             source + ' ' +
             'd:/temp/'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/'}, context, feedback),
            ['gdal2tiles.py',
             '-p mercator -w all -r average -a 0.0 ' +
             source + ' ' +
             'd:/temp/'])

        # with input srs
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'OUTPUT': 'd:/temp/'}, context, feedback),
            ['gdal2tiles.py',
             '-p mercator -w all -r average -s EPSG:3111 ' +
             source + ' ' +
             'd:/temp/'])

        # with target using proj string
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': custom_crs,
                                    'OUTPUT': 'd:/temp/'}, context, feedback),
            ['gdal2tiles.py',
             '-p mercator -w all -r average -s EPSG:20936 ' +
             source + ' ' +
             'd:/temp/'])

        # with target using custom projection
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': custom_crs,
                                    'OUTPUT': 'd:/temp/'}, context, feedback),
            ['gdal2tiles.py',
             '-p mercator -w all -r average -s "+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs" ' +
             source + ' ' +
             'd:/temp/'])

        # with non-EPSG crs code
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': 'POSTGIS:3111',
                                    'OUTPUT': 'd:/temp/'}, context, feedback),
            ['gdal2tiles.py',
             '-p mercator -w all -r average -s EPSG:3111 ' +
             source + ' ' +
             'd:/temp/'])

    def testGdalCalc(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = gdalcalc()
        alg.initAlgorithm()

        output = 'd:/temp/check.jpg'

        # default execution
        formula = 'A*2' # default formula
        self.assertEqual(
            alg.getConsoleCommands({
                'INPUT_A': source,
                'BAND_A': 1,
                'FORMULA': formula,
                'BAND_D': -1,
                'NO_DATA': None,
                'BAND_F': -1,
                'BAND_B': -1,
                'RTYPE': 5,
                'INPUT_F': None,
                'BAND_E': -1,
                'INPUT_D': None,
                'INPUT_B': None,
                'BAND_C': -1,
                'INPUT_E': None,
                'INPUT_C': None,
                'OUTPUT': output}, context, feedback),
            ['gdal_calc', '--calc "{}" --format JPEG --type Float32 -A {} --A_band 1 --outfile {}'.format(formula, source, output)])

        # check that formula is not escaped and formula is returned as it is
        formula = 'A * 2'  # <--- add spaces in the formula
        self.assertEqual(
            alg.getConsoleCommands({
                'INPUT_A': source,
                'BAND_A': 1,
                'FORMULA': formula,
                'BAND_D': -1,
                'NO_DATA': None,
                'BAND_F': -1,
                'BAND_B': -1,
                'RTYPE': 5,
                'INPUT_F': None,
                'BAND_E': -1,
                'INPUT_D': None,
                'INPUT_B': None,
                'BAND_C': -1,
                'INPUT_E': None,
                'INPUT_C': None,
                'OUTPUT': output}, context, feedback),
            ['gdal_calc', '--calc "{}" --format JPEG --type Float32 -A {} --A_band 1 --outfile {}'.format(formula, source, output)])

    def testBuildVrt(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = buildvrt()
        alg.initAlgorithm()

        commands = alg.getConsoleCommands({'LAYERS': [source],
                                           'OUTPUT': 'd:/temp/test.vrt'}, context, feedback)
        self.assertEqual(len(commands), 2)
        self.assertEqual(commands[0], 'gdalbuildvrt')
        self.assertIn('-resolution average', commands[1])
        self.assertIn('-separate', commands[1])
        self.assertNotIn('-allow_projection_difference', commands[1])
        self.assertNotIn('-add_alpha', commands[1])
        self.assertNotIn('-a_srs', commands[1])
        self.assertIn('-r nearest', commands[1])
        self.assertIn('-input_file_list', commands[1])
        self.assertIn('d:/temp/test.vrt', commands[1])

        commands = alg.getConsoleCommands({'LAYERS': [source],
                                           'RESOLUTION': 2,
                                           'OUTPUT': 'd:/temp/test.vrt'}, context, feedback)
        self.assertEqual(len(commands), 2)
        self.assertEqual(commands[0], 'gdalbuildvrt')
        self.assertIn('-resolution lowest', commands[1])
        self.assertIn('-separate', commands[1])
        self.assertNotIn('-allow_projection_difference', commands[1])
        self.assertNotIn('-add_alpha', commands[1])
        self.assertNotIn('-a_srs', commands[1])
        self.assertIn('-r nearest', commands[1])
        self.assertIn('-input_file_list', commands[1])
        self.assertIn('d:/temp/test.vrt', commands[1])

        commands = alg.getConsoleCommands({'LAYERS': [source],
                                           'SEPARATE': False,
                                           'OUTPUT': 'd:/temp/test.vrt'}, context, feedback)
        self.assertEqual(len(commands), 2)
        self.assertEqual(commands[0], 'gdalbuildvrt')
        self.assertIn('-resolution average', commands[1])
        self.assertNotIn('-allow_projection_difference', commands[1])
        self.assertNotIn('-separate', commands[1])
        self.assertNotIn('-add_alpha', commands[1])
        self.assertNotIn('-a_srs', commands[1])
        self.assertIn('-r nearest', commands[1])
        self.assertIn('-input_file_list', commands[1])
        self.assertIn('d:/temp/test.vrt', commands[1])

        commands = alg.getConsoleCommands({'LAYERS': [source],
                                           'PROJ_DIFFERENCE': True,
                                           'OUTPUT': 'd:/temp/test.vrt'}, context, feedback)
        self.assertEqual(len(commands), 2)
        self.assertEqual(commands[0], 'gdalbuildvrt')
        self.assertIn('-resolution average', commands[1])
        self.assertIn('-allow_projection_difference', commands[1])
        self.assertIn('-separate', commands[1])
        self.assertNotIn('-add_alpha', commands[1])
        self.assertNotIn('-a_srs', commands[1])
        self.assertIn('-r nearest', commands[1])
        self.assertIn('-input_file_list', commands[1])
        self.assertIn('d:/temp/test.vrt', commands[1])

        commands = alg.getConsoleCommands({'LAYERS': [source],
                                           'ADD_ALPHA': True,
                                           'OUTPUT': 'd:/temp/test.vrt'}, context, feedback)
        self.assertEqual(len(commands), 2)
        self.assertEqual(commands[0], 'gdalbuildvrt')
        self.assertIn('-resolution average', commands[1])
        self.assertIn('-separate', commands[1])
        self.assertNotIn('-allow_projection_difference', commands[1])
        self.assertNotIn('-add_alpha', commands[1])
        self.assertNotIn('-a_srs', commands[1])
        self.assertIn('-r nearest', commands[1])
        self.assertIn('-input_file_list', commands[1])
        self.assertIn('d:/temp/test.vrt', commands[1])

        commands = alg.getConsoleCommands({'LAYERS': [source],
                                           'ASSIGN_CRS': 'EPSG:3111',
                                           'OUTPUT': 'd:/temp/test.vrt'}, context, feedback)
        self.assertEqual(len(commands), 2)
        self.assertEqual(commands[0], 'gdalbuildvrt')
        self.assertIn('-resolution average', commands[1])
        self.assertIn('-separate', commands[1])
        self.assertNotIn('-allow_projection_difference', commands[1])
        self.assertNotIn('-add_alpha', commands[1])
        self.assertIn('-a_srs EPSG:3111', commands[1])
        self.assertIn('-r nearest', commands[1])
        self.assertIn('-input_file_list', commands[1])
        self.assertIn('d:/temp/test.vrt', commands[1])

        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        commands = alg.getConsoleCommands({'LAYERS': [source],
                                           'ASSIGN_CRS': custom_crs,
                                           'OUTPUT': 'd:/temp/test.vrt'}, context, feedback)
        self.assertEqual(len(commands), 2)
        self.assertEqual(commands[0], 'gdalbuildvrt')
        self.assertIn('-resolution average', commands[1])
        self.assertIn('-separate', commands[1])
        self.assertNotIn('-allow_projection_difference', commands[1])
        self.assertNotIn('-add_alpha', commands[1])
        self.assertIn('-a_srs EPSG:20936', commands[1])
        self.assertIn('-r nearest', commands[1])
        self.assertIn('-input_file_list', commands[1])
        self.assertIn('d:/temp/test.vrt', commands[1])

        commands = alg.getConsoleCommands({'LAYERS': [source],
                                           'RESAMPLING': 4,
                                           'OUTPUT': 'd:/temp/test.vrt'}, context, feedback)
        self.assertEqual(len(commands), 2)
        self.assertEqual(commands[0], 'gdalbuildvrt')
        self.assertIn('-resolution average', commands[1])
        self.assertIn('-separate', commands[1])
        self.assertNotIn('-allow_projection_difference', commands[1])
        self.assertNotIn('-add_alpha', commands[1])
        self.assertNotIn('-a_srs', commands[1])
        self.assertIn('-r lanczos', commands[1])
        self.assertIn('-input_file_list', commands[1])
        self.assertIn('d:/temp/test.vrt', commands[1])

    def testGdalInfo(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = gdalinfo()
        alg.initAlgorithm()

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MIN_MAX': False,
                                    'NOGCP': False,
                                    'NO_METADATA': False,
                                    'STATS': False}, context, feedback),
            ['gdalinfo',
             source])

        source = os.path.join(testDataPath, 'raster with spaces.tif')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MIN_MAX': False,
                                    'NOGCP': False,
                                    'NO_METADATA': False,
                                    'STATS': False}, context, feedback),
            ['gdalinfo',
             '"' + source + '"'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MIN_MAX': True,
                                    'NOGCP': False,
                                    'NO_METADATA': False,
                                    'STATS': False}, context, feedback),
            ['gdalinfo',
             '-mm "' + source + '"'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MIN_MAX': False,
                                    'NOGCP': True,
                                    'NO_METADATA': False,
                                    'STATS': False}, context, feedback),
            ['gdalinfo',
             '-nogcp "' + source + '"'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MIN_MAX': False,
                                    'NOGCP': False,
                                    'NO_METADATA': True,
                                    'STATS': False}, context, feedback),
            ['gdalinfo',
             '-nomd "' + source + '"'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MIN_MAX': False,
                                    'NOGCP': False,
                                    'NO_METADATA': False,
                                    'STATS': True}, context, feedback),
            ['gdalinfo',
             '-stats "' + source + '"'])

    def testGdalTindex(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = gdaltindex()
        alg.initAlgorithm()

        self.assertEqual(
            alg.getConsoleCommands({'LAYERS': [source],
                                    'OUTPUT': 'd:/temp/test.shp'}, context, feedback),
            ['gdaltindex',
             '-tileindex location -f "ESRI Shapefile" ' +
             'd:/temp/test.shp ' +
             source])

        # with input srs
        self.assertEqual(
            alg.getConsoleCommands({'LAYERS': [source],
                                    'TARGET_CRS': 'EPSG:3111',
                                    'OUTPUT': 'd:/temp/test.shp'}, context, feedback),
            ['gdaltindex',
             '-tileindex location -t_srs EPSG:3111 -f "ESRI Shapefile" ' +
             'd:/temp/test.shp ' +
             source])

        # with target using proj string
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'LAYERS': [source],
                                    'TARGET_CRS': custom_crs,
                                    'OUTPUT': 'd:/temp/test.shp'}, context, feedback),
            ['gdaltindex',
             '-tileindex location -t_srs EPSG:20936 -f "ESRI Shapefile" ' +
             'd:/temp/test.shp ' +
             source])

        # with target using custom projection
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'LAYERS': [source],
                                    'TARGET_CRS': custom_crs,
                                    'OUTPUT': 'd:/temp/test.shp'}, context, feedback),
            ['gdaltindex',
             '-tileindex location -t_srs "+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs" -f "ESRI Shapefile" ' +
             'd:/temp/test.shp ' +
             source])

        # with non-EPSG crs code
        self.assertEqual(
            alg.getConsoleCommands({'LAYERS': [source],
                                    'TARGET_CRS': 'POSTGIS:3111',
                                    'OUTPUT': 'd:/temp/test.shp'}, context, feedback),
            ['gdaltindex',
             '-tileindex location -t_srs EPSG:3111 -f "ESRI Shapefile" ' +
             'd:/temp/test.shp ' +
             source])

    def testGridAverage(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridAverage()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=9999.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testGridDataMetrics(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridDataMetrics()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=9999.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testGridInverseDistance(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridInverseDistance()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a invdist:power=2.0:smothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a invdist:power=2.0:smothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=9999.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a invdist:power=2.0:smothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testGridInverseDistanceNearestNeighbour(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridInverseDistanceNearestNeighbor()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a invdistnn:power=2.0:smothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a invdistnn:power=2.0:smothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=9999.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a invdistnn:power=2.0:smothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testGridLinear(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridLinear()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a linear:radius=-1.0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a linear:radius=-1.0:nodata=9999.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a linear:radius=-1.0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testGridNearestNeighbour(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridNearestNeighbor()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=9999.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_grid',
             '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testOgr2Ogr(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        alg = ogr2ogr()
        alg.initAlgorithm()

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/check.shp'}, context, feedback),
            ['ogr2ogr',
             '-f "ESRI Shapefile" d:/temp/check.shp ' +
             source + ' polys2'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/check.kml'}, context, feedback),
            ['ogr2ogr',
             '-f "LIBKML" d:/temp/check.kml ' +
             source + ' polys2'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OUTPUT': 'd:/temp/my out/check.kml'}, context, feedback),
            ['ogr2ogr',
             '-f "LIBKML" "d:/temp/my out/check.kml" ' +
             source + ' polys2'])

    def testOgrInfo(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        alg = ogrinfo()
        alg.initAlgorithm()

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SUMMARY_ONLY': True,
                                    'NO_METADATA': False}, context, feedback),
            ['ogrinfo',
             '-al -so ' +
             source])

        source = os.path.join(testDataPath, 'filename with spaces.gml')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SUMMARY_ONLY': True,
                                    'NO_METADATA': False}, context, feedback),
            ['ogrinfo',
             '-al -so "' +
             source + '"'])

        source = os.path.join(testDataPath, 'filename with spaces.gml')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SUMMARY_ONLY': False,
                                    'NO_METADATA': False}, context, feedback),
            ['ogrinfo',
             '-al "' +
             source + '"'])

        source = os.path.join(testDataPath, 'filename with spaces.gml')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SUMMARY_ONLY': True,
                                    'NO_METADATA': True}, context, feedback),
            ['ogrinfo',
             '-al -so -nomd "' +
             source + '"'])

    def testHillshade(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = hillshade()
        alg.initAlgorithm()

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'Z_FACTOR': 5,
                                    'SCALE': 2,
                                    'AZIMUTH': 90,
                                    'ALTITUDE': 20,
                                    'OUTPUT': 'd:/temp/check.tif'}, context, feedback),
            ['gdaldem',
             'hillshade ' +
             source + ' ' +
             'd:/temp/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0'])

        #paths with space
        source_with_space = os.path.join(testDataPath, 'raster with spaces.tif')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source_with_space,
                                    'BAND': 1,
                                    'Z_FACTOR': 5,
                                    'SCALE': 2,
                                    'AZIMUTH': 90,
                                    'ALTITUDE': 20,
                                    'OUTPUT': 'd:/temp/check out.tif'}, context, feedback),
            ['gdaldem',
             'hillshade ' +
             '"' + source_with_space + '" ' +
             '"d:/temp/check out.tif" -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0'])

        # compute edges
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'Z_FACTOR': 5,
                                    'SCALE': 2,
                                    'AZIMUTH': 90,
                                    'ALTITUDE': 20,
                                    'COMPUTE_EDGES': True,
                                    'OUTPUT': 'd:/temp/check.tif'}, context, feedback),
            ['gdaldem',
             'hillshade ' +
             source + ' ' +
             'd:/temp/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0 -compute_edges'])

        # with ZEVENBERGEN
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'Z_FACTOR': 5,
                                    'SCALE': 2,
                                    'AZIMUTH': 90,
                                    'ALTITUDE': 20,
                                    'ZEVENBERGEN': True,
                                    'OUTPUT': 'd:/temp/check.tif'}, context, feedback),
            ['gdaldem',
             'hillshade ' +
             source + ' ' +
             'd:/temp/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0 -alg ZevenbergenThorne'])

        # with COMBINED
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'Z_FACTOR': 5,
                                    'SCALE': 2,
                                    'AZIMUTH': 90,
                                    'ALTITUDE': 20,
                                    'COMBINED': True,
                                    'OUTPUT': 'd:/temp/check.tif'}, context, feedback),
            ['gdaldem',
             'hillshade ' +
             source + ' ' +
             'd:/temp/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0 -combined'])

        # with multidirectional - "az" argument is not allowed!
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'Z_FACTOR': 5,
                                    'SCALE': 2,
                                    'AZIMUTH': 90,
                                    'ALTITUDE': 20,
                                    'MULTIDIRECTIONAL': True,
                                    'OUTPUT': 'd:/temp/check.tif'}, context, feedback),
            ['gdaldem',
             'hillshade ' +
             source + ' ' +
             'd:/temp/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -alt 20.0 -multidirectional'])

    def testProximity(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = proximity()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_proximity.py',
             '-srcband 1 -distunits PIXEL -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'BAND': 2,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_proximity.py',
             '-srcband 2 -distunits PIXEL -nodata 9999.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'BAND': 1,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_proximity.py',
             '-srcband 1 -distunits PIXEL -nodata 0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testRasterize(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        alg = rasterize()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'FIELD': 'id',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_rasterize',
             '-l polys2 -a id -ts 0.0 0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'FIELD': 'id',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_rasterize',
             '-l polys2 -a id -ts 0.0 0.0 -a_nodata 9999.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'FIELD': 'id',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_rasterize',
             '-l polys2 -a id -ts 0.0 0.0 -a_nodata 0.0 -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testRetile(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = retile()
        alg.initAlgorithm()

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': [source],
                                    'OUTPUT': 'd:/temp'}, context, feedback),
            ['gdal_retile.py',
             '-ps 256 256 -overlap 0 -levels 1 -r near -ot Float32 -targetDir d:/temp ' +
             source])

        # with input srs
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': [source],
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'OUTPUT': 'd:/temp'}, context, feedback),
            ['gdal_retile.py',
             '-ps 256 256 -overlap 0 -levels 1 -s_srs EPSG:3111 -r near -ot Float32 -targetDir d:/temp ' +
             source])

        # with target using proj string
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': [source],
                                    'SOURCE_CRS': custom_crs,
                                    'OUTPUT': 'd:/temp'}, context, feedback),
            ['gdal_retile.py',
             '-ps 256 256 -overlap 0 -levels 1 -s_srs EPSG:20936 -r near -ot Float32 -targetDir d:/temp ' +
             source])

        # with target using custom projection
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': [source],
                                    'SOURCE_CRS': custom_crs,
                                    'OUTPUT': 'd:/temp'}, context, feedback),
            ['gdal_retile.py',
             '-ps 256 256 -overlap 0 -levels 1 -s_srs "+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs" -r near -ot Float32 -targetDir d:/temp ' +
             source])

        # with non-EPSG crs code
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': [source],
                                    'SOURCE_CRS': 'POSTGIS:3111',
                                    'OUTPUT': 'd:/temp'}, context, feedback),
            ['gdal_retile.py',
             '-ps 256 256 -overlap 0 -levels 1 -s_srs EPSG:3111 -r near -ot Float32 -targetDir d:/temp ' +
             source])

    def testWarp(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = warp()
        alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:4326 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with None NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': None,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:4326 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 9999,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:4326 -dstnodata 9999.0 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:4326 -dstnodata 0.0 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value and custom data type
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'NODATA': 0,
                                    'DATA_TYPE': 6,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:4326 -dstnodata 0.0 -r near -ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with target using proj string
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': custom_crs,
                                    'TARGET_CRS': custom_crs,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:20936 -t_srs EPSG:20936 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with target using custom projection
        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': custom_crs,
                                    'TARGET_CRS': custom_crs,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs "+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs" -t_srs "+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs" -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with non-EPSG crs code
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': 'POSTGIS:3111',
                                    'TARGET_CRS': 'POSTGIS:3111',
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:3111 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # with target resolution with None value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'TARGET_RESOLUTION': None,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:4326 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # test target resolution with a valid value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'TARGET_RESOLUTION': 10.0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:4326 -tr 10.0 10.0 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

        # test target resolution with a value of zero, to be ignored
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SOURCE_CRS': 'EPSG:3111',
                                    'TARGET_RESOLUTION': 0.0,
                                    'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdalwarp',
             '-s_srs EPSG:3111 -t_srs EPSG:4326 -r near -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])

    def testRearrangeBands(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        outsource = 'd:/temp/check.tif'

        alg = rearrange_bands()
        alg.initAlgorithm()

        # single band
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BANDS': 1,
                                    'OUTPUT': outsource}, context, feedback),
            ['gdal_translate', '-b 1 ' +
             '-of GTiff ' +
             source + ' ' + outsource])
        # three bands, re-ordered
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BANDS': [3, 2, 1],
                                    'OUTPUT': outsource}, context, feedback),
            ['gdal_translate', '-b 3 -b 2 -b 1 ' +
             '-of GTiff ' +
             source + ' ' + outsource])
        # three bands, re-ordered with custom data type
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BANDS': [3, 2, 1],
                                    'DATA_TYPE': 6,
                                    'OUTPUT': outsource}, context, feedback),
            ['gdal_translate', '-b 3 -b 2 -b 1 ' +
             '-ot Float32 -of GTiff ' +
             source + ' ' + outsource])

    def testFillnodata(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        mask = os.path.join(testDataPath, 'raster.tif')
        outsource = 'd:/temp/check.tif'
        alg = fillnodata()
        alg.initAlgorithm()

        # with mask value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'DISTANCE': 10,
                                    'ITERATIONS': 0,
                                    'MASK_LAYER': mask,
                                    'NO_MASK': False,
                                    'OUTPUT': outsource}, context, feedback),
            ['gdal_fillnodata.py',
             '-md 10 -b 1 -mask ' +
             mask +
             ' -of GTiff ' +
             source + ' ' +
             outsource])

        # without mask value
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'DISTANCE': 10,
                                    'ITERATIONS': 0,
                                    'NO_MASK': False,
                                    'OUTPUT': outsource}, context, feedback),
            ['gdal_fillnodata.py',
             '-md 10 -b 1 ' +
             '-of GTiff ' +
             source + ' ' +
             outsource])

        # nomask true
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'BAND': 1,
                                    'DISTANCE': 10,
                                    'ITERATIONS': 0,
                                    'NO_MASK': True,
                                    'OUTPUT': outsource}, context, feedback),
            ['gdal_fillnodata.py',
             '-md 10 -b 1 -nomask ' +
             '-of GTiff ' +
             source + ' ' +
             outsource])


class TestGdalOgrToPostGis(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # start_app()
        from processing.core.Processing import Processing
        Processing.initialize()

    @classmethod
    def tearDownClass(cls):
        pass

    # See https://issues.qgis.org/issues/15706
    def test_getConnectionString(self):
        obj = OgrToPostGis()
        obj.initAlgorithm({})

        parameters = {}
        context = QgsProcessingContext()

        # NOTE: defaults are debatable, see
        # https://github.com/qgis/QGIS/pull/3607#issuecomment-253971020
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "host=localhost port=5432 active_schema=public")

        parameters['HOST'] = 'remote'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "host=remote port=5432 active_schema=public")

        parameters['HOST'] = ''
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "port=5432 active_schema=public")

        parameters['PORT'] = '5555'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "port=5555 active_schema=public")

        parameters['PORT'] = ''
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "active_schema=public")

        parameters['USER'] = 'usr'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "active_schema=public user=usr")

        parameters['PASSWORD'] = 'pwd'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "password=pwd active_schema=public user=usr")


if __name__ == '__main__':
    nose2.main()
