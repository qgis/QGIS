"""QGIS Unit tests for Processing algorithm runner(s).

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alessandro Pasotti"
__date__ = "2019-02"
__copyright__ = "Copyright 2019, The QGIS Project"

from processing.core.Processing import Processing
from qgis.PyQt.QtCore import QCoreApplication
from qgis.analysis import QgsNativeAlgorithms
from qgis.core import (
    QgsApplication,
    QgsProcessingAlgorithm,
    QgsProcessingAlgRunnerTask,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsProject,
    QgsSettings,
    QgsTask,
    QgsProcessingException,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class ConsoleFeedBack(QgsProcessingFeedback):

    _error = ""

    def reportError(self, error, fatalError=False):
        self._error = error
        print(error)


class CrashingProcessingAlgorithm(QgsProcessingAlgorithm):
    """
    Wrong class in factory createInstance()
    """

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def tr(self, string):
        return QCoreApplication.translate("Processing", string)

    def createInstance(self):
        """Wrong!"""
        return ExampleProcessingAlgorithm()  # noqa

    def name(self):
        return "mycrashingscript"

    def displayName(self):
        return self.tr("My Crashing Script")

    def group(self):
        return self.tr("Example scripts")

    def groupId(self):
        return "examplescripts"

    def shortHelpString(self):
        return self.tr("Example algorithm short description")

    def initAlgorithm(self, config=None):
        pass

    def processAlgorithm(self, parameters, context, feedback):
        return {self.OUTPUT: "an_id"}


class TestAlgorithm(QgsProcessingAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def createInstance(self):
        return TestAlgorithm()

    def name(self):
        return "test"

    def displayName(self):
        return "test"

    def group(self):
        return "test"

    def groupId(self):
        return "test"

    def shortHelpString(self):
        return "test"

    def initAlgorithm(self, config=None):
        pass

    def processAlgorithm(self, parameters, context, feedback):
        context.temporaryLayerStore().addMapLayer(
            QgsVectorLayer("Point?crs=epsg:3111", "v1", "memory")
        )
        return {self.OUTPUT: "an_id"}


class ExceptionAlgorithm(QgsProcessingAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def createInstance(self):
        return ExceptionAlgorithm()

    def name(self):
        return "test"

    def displayName(self):
        return "test"

    def group(self):
        return "test"

    def groupId(self):
        return "test"

    def shortHelpString(self):
        return "test"

    def initAlgorithm(self, config=None):
        pass

    def processAlgorithm(self, parameters, context, feedback):
        context.temporaryLayerStore().addMapLayer(
            QgsVectorLayer("Point?crs=epsg:3111", "v1", "memory")
        )
        raise QgsProcessingException("error")


class TestQgsProcessingAlgRunner(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsProcessingInPlace.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsProcessingInPlace")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()

    def test_flags(self):
        """
        Test task flags
        """
        thread_safe_alg = QgsApplication.processingRegistry().algorithmById(
            "native:buffer"
        )
        nonthread_safe_alg = QgsApplication.processingRegistry().algorithmById(
            "native:setprojectvariable"
        )
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        task = QgsProcessingAlgRunnerTask(
            thread_safe_alg, {}, context=context, feedback=feedback
        )
        self.assertEqual(task.flags(), QgsTask.Flag.CanCancel)
        task = QgsProcessingAlgRunnerTask(
            thread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flags(),
        )
        self.assertEqual(task.flags(), QgsTask.Flags())
        task = QgsProcessingAlgRunnerTask(
            thread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.CanCancel,
        )
        self.assertEqual(task.flags(), QgsTask.Flag.CanCancel)
        task = QgsProcessingAlgRunnerTask(
            thread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.CancelWithoutPrompt,
        )
        self.assertEqual(task.flags(), QgsTask.Flag.CancelWithoutPrompt)
        task = QgsProcessingAlgRunnerTask(
            thread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.CancelWithoutPrompt | QgsTask.Flag.CanCancel,
        )
        self.assertEqual(
            task.flags(), QgsTask.Flag.CancelWithoutPrompt | QgsTask.Flag.CanCancel
        )

        # alg which can't be canceled
        task = QgsProcessingAlgRunnerTask(
            nonthread_safe_alg, {}, context=context, feedback=feedback
        )
        self.assertEqual(task.flags(), QgsTask.Flags())
        # we clear the CanCancel flag automatically, since the algorithm itself cannot be canceled
        task = QgsProcessingAlgRunnerTask(
            nonthread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.CanCancel,
        )
        self.assertEqual(task.flags(), QgsTask.Flags())

        # hidden task
        task = QgsProcessingAlgRunnerTask(
            thread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.Hidden,
        )
        self.assertEqual(task.flags(), QgsTask.Flag.Hidden)
        task = QgsProcessingAlgRunnerTask(
            thread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.Hidden | QgsTask.Flag.CanCancel,
        )
        self.assertEqual(task.flags(), QgsTask.Flag.Hidden | QgsTask.Flag.CanCancel)
        task = QgsProcessingAlgRunnerTask(
            thread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.Hidden
            | QgsTask.Flag.CanCancel
            | QgsTask.Flag.CancelWithoutPrompt,
        )
        self.assertEqual(
            task.flags(),
            QgsTask.Flag.Hidden
            | QgsTask.Flag.CanCancel
            | QgsTask.Flag.CancelWithoutPrompt,
        )

        task = QgsProcessingAlgRunnerTask(
            nonthread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.Hidden,
        )
        self.assertEqual(task.flags(), QgsTask.Flag.Hidden)
        task = QgsProcessingAlgRunnerTask(
            nonthread_safe_alg,
            {},
            context=context,
            feedback=feedback,
            flags=QgsTask.Flag.Hidden | QgsTask.Flag.CanCancel,
        )
        self.assertEqual(task.flags(), QgsTask.Flag.Hidden)

    def test_bad_script_dont_crash(self):  # spellok
        """Test regression #21270 (segfault)"""

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        task = QgsProcessingAlgRunnerTask(
            CrashingProcessingAlgorithm(), {}, context=context, feedback=feedback
        )
        self.assertTrue(task.isCanceled())
        self.assertIn(
            "name 'ExampleProcessingAlgorithm' is not defined", feedback._error
        )

    def test_good(self):
        """
        Test a good algorithm
        """

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        task = QgsProcessingAlgRunnerTask(
            TestAlgorithm(), {}, context=context, feedback=feedback
        )
        self.assertFalse(task.isCanceled())
        TestQgsProcessingAlgRunner.finished = False
        TestQgsProcessingAlgRunner.success = None

        def on_executed(success, results):
            TestQgsProcessingAlgRunner.finished = True
            TestQgsProcessingAlgRunner.success = success

        task.executed.connect(on_executed)
        QgsApplication.taskManager().addTask(task)
        task.waitForFinished()

        self.assertTrue(TestQgsProcessingAlgRunner.finished)
        self.assertTrue(TestQgsProcessingAlgRunner.success)
        self.assertEqual(context.temporaryLayerStore().count(), 1)

    def test_raises_exception(self):
        """
        Test an algorithm which raises an exception
        """

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        task = QgsProcessingAlgRunnerTask(
            ExceptionAlgorithm(), {}, context=context, feedback=feedback
        )
        self.assertFalse(task.isCanceled())
        TestQgsProcessingAlgRunner.finished = False
        TestQgsProcessingAlgRunner.success = None

        def on_executed(success, results):
            TestQgsProcessingAlgRunner.finished = True
            TestQgsProcessingAlgRunner.success = success

        task.executed.connect(on_executed)
        QgsApplication.taskManager().addTask(task)
        task.waitForFinished()

        self.assertTrue(TestQgsProcessingAlgRunner.finished)
        self.assertFalse(TestQgsProcessingAlgRunner.success)
        # layer added by algorithm should have been transferred to the context, even when an
        # exception was raised
        self.assertEqual(context.temporaryLayerStore().count(), 1)


if __name__ == "__main__":
    unittest.main()
