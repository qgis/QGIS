# -*- coding: utf-8 -*-

"""
***************************************************************************
    OtbAlgorithmsTest.py
    ---------------------
    Date                 : January 2019
    Copyright            : (C) 2019 by CNES
    Author               : otb att cnes dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Rashad Kanavath'
__date__ = 'Janauary 2019'
__copyright__ = '(C) 2019, CNES'

import os
import sys
import unittest
import hashlib
import shutil
import nose2
import tempfile
from qgis.core import (QgsProcessingParameterNumber,
                       QgsApplication,
                       QgsCoordinateReferenceSystem,
                       QgsRasterLayer,
                       QgsMapLayer,
                       QgsProject,
                       QgsProcessingContext,
                       QgsProcessingUtils,
                       QgsProcessingFeedback,
                       QgsProcessingParameterDefinition,
                       QgsProcessingModelAlgorithm)
from qgis.testing import start_app, unittest
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.gui.wrappers import WidgetWrapperFactory
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from otbprovider.OtbAlgorithm import OtbAlgorithm
from otbprovider.OtbAlgorithmProvider import OtbAlgorithmProvider
from otbprovider.OtbUtils import OtbUtils
from otbprovider.OtbChoiceWidget import OtbParameterChoice, OtbChoiceWidgetWrapper
import AlgorithmsTestBase

import processing

OTB_INSTALL_DIR = os.environ.get('OTB_INSTALL_DIR')


class TestOtbAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @staticmethod
    def __input_raster_layer():
        options = QgsRasterLayer.LayerOptions()
        options.loadDefaultStyle = False
        return QgsRasterLayer(os.path.join(AlgorithmsTestBase.processingTestDataPath(), 'raster.tif'),
                              "raster_input",
                              'gdal',
                              options)

    def test_bug21373_mode_vector(self):
        """
        This issue is reported on qgis bug tracker: #21373
        This issue is reported on qgis-otb-plugin tracker: #30
        """
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = QgsProcessingFeedback()
        parameters = {
            'in': TestOtbAlgorithms.__input_raster_layer(),
            'filter': 'meanshift',
            'mode.vector.out': 'vector.shp'
        }
        alg = OtbAlgorithm('Segmentation', 'Segmentation', os.path.join(self.descrFolder, 'Segmentation.txt'))
        results = alg.processAlgorithm(parameters, context, feedback)
        self.assertDictEqual(results, {'mode.vector.out': 'vector.shp'})

    def test_bug21373_mode_raster(self):
        """
        This issue is reported on qgis bug tracker: #21373
        """
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = QgsProcessingFeedback()
        parameters = {
            'in': TestOtbAlgorithms.__input_raster_layer(),
            'filter': 'meanshift',
            'mode': 'raster',
            'mode.raster.out': 'raster.tif'
        }
        alg = OtbAlgorithm('Segmentation', 'Segmentation', os.path.join(self.descrFolder, 'Segmentation.txt'))
        results = alg.processAlgorithm(parameters, context, feedback)
        self.assertDictEqual(results, {'mode.raster.out': 'raster.tif'})

    def test_bug21374_Fail(self):
        """
        This issue is reported on qgis bug tracker: #21374
        """
        outdir = tempfile.mkdtemp()
        self.cleanup_paths.append(outdir)
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = QgsProcessingFeedback()
        parameters = {
            'in': TestOtbAlgorithms.__input_raster_layer(),
            'filter': 'cc',
            'mode.vector.out': os.path.join(outdir, 'vector.shp')
        }

        alg = OtbAlgorithm('Segmentation', 'Segmentation', os.path.join(self.descrFolder, 'Segmentation.txt'))
        ok, msg = alg.checkParameterValues(parameters, context)
        self.assertFalse(ok, 'Algorithm failed checkParameterValues with result {}'.format(msg))

    def test_init_algorithms(self):
        """
        This test will read each otb algorithm in 'algs.txt'
        and creates an instance of OtbAlgorithm and check if it can be executed
        This is done in :class: `OtbAlgorithmProvider` load() method
        """
        algs_txt = os.path.join(self.descrFolder, 'algs.txt')
        with open(algs_txt) as lines:
            line = lines.readline().strip('\n').strip()
            if line != '' and line.startswith('#'):
                version = line[1:]
                print('version =', version)
                line = lines.readline().strip('\n').strip()
            while line != '' and not line.startswith('#'):
                data = line.split('|')
                descriptionFile = os.path.join(self.descrFolder, str(data[1]) + '.txt')
                alg = OtbAlgorithm(data[0], data[1], descriptionFile)
                self.assertIsInstance(alg, OtbAlgorithm)
                ret, msg = alg.canExecute()
                print("canExecute '{}' - {}".format(alg.id(), ret))
                self.assertEqual(ret, True)
                line = lines.readline().strip('\n').strip()

    def test_parameterAs_ScriptMode(self):
        """
        This test will pass an instance of QgsCoordinateReferenceSystem for 'epsg' parameter
        of otb::Rasterization. There is same test in otb_algorithm_tests.yaml which passes
        an instance of str for epsg parameter.
        """
        outdir = tempfile.mkdtemp()
        self.cleanup_paths.append(outdir)

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = QgsProcessingFeedback()

        vectorFile = os.path.join(AlgorithmsTestBase.processingTestDataPath(), 'polys.gml')
        vectorLayer = QgsProcessingUtils.mapLayerFromString(vectorFile, context)
        parameters = {
            'in': vectorLayer,
            'epsg': QgsCoordinateReferenceSystem('EPSG:4326'),
            'spx': 1.0,
            'spy': 1.0,
            'outputpixeltype': 1,
            'out': os.path.join(outdir, 'raster.tif')
        }
        results = processing.run('otb:Rasterization', parameters, None, feedback)
        result_lyr = QgsProcessingUtils.mapLayerFromString(results['out'], context)
        self.assertTrue(result_lyr.isValid())

    def test_OTBParameterChoiceExists(self):
        """
        This test is here to know if we have change `type()` method of :class: `OtbParameterChoice`
        That value is used by Otb when it creates descriptor files. So changes to this string must be test
        in a unit-test.
        """
        alg_smoothing = OtbAlgorithm('Image Filtering', 'Smoothing', os.path.join(self.descrFolder, 'Smoothing.txt'))
        found = False
        for param in alg_smoothing.parameterDefinitions():
            # print (param.name(), param.type())
            if param.type() == 'OTBParameterChoice':
                found = True
                break
        self.assertEqual(found, True)

    def test_OTBParameterChoice_Gui(self):
        """
        This test is similar to GuiTests in processing that is done on other parameter widget in processing
        Main difference is this test uses create_wrapper_from_metadata() rather than create_wrapper_from_class()
        like rest of processing widgets.
        """
        param = OtbParameterChoice('test')

        alg = QgsApplication.processingRegistry().createAlgorithmById('otb:Smoothing')
        # algorithm dialog
        dlg = AlgorithmDialog(alg)
        wrapper = WidgetWrapperFactory.create_wrapper_from_metadata(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, OtbChoiceWidgetWrapper)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)

        alg = QgsApplication.processingRegistry().createAlgorithmById('otb:Smoothing')
        # batch dialog
        dlg = BatchAlgorithmDialog(alg)
        wrapper = WidgetWrapperFactory.create_wrapper_from_metadata(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, OtbChoiceWidgetWrapper)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)

        alg = QgsApplication.processingRegistry().createAlgorithmById('otb:Smoothing')
        # modeler dialog
        model = QgsProcessingModelAlgorithm()
        dlg = ModelerParametersDialog(alg, model)
        wrapper = WidgetWrapperFactory.create_wrapper_from_metadata(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, OtbChoiceWidgetWrapper)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)

    @classmethod
    def setUpClass(cls):
        start_app()
        cls.provider = OtbAlgorithmProvider()
        QgsApplication.processingRegistry().addProvider(cls.provider)
        ProcessingConfig.setSettingValue(OtbUtils.FOLDER, OTB_INSTALL_DIR)
        ProcessingConfig.setSettingValue(OtbUtils.APP_FOLDER, os.path.join(OTB_INSTALL_DIR, 'lib', 'otb', 'applications'))
        ProcessingConfig.readSettings()
        # Refresh OTB Algorithms after settings are changed.
        for p in QgsApplication.processingRegistry().providers():
            if p.id() == "otb":
                p.refreshAlgorithms()
        cls.descrFolder = os.path.join(OTB_INSTALL_DIR, 'share', 'otb', 'description')
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        QgsApplication.processingRegistry().removeProvider(cls.provider)
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        """
        return name of yaml file containing test definitions
        """
        print("OTB_INSTALL_DIR = '{}'".format(OTB_INSTALL_DIR))
        return 'otb_algorithm_tests.yaml'


if __name__ == '__main__':
    nose2.main()
