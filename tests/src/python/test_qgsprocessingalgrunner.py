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
    QgsTask
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
        return ExampleProcessingAlgorithm()  # noqa

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

    def test_flags(self):
        """
        Test task flags
        """
        thread_safe_alg = QgsApplication.processingRegistry().algorithmById('native:buffer')
        nonthread_safe_alg = QgsApplication.processingRegistry().algorithmById('native:setprojectvariable')
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        task = QgsProcessingAlgRunnerTask(thread_safe_alg, {}, context=context, feedback=feedback)
        self.assertEqual(task.flags(), QgsTask.CanCancel)
        task = QgsProcessingAlgRunnerTask(thread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.Flags())
        self.assertEqual(task.flags(), QgsTask.Flags())
        task = QgsProcessingAlgRunnerTask(thread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.CanCancel)
        self.assertEqual(task.flags(), QgsTask.CanCancel)
        task = QgsProcessingAlgRunnerTask(thread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.CancelWithoutPrompt)
        self.assertEqual(task.flags(), QgsTask.CancelWithoutPrompt)
        task = QgsProcessingAlgRunnerTask(thread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.CancelWithoutPrompt | QgsTask.CanCancel)
        self.assertEqual(task.flags(), QgsTask.CancelWithoutPrompt | QgsTask.CanCancel)

        # alg which can't be canceled
        task = QgsProcessingAlgRunnerTask(nonthread_safe_alg, {}, context=context, feedback=feedback)
        self.assertEqual(task.flags(), QgsTask.Flags())
        # we clear the CanCancel flag automatically, since the algorithm itself cannot be canceled
        task = QgsProcessingAlgRunnerTask(nonthread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.CanCancel)
        self.assertEqual(task.flags(), QgsTask.Flags())

        # hidden task
        task = QgsProcessingAlgRunnerTask(thread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.Hidden)
        self.assertEqual(task.flags(), QgsTask.Hidden)
        task = QgsProcessingAlgRunnerTask(thread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.Hidden | QgsTask.CanCancel)
        self.assertEqual(task.flags(), QgsTask.Hidden | QgsTask.CanCancel)
        task = QgsProcessingAlgRunnerTask(thread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.Hidden | QgsTask.CanCancel | QgsTask.CancelWithoutPrompt)
        self.assertEqual(task.flags(), QgsTask.Hidden | QgsTask.CanCancel | QgsTask.CancelWithoutPrompt)

        task = QgsProcessingAlgRunnerTask(nonthread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.Hidden)
        self.assertEqual(task.flags(), QgsTask.Hidden)
        task = QgsProcessingAlgRunnerTask(nonthread_safe_alg, {}, context=context, feedback=feedback, flags=QgsTask.Hidden | QgsTask.CanCancel)
        self.assertEqual(task.flags(), QgsTask.Hidden)

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
