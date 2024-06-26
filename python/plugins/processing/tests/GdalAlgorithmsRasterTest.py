"""
***************************************************************************
    GdalAlgorithmRasterTest.py
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

import nose2
import os
import shutil
import tempfile

from qgis.core import (QgsProcessingContext,
                       QgsProcessingException,
                       QgsProcessingFeedback,
                       QgsRectangle,
                       QgsReferencedRectangle,
                       QgsRasterLayer,
                       QgsProject,
                       QgsProjUtils,
                       QgsPointXY,
                       QgsCoordinateReferenceSystem)

from qgis.testing import (QgisTestCase,
                          start_app,
                          unittest)

import AlgorithmsTestBase
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.algs.gdal.AssignProjection import AssignProjection
from processing.algs.gdal.ClipRasterByExtent import ClipRasterByExtent
from processing.algs.gdal.ClipRasterByMask import ClipRasterByMask
from processing.algs.gdal.ColorRelief import ColorRelief
from processing.algs.gdal.GridAverage import GridAverage
from processing.algs.gdal.GridDataMetrics import GridDataMetrics
from processing.algs.gdal.GridInverseDistance import GridInverseDistance
from processing.algs.gdal.GridInverseDistanceNearestNeighbor import GridInverseDistanceNearestNeighbor
from processing.algs.gdal.GridLinear import GridLinear
from processing.algs.gdal.GridNearestNeighbor import GridNearestNeighbor
from processing.algs.gdal.gdal2tiles import gdal2tiles
from processing.algs.gdal.gdalcalc import gdalcalc
from processing.algs.gdal.gdaltindex import gdaltindex
from processing.algs.gdal.contour import contour, contour_polygon
from processing.algs.gdal.gdalinfo import gdalinfo
from processing.algs.gdal.hillshade import hillshade
from processing.algs.gdal.aspect import aspect
from processing.algs.gdal.buildvrt import buildvrt
from processing.algs.gdal.proximity import proximity
from processing.algs.gdal.rasterize import rasterize
from processing.algs.gdal.retile import retile
from processing.algs.gdal.translate import translate
from processing.algs.gdal.warp import warp
from processing.algs.gdal.fillnodata import fillnodata
from processing.algs.gdal.rearrange_bands import rearrange_bands
from processing.algs.gdal.gdaladdo import gdaladdo
from processing.algs.gdal.sieve import sieve
from processing.algs.gdal.gdal2xyz import gdal2xyz
from processing.algs.gdal.polygonize import polygonize
from processing.algs.gdal.pansharp import pansharp
from processing.algs.gdal.merge import merge
from processing.algs.gdal.nearblack import nearblack
from processing.algs.gdal.slope import slope
from processing.algs.gdal.rasterize_over import rasterize_over
from processing.algs.gdal.rasterize_over_fixed_value import rasterize_over_fixed_value
from processing.algs.gdal.viewshed import viewshed
from processing.algs.gdal.roughness import roughness
from processing.algs.gdal.pct2rgb import pct2rgb
from processing.algs.gdal.rgb2pct import rgb2pct

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')


class TestGdalRasterAlgorithms(QgisTestCase, AlgorithmsTestBase.AlgorithmsTest):

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

    def definition_file(self):
        return 'gdal_algorithm_raster_tests.yaml'

    @staticmethod
    def get_param_value_and_expected_string_for_custom_crs(proj_def):
        crs = QgsCoordinateReferenceSystem.fromProj(proj_def)
        custom_crs = f'proj4: {proj_def}'
        return custom_crs, crs.toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED_GDAL).replace('"', '"""')

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
        custom_crs, expected_crs_string = self.get_param_value_and_expected_string_for_custom_crs(
            '+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'CRS': custom_crs}, context, feedback),
            ['gdal_edit.py',
             f'-a_srs "{expected_crs_string}" ' +
             source])

        # with non-EPSG crs code
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'CRS': 'POSTGIS:3111'}, context, feedback),
            ['gdal_edit.py',
             '-a_srs EPSG:3111 ' +
             source])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                    'CRS': 'EPSG:3111'}, context, feedback),
            ['gdal_edit.py',
             '-a_srs EPSG:3111 ' +
             source + ' -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                    'CRS': 'EPSG:3111'}, context, feedback),
            ['gdal_edit.py',
             '-a_srs EPSG:3111 ' +
             source + ' --config X Y --config Z A'])

    @unittest.skipIf(os.environ.get('TRAVIS', '') == 'true',
                     'gdal_edit.py: not found')
    def testRunAssignProjection(self):
        # Check that assign projection updates QgsRasterLayer info
        # GDAL Assign Projection is based on gdal_edit.py

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = AssignProjection()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            fake_dem = os.path.join(outdir, 'dem-fake-crs.tif')

            shutil.copy(source, fake_dem)
            self.assertTrue(os.path.exists(fake_dem))

            rlayer = QgsRasterLayer(fake_dem, "Fake dem")
            self.assertTrue(rlayer.isValid())
            self.assertEqual(rlayer.crs().authid(), 'EPSG:4326')

            project = QgsProject()
            project.setFileName(os.path.join(outdir, 'dem-fake-crs.qgs'))
            project.addMapLayer(rlayer)
            self.assertEqual(project.count(), 1)

            context.setProject(project)

            alg.run({'INPUT': fake_dem, 'CRS': 'EPSG:3111'},
                    context, feedback)
            self.assertEqual(rlayer.crs().authid(), 'EPSG:3111')

    def testGdalTranslate(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        translate_alg = translate()
        translate_alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # without NODATA value
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with None NODATA value
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'NODATA': None,
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'NODATA': 9999,
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_nodata 9999.0 ' +
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'NODATA': 0,
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_nodata 0.0 ' +
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value and custom data type
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'NODATA': 0,
                                                  'DATA_TYPE': 6,
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_nodata 0.0 ' +
                 '-ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with target srs
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'TARGET_CRS': 'EPSG:3111',
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_srs EPSG:3111 ' +
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with target using proj string
            custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'TARGET_CRS': custom_crs,
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_srs EPSG:20936 ' +
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with target using custom projection
            custom_crs, expected_crs_string = self.get_param_value_and_expected_string_for_custom_crs('+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'TARGET_CRS': custom_crs,
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 f'-a_srs "{expected_crs_string}" ' +
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with non-EPSG crs code
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'TARGET_CRS': 'POSTGIS:3111',
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_srs EPSG:3111 ' +
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with copy subdatasets
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'COPY_SUBDATASETS': True,
                                                  'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_translate',
                 '-sds ' +
                 '-of GTiff ' +
                 source + ' ' +
                 outdir + '/check.tif'])

            # additional parameters
            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source,
                                                  'EXTRA': '-strict -unscale -epo',
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG -strict -unscale -epo ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y'])

            self.assertEqual(
                translate_alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                                  'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg --config X Y --config Z A'])

    def testClipRasterByExtent(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = ClipRasterByExtent()
        alg.initAlgorithm()
        extent = QgsRectangle(1, 2, 3, 4)

        with tempfile.TemporaryDirectory() as outdir:
            # with no NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTENT': extent,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTENT': extent,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_nodata 9999.0 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTENT': extent,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_nodata 0.0 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value and custom data type
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTENT': extent,
                                        'NODATA': 0,
                                        'DATA_TYPE': 6,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_nodata 0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with creation options
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTENT': extent,
                                        'OPTIONS': 'COMPRESS=DEFLATE|PREDICTOR=2|ZLEVEL=9',
                                        'DATA_TYPE': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG -co COMPRESS=DEFLATE -co PREDICTOR=2 -co ZLEVEL=9 ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTENT': extent,
                                        'EXTRA': '-s_srs EPSG:4326 -tps -tr 0.1 0.1',
                                        'DATA_TYPE': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG -s_srs EPSG:4326 -tps -tr 0.1 0.1 ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # override CRS
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTENT': extent,
                                        'OVERCRS': True,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-a_srs EPSG:4326 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                        'EXTENT': extent,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'EXTENT': extent,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_translate',
                 '-of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg --config X Y --config Z A'])

    def testClipRasterByMask(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        mask = os.path.join(testDataPath, 'polys.gml')
        extent = QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:4236'))
        alg = ClipRasterByMask()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # with no NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'MASK': mask,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline ' + source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'MASK': mask,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline -dstnodata 9999.0 ' + source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'MASK': mask,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline -dstnodata 0.0 ' + source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value and custom data type
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'MASK': mask,
                                        'NODATA': 0,
                                        'DATA_TYPE': 6,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -ot Float32 -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline -dstnodata 0.0 ' + source + ' ' +
                 outdir + '/check.jpg'])
            # with creation options
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'MASK': mask,
                                        'OPTIONS': 'COMPRESS=DEFLATE|PREDICTOR=2|ZLEVEL=9',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline -co COMPRESS=DEFLATE -co PREDICTOR=2 -co ZLEVEL=9 ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with multothreading and additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'MASK': mask,
                                        'MULTITHREADING': True,
                                        'EXTRA': '-nosrcalpha -wm 2048 -nomd',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline -multi -nosrcalpha -wm 2048 -nomd ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with target extent value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'MASK': mask,
                                        'TARGET_EXTENT': extent,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -te 1.0 2.0 3.0 4.0 -te_srs EPSG:4236 -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline ' + source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                        'MASK': mask,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline ' + source + ' ' +
                 outdir + '/check.jpg -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'MASK': mask,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -of JPEG -cutline ' +
                 mask + ' -cl polys2 -crop_to_cutline ' + source + ' ' +
                 outdir + '/check.jpg --config X Y --config Z A'])

    def testContourPolygon(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = contour_polygon()
        alg.initAlgorithm()
        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD_NAME_MIN': 'min',
                                        'FIELD_NAME_MAX': 'max',
                                        'INTERVAL': 5,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['gdal_contour',
                    '-p -amax max -amin min -b 1 -i 5.0 -f "ESRI Shapefile" ' +
                    source + ' ' +
                    outdir + '/check.shp'])

    def testContour(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = contour()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # with no NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD_NAME': 'elev',
                                        'INTERVAL': 5,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['gdal_contour',
                 '-b 1 -a elev -i 5.0 -f "ESRI Shapefile" ' +
                 source + ' ' +
                 outdir + '/check.shp'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD_NAME': 'elev',
                                        'INTERVAL': 5,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['gdal_contour',
                 '-b 1 -a elev -i 5.0 -snodata 9999.0 -f "ESRI Shapefile" ' +
                 source + ' ' +
                 outdir + '/check.shp'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD_NAME': 'elev',
                                        'INTERVAL': 5,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.gpkg'}, context, feedback),
                ['gdal_contour',
                 '-b 1 -a elev -i 5.0 -snodata 0.0 -f "GPKG" ' +
                 source + ' ' +
                 outdir + '/check.gpkg'])
            # with CREATE_3D
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'CREATE_3D': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['gdal_contour',
                 '-b 1 -a ELEV -i 10.0 -3d -f "ESRI Shapefile" ' +
                 source + ' ' +
                 outdir + '/check.shp'])
            # with IGNORE_NODATA and OFFSET
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'IGNORE_NODATA': True,
                                        'OFFSET': 100,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['gdal_contour',
                 '-b 1 -a ELEV -i 10.0 -inodata -off 100.0 -f "ESRI Shapefile" ' +
                 source + ' ' +
                 outdir + '/check.shp'])
            # with additional command line parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'EXTRA': '-e 3 -amin MIN_H',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['gdal_contour',
                 '-b 1 -a ELEV -i 10.0 -f "ESRI Shapefile" -e 3 -amin MIN_H ' +
                 source + ' ' +
                 outdir + '/check.shp'])
            # obsolete OPTIONS param
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'OPTIONS': '-fl 100 125 150 200',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['gdal_contour',
                 '-b 1 -a ELEV -i 10.0 -f "ESRI Shapefile" -fl 100 125 150 200 ' +
                 source + ' ' +
                 outdir + '/check.shp'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'FIELD_NAME': 'elev',
                                        'INTERVAL': 5,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['gdal_contour',
                 '-b 1 -a elev -i 5.0 -f "ESRI Shapefile" --config X Y --config Z A ' +
                 source + ' ' +
                 outdir + '/check.shp'])

    def testGdal2Tiles(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = gdal2tiles()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # with no NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/'}, context, feedback),
                ['gdal2tiles.py',
                 '-p mercator -w all -r average ' +
                 source + ' ' +
                 outdir + '/'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': -9999,
                                        'OUTPUT': outdir + '/'}, context, feedback),
                ['gdal2tiles.py',
                 '-p mercator -w all -r average -a -9999.0 ' +
                 source + ' ' +
                 outdir + '/'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/'}, context, feedback),
                ['gdal2tiles.py',
                 '-p mercator -w all -r average -a 0.0 ' +
                 source + ' ' +
                 outdir + '/'])

            # with input srs
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir + '/'}, context, feedback),
                ['gdal2tiles.py',
                 '-p mercator -w all -r average -s EPSG:3111 ' +
                 source + ' ' +
                 outdir + '/'])

            # with target using proj string
            custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': custom_crs,
                                        'OUTPUT': outdir + '/'}, context, feedback),
                ['gdal2tiles.py',
                 '-p mercator -w all -r average -s EPSG:20936 ' +
                 source + ' ' +
                 outdir + '/'])

            # with target using custom projection
            custom_crs, expected_crs_string = self.get_param_value_and_expected_string_for_custom_crs('+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': custom_crs,
                                        'OUTPUT': outdir + '/'}, context, feedback),
                ['gdal2tiles.py',
                 f'-p mercator -w all -r average -s "{expected_crs_string}" ' +
                 source + ' ' +
                 outdir + '/'])

            # with non-EPSG crs code
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': 'POSTGIS:3111',
                                        'OUTPUT': outdir + '/'}, context, feedback),
                ['gdal2tiles.py',
                 '-p mercator -w all -r average -s EPSG:3111 ' +
                 source + ' ' +
                 outdir + '/'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'OUTPUT': outdir + '/'}, context, feedback),
                ['gdal2tiles.py',
                 '-p mercator -w all -r average ' +
                 source + ' ' +
                 outdir + '/ --config X Y --config Z A'])

    def testGdalCalc(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        source2 = os.path.join(testDataPath, 'raster.tif')
        source3 = os.path.join(testDataPath, 'raster with spaces.tif')
        alg = gdalcalc()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            output = outdir + '/check.jpg'

            # default execution
            formula = 'A*2'  # default formula
            self.assertEqual(
                alg.getConsoleCommands({'INPUT_A': source,
                                        'BAND_A': 1,
                                        'FORMULA': formula,
                                        'OUTPUT': output}, context, feedback),
                ['gdal_calc.py',
                 f'--overwrite --calc "{formula}" --format JPEG --type Float32 -A {source} --A_band 1 --outfile {output}'])

            if GdalUtils.version() >= 3030000:
                extent = QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:4326'))
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT_A': source,
                                            'BAND_A': 1,
                                            'FORMULA': formula,
                                            'PROJWIN': extent,
                                            'OUTPUT': output}, context, feedback),
                    ['gdal_calc.py',
                     f'--overwrite --calc "{formula}" --format JPEG --type Float32 --projwin 1.0 4.0 3.0 2.0 -A {source} --A_band 1 --outfile {output}'])

                # Inputs A and B share same pixel size and CRS
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT_A': source2,
                                            'BAND_A': 1,
                                            'INPUT_B': source3,
                                            'BAND_B': 1,
                                            'FORMULA': formula,
                                            'EXTENT_OPT': 3,
                                            'OUTPUT': output}, context, feedback),
                    ['gdal_calc.py',
                     f'--overwrite --calc "{formula}" --format JPEG --type Float32 --extent=intersect -A {source2} --A_band 1 -B "{source3}" --B_band 1 --outfile {output}'])

                # Test mutually exclusive --extent and --projwin. Should raise an exception
                self.assertRaises(
                    QgsProcessingException,
                    lambda: alg.getConsoleCommands({'INPUT_A': source,
                                                    'BAND_A': 1,
                                                    'FORMULA': formula,
                                                    'PROJWIN': extent,
                                                    'EXTENT_OPT': 3,
                                                    'OUTPUT': output}, context, feedback))

                # Inputs A and B do not share same pixel size and CRS. Should raise an exception
                source2 = os.path.join(testDataPath, 'raster.tif')
                self.assertRaises(
                    QgsProcessingException,
                    lambda: alg.getConsoleCommands({'INPUT_A': source,
                                                    'BAND_A': 1,
                                                    'INPUT_B': source2,
                                                    'BAND_B': 1,
                                                    'FORMULA': formula,
                                                    'EXTENT_OPT': 3,
                                                    'OUTPUT': output}, context, feedback))

            # check that formula is not escaped and formula is returned as it is
            formula = 'A * 2'  # <--- add spaces in the formula
            self.assertEqual(
                alg.getConsoleCommands({'INPUT_A': source,
                                        'BAND_A': 1,
                                        'FORMULA': formula,
                                        'OUTPUT': output}, context, feedback),
                ['gdal_calc.py',
                 f'--overwrite --calc "{formula}" --format JPEG --type Float32 -A {source} --A_band 1 --outfile {output}'])

            # additional creation options
            formula = 'A*2'
            self.assertEqual(
                alg.getConsoleCommands({'INPUT_A': source,
                                        'BAND_A': 1,
                                        'FORMULA': formula,
                                        'OPTIONS': 'COMPRESS=JPEG|JPEG_QUALITY=75',
                                        'OUTPUT': output}, context, feedback),
                ['gdal_calc.py',
                 f'--overwrite --calc "{formula}" --format JPEG --type Float32 -A {source} --A_band 1 --co COMPRESS=JPEG --co JPEG_QUALITY=75 --outfile {output}'])

            # additional parameters
            formula = 'A*2'
            self.assertEqual(
                alg.getConsoleCommands({'INPUT_A': source,
                                        'BAND_A': 1,
                                        'FORMULA': formula,
                                        'EXTRA': '--debug --quiet',
                                        'OUTPUT': output}, context, feedback),
                ['gdal_calc.py',
                 f'--overwrite --calc "{formula}" --format JPEG --type Float32 -A {source} --A_band 1 --debug --quiet --outfile {output}'])

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

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'MIN_MAX': False,
                                    'NOGCP': False,
                                    'NO_METADATA': False,
                                    'STATS': False,
                                    'EXTRA': '-proj4 -listmdd -checksum'}, context, feedback),
            ['gdalinfo',
             '-proj4 -listmdd -checksum "' + source + '"'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                    'MIN_MAX': False,
                                    'NOGCP': False,
                                    'NO_METADATA': False,
                                    'STATS': False}, context, feedback),
            ['gdalinfo',
             '"' + source + '" -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                    'MIN_MAX': False,
                                    'NOGCP': False,
                                    'NO_METADATA': False,
                                    'STATS': False}, context, feedback),
            ['gdalinfo',
             '"' + source + '" --config X Y --config Z A'])

    def testGdalTindex(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = gdaltindex()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            commands = alg.getConsoleCommands({'LAYERS': [source],
                                               'OUTPUT': outdir + '/test.shp'}, context, feedback)
            self.assertEqual(len(commands), 2)
            self.assertEqual(commands[0], 'gdaltindex')
            self.assertIn('-tileindex location -f "ESRI Shapefile" ' + outdir + '/test.shp', commands[1])
            self.assertIn('--optfile ', commands[1])

            # with input srs
            commands = alg.getConsoleCommands({'LAYERS': [source],
                                               'TARGET_CRS': 'EPSG:3111',
                                               'OUTPUT': outdir + '/test.shp'}, context, feedback)
            self.assertEqual(len(commands), 2)
            self.assertEqual(commands[0], 'gdaltindex')
            self.assertIn('-tileindex location -t_srs EPSG:3111 -f "ESRI Shapefile" ' + outdir + '/test.shp', commands[1])
            self.assertIn('--optfile ', commands[1])

            # with target using proj string
            custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
            commands = alg.getConsoleCommands({'LAYERS': [source],
                                               'TARGET_CRS': custom_crs,
                                               'OUTPUT': outdir + '/test.shp'}, context, feedback)
            self.assertEqual(len(commands), 2)
            self.assertEqual(commands[0], 'gdaltindex')
            self.assertIn('-tileindex location -t_srs EPSG:20936 -f "ESRI Shapefile" ' + outdir + '/test.shp', commands[1])
            self.assertIn('--optfile ', commands[1])

            # with target using custom projection
            custom_crs, expected_crs_string = self.get_param_value_and_expected_string_for_custom_crs('+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')
            commands = alg.getConsoleCommands({'LAYERS': [source],
                                               'TARGET_CRS': custom_crs,
                                               'OUTPUT': outdir + '/test.shp'}, context, feedback)
            self.assertEqual(len(commands), 2)
            self.assertEqual(commands[0], 'gdaltindex')
            self.assertIn(f'-tileindex location -t_srs "{expected_crs_string}" -f "ESRI Shapefile" ' + outdir + '/test.shp', commands[1])
            self.assertIn('--optfile ', commands[1])

            # with non-EPSG crs code
            commands = alg.getConsoleCommands({'LAYERS': [source],
                                               'TARGET_CRS': 'POSTGIS:3111',
                                               'OUTPUT': outdir + '/test.shp'}, context, feedback)
            self.assertEqual(len(commands), 2)
            self.assertEqual(commands[0], 'gdaltindex')
            self.assertIn(
                '-tileindex location -t_srs EPSG:3111 -f "ESRI Shapefile" ' + outdir + '/test.shp',
                commands[1])
            self.assertIn('--optfile ', commands[1])

    def testGridAverage(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridAverage()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # with no NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=9999.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG -z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                            'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                    ['gdal_grid',
                     '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y -z_multiply 1.5 -outsize 1754 1394 ' +
                     source + ' ' +
                     outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a average:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG -z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.jpg --config X Y --config Z A'])

    def testGridDataMetrics(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridDataMetrics()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # without NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=9999.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # non-default datametrics
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'METRIC': 4,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a average_distance:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 ' +
                 '-ot Float32 -of GTiff -z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif'])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                            'OUTPUT': outdir + '/check.tif'}, context, feedback),
                    ['gdal_grid',
                     '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 ' +
                     '-ot Float32 -of GTiff -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y -z_multiply 1.5 -outsize 1754 1394 ' +
                     source + ' ' +
                     outdir + '/check.tif'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a minimum:radius1=0.0:radius2=0.0:angle=0.0:min_points=0:nodata=0.0 ' +
                 '-ot Float32 -of GTiff -z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif --config X Y --config Z A'])

    def testGridInverseDistance(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridInverseDistance()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # without NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdist:power=2.0:smoothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdist:power=2.0:smoothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=9999.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdist:power=2.0:smoothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdist:power=2.0:smoothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=0.0 ' +
                 '-ot Float32 -of GTiff -z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif'])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                            'OUTPUT': outdir + '/check.tif'}, context, feedback),
                    ['gdal_grid',
                     '-l points -a invdist:power=2.0:smoothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=0.0 ' +
                     '-ot Float32 -of GTiff -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y -z_multiply 1.5 -outsize 1754 1394 ' +
                     source + ' ' +
                     outdir + '/check.tif'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdist:power=2.0:smoothing=0.0:radius1=0.0:radius2=0.0:angle=0.0:max_points=0:min_points=0:nodata=0.0 ' +
                 '-ot Float32 -of GTiff -z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif --config X Y --config Z A'])

    def testGridInverseDistanceNearestNeighbour(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridInverseDistanceNearestNeighbor()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # without NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdistnn:power=2.0:smoothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdistnn:power=2.0:smoothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=9999.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdistnn:power=2.0:smoothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdistnn:power=2.0:smoothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=0.0 ' +
                 '-ot Float32 -of GTiff -z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif'])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                            'OUTPUT': outdir + '/check.tif'}, context, feedback),
                    ['gdal_grid',
                     '-l points -a invdistnn:power=2.0:smoothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=0.0 ' +
                     '-ot Float32 -of GTiff -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y -z_multiply 1.5 -outsize 1754 1394 ' +
                     source + ' ' +
                     outdir + '/check.tif'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a invdistnn:power=2.0:smoothing=0.0:radius=1.0:max_points=12:min_points=0:nodata=0.0 ' +
                 '-ot Float32 -of GTiff -z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif --config X Y --config Z A'])

    def testGridLinear(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridLinear()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # without NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a linear:radius=-1.0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a linear:radius=-1.0:nodata=9999.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a linear:radius=-1.0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a linear:radius=-1.0:nodata=0.0 -ot Float32 -of GTiff ' +
                 '-z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif'])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                            'OUTPUT': outdir + '/check.tif'}, context, feedback),
                    ['gdal_grid',
                     '-l points -a linear:radius=-1.0:nodata=0.0 -ot Float32 -of GTiff -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y ' +
                     '-z_multiply 1.5 -outsize 1754 1394 ' +
                     source + ' ' +
                     outdir + '/check.tif'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a linear:radius=-1.0:nodata=0.0 -ot Float32 -of GTiff ' +
                 '-z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif --config X Y --config Z A'])

    def testGridNearestNeighbour(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'points.gml')
        alg = GridNearestNeighbor()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # without NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=9999.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_grid',
                 '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=0.0 -ot Float32 -of GTiff ' +
                 '-z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif'])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                            'OUTPUT': outdir + '/check.tif'}, context, feedback),
                    ['gdal_grid',
                     '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=0.0 -ot Float32 -of GTiff -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y ' +
                     '-z_multiply 1.5 -outsize 1754 1394 ' +
                     source + ' ' +
                     outdir + '/check.tif'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'EXTRA': '-z_multiply 1.5 -outsize 1754 1394',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdal_grid',
                 '-l points -a nearest:radius1=0.0:radius2=0.0:angle=0.0:nodata=0.0 -ot Float32 -of GTiff ' +
                 '-z_multiply 1.5 -outsize 1754 1394 ' +
                 source + ' ' +
                 outdir + '/check.tif --config X Y --config Z A'])

    def testHillshade(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = hillshade()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'Z_FACTOR': 5,
                                        'SCALE': 2,
                                        'AZIMUTH': 90,
                                        'ALTITUDE': 20,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'hillshade ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0'])

            # paths with space
            source_with_space = os.path.join(testDataPath, 'raster with spaces.tif')
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source_with_space,
                                        'BAND': 1,
                                        'Z_FACTOR': 5,
                                        'SCALE': 2,
                                        'AZIMUTH': 90,
                                        'ALTITUDE': 20,
                                        'OUTPUT': outdir + '/check out.tif'}, context, feedback),
                ['gdaldem',
                 'hillshade ' +
                 '"' + source_with_space + '" ' +
                 f'"{outdir}/check out.tif" -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0'])

            # compute edges
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'Z_FACTOR': 5,
                                        'SCALE': 2,
                                        'AZIMUTH': 90,
                                        'ALTITUDE': 20,
                                        'COMPUTE_EDGES': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'hillshade ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0 -compute_edges'])

            # with ZEVENBERGEN
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'Z_FACTOR': 5,
                                        'SCALE': 2,
                                        'AZIMUTH': 90,
                                        'ALTITUDE': 20,
                                        'ZEVENBERGEN': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'hillshade ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0 -alg ZevenbergenThorne'])

            # with COMBINED
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'Z_FACTOR': 5,
                                        'SCALE': 2,
                                        'AZIMUTH': 90,
                                        'ALTITUDE': 20,
                                        'COMBINED': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'hillshade ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0 -combined'])

            # with multidirectional - "az" argument is not allowed!
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'Z_FACTOR': 5,
                                        'SCALE': 2,
                                        'AZIMUTH': 90,
                                        'ALTITUDE': 20,
                                        'MULTIDIRECTIONAL': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'hillshade ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -alt 20.0 -multidirectional'])

            # defaults with additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'EXTRA': '-q',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'hillshade ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -z 1.0 -s 1.0 -az 315.0 -alt 45.0 -q'])

            # multidirectional and combined are mutually exclusive
            self.assertRaises(
                QgsProcessingException,
                lambda: alg.getConsoleCommands({'INPUT': source,
                                                'BAND': 1,
                                                'Z_FACTOR': 5,
                                                'SCALE': 2,
                                                'AZIMUTH': 90,
                                                'COMBINED': True,
                                                'MULTIDIRECTIONAL': True,
                                                'OUTPUT': outdir + '/check.tif'}, context, feedback))
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'Z_FACTOR': 5,
                                        'SCALE': 2,
                                        'AZIMUTH': 90,
                                        'ALTITUDE': 20,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'hillshade ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -z 5.0 -s 2.0 -az 90.0 -alt 20.0 --config X Y --config Z A'])

    def testAspect(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = aspect()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'TRIG_ANGLE': False,
                                        'ZERO_FLAT': False,
                                        'COMPUTE_EDGES': False,
                                        'ZEVENBERGEN': False,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1'])

            # paths with space
            source_with_space = os.path.join(testDataPath, 'raster with spaces.tif')
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source_with_space,
                                        'BAND': 1,
                                        'TRIG_ANGLE': False,
                                        'ZERO_FLAT': False,
                                        'COMPUTE_EDGES': False,
                                        'ZEVENBERGEN': False,
                                        'OUTPUT': outdir + '/check out.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 '"' + source_with_space + '" ' +
                 f'"{outdir}/check out.tif" -of GTiff -b 1'])

            # compute edges
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'TRIG_ANGLE': False,
                                        'ZERO_FLAT': False,
                                        'COMPUTE_EDGES': True,
                                        'ZEVENBERGEN': False,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -compute_edges'])

            # with ZEVENBERGEN
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'TRIG_ANGLE': False,
                                        'ZERO_FLAT': False,
                                        'COMPUTE_EDGES': False,
                                        'ZEVENBERGEN': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -alg ZevenbergenThorne'])

            # with ZERO_FLAT
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'TRIG_ANGLE': False,
                                        'ZERO_FLAT': True,
                                        'COMPUTE_EDGES': False,
                                        'ZEVENBERGEN': False,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -zero_for_flat'])

            # with TRIG_ANGLE
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'TRIG_ANGLE': True,
                                        'ZERO_FLAT': False,
                                        'COMPUTE_EDGES': False,
                                        'ZEVENBERGEN': False,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -trigonometric'])

            # with creation options
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'TRIG_ANGLE': False,
                                        'ZERO_FLAT': False,
                                        'COMPUTE_EDGES': False,
                                        'ZEVENBERGEN': False,
                                        'OPTIONS': 'COMPRESS=JPEG|JPEG_QUALITY=75',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -co COMPRESS=JPEG -co JPEG_QUALITY=75'])

            # with additional parameter
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'TRIG_ANGLE': False,
                                        'ZERO_FLAT': False,
                                        'COMPUTE_EDGES': False,
                                        'ZEVENBERGEN': False,
                                        'EXTRA': '-q',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -q'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'TRIG_ANGLE': False,
                                        'ZERO_FLAT': False,
                                        'COMPUTE_EDGES': False,
                                        'ZEVENBERGEN': False,
                                        'EXTRA': '-q',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'aspect ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 --config X Y --config Z A -q'])

    def testSlope(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = slope()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'slope ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -s 1.0'])

            # compute edges
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'COMPUTE_EDGES': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'slope ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -s 1.0 -compute_edges'])

            # with ZEVENBERGEN
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'ZEVENBERGEN': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'slope ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -s 1.0 -alg ZevenbergenThorne'])

            # custom ratio
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'SCALE': 2.0,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'slope ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -s 2.0'])

            # with creation options
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'OPTIONS': 'COMPRESS=JPEG|JPEG_QUALITY=75',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'slope ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -s 1.0 -co COMPRESS=JPEG -co JPEG_QUALITY=75'])

            # with additional parameter
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'EXTRA': '-q',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdaldem',
                 'slope ' +
                 source + ' ' +
                 outdir + '/check.jpg -of JPEG -b 1 -s 1.0 -q'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'slope ' +
                 source + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -s 1.0 --config X Y --config Z A'])

    def testColorRelief(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        colorTable = os.path.join(testDataPath, 'colors.txt')
        alg = ColorRelief()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'COLOR_TABLE': colorTable,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'color-relief ' +
                 source + ' ' +
                 colorTable + ' ' +
                 outdir + '/check.tif -of GTiff -b 1'])

            # paths with space
            source_with_space = os.path.join(testDataPath, 'raster with spaces.tif')
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source_with_space,
                                        'BAND': 1,
                                        'COLOR_TABLE': colorTable,
                                        'OUTPUT': outdir + '/check out.tif'}, context, feedback),
                ['gdaldem',
                 'color-relief ' +
                 '"' + source_with_space + '" ' +
                 colorTable + ' ' +
                 f'"{outdir}/check out.tif" -of GTiff -b 1'])

            # compute edges
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'COLOR_TABLE': colorTable,
                                        'COMPUTE_EDGES': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'color-relief ' +
                 source + ' ' +
                 colorTable + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -compute_edges'])

            # with custom matching mode
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'COLOR_TABLE': colorTable,
                                        'MATCH_MODE': 1,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'color-relief ' +
                 source + ' ' +
                 colorTable + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -nearest_color_entry'])

            # with creation options
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'COLOR_TABLE': colorTable,
                                        'MATCH_MODE': 1,
                                        'OPTIONS': 'COMPRESS=JPEG|JPEG_QUALITY=75',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'color-relief ' +
                 source + ' ' +
                 colorTable + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -nearest_color_entry -co COMPRESS=JPEG -co JPEG_QUALITY=75'])

            # with additional parameter
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'COLOR_TABLE': colorTable,
                                        'EXTRA': '-alpha -q',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'color-relief ' +
                 source + ' ' +
                 colorTable + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 -alpha -q'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'COLOR_TABLE': colorTable,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'color-relief ' +
                 source + ' ' +
                 colorTable + ' ' +
                 outdir + '/check.tif -of GTiff -b 1 --config X Y --config Z A'])

    def testProximity(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = proximity()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # without NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_proximity.py',
                 '-srcband 1 -distunits PIXEL -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'BAND': 2,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_proximity.py',
                 '-srcband 2 -distunits PIXEL -nodata 9999.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'BAND': 1,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_proximity.py',
                 '-srcband 1 -distunits PIXEL -nodata 0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'EXTRA': '-dstband 2 -values 3,4,12',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_proximity.py',
                 '-srcband 1 -distunits PIXEL -ot Float32 -of JPEG -dstband 2 -values 3,4,12 ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_proximity.py',
                 '-srcband 1 -distunits PIXEL -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg --config X Y --config Z A'])

    def testRasterize(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        sourceZ = os.path.join(testDataPath, 'pointsz.gml')
        extent4326 = QgsReferencedRectangle(QgsRectangle(-1, -3, 10, 6), QgsCoordinateReferenceSystem('EPSG:4326'))
        extent3857 = QgsReferencedRectangle(QgsRectangle(-111319.491, -334111.171, 1113194.908, 669141.057), QgsCoordinateReferenceSystem('EPSG:3857'))
        alg = rasterize()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # with no NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'id',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -ts 0.0 0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'FIELD': 'id',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -ts 0.0 0.0 -a_nodata 9999.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" INIT value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'INIT': 0,
                                        'FIELD': 'id',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -ts 0.0 0.0 -init 0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'FIELD': 'id',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -ts 0.0 0.0 -a_nodata 0.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'id',
                                        'EXTRA': '-at -add',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -ts 0.0 0.0 -ot Float32 -of JPEG -at -add ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # use_Z selected with no field
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': sourceZ,
                                        'USE_Z': True,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l pointsz -3d -ts 0.0 0.0 -ot Float32 -of JPEG ' +
                 sourceZ + ' ' +
                 outdir + '/check.jpg'])

            # use_Z selected with field indicated (should prefer use_Z)
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': sourceZ,
                                        'FIELD': 'elev',
                                        'USE_Z': True,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l pointsz -3d -ts 0.0 0.0 -ot Float32 -of JPEG ' +
                 sourceZ + ' ' +
                 outdir + '/check.jpg'])

            # with EXTENT in the same CRS as the input layer source
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'id',
                                        'EXTENT': extent4326,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -ts 0.0 0.0 -te -1.0 -3.0 10.0 6.0 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with EXTENT in a different CRS than that of the input layer source
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'id',
                                        'EXTENT': extent3857,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -ts 0.0 0.0 -te -1.000000001857055 -2.9999999963940835 10.000000000604244 5.99999999960471 -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'FIELD': 'id',
                                            'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                    ['gdal_rasterize',
                     '-l polys2 -a id -ts 0.0 0.0 -ot Float32 -of JPEG -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y ' +
                     source + ' ' +
                     outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'FIELD': 'id',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -ts 0.0 0.0 -ot Float32 -of JPEG --config X Y --config Z A ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

    def testRasterizeOver(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        raster = os.path.join(testDataPath, 'dem.tif')
        vector = os.path.join(testDataPath, 'polys.gml')
        alg = rasterize_over()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': vector,
                                        'FIELD': 'id',
                                        'INPUT_RASTER': raster}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id ' +
                 vector + ' ' + raster])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': vector,
                                        'FIELD': 'id',
                                        'ADD': True,
                                        'INPUT_RASTER': raster}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -add ' +
                 vector + ' ' + raster])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': vector,
                                        'FIELD': 'id',
                                        'EXTRA': '-i',
                                        'INPUT_RASTER': raster}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id -i ' +
                 vector + ' ' + raster])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': vector + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'FIELD': 'id',
                                            'INPUT_RASTER': raster}, context, feedback),
                    ['gdal_rasterize',
                     '-l polys2 -a id -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y ' +
                     vector + ' ' + raster])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': vector + '|credential:X=Y|credential:Z=A',
                                        'FIELD': 'id',
                                        'INPUT_RASTER': raster}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -a id --config X Y --config Z A ' +
                 vector + ' ' + raster])

    def testRasterizeOverFixed(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        raster = os.path.join(testDataPath, 'dem.tif')
        vector = os.path.join(testDataPath, 'polys.gml')
        alg = rasterize_over_fixed_value()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': vector,
                                        'BURN': 100,
                                        'INPUT_RASTER': raster}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -burn 100.0 ' +
                 vector + ' ' + raster])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': vector,
                                        'BURN': 100,
                                        'ADD': True,
                                        'INPUT_RASTER': raster}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -burn 100.0 -add ' +
                 vector + ' ' + raster])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': vector,
                                        'BURN': 100,
                                        'EXTRA': '-i',
                                        'INPUT_RASTER': raster}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -burn 100.0 -i ' +
                 vector + ' ' + raster])

            if GdalUtils.version() >= 3070000:
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': vector + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                            'BURN': 100,
                                            'INPUT_RASTER': raster}, context, feedback),
                    ['gdal_rasterize',
                     '-l polys2 -burn 100.0 -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y ' +
                     vector + ' ' + raster])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': vector + '|credential:X=Y|credential:Z=A',
                                        'BURN': 100,
                                        'INPUT_RASTER': raster}, context, feedback),
                ['gdal_rasterize',
                 '-l polys2 -burn 100.0 --config X Y --config Z A ' +
                 vector + ' ' + raster])

    def testRasterizeOverRun(self):
        # Check that rasterize over tools update QgsRasterLayer

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source_vector = os.path.join(testDataPath, 'rasterize_zones.gml')
        source_raster = os.path.join(testDataPath, 'dem.tif')

        with tempfile.TemporaryDirectory() as outdir:
            # fixed value
            alg = rasterize_over_fixed_value()
            alg.initAlgorithm()

            test_dem = os.path.join(outdir, 'rasterize-fixed.tif')
            shutil.copy(source_raster, test_dem)
            self.assertTrue(os.path.exists(test_dem))

            layer = QgsRasterLayer(test_dem, 'test')
            self.assertTrue(layer.isValid())

            val, ok = layer.dataProvider().sample(QgsPointXY(18.68704, 45.79568), 1)
            self.assertEqual(val, 172.2267303466797)

            project = QgsProject()
            project.setFileName(os.path.join(outdir, 'rasterize-fixed.qgs'))
            project.addMapLayer(layer)
            self.assertEqual(project.count(), 1)

            context.setProject(project)

            alg.run({'INPUT': source_vector,
                     'INPUT_RASTER': test_dem,
                     'BURN': 200
                     }, context, feedback)

            val, ok = layer.dataProvider().sample(QgsPointXY(18.68704, 45.79568), 1)
            self.assertTrue(ok)
            self.assertEqual(val, 200.0)

            # attribute value
            alg = rasterize_over()
            alg.initAlgorithm()

            test_dem = os.path.join(outdir, 'rasterize-over.tif')
            shutil.copy(source_raster, test_dem)
            self.assertTrue(os.path.exists(test_dem))

            layer = QgsRasterLayer(test_dem, 'test')
            self.assertTrue(layer.isValid())

            val, ok = layer.dataProvider().sample(QgsPointXY(18.68704, 45.79568), 1)
            self.assertEqual(val, 172.2267303466797)

            project = QgsProject()
            project.setFileName(os.path.join(outdir, 'rasterize-over.qgs'))
            project.addMapLayer(layer)
            self.assertEqual(project.count(), 1)

            context.setProject(project)

            alg.run({'INPUT': source_vector,
                     'INPUT_RASTER': test_dem,
                     'FIELD': 'value'
                     }, context, feedback)

            val, ok = layer.dataProvider().sample(QgsPointXY(18.68704, 45.79568), 1)
            self.assertTrue(ok)
            self.assertEqual(val, 100.0)

    def testRetile(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = retile()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -r near -ot Float32 -targetDir {outdir} ' +
                 source])

            # with input srs
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -s_srs EPSG:3111 -r near -ot Float32 -targetDir {outdir} {source}'
                 ])

            # with target using proj string
            custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'SOURCE_CRS': custom_crs,
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -s_srs EPSG:20936 -r near -ot Float32 -targetDir {outdir} {source}'
                 ])

            # with target using custom projection
            custom_crs, expected_crs_string = self.get_param_value_and_expected_string_for_custom_crs('+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'SOURCE_CRS': custom_crs,
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -s_srs "{expected_crs_string}" -r near -ot Float32 -targetDir {outdir} {source}'
                 ])

            # with non-EPSG crs code
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'SOURCE_CRS': 'POSTGIS:3111',
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -s_srs EPSG:3111 -r near -ot Float32 -targetDir {outdir} {source}'
                 ])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'OUTPUT_CSV': 'out.csv',
                                        'DELIMITER': '',
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -r near -ot Float32 -csv out.csv -targetDir {outdir} ' +
                 source])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'OUTPUT_CSV': 'out.csv',
                                        'DELIMITER': ';',
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -r near -ot Float32 -csv out.csv -csvDelim ";" -targetDir {outdir} ' +
                 source])

            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'EXTRA': '-v -tileIndex tindex.shp',
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -r near -ot Float32 -v -tileIndex tindex.shp -targetDir {outdir} ' +
                 source])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'ONLY_PYRAMIDS': True,
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -r near -ot Float32 -pyramidOnly -targetDir {outdir} ' +
                 source])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source],
                                        'DIR_FOR_ROW': True,
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -r near -ot Float32 -useDirForEachRow -targetDir {outdir} ' +
                 source])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': [source + '|credential:X=Y|credential:Z=A'],
                                        'DIR_FOR_ROW': True,
                                        'OUTPUT': outdir}, context, feedback),
                ['gdal_retile.py',
                 f'-ps 256 256 -overlap 0 -levels 1 -r near -ot Float32 -useDirForEachRow -targetDir {outdir} ' +
                 source + ' --config X Y --config Z A'])

    def testWarp(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = warp()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # with no NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with None NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': None,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 9999,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -dstnodata 9999.0 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -dstnodata 0.0 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])
            # with "0" NODATA value and custom data type
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NODATA': 0,
                                        'DATA_TYPE': 6,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -dstnodata 0.0 -r near -ot Float32 -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with target using EPSG
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'TARGET_CRS': 'EPSG:4326',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -t_srs EPSG:4326 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with target using proj string
            custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': custom_crs,
                                        'TARGET_CRS': custom_crs,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:20936 -t_srs EPSG:20936 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with target using custom projection
            custom_crs, expected_crs_string = self.get_param_value_and_expected_string_for_custom_crs('+proj=utm +zone=36 +south +a=63785 +b=6357 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': custom_crs,
                                        'TARGET_CRS': custom_crs,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 f'-overwrite -s_srs "{expected_crs_string}" -t_srs "{expected_crs_string}" -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with target using custom projection and user-defined extent
            custom_crs2, expected_crs_string2 = self.get_param_value_and_expected_string_for_custom_crs('+proj=longlat +a=6378388 +b=6356912 +no_defs')
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': custom_crs2,
                                        'TARGET_CRS': custom_crs2,
                                        'TARGET_EXTENT': '18.67,18.70,45.78,45.81',
                                        'TARGET_EXTENT_CRS': custom_crs2,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdalwarp',
                 f'-overwrite -s_srs "{expected_crs_string2}" -t_srs "{expected_crs_string2}" -r near -te 18.67 45.78 18.7 45.81 -te_srs "{expected_crs_string2}" -of GTiff ' +
                 source + ' ' +
                 outdir + '/check.tif'])

            # with non-EPSG crs code
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': 'POSTGIS:3111',
                                        'TARGET_CRS': 'POSTGIS:3111',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -t_srs EPSG:3111 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with target resolution with None value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'TARGET_RESOLUTION': None,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # test target resolution with a valid value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'TARGET_RESOLUTION': 10.0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -tr 10.0 10.0 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # test target resolution with a value of zero, to be ignored
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'TARGET_RESOLUTION': 0.0,
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            # with additional command-line parameter
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-dstalpha',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -r near -of JPEG -dstalpha ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-dstalpha -srcnodata -9999',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -r near -of JPEG -dstalpha -srcnodata -9999 ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-dstalpha -srcnodata "-9999 -8888"',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -r near -of JPEG -dstalpha -srcnodata "-9999 -8888" ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'SOURCE_CRS': 'EPSG:3111',
                                        'OUTPUT': outdir + '/check.jpg'}, context, feedback),
                ['gdalwarp',
                 '-overwrite -s_srs EPSG:3111 -r near -of JPEG ' +
                 source + ' ' +
                 outdir + '/check.jpg --config X Y --config Z A'])

    def testMerge(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = [os.path.join(testDataPath, 'dem1.tif'), os.path.join(testDataPath, 'dem1.tif')]
        alg = merge()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # this algorithm creates temporary text file with input layers
            # so we strip its path, leaving only filename
            cmd = alg.getConsoleCommands({'INPUT': source,
                                          'OUTPUT': outdir + '/check.tif'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('--optfile') + 10] + t[t.find('mergeInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdal_merge.py',
                              '-ot Float32 -of GTiff ' +
                              '-o ' + outdir + '/check.tif ' +
                              '--optfile mergeInputFiles.txt'])
            # separate
            cmd = alg.getConsoleCommands({'INPUT': source,
                                          'SEPARATE': True,
                                          'OUTPUT': outdir + '/check.tif'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('--optfile') + 10] + t[t.find('mergeInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdal_merge.py',
                              '-separate -ot Float32 -of GTiff ' +
                              '-o ' + outdir + '/check.tif ' +
                              '--optfile mergeInputFiles.txt'])

            # assign nodata
            cmd = alg.getConsoleCommands({'INPUT': source,
                                          'EXTRA': '-tap -ps 0.1 0.1',
                                          'OUTPUT': outdir + '/check.tif'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('--optfile') + 10] + t[t.find('mergeInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdal_merge.py',
                              '-ot Float32 -of GTiff -tap -ps 0.1 0.1 ' +
                              '-o ' + outdir + '/check.tif ' +
                              '--optfile mergeInputFiles.txt'])

            # additional parameters
            cmd = alg.getConsoleCommands({'INPUT': source,
                                          'NODATA_OUTPUT': -9999,
                                          'OUTPUT': outdir + '/check.tif'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('--optfile') + 10] + t[t.find('mergeInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdal_merge.py',
                              '-a_nodata -9999.0 -ot Float32 -of GTiff ' +
                              '-o ' + outdir + '/check.tif ' +
                              '--optfile mergeInputFiles.txt'])

    def testNearblack(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = nearblack()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['nearblack',
                 source + ' -of GTiff -o ' + outdir + '/check.tif ' +
                 '-near 15'])

            # search white pixels
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'WHITE': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['nearblack',
                 source + ' -of GTiff -o ' + outdir + '/check.tif ' +
                 '-near 15 -white'])

            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-nb 5 -setalpha',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['nearblack',
                 source + ' -of GTiff -o ' + outdir + '/check.tif ' +
                 '-near 15 -nb 5 -setalpha'])

            # additional parameters and creation options
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OPTIONS': 'COMPRESS=JPEG|JPEG_QUALITY=75',
                                        'EXTRA': '-nb 5 -setalpha',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['nearblack',
                 source + ' -of GTiff -o ' + outdir + '/check.tif ' +
                 '-near 15 -co COMPRESS=JPEG -co JPEG_QUALITY=75 -nb 5 -setalpha'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['nearblack',
                 source + ' -of GTiff -o ' + outdir + '/check.tif ' +
                 '-near 15 --config X Y --config Z A'])

    def testRearrangeBands(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')

        with tempfile.TemporaryDirectory() as outdir:
            outsource = outdir + '/check.tif'

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

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|option:X_POSSIBLE_NAMES=geom_x|option:Y_POSSIBLE_NAMES=geom_y',
                                        'BANDS': 1,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_translate', '-b 1 ' +
                 '-of GTiff ' +
                 source + ' ' + outsource + ' -oo X_POSSIBLE_NAMES=geom_x -oo Y_POSSIBLE_NAMES=geom_y'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BANDS': 1,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_translate', '-b 1 ' +
                 '-of GTiff ' +
                 source + ' ' + outsource + ' --config X Y --config Z A'])

    def testFillnodata(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        mask = os.path.join(testDataPath, 'raster.tif')

        with tempfile.TemporaryDirectory() as outdir:
            outsource = outdir + '/check.tif'
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
                 f'{source} {outsource} -md 10 -b 1 -mask {mask} -of GTiff'])

            # without mask value
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'DISTANCE': 10,
                                        'ITERATIONS': 0,
                                        'NO_MASK': False,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_fillnodata.py',
                 f'{source} {outsource} -md 10 -b 1 -of GTiff'])

            # The -nomask option is no longer supported since GDAL 3.4 and
            # it doesn't work as expected even using GDAL < 3.4 https://github.com/OSGeo/gdal/pull/4201
            # Silently ignore the NO_MASK parameter
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'DISTANCE': 10,
                                        'ITERATIONS': 0,
                                        'NO_MASK': True,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_fillnodata.py',
                 f'{source} {outsource} -md 10 -b 1 -of GTiff'])

            # creation options
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'OPTIONS': 'COMPRESS=JPEG|JPEG_QUALITY=75',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_fillnodata.py',
                 f'{source} {outsource} -md 10 -b 1 -of GTiff -co COMPRESS=JPEG -co JPEG_QUALITY=75'])

            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'EXTRA': '-q',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_fillnodata.py',
                 f'{source} {outsource} -md 10 -b 1 -of GTiff -q'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'DISTANCE': 10,
                                        'ITERATIONS': 0,
                                        'MASK_LAYER': mask,
                                        'NO_MASK': False,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_fillnodata.py',
                 f'{source} {outsource} -md 10 -b 1 -mask {mask} -of GTiff --config X Y --config Z A'])

    def testGdalAddo(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')

        with tempfile.TemporaryDirectory() as outdir:
            alg = gdaladdo()
            alg.initAlgorithm()

            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'LEVELS': '2 4 8 16',
                                        'CLEAN': False,
                                        'RESAMPLING': 0,
                                        'FORMAT': 0}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '-r nearest 2 4 8 16'])

            # with "clean" option
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'LEVELS': '2 4 8 16',
                                        'CLEAN': True,
                                        'RESAMPLING': 0,
                                        'FORMAT': 0}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '-r nearest -clean 2 4 8 16'])

            # ovr format
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'LEVELS': '2 4 8 16',
                                        'CLEAN': False,
                                        'RESAMPLING': 0,
                                        'FORMAT': 1}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '-r nearest -ro 2 4 8 16'])

            # Erdas format
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'LEVELS': '2 4 8 16',
                                        'CLEAN': False,
                                        'RESAMPLING': 0,
                                        'FORMAT': 2}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '-r nearest --config USE_RRD YES 2 4 8 16'])

            # custom resampling method format
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'LEVELS': '2 4 8 16',
                                        'CLEAN': False,
                                        'RESAMPLING': 4,
                                        'FORMAT': 0}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '-r cubicspline 2 4 8 16'])

            # more levels
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'LEVELS': '2 4 8 16 32 64',
                                        'CLEAN': False,
                                        'RESAMPLING': 0,
                                        'FORMAT': 0}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '-r nearest 2 4 8 16 32 64'])

            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'LEVELS': '2 4 8 16',
                                        'CLEAN': False,
                                        'EXTRA': '--config COMPRESS_OVERVIEW JPEG'}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '--config COMPRESS_OVERVIEW JPEG 2 4 8 16'])

            if GdalUtils.version() >= 230000:
                # without levels
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source,
                                            'CLEAN': False}, context, feedback),
                    ['gdaladdo',
                     source])

            # without advanced params
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'LEVELS': '2 4 8 16',
                                        'CLEAN': False}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '2 4 8 16'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'LEVELS': '2 4 8 16',
                                        'CLEAN': False,
                                        'RESAMPLING': 0,
                                        'FORMAT': 0}, context, feedback),
                ['gdaladdo',
                 source + ' ' + '-r nearest 2 4 8 16 --config X Y --config Z A'])

    def testSieve(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        mask = os.path.join(testDataPath, 'raster.tif')

        with tempfile.TemporaryDirectory() as outdir:
            outsource = outdir + '/check.tif'
            alg = sieve()
            alg.initAlgorithm()

            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_sieve.py',
                 '-st 10 -4 -of GTiff ' +
                 source + ' ' +
                 outsource])

            # Eight connectedness and custom threshold
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'THRESHOLD': 16,
                                        'EIGHT_CONNECTEDNESS': True,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_sieve.py',
                 '-st 16 -8 -of GTiff ' +
                 source + ' ' +
                 outsource])

            # without default mask layer
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NO_MASK': True,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_sieve.py',
                 '-st 10 -4 -nomask -of GTiff ' +
                 source + ' ' +
                 outsource])

            # defaults with external validity mask
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'MASK_LAYER': mask,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_sieve.py',
                 '-st 10 -4 -mask ' +
                 mask +
                 ' -of GTiff ' +
                 source + ' ' +
                 outsource])

            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'EXTRA': '-q',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_sieve.py',
                 '-st 10 -4 -of GTiff -q ' +
                 source + ' ' +
                 outsource])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_sieve.py',
                 '-st 10 -4 -of GTiff ' +
                 source + ' ' +
                 outsource + ' --config X Y --config Z A'])

    def testGdal2Xyz(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')

        with tempfile.TemporaryDirectory() as outdir:
            outsource = outdir + '/check.csv'
            alg = gdal2xyz()
            alg.initAlgorithm()

            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'CSV': False,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal2xyz.py',
                 '-band 1 ' +
                 source + ' ' +
                 outsource])

            # csv output
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'CSV': True,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal2xyz.py',
                 '-band 1 -csv ' +
                 source + ' ' +
                    outsource])

            if GdalUtils.version() >= 3030000:
                # skip nodata output
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source,
                                            'BAND': 1,
                                            'CSV': False,
                                            'SKIP_NODATA': True,
                                            'OUTPUT': outsource}, context, feedback),
                    ['gdal2xyz.py',
                     '-band 1 -skipnodata ' +
                     source + ' ' +
                     outsource])

            if GdalUtils.version() > 3060300:
                # srcnodata output
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source,
                                            'BAND': 1,
                                            'CSV': False,
                                            'NODATA_INPUT': -999,
                                            'SKIP_NODATA': False,
                                            'OUTPUT': outsource}, context, feedback),
                    ['gdal2xyz.py',
                     '-band 1 -srcnodata -999.0 ' +
                     source + ' ' +
                     outsource])

                # dstnodata output
                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source,
                                            'BAND': 1,
                                            'CSV': False,
                                            'NODATA_OUTPUT': -999,
                                            'SKIP_NODATA': False,
                                            'OUTPUT': outsource}, context, feedback),
                    ['gdal2xyz.py',
                     '-band 1 -dstnodata -999.0 ' +
                     source + ' ' +
                     outsource])

                self.assertEqual(
                    alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                            'BAND': 1,
                                            'CSV': False,
                                            'OUTPUT': outsource}, context,
                                           feedback),
                    ['gdal2xyz.py',
                     '-band 1 ' +
                     source + ' ' +
                     outsource + ' --config X Y --config Z A'])

    def testGdalPolygonize(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')

        with tempfile.TemporaryDirectory() as outdir:
            outsource = outdir + '/check.shp'
            alg = polygonize()
            alg.initAlgorithm()

            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD': 'DN',
                                        'EIGHT_CONNECTEDNESS': False,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_polygonize.py',
                 source + ' ' +
                 '-b 1 -f "ESRI Shapefile"' + ' ' + outsource + ' ' + 'check DN'
                 ])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD': 'VAL',
                                        'EIGHT_CONNECTEDNESS': False,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_polygonize.py',
                 source + ' ' +
                 '-b 1 -f "ESRI Shapefile"' + ' ' + outsource + ' ' + 'check VAL'
                 ])

            # 8 connectedness
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD': 'DN',
                                        'EIGHT_CONNECTEDNESS': True,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_polygonize.py',
                 '-8' + ' ' + source + ' ' +
                 '-b 1 -f "ESRI Shapefile"' + ' ' + outsource + ' ' + 'check DN'
                 ])

            # custom output format
            outsource = outdir + '/check.gpkg'
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD': 'DN',
                                        'EIGHT_CONNECTEDNESS': False,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_polygonize.py',
                 source + ' ' +
                 '-b 1 -f "GPKG"' + ' ' + outsource + ' ' + 'check DN'
                 ])

            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 1,
                                        'FIELD': 'DN',
                                        'EXTRA': '-nomask -q',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_polygonize.py',
                 '-nomask -q' + ' ' + source + ' ' +
                 '-b 1 -f "GPKG"' + ' ' + outsource + ' ' + 'check DN'
                 ])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'FIELD': 'DN',
                                        'EIGHT_CONNECTEDNESS': False,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_polygonize.py',
                 source + ' ' +
                 '-b 1 -f "GPKG"' + ' ' + outsource + ' ' + 'check DN --config X Y --config Z A'
                 ])

    def testGdalPansharpen(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        panchrom = os.path.join(testDataPath, 'dem.tif')
        spectral = os.path.join(testDataPath, 'raster.tif')

        with tempfile.TemporaryDirectory() as outdir:
            outsource = outdir + '/out.tif'
            alg = pansharp()
            alg.initAlgorithm()

            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'SPECTRAL': spectral,
                                        'PANCHROMATIC': panchrom,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_pansharpen.py',
                 panchrom + ' ' +
                 spectral + ' ' +
                 outsource + ' ' +
                 '-r cubic -of GTiff'
                 ])

            # custom resampling
            self.assertEqual(
                alg.getConsoleCommands({'SPECTRAL': spectral,
                                        'PANCHROMATIC': panchrom,
                                        'RESAMPLING': 4,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_pansharpen.py',
                 panchrom + ' ' +
                 spectral + ' ' +
                 outsource + ' ' +
                 '-r lanczos -of GTiff'
                 ])

            # additional parameters
            self.assertEqual(
                alg.getConsoleCommands({'SPECTRAL': spectral,
                                        'PANCHROMATIC': panchrom,
                                        'EXTRA': '-bitdepth 12 -threads ALL_CPUS',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_pansharpen.py',
                 panchrom + ' ' +
                 spectral + ' ' +
                 outsource + ' ' +
                 '-r cubic -of GTiff -bitdepth 12 -threads ALL_CPUS'
                 ])

            self.assertEqual(
                alg.getConsoleCommands({'SPECTRAL': spectral,
                                        'PANCHROMATIC': panchrom + '|credential:X=Y|credential:Z=A',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_pansharpen.py',
                 panchrom + ' ' +
                 spectral + ' ' +
                 outsource + ' ' +
                 '-r cubic -of GTiff --config X Y --config Z A'
                 ])

    def testGdalViewshed(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        dem = os.path.join(testDataPath, 'dem.tif')

        with tempfile.TemporaryDirectory() as outdir:
            outsource = outdir + '/out.tif'
            alg = viewshed()
            alg.initAlgorithm()

            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': dem,
                                        'BAND': 1,
                                        'OBSERVER': '18.67274,45.80599',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_viewshed',
                 '-b 1 -ox 18.67274 -oy 45.80599 -oz 1.0 -tz 1.0 -md 100.0 -f GTiff ' +
                 dem + ' ' + outsource
                 ])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': dem,
                                        'BAND': 2,
                                        'OBSERVER': '18.67274,45.80599',
                                        'OBSERVER_HEIGHT': 1.8,
                                        'TARGET_HEIGHT': 20,
                                        'MAX_DISTANCE': 1000,
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_viewshed',
                 '-b 2 -ox 18.67274 -oy 45.80599 -oz 1.8 -tz 20.0 -md 1000.0 -f GTiff ' +
                 dem + ' ' + outsource
                 ])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': dem,
                                        'BAND': 1,
                                        'OBSERVER': '18.67274,45.80599',
                                        'EXTRA': '-a_nodata=-9999 -cc 0.2',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_viewshed',
                 '-b 1 -ox 18.67274 -oy 45.80599 -oz 1.0 -tz 1.0 -md 100.0 -f GTiff ' +
                 '-a_nodata=-9999 -cc 0.2 ' + dem + ' ' + outsource
                 ])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': dem,
                                        'BAND': 1,
                                        'OBSERVER': '18.67274,45.80599',
                                        'OPTIONS': 'COMPRESS=DEFLATE|PREDICTOR=2|ZLEVEL=9',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_viewshed',
                 '-b 1 -ox 18.67274 -oy 45.80599 -oz 1.0 -tz 1.0 -md 100.0 -f GTiff ' +
                 '-co COMPRESS=DEFLATE -co PREDICTOR=2 -co ZLEVEL=9 ' + dem + ' ' + outsource
                 ])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': dem + '|credential:X=Y|credential:Z=A',
                                        'BAND': 1,
                                        'OBSERVER': '18.67274,45.80599',
                                        'OUTPUT': outsource}, context, feedback),
                ['gdal_viewshed',
                 '-b 1 -ox 18.67274 -oy 45.80599 -oz 1.0 -tz 1.0 -md 100.0 -f GTiff ' +
                 dem + ' ' + outsource + ' --config X Y --config Z A'
                 ])

    def testBuildVrt(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = buildvrt()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # defaults
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -r nearest ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])
            # custom resolution
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'RESOLUTION': 2,
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution lowest -separate -r nearest ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])
            # single layer
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'SEPARATE': False,
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -r nearest ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])
            # projection difference
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'PROJ_DIFFERENCE': True,
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -allow_projection_difference -r nearest ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])
            # add alpha band
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'ADD_ALPHA': True,
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -addalpha -r nearest ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])
            # assign CRS
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'ASSIGN_CRS': 'EPSG:3111',
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -a_srs EPSG:3111 -r nearest ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])

            custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'ASSIGN_CRS': custom_crs,
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -a_srs EPSG:20936 -r nearest ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])
            # source NODATA
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'SRC_NODATA': '-9999',
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -r nearest -srcnodata -9999 ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])

            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'SRC_NODATA': '-9999 9999',
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -r nearest -srcnodata "-9999 9999" ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])

            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'SRC_NODATA': '',
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -r nearest ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])
            # additional parameters
            cmd = alg.getConsoleCommands({'INPUT': [source],
                                          'EXTRA': '-overwrite -optim RASTER -vrtnodata -9999',
                                          'OUTPUT': outdir + '/check.vrt'}, context, feedback)
            t = cmd[1]
            cmd[1] = t[:t.find('-input_file_list') + 17] + t[t.find('buildvrtInputFiles.txt'):]
            self.assertEqual(cmd,
                             ['gdalbuildvrt',
                              '-overwrite -resolution average -separate -r nearest -overwrite -optim RASTER -vrtnodata -9999 ' +
                              '-input_file_list buildvrtInputFiles.txt ' +
                              outdir + '/check.vrt'])

    def testPct2Rgb(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = pct2rgb()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['pct2rgb.py',
                 source + ' ' + outdir + '/check.tif ' +
                 '-of GTiff -b 1'])

            # set band
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 3,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['pct2rgb.py',
                 source + ' ' + outdir + '/check.tif ' +
                 '-of GTiff -b 3'])

            # set RGBA
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'RGBA': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['pct2rgb.py',
                 source + ' ' + outdir + '/check.tif ' +
                 '-of GTiff -b 1 -rgba'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['pct2rgb.py',
                 source + ' ' + outdir + '/check.tif ' +
                 '-of GTiff -b 1 --config X Y --config Z A'])

    def testRgb2Pct(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = rgb2pct()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['rgb2pct.py',
                 '-n 2 -of GTiff ' + source + ' ' + outdir + '/check.tif'])

            # set number of colors
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'NCOLORS': 8,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['rgb2pct.py',
                 '-n 8 -of GTiff ' + source + ' ' + outdir + '/check.tif'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['rgb2pct.py',
                 '-n 2 -of GTiff ' + source + ' ' + outdir + '/check.tif --config X Y --config Z A'])

    def testRoughness(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        alg = roughness()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            # defaults
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'roughness ' + source + ' ' + outdir + '/check.tif ' + '-of GTiff -b 1'])

            # set band
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'BAND': 3,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'roughness ' + source + ' ' + outdir + '/check.tif ' + '-of GTiff -b 3'])

            # compute edges
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'COMPUTE_EDGES': True,
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'roughness ' + source + ' ' + outdir + '/check.tif ' + '-of GTiff -b 1 -compute_edges'])

            # creation options
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OPTIONS': 'COMPRESS=DEFLATE|PREDICTOR=2|ZLEVEL=9',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'roughness ' + source + ' ' + outdir + '/check.tif ' +
                 '-of GTiff -b 1 -co COMPRESS=DEFLATE -co PREDICTOR=2 -co ZLEVEL=9'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source + '|credential:X=Y|credential:Z=A',
                                        'OUTPUT': outdir + '/check.tif'}, context, feedback),
                ['gdaldem',
                 'roughness ' + source + ' ' + outdir + '/check.tif ' + '-of GTiff -b 1 --config X Y --config Z A'])


if __name__ == '__main__':
    nose2.main()
