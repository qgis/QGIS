# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTaskManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '26/04/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import os

from qgis.core import (
    QgsTask,
    QgsTaskManager,
    QgsApplication
)
from qgis.PyQt.QtCore import (QCoreApplication)

from qgis.testing import start_app, unittest
from time import sleep

start_app()


def run(task, result):
    if not result:
        raise Exception('cancelled')
    else:
        return result


def run_with_kwargs(task, password, result):
    if not password == 1:
        raise Exception('bad password value')
    else:
        return result


def cancellable(task):
    while not task.isCancelled():
        pass
    if task.isCancelled():
        raise Exception('cancelled')


def progress_function(task):
    task.setProgress(50)

    while not task.isCancelled():
        pass
    if task.isCancelled():
        raise Exception('cancelled')


def run_no_result(task):
    return


def finished_no_val(result):
    finished_no_val.called = True
    return


def run_single_val_result(task):
    return 5


def finished_single_value_result(result, value):
    finished_single_value_result.value = value
    return


def run_multiple_val_result(task):
    return 5, 'whoo'


def finished_multiple_value_result(result, value, statement):
    finished_multiple_value_result.value = value
    finished_multiple_value_result.statement = statement
    return


class TestQgsTaskManager(unittest.TestCase):

    def testTaskFromFunction(self):
        """ test creating task from function """

        task = QgsTask.fromFunction('test task', run, 20)
        QgsApplication.taskManager().addTask(task)
        while task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass

        self.assertEqual(task.returned_values, 20)
        self.assertFalse(task.exception)
        self.assertEqual(task.status(), QgsTask.Complete)

        # try a task which cancels itself
        bad_task = QgsTask.fromFunction('test task2', run, None)
        QgsApplication.taskManager().addTask(bad_task)
        while bad_task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass

        self.assertFalse(bad_task.returned_values)
        self.assertTrue(bad_task.exception)
        self.assertEqual(bad_task.status(), QgsTask.Terminated)

    def testTaskFromFunctionWithKwargs(self):
        """ test creating task from function using kwargs """

        task = QgsTask.fromFunction('test task3', run_with_kwargs, result=5, password=1)
        QgsApplication.taskManager().addTask(task)
        while task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass

        self.assertEqual(task.returned_values, 5)
        self.assertFalse(task.exception)
        self.assertEqual(task.status(), QgsTask.Complete)

    def testTaskFromFunctionIsCancellable(self):
        """ test that task from function can check cancelled status """
        bad_task = QgsTask.fromFunction('test task4', cancellable)
        QgsApplication.taskManager().addTask(bad_task)
        while bad_task.status() != QgsTask.Running:
            pass

        bad_task.cancel()
        while bad_task.status() == QgsTask.Running:
            pass
        while QgsApplication.taskManager().countActiveTasks() > 0:
            QCoreApplication.processEvents()

        self.assertEqual(bad_task.status(), QgsTask.Terminated)
        self.assertTrue(bad_task.exception)

    def testTaskFromFunctionCanSetProgress(self):
        """ test that task from function can set progress """
        task = QgsTask.fromFunction('test task5', progress_function)
        QgsApplication.taskManager().addTask(task)
        while task.status() != QgsTask.Running:
            pass

        #wait a fraction so that setProgress gets a chance to be called
        sleep(0.001)
        self.assertEqual(task.progress(), 50)
        self.assertFalse(task.exception)

        task.cancel()
        while task.status() == QgsTask.Running:
            pass
        while QgsApplication.taskManager().countActiveTasks() > 0:
            QCoreApplication.processEvents()

    def testTaskFromFunctionFinished(self):
        """ test that task from function can have callback finished function"""
        task = QgsTask.fromFunction('test task', run_no_result, on_finished=finished_no_val)
        QgsApplication.taskManager().addTask(task)
        while task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass
        while QgsApplication.taskManager().countActiveTasks() > 0:
            QCoreApplication.processEvents()

        # check that the finished function was called
        self.assertFalse(task.returned_values)
        self.assertFalse(task.exception)
        self.assertTrue(finished_no_val.called)

    def testTaskFromFunctionFinishedWithVal(self):
        """ test that task from function can have callback finished function and is passed result values"""
        task = QgsTask.fromFunction('test task', run_single_val_result, on_finished=finished_single_value_result)
        QgsApplication.taskManager().addTask(task)
        while task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass
        while QgsApplication.taskManager().countActiveTasks() > 0:
            QCoreApplication.processEvents()

        # check that the finished function was called
        self.assertEqual(task.returned_values, (5))
        self.assertFalse(task.exception)
        self.assertEqual(finished_single_value_result.value, 5)

    def testTaskFromFunctionFinishedWithMultipleValues(self):
        """ test that task from function can have callback finished function and is passed multiple result values"""
        task = QgsTask.fromFunction('test task', run_multiple_val_result, on_finished=finished_multiple_value_result)
        QgsApplication.taskManager().addTask(task)
        while task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass
        while QgsApplication.taskManager().countActiveTasks() > 0:
            QCoreApplication.processEvents()

        # check that the finished function was called
        self.assertEqual(task.returned_values, (5, 'whoo'))
        self.assertFalse(task.exception)
        self.assertEqual(finished_multiple_value_result.value, 5)
        self.assertEqual(finished_multiple_value_result.statement, 'whoo')

if __name__ == '__main__':
    unittest.main()
