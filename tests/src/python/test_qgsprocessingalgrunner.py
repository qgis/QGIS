# -*- coding: utf-8 -*-
"""QGIS Unit tests for Processing algorithm runner(s).

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2019-02'
__copyright__ = 'Copyright 2019, The QGIS Project'

import re
from qgis.PyQt.QtCore import QCoreApplication
from qgis.testing import start_app, unittest
from qgis.core import QgsProcessingAlgRunnerTask

from processing.core.Processing import Processing
from processing.core.ProcessingConfig import ProcessingConfig
from qgis.testing import start_app, unittest
from qgis.analysis import QgsNativeAlgorithms
from qgis.core import (
    QgsApplication,
    QgsSettings,
    QgsProcessingContext,
    QgsProcessingAlgRunnerTask,
    QgsProcessingAlgorithm,
    QgsProject,
    QgsProcessingFeedback,
)

start_app()


class ConsoleFeedBack(QgsProcessingFeedback):

    _error = ''

    def reportError(self, error, fatalError=False):
        self._error = error
        print(error)


class CrashingProcessingAlgorithm(QgsProcessingAlgorithm):
    """
    Wrong class in factory createInstance()
    """

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def tr(self, string):
        return QCoreApplication.translate('Processing', string)

    def createInstance(self):
        """Wrong!"""
        return ExampleProcessingAlgorithm()

    def name(self):
        return 'mycrashingscript'

    def displayName(self):
        return self.tr('My Crashing Script')

    def group(self):
        return self.tr('Example scripts')

    def groupId(self):
        return 'examplescripts'

    def shortHelpString(self):
        return self.tr("Example algorithm short description")

    def initAlgorithm(self, config=None):
        pass

    def processAlgorithm(self, parameters, context, feedback):
        return {self.OUTPUT: 'an_id'}


class TestQgsProcessingAlgRunner(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsProcessingInPlace.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsProcessingInPlace")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()

    def test_bad_script_dont_crash(self):  # spellok
        """Test regression #21270 (segfault)"""

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        task = QgsProcessingAlgRunnerTask(CrashingProcessingAlgorithm(), {}, context=context, feedback=feedback)
        self.assertTrue(task.isCanceled())
        self.assertIn('name \'ExampleProcessingAlgorithm\' is not defined', feedback._error)


if __name__ == '__main__':
    unittest.main()
