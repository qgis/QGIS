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

import nose2
import os
import shutil
import tempfile

from qgis.core import (QgsProcessingContext,
                       QgsProcessingFeedback,
                       QgsCoordinateReferenceSystem,
                       QgsApplication,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsProject,
                       QgsVectorLayer,
                       QgsRectangle,
                       QgsProjUtils,
                       QgsProcessingException,
                       QgsProcessingFeatureSourceDefinition)

from qgis.testing import (QgisTestCase,
                          start_app)

from processing.algs.gdal.GdalUtils import GdalUtils
from processing.algs.gdal.ogr2ogr import ogr2ogr
from processing.algs.gdal.OgrToPostGis import OgrToPostGis

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')


class TestGdalAlgorithms(QgisTestCase):

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

    def testCommandName(self):
        # Test that algorithms report a valid commandName
        p = QgsApplication.processingRegistry().providerById('gdal')
        for a in p.algorithms():
            if a.id() in ('gdal:buildvirtualvector'):
                # build virtual vector is an exception
                continue
            self.assertTrue(a.commandName(), f'Algorithm {a.id()} has no commandName!')

    def testCommandNameInTags(self):
        # Test that algorithms commandName is present in provided tags
        p = QgsApplication.processingRegistry().providerById('gdal')
        for a in p.algorithms():
            if not a.commandName():
                continue
            self.assertTrue(a.commandName() in a.tags(), f'Algorithm {a.id()} commandName not found in tags!')

    def testNoParameters(self):
        # Test that algorithms throw QgsProcessingException and not base Python
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
        # check that memory layer is automatically saved out to geopackage when required by GDAL algorithms
        input_details = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback,
                                                   executing=True)
        self.assertTrue(input_details.connection_string)
        self.assertTrue(input_details.connection_string.endswith('.gpkg'))
        self.assertTrue(os.path.exists(input_details.connection_string))
        self.assertTrue(input_details.connection_string)

        # make sure that layer has correct features
        res = QgsVectorLayer(input_details.connection_string, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 2)

        # with memory layers - if not executing layer source should be ignored and replaced
        # with a dummy path, because:
        # - it has no meaning for the gdal command outside of QGIS, memory layers don't exist!
        # - we don't want to force an export of the whole memory layer to a temp file just to show the command preview
        # this might be very slow!
        input_details = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback,
                                                   executing=False)
        self.assertEqual(input_details.connection_string, 'path_to_data_file')
        self.assertEqual(input_details.layer_name, 'layer_name')

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
        input_details = alg.getOgrCompatibleSource('INPUT', {'INPUT': vl.id()}, context, feedback, True)
        self.assertEqual(input_details.connection_string, source)
        input_details = alg.getOgrCompatibleSource('INPUT', {'INPUT': vl.id()}, context, feedback, False)
        self.assertEqual(input_details.connection_string, source)

        # with selected features only - if not executing, the 'selected features only' setting
        # should be ignored (because it has no meaning for the gdal command outside of QGIS!)
        parameters = {'INPUT': QgsProcessingFeatureSourceDefinition(vl.id(), True)}
        input_details = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback, False)
        self.assertEqual(input_details.connection_string, source)

        # with subset string
        vl.setSubsetString('x')
        input_details = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback, False)
        self.assertEqual(input_details.connection_string, source)
        # subset of layer must be exported
        input_details = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback, True)
        self.assertNotEqual(input_details.connection_string, source)
        self.assertTrue(input_details.connection_string)
        self.assertTrue(input_details.connection_string.endswith('.gpkg'))
        self.assertTrue(os.path.exists(input_details.connection_string))
        self.assertTrue(input_details.layer_name)

        # geopackage with layer
        source = os.path.join(testDataPath, 'custom', 'circular_strings.gpkg')
        vl2 = QgsVectorLayer(source + '|layername=circular_strings')
        self.assertTrue(vl2.isValid())
        p.addMapLayer(vl2)
        input_details = alg.getOgrCompatibleSource('INPUT', {'INPUT': vl2.id()}, context, feedback, True)
        self.assertEqual(input_details.connection_string, source)
        self.assertEqual(input_details.layer_name, 'circular_strings')
        vl3 = QgsVectorLayer(source + '|layername=circular_strings_with_line')
        self.assertTrue(vl3.isValid())
        p.addMapLayer(vl3)
        input_details = alg.getOgrCompatibleSource('INPUT', {'INPUT': vl3.id()}, context, feedback, True)
        self.assertEqual(input_details.connection_string, source)
        self.assertEqual(input_details.layer_name, 'circular_strings_with_line')

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
        # check that memory layer is automatically saved out to geopackage when required by GDAL algorithms
        input_details = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback,
                                                   executing=True)
        self.assertTrue(input_details.connection_string)
        self.assertTrue(input_details.connection_string.endswith('.gpkg'))
        self.assertTrue(os.path.exists(input_details.connection_string))
        self.assertTrue(input_details.layer_name)

        # make sure that layer has only selected feature
        res = QgsVectorLayer(input_details.connection_string, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 1)

        QgsProject.instance().removeMapLayer(layer)

    def testOgrOutputLayerName(self):
        self.assertEqual(GdalUtils.ogrOutputLayerName('/home/me/out.shp'), 'out')
        self.assertEqual(GdalUtils.ogrOutputLayerName('d:/test/test_out.shp'), 'test_out')
        self.assertEqual(GdalUtils.ogrOutputLayerName('d:/test/TEST_OUT.shp'), 'TEST_OUT')
        self.assertEqual(GdalUtils.ogrOutputLayerName('d:/test/test_out.gpkg'), 'test_out')

    def testOgrLayerNameExtraction(self):
        with tempfile.TemporaryDirectory() as outdir:
            def _copyFile(dst):
                shutil.copyfile(os.path.join(testDataPath, 'custom', 'weighted.csv'), dst)

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

            # SQLite provider
            name = GdalUtils.ogrLayerName('dbname=\'/tmp/x.sqlite\' table="t" (geometry) sql=')
            self.assertEqual(name, 't')

            # PostgreSQL provider
            name = GdalUtils.ogrLayerName(
                'port=5493 sslmode=disable key=\'edge_id\' srid=0 type=LineString table="city_data"."edge" (geom) sql=')
            self.assertEqual(name, 'city_data.edge')

    def test_gdal_connection_details_from_uri(self):
        context = QgsProcessingContext()
        output_details = GdalUtils.gdal_connection_details_from_uri('d:/test/test.shp', context)
        self.assertEqual(output_details.connection_string, 'd:/test/test.shp')
        self.assertEqual(output_details.format, '"ESRI Shapefile"')
        output_details = GdalUtils.gdal_connection_details_from_uri('d:/test/test.mif', context)
        self.assertEqual(output_details.connection_string, 'd:/test/test.mif')
        self.assertEqual(output_details.format, '"MapInfo File"')

    def testConnectionString(self):
        alg = OgrToPostGis()
        alg.initAlgorithm()
        parameters = {}
        feedback = QgsProcessingFeedback()
        context = QgsProcessingContext()

        # NOTE: defaults are debatable, see
        # https://github.com/qgis/QGIS/pull/3607#issuecomment-253971020
        self.assertEqual(alg.getConnectionString(parameters, context),
                         "host=localhost port=5432 active_schema=public")

        parameters['HOST'] = 'remote'
        self.assertEqual(alg.getConnectionString(parameters, context),
                         "host=remote port=5432 active_schema=public")

        parameters['HOST'] = ''
        self.assertEqual(alg.getConnectionString(parameters, context),
                         "port=5432 active_schema=public")

        parameters['PORT'] = '5555'
        self.assertEqual(alg.getConnectionString(parameters, context),
                         "port=5555 active_schema=public")

        parameters['PORT'] = ''
        self.assertEqual(alg.getConnectionString(parameters, context),
                         "active_schema=public")

        parameters['USER'] = 'usr'
        self.assertEqual(alg.getConnectionString(parameters, context),
                         "active_schema=public user=usr")

        parameters['PASSWORD'] = 'pwd'
        self.assertEqual(alg.getConnectionString(parameters, context),
                         "password=pwd active_schema=public user=usr")

    def testCrsConversion(self):
        self.assertFalse(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem()))
        self.assertEqual(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem('EPSG:3111')), 'EPSG:3111')
        self.assertEqual(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem('POSTGIS:3111')), 'EPSG:3111')
        self.assertEqual(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem(
            'proj4: +proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')),
            'EPSG:20936')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromProj(
            '+proj=utm +zone=36 +south +a=600000 +b=70000 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs')
        self.assertTrue(crs.isValid())

        # proj 6, WKT should be used
        self.assertEqual(GdalUtils.gdal_crs_string(crs)[:40], 'BOUNDCRS[SOURCECRS[PROJCRS["unknown",BAS')

        self.assertEqual(GdalUtils.gdal_crs_string(QgsCoordinateReferenceSystem('ESRI:102003')), 'ESRI:102003')

    def testEscapeAndJoin(self):
        self.assertEqual(GdalUtils.escapeAndJoin([1, "a", "a b", "a&b", "a(b)", ";"]), '1 a "a b" "a&b" "a(b)" ";"')
        self.assertEqual(GdalUtils.escapeAndJoin([1, "-srcnodata", "--srcnodata", "-9999 9999"]), '1 -srcnodata --srcnodata "-9999 9999"')


if __name__ == '__main__':
    nose2.main()
