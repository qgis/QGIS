# -*- coding: utf-8 -*-
"""QGIS Unit tests for the @alg processing algorithm.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nathan Woodrow'
__date__ = '10.12.2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import sys
import os
import qgis  # NOQA

from qgis.testing import unittest, start_app
from qgis.processing import alg
from qgis.core import QgsSettings
from qgis.PyQt.QtCore import QCoreApplication

start_app()

ARGNAME = "TEST_ALG{0}"
HELPSTRING = "TEST_HELP STRING{0}"


def define_new_no_inputs(newid=1):
    @alg(name="noinputs", label=alg.tr("Test func"), group="unittest",
         group_label=alg.tr("Test label"))
    @alg.output(type=str, name="DISTANCE_OUT", label="Distance out")
    def testalg(instance, parameters, context, feedback, inputs):
        """
        Test doc string text
        """


def define_new_no_outputs_but_sink_instead(newid=1):
    @alg(name=ARGNAME.format(newid), label=alg.tr("Test func"), group="unittest",
         group_label=alg.tr("Test label"))
    @alg.help(HELPSTRING.format(newid))
    @alg.input(type=alg.SOURCE, name="INPUT", label="Input layer")
    @alg.input(type=alg.DISTANCE, name="DISTANCE", label="Distance", default=30)
    @alg.input(type=alg.SINK, name="SINK", label="Output layer")
    def testalg(instance, parameters, context, feedback, inputs):
        """
        Given a distance will split a line layer into segments of the distance
        """


def define_new_doc_string(newid=1):
    @alg(name=ARGNAME.format(newid), label=alg.tr("Test func"), group="unittest",
         group_label=alg.tr("Test label"))
    @alg.input(type=alg.SOURCE, name="INPUT", label="Input layer")
    @alg.output(type=str, name="DISTANCE_OUT", label="Distance out")
    def testalg(instance, parameters, context, feedback, inputs):
        """
        Test doc string text
        """


def define_new(newid=1):
    @alg(name=ARGNAME.format(newid), label=alg.tr("Test func"), group="unittest",
         group_label=alg.tr("Test label"))
    @alg.help(HELPSTRING.format(newid))
    @alg.input(type=alg.SOURCE, name="INPUT", label="Input layer")
    @alg.input(type=alg.DISTANCE, name="DISTANCE", label="Distance", default=30)
    @alg.input(type=alg.SINK, name="SINK", label="Output layer")
    @alg.output(type=str, name="DISTANCE_OUT", label="Distance out")
    def testalg(instance, parameters, context, feedback, inputs):
        """
        Given a distance will split a line layer into segments of the distance
        """


def cleanup():
    alg.instances.clear()


class AlgNoInputs(unittest.TestCase):

    def setUp(self):
        cleanup()

    def test_can_have_no_inputs(self):
        define_new_no_inputs()


class AlgNoOutputsButSinkInstead(unittest.TestCase):

    def setUp(self):
        cleanup()

    def test_can_have_no_outputs_if_there_is_destination(self):
        define_new_no_outputs_but_sink_instead()


class AlgInstanceTests(unittest.TestCase):
    """
    Tests to check the createInstance method will work as expected.
    """

    def setUp(self):
        cleanup()
        define_new()
        self.current = alg.instances.pop().createInstance()

    def test_correct_number_of_inputs_and_outputs(self):
        self.assertEqual(3, len(self.current.inputs))
        self.assertEqual(1, len(self.current.outputs))

    def test_correct_number_of_inputs_and_outputs_after_init(self):
        self.current.initAlgorithm()
        defs = self.current.parameterDefinitions()
        self.assertEqual(3, len(defs))
        inputs = [
            ("INPUT", "Input layer"),
            ("DISTANCE", "Distance"),
            ("SINK", "Output layer"),
        ]
        for count, data in enumerate(inputs):
            parmdef = defs[count]
            self.assertEqual(data[0], parmdef.name())
            self.assertEqual(data[1], parmdef.description())

    def test_func_is_set(self):
        self.assertIsNotNone(self.current._func)

    def test_has_help_from_help_decorator(self):
        self.assertEqual(HELPSTRING.format(1), self.current.shortHelpString())

    def test_name_and_label(self):
        self.assertEqual(ARGNAME.format(1), self.current.name())
        self.assertEqual("Test func", self.current.displayName())

    def test_group(self):
        self.assertEqual("Test label", self.current.group())
        self.assertEqual("unittest", self.current.groupId())


class AlgHelpTests(unittest.TestCase):

    def test_has_help_from_help_decorator(self):
        cleanup()
        define_new()
        current = alg.instances.pop()
        self.assertEqual(HELPSTRING.format(1), current.shortHelpString())

    def test_has_help_from_docstring(self):
        define_new_doc_string()
        current = alg.instances.pop()
        self.assertEqual("Test doc string text", current.shortHelpString())


class TestAlg(unittest.TestCase):

    def setUp(self):
        cleanup()
        define_new()

    def test_correct_number_of_inputs_and_outputs(self):
        current = alg.instances.pop()
        self.assertEqual(3, len(current.inputs))
        self.assertEqual(1, len(current.outputs))
        self.assertTrue(current.has_inputs)
        self.assertTrue(current.has_outputs)

    def test_correct_number_defined_in_stack_before_and_after(self):
        self.assertEqual(1, len(alg.instances))
        alg.instances.pop()
        self.assertEqual(0, len(alg.instances))

    def test_current_has_correct_name(self):
        alg.instances.pop()
        for i in range(3):
            define_new(i)

        self.assertEqual(3, len(alg.instances))
        for i in range(3, 1):
            current = alg.instances.pop()
            self.assertEqual(ARGNAME.format(i), current.name())


if __name__ == "__main__":
    unittest.main()
