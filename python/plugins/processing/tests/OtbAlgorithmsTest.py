# -*- coding: utf-8 -*-

"""
***************************************************************************
    OtbAlgorithmsTests.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import os
import sys
import unittest
import hashlib
import shutil
import nose2

from qgis.core import (QgsProcessingParameterNumber,
                       QgsApplication,
                       QgsProcessingParameterDefinition)
from qgis.testing import start_app, unittest
#from processing.algs.otb.OtbChoiceWidget import OtbParameterChoice
from processing.algs.otb.OtbAlgorithm import OtbAlgorithm
from processing.algs.otb.OtbAlgorithmProvider import OtbAlgorithmProvider
from processing.algs.otb.OtbSettings import OtbSettings
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.tools import dataobjects
import AlgorithmsTestBase

OTB_INSTALL_DIR = os.environ.get('OTB_INSTALL_DIR')


class TestOtbAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    def test_init_algorithms(self):
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

    def test_choice_parameter_smoothing(self):
        alg_smoothing = OtbAlgorithm('Image Filtering', 'Smoothing', os.path.join(self.descrFolder, 'Smoothing.txt'))
        found = False
        for param in alg_smoothing.parameterDefinitions():
            ## print (param.name(), param.type())
            if param.type() == 'OTBParameterChoice':
                found = True
                break
        self.assertEqual(found, True)

    @classmethod
    def setUpClass(cls):
        start_app()
        cls.descrFolder = os.path.join(OTB_INSTALL_DIR, 'share', 'otb', 'description')
        from processing.core.Processing import Processing
        Processing.initialize()
        ProcessingConfig.setSettingValue("OTB_ACTIVATE", True)
        ProcessingConfig.setSettingValue(OtbSettings.FOLDER, OTB_INSTALL_DIR)
        ProcessingConfig.setSettingValue(OtbSettings.APP_FOLDER, os.path.join(OTB_INSTALL_DIR, 'lib', 'otb', 'applications'))
        ProcessingConfig.readSettings()
        #refresh OTB Algorithms after settings are changed.
        for p in QgsApplication.processingRegistry().providers():
            if p.id() == "otb":
                p.refreshAlgorithms()

        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        from processing.core.Processing import Processing
        Processing.deinitialize()
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        print("OTB_INSTALL_DIR = '{}'".format(OTB_INSTALL_DIR))
        return 'otb_algorithm_tests.yaml'


if __name__ == '__main__':
    nose2.main()
