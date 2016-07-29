# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsActionManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '28/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis # switch sip api

from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsActionManager,
                       QgsAction,
                       QgsExpressionContext,
                       QgsField,
                       QgsFields
                       )
from qgis.PyQt.QtCore import QDir, QTemporaryFile

from qgis.testing import (start_app,
                          unittest
                          )
import os
import time
import platform

start_app()


class TestQgsActionManager(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        self.layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=flddate:datetime",
                                    "test_layer", "memory")
        self.manager = QgsActionManager(self.layer)

        # make a little script to aid in recording action outputs
        # this is just a little python file which writes out it's arguments to a text file
        self.run_script_file = os.path.join(QDir.tempPath(), 'run_action.py')
        with open(self.run_script_file, 'w') as s:
            s.write('import sys\n')
            s.write('open(sys.argv[1], "w").write(" ".join(sys.argv[2:]))\n')

    def get_temp_filename(self):
        tmpFile = QTemporaryFile()
        tmpFile.open()  # fileName is no available until open
        tmpName = tmpFile.fileName()
        tmpFile.close()
        tmpFile.setAutoRemove(False)
        return tmpName

    def create_action(self, dest_file, text_to_write):
        """ returns an action which writes some output to a file """
        return 'python {} {} {}'.format(self.run_script_file, dest_file, text_to_write)

    def testLayer(self):
        self.assertEqual(self.manager.layer(), self.layer)

    def testAddAction(self):
        """ Test adding actions """

        # should be empty to start with
        self.assertEqual(self.manager.size(), 0)
        self.assertEqual(self.manager.listActions(), [])

        # add an action
        self.manager.addAction(QgsAction.GenericPython, 'test_action', 'i=1')
        self.assertEqual(self.manager.size(), 1)
        self.assertEqual(self.manager.listActions()[0].type(), QgsAction.GenericPython)
        self.assertEqual(self.manager.listActions()[0].name(), 'test_action')
        self.assertEqual(self.manager.listActions()[0].action(), 'i=1')
        self.assertEqual(self.manager.at(0).name(), 'test_action')
        self.assertEqual(self.manager[0].name(), 'test_action')

        # add another action
        self.manager.addAction(QgsAction.Windows, 'test_action2', 'i=2')
        self.assertEqual(self.manager.size(), 2)
        self.assertEqual(self.manager.listActions()[1].type(), QgsAction.Windows)
        self.assertEqual(self.manager.listActions()[1].name(), 'test_action2')
        self.assertEqual(self.manager.listActions()[1].action(), 'i=2')
        self.assertEqual(self.manager.at(1).name(), 'test_action2')
        self.assertEqual(self.manager[1].name(), 'test_action2')

        # add a predefined action
        action = QgsAction(QgsAction.Unix, 'test_action3', 'i=3', False)
        self.manager.addAction(action)
        self.assertEqual(self.manager.size(), 3)
        self.assertEqual(self.manager.listActions()[2].type(), QgsAction.Unix)
        self.assertEqual(self.manager.listActions()[2].name(), 'test_action3')
        self.assertEqual(self.manager.listActions()[2].action(), 'i=3')
        self.assertEqual(self.manager.at(2).name(), 'test_action3')
        self.assertEqual(self.manager[2].name(), 'test_action3')

    def testRemoveActions(self):
        """ test removing actions """

        # add an action
        self.manager.addAction(QgsAction.GenericPython, 'test_action', 'i=1')

        # clear the manager and check that it's empty
        self.manager.clearActions()
        self.assertEqual(self.manager.size(), 0)
        self.assertEqual(self.manager.listActions(), [])

        # add some actions
        self.manager.addAction(QgsAction.GenericPython, 'test_action', 'i=1')
        self.manager.addAction(QgsAction.GenericPython, 'test_action2', 'i=2')
        self.manager.addAction(QgsAction.GenericPython, 'test_action3', 'i=3')

        # remove non-existant action
        self.manager.removeAction(5)

        # remove them one by one
        self.manager.removeAction(1)
        self.assertEqual(self.manager.size(), 2)
        self.assertEqual(self.manager.listActions()[0].name(), 'test_action')
        self.assertEqual(self.manager.listActions()[1].name(), 'test_action3')
        self.manager.removeAction(0)
        self.assertEqual(self.manager.size(), 1)
        self.assertEqual(self.manager.listActions()[0].name(), 'test_action3')
        self.manager.removeAction(0)
        self.assertEqual(self.manager.size(), 0)

    def testRetrieveAction(self):
        """ test retrieving actions """
        self.manager.clearActions()

        # test that exceptions are thrown when retrieving bad indices

        with self.assertRaises(KeyError):
            self.manager[0]

        with self.assertRaises(KeyError):
            self.manager.at(0)

        self.manager.addAction(QgsAction.GenericPython, 'test_action', 'i=1')

        with self.assertRaises(KeyError):
            self.manager[-1]

        with self.assertRaises(KeyError):
            self.manager.at(-1)

        with self.assertRaises(KeyError):
            self.manager[5]

        with self.assertRaises(KeyError):
            self.manager.at(5)

    def testDefaultAction(self):
        """ test default action for layer"""

        self.manager.clearActions()
        self.manager.addAction(QgsAction.GenericPython, 'test_action', 'i=1')
        self.manager.addAction(QgsAction.GenericPython, 'test_action2', 'i=2')

        # initially should be not set
        self.assertEqual(self.manager.defaultAction(), -1)

        # set bad default action
        self.manager.setDefaultAction(10)
        self.assertEqual(self.manager.defaultAction(), -1)

        # set good default action
        self.manager.setDefaultAction(1)
        self.assertEqual(self.manager.defaultAction(), 1)

        # if default action is removed, should be reset to -1
        self.manager.clearActions()
        self.assertEqual(self.manager.defaultAction(), -1)

    def check_action_result(self, temp_file):
        with open(temp_file, 'r') as result:
            output = result.read()
        return output

    @unittest.expectedFailure(platform.system() != 'Linux')
    def testDoAction(self):
        """ test running action """

        self.manager.clearActions()

        # simple action
        temp_file = self.get_temp_filename()
        self.manager.addAction(QgsAction.Unix, 'test_action', self.create_action(temp_file, 'test output'))

        fields = QgsFields()
        fields.append(QgsField('my_field'))
        fields.append(QgsField('my_other_field'))

        f = QgsFeature(fields, 1)
        f.setAttributes([5, 'val'])

        c = QgsExpressionContext()
        self.manager.doAction(0, f, c)
        time.sleep(0.05)

        self.assertEqual(self.check_action_result(temp_file), 'test output')

        # action with substitutions
        temp_file = self.get_temp_filename()
        self.manager.addAction(QgsAction.Unix, 'test_action', self.create_action(temp_file, 'test [% $id %] output [% @layer_name %]'))
        self.manager.doAction(1, f, c)
        time.sleep(0.05)

        self.assertEqual(self.check_action_result(temp_file), 'test 1 output test_layer')

        # test doAction using field variant
        temp_file = self.get_temp_filename()
        self.manager.addAction(QgsAction.Unix, 'test_action', self.create_action(temp_file, 'test [% @current_field %]'))
        self.manager.doActionFeature(2, f, 0)
        time.sleep(0.05)
        self.assertEqual(self.check_action_result(temp_file), 'test 5')
        self.manager.doActionFeature(2, f, 1)
        time.sleep(0.05)
        self.assertEqual(self.check_action_result(temp_file), 'test val')

if __name__ == '__main__':
    unittest.main()
