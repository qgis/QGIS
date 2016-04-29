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
    QgsTaskManager
)

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


class TestQgsTaskManager(unittest.TestCase):

    def testTaskFromFunction(self):
        """ test creating task from function """

        task = QgsTask.fromFunction('test task', run, 20)
        QgsTaskManager.instance().addTask(task)
        while task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass

        self.assertEqual(task.result, 20)
        self.assertEqual(task.status(), QgsTask.Complete)

        # try a task which cancels itself
        bad_task = QgsTask.fromFunction('test task', run)
        QgsTaskManager.instance().addTask(bad_task)
        while bad_task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass

        self.assertFalse(bad_task.result)
        self.assertEqual(bad_task.status(), QgsTask.Terminated)

    def testTaskFromFunctionWithKwargs(self):
        """ test creating task from function using kwargs """

        task = QgsTask.fromFunction('test task', run_with_kwargs, result=5, password=1)
        QgsTaskManager.instance().addTask(task)
        while task.status() not in [QgsTask.Complete, QgsTask.Terminated]:
            pass

        self.assertEqual(task.result, 5)
        self.assertEqual(task.status(), QgsTask.Complete)

    def testTaskFromFunctionIsCancellable(self):
        """ test that task from function can check cancelled status """
        bad_task = QgsTask.fromFunction('test task', cancellable)
        QgsTaskManager.instance().addTask(bad_task)
        while bad_task.status() != QgsTask.Running:
            pass

        bad_task.cancel()
        while bad_task.status() == QgsTask.Running:
            pass

        self.assertEqual(bad_task.status(), QgsTask.Terminated)

    def testTaskFromFunctionCanSetProgress(self):
        """ test that task from function can set progress """
        task = QgsTask.fromFunction('test task', progress_function)
        QgsTaskManager.instance().addTask(task)
        while task.status() != QgsTask.Running:
            pass

        #wait a fraction so that setProgress gets a chance to be called
        sleep(0.001)
        self.assertEqual(task.progress(), 50)

        task.cancel()
        while task.status() == QgsTask.Running:
            pass


if __name__ == '__main__':
    unittest.main()
