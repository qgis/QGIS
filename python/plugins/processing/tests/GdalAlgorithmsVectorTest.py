# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithmVectorTest.py
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
                       QgsProcessingFeedback,
                       QgsCoordinateReferenceSystem,
                       QgsRectangle)

from qgis.testing import (start_app,
                          unittest)

import AlgorithmsTestBase
from processing.algs.gdal.ogr2ogr import ogr2ogr
from processing.algs.gdal.ogrinfo import ogrinfo
from processing.algs.gdal.Buffer import Buffer
from processing.algs.gdal.Dissolve import Dissolve
from processing.algs.gdal.OffsetCurve import OffsetCurve
from processing.algs.gdal.OgrToPostGis import OgrToPostGis
from processing.algs.gdal.OneSideBuffer import OneSideBuffer
from processing.algs.gdal.PointsAlongLines import PointsAlongLines

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')


class TestGdalVectorAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

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
        return 'gdal_algorithm_vector_tests.yaml'

    def testOgr2Ogr(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        multi_source = os.path.join(testDataPath, 'multi_layers.gml')
        alg = ogr2ogr()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 '-f "ESRI Shapefile" ' + outdir + '/check.shp ' +
                 source + ' polys2'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.kml'}, context, feedback),
                ['ogr2ogr',
                 '-f "LIBKML" ' + outdir + '/check.kml ' +
                 source + ' polys2'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/my out/check.kml'}, context, feedback),
                ['ogr2ogr',
                 '-f "LIBKML" "' + outdir + '/my out/check.kml" ' +
                 source + ' polys2'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.gpkg'}, context, feedback),
                ['ogr2ogr',
                 '-f "GPKG" ' + outdir + '/check.gpkg ' +
                 source + ' polys2'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': multi_source + '|layername=lines',
                                        'CONVERT_ALL_LAYERS': False,
                                        'OUTPUT': outdir + '/check.gpkg'}, context, feedback),
                ['ogr2ogr',
                 '-f "GPKG" ' + outdir + '/check.gpkg ' +
                 multi_source + ' lines'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': multi_source + '|layername=lines',
                                        'CONVERT_ALL_LAYERS': True,
                                        'OUTPUT': outdir + '/check.gpkg'}, context, feedback),
                ['ogr2ogr',
                 '-f "GPKG" ' + outdir + '/check.gpkg ' +
                 multi_source])

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
             source + ' polys2'])

        source = os.path.join(testDataPath, 'filename with spaces.gml')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SUMMARY_ONLY': True,
                                    'NO_METADATA': False}, context, feedback),
            ['ogrinfo',
             '-al -so "' +
             source + '" filename_with_spaces'])

        source = os.path.join(testDataPath, 'filename with spaces.gml')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SUMMARY_ONLY': False,
                                    'NO_METADATA': False}, context, feedback),
            ['ogrinfo',
             '-al "' +
             source + '" filename_with_spaces'])

        source = os.path.join(testDataPath, 'filename with spaces.gml')
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SUMMARY_ONLY': True,
                                    'NO_METADATA': True}, context, feedback),
            ['ogrinfo',
             '-al -so -nomd "' +
             source + '" filename_with_spaces'])

    def testBuffer(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        source_with_space = os.path.join(testDataPath, 'filename with spaces.gml')
        alg = Buffer()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Buffer(geometry, 5.0) AS geometry,* FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': -5,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Buffer(geometry, -5.0) AS geometry,* FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'DISSOLVE': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Union(ST_Buffer(geometry, 5.0)) AS geometry,* FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 1,
                                        'DISSOLVE': True,
                                        'EXPLODE_COLLECTIONS': False,
                                        'GEOMETRY': 'geom',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Union(ST_Buffer(geom, 1.0)) AS geom,* FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'EXPLODE_COLLECTIONS': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Buffer(geometry, 5.0) AS geometry,* FROM """polys2"""" ' +
                 '-explodecollections -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'FIELD': 'total population',
                                        'EXPLODE_COLLECTIONS': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Union(ST_Buffer(geometry, 5.0)) AS geometry,* FROM """polys2""" GROUP BY """total population"""" ' +
                 '-explodecollections -f "ESRI Shapefile"'])

    def testDissolve(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        source_with_space = os.path.join(testDataPath, 'filename with spaces.gml')
        alg = Dissolve()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""" FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'total population',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """total population""" FROM """polys2""" ' +
                 'GROUP BY """total population"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source_with_space,
                                        'FIELD': 'my_field',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 '"' + source_with_space + '" ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""" FROM """filename_with_spaces""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'GEOMETRY': 'the_geom',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(the_geom) AS the_geom, """my_field""" FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'KEEP_ATTRIBUTES': False,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""" FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'KEEP_ATTRIBUTES': False,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'EXPLODE_COLLECTIONS': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""" FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -explodecollections -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'COUNT_FEATURES': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""", COUNT(geometry) AS count FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'COUNT_FEATURES': True,
                                        'GEOMETRY': 'the_geom',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(the_geom) AS the_geom, """my_field""", COUNT(the_geom) AS count FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'COMPUTE_AREA': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""", SUM(ST_Area(geometry)) AS area, ' +
                 'ST_Perimeter(ST_Union(geometry)) AS perimeter FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'COMPUTE_AREA': True,
                                        'GEOMETRY': 'the_geom',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(the_geom) AS the_geom, """my_field""", SUM(ST_Area(the_geom)) AS area, ' +
                 'ST_Perimeter(ST_Union(the_geom)) AS perimeter FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'COMPUTE_STATISTICS': True,
                                        'STATISTICS_ATTRIBUTE': 'my_val',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""", ' +
                 'SUM("""my_val""") AS sum, MIN("""my_val""") AS min, MAX("""my_val""") AS max, AVG("""my_val""") AS avg FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'test field',
                                        'COMPUTE_STATISTICS': True,
                                        'STATISTICS_ATTRIBUTE': 'total population',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """test field""", ' +
                 'SUM("""total population""") AS sum, MIN("""total population""") AS min, MAX("""total population""") AS max, ' +
                 'AVG("""total population""") AS avg FROM """polys2""" ' +
                 'GROUP BY """test field"""" -f "ESRI Shapefile"'])

            # compute stats without stats attribute, and vice versa (should be ignored)
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'COMPUTE_STATISTICS': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""" FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'STATISTICS_ATTRIBUTE': 'my_val',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""" FROM """polys2""" ' +
                 'GROUP BY """my_field"""" -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'FIELD': 'my_field',
                                        'OPTIONS': 'my opts',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-nlt PROMOTE_TO_MULTI -dialect sqlite -sql "SELECT ST_Union(geometry) AS geometry, """my_field""" FROM """polys2""" ' +
                 'GROUP BY """my_field"""" "my opts" -f "ESRI Shapefile"'])

    def testOgr2PostGis(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        source_with_space = os.path.join(testDataPath, 'filename with spaces.gml')
        alg = OgrToPostGis()
        alg.initAlgorithm()

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source_with_space}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 "' + source_with_space + '" filename_with_spaces '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.filename_with_spaces -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'HOST': 'google.com'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=google.com port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'PORT': 3333}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=3333 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'USER': 'kevin_bacon'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public user=kevin_bacon" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'DBNAME': 'secret_stuff'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 dbname=secret_stuff active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'PASSWORD': 'passw0rd'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 password=passw0rd active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SCHEMA': 'desktop'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=desktop" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln desktop.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'TABLE': 'out_table'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.out_table -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'PK': ''}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'PK': 'new_fid'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=new_fid -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'PK': '',
                                    'PRIMARY_KEY': 'objectid'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=objectid -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'PK': 'new_id',
                                    'PRIMARY_KEY': 'objectid'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=new_id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'GEOCOLUMN': 'my_geom'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=my_geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'DIM': 1}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=3 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SIMPLIFY': 5}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -simplify 5 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SEGMENTIZE': 4}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -segmentize 4 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SPAT': QgsRectangle(1, 2, 3, 4)}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -spat 1.0 2.0 3.0 4.0 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'FIELDS': ['f1', 'f2']}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 -select "f1,f2" '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'WHERE': '0=1'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -where "0=1" -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'GT': 2}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -gt 2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OVERWRITE': False}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'APPEND': True}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-append -overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'ADDFIELDS': True}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-addfields -overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'LAUNDER': True}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-lco LAUNDER=NO -overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'INDEX': True}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-lco SPATIAL_INDEX=OFF -overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SKIPFAILURES': True}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -skipfailures -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'PROMOTETOMULTI': False}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'PRECISION': False}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI -lco PRECISION=NO'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'OPTIONS': 'blah'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI blah'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'SHAPE_ENCODING': 'blah'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES --config SHAPE_ENCODING blah -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'GTYPE': 4}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -nlt LINESTRING -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'A_SRS': 'EPSG:3111'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -a_srs EPSG:3111 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'A_SRS': QgsCoordinateReferenceSystem('EPSG:3111')}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -a_srs EPSG:3111 -nlt PROMOTE_TO_MULTI'])

        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'A_SRS': custom_crs}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -a_srs EPSG:20936 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'T_SRS': 'EPSG:3111'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -t_srs EPSG:3111 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'T_SRS': QgsCoordinateReferenceSystem('EPSG:3111')}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -t_srs EPSG:3111 -nlt PROMOTE_TO_MULTI'])

        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'T_SRS': custom_crs}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -t_srs EPSG:20936 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'S_SRS': 'EPSG:3111'}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -s_srs EPSG:3111 -nlt PROMOTE_TO_MULTI'])

        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'S_SRS': QgsCoordinateReferenceSystem('EPSG:3111')}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -s_srs EPSG:3111 -nlt PROMOTE_TO_MULTI'])

        custom_crs = 'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs'
        self.assertEqual(
            alg.getConsoleCommands({'INPUT': source,
                                    'S_SRS': custom_crs}, context, feedback),
            ['ogr2ogr',
             '-progress --config PG_USE_COPY YES -f PostgreSQL "PG:host=localhost port=5432 active_schema=public" '
             '-lco DIM=2 ' + source + ' polys2 '
             '-overwrite -lco GEOMETRY_NAME=geom -lco FID=id -nln public.polys2 -s_srs EPSG:20936 -nlt PROMOTE_TO_MULTI'])

    def testOffsetCurve(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        source_with_space = os.path.join(testDataPath, 'filename with spaces.gml')
        alg = OffsetCurve()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_OffsetCurve(geometry, 5.0) AS geometry,* FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

    def testOneSidedBuffer(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        source_with_space = os.path.join(testDataPath, 'filename with spaces.gml')
        alg = OneSideBuffer()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_SingleSidedBuffer(geometry, 5.0, 0) AS geometry,* FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'DISSOLVE': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Union(ST_SingleSidedBuffer(geometry, 5.0, 0)) AS geometry,* FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'EXPLODE_COLLECTIONS': True,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_SingleSidedBuffer(geometry, 5.0, 0) AS geometry,* FROM """polys2"""" ' +
                 '-explodecollections -f "ESRI Shapefile"'])

            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 5,
                                        'FIELD': 'total population',
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Union(ST_SingleSidedBuffer(geometry, 5.0, 0)) AS geometry,* ' +
                 'FROM """polys2""" GROUP BY """total population"""" -f "ESRI Shapefile"'])

    def testPointsAlongLines(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'polys.gml')
        source_with_space = os.path.join(testDataPath, 'filename with spaces.gml')
        alg = PointsAlongLines()
        alg.initAlgorithm()

        with tempfile.TemporaryDirectory() as outdir:
            self.assertEqual(
                alg.getConsoleCommands({'INPUT': source,
                                        'DISTANCE': 0.2,
                                        'OUTPUT': outdir + '/check.shp'}, context, feedback),
                ['ogr2ogr',
                 outdir + '/check.shp ' +
                 source + ' ' +
                 '-dialect sqlite -sql "SELECT ST_Line_Interpolate_Point(geometry, 0.2) AS geometry,* FROM """polys2"""" ' +
                 '-f "ESRI Shapefile"'])


if __name__ == '__main__':
    nose2.main()
