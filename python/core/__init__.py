# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2014 by Nathan Woodrow
    Email                : woodrow dot nathan at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str
from builtins import object

__author__ = 'Nathan Woodrow'
__date__ = 'May 2014'
__copyright__ = '(C) 2014, Nathan Woodrow'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QCoreApplication, NULL

import inspect
import string
import types
import functools
from qgis._core import *


# Boolean evaluation of QgsGeometry


def _geometryNonZero(self):
    return not self.isEmpty()


def _isValid(self):
    return self.isValid()


QgsGeometry.__nonzero__ = _geometryNonZero
QgsGeometry.__bool__ = _geometryNonZero

QgsDefaultValue.__bool__ = _isValid


def register_function(function, arg_count, group, usesgeometry=False,
                      referenced_columns=[QgsFeatureRequest.ALL_ATTRIBUTES], **kwargs):
    """
    Register a Python function to be used as a expression function.

    Functions should take (values, feature, parent) as args:

    Example:
        def myfunc(values, feature, parent):
            pass

    They can also shortcut naming feature and parent args by using *args
    if they are not needed in the function.

    Example:
        def myfunc(values, *args):
            pass

    Functions should return a value compatible with QVariant

    Eval errors can be raised using parent.setEvalErrorString("Error message")

    :param function:
    :param arg_count:
    :param group:
    :param usesgeometry:
    :return:
    """

    class QgsPyExpressionFunction(QgsExpressionFunction):

        def __init__(self, func, name, args, group, helptext='', usesGeometry=True,
                     referencedColumns=QgsFeatureRequest.ALL_ATTRIBUTES, expandargs=False):
            QgsExpressionFunction.__init__(self, name, args, group, helptext)
            self.function = func
            self.expandargs = expandargs
            self.uses_geometry = usesGeometry
            self.referenced_columns = referencedColumns

        def func(self, values, context, parent, node):
            feature = None
            if context:
                feature = context.feature()

            try:
                if self.expandargs:
                    values.append(feature)
                    values.append(parent)
                    if inspect.getargspec(self.function).args[-1] == 'context':
                        values.append(context)
                    return self.function(*values)
                else:
                    if inspect.getargspec(self.function).args[-1] == 'context':
                        self.function(values, feature, parent, context)
                    return self.function(values, feature, parent)
            except Exception as ex:
                parent.setEvalErrorString(str(ex))
                return None

        def usesGeometry(self, node):
            return self.uses_geometry

        def referencedColumns(self, node):
            return self.referenced_columns

    helptemplate = string.Template("""<h3>$name function</h3><br>$doc""")
    name = kwargs.get('name', function.__name__)
    helptext = kwargs.get('helpText') or function.__doc__ or ''
    helptext = helptext.strip()
    expandargs = False

    if arg_count == "auto":
        # Work out the number of args we need.
        # Number of function args - 2.  The last two args are always feature, parent.
        args = inspect.getargspec(function).args
        number = len(args)
        arg_count = number - 2
        if args[-1] == 'context':
            arg_count -= 1
        expandargs = True

    register = kwargs.get('register', True)
    if register and QgsExpression.isFunctionName(name):
        if not QgsExpression.unregisterFunction(name):
            msgtitle = QCoreApplication.translate("UserExpressions", "User expressions")
            msg = QCoreApplication.translate("UserExpressions",
                                             "The user expression {0} already exists and could not be unregistered.").format(
                name)
            QgsMessageLog.logMessage(msg + "\n", msgtitle, QgsMessageLog.WARNING)
            return None

    function.__name__ = name
    helptext = helptemplate.safe_substitute(name=name, doc=helptext)
    f = QgsPyExpressionFunction(function, name, arg_count, group, helptext, usesgeometry, referenced_columns,
                                expandargs)

    # This doesn't really make any sense here but does when used from a decorator context
    # so it can stay.
    if register:
        QgsExpression.registerFunction(f)
    return f


def qgsfunction(args='auto', group='custom', **kwargs):
    """
    Decorator function used to define a user expression function.

    Example:
      @qgsfunction(2, 'test'):
      def add(values, feature, parent):
        pass

    Will create and register a function in QgsExpression called 'add' in the
    'test' group that takes two arguments.

    or not using feature and parent:

    Example:
      @qgsfunction(2, 'test'):
      def add(values, *args):
        pass
    """

    def wrapper(func):
        return register_function(func, args, group, **kwargs)

    return wrapper


class QgsEditError(Exception):

    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


# Define a `with edit(layer)` statement


class edit(object):

    def __init__(self, layer):
        self.layer = layer

    def __enter__(self):
        assert self.layer.startEditing()
        return self.layer

    def __exit__(self, ex_type, ex_value, traceback):
        if ex_type is None:
            if not self.layer.commitChanges():
                raise QgsEditError(self.layer.commitErrors())
            return True
        else:
            self.layer.rollBack()
            return False


class QgsTaskWrapper(QgsTask):

    def __init__(self, description, flags, function, on_finished, *args, **kwargs):
        QgsTask.__init__(self, description, flags)
        self.args = args
        self.kwargs = kwargs
        self.function = function
        self.on_finished = on_finished
        self.returned_values = None
        self.exception = None

    def run(self):
        try:
            self.returned_values = self.function(self, *self.args, **self.kwargs)
        except Exception as ex:
            # report error
            self.exception = ex
            return False

        return True

    def finished(self, result):
        if not self.on_finished:
            return

        if not result and self.exception is None:
            self.exception = Exception('Task canceled')

        try:
            if self.returned_values:
                self.on_finished(self.exception, self.returned_values)
            else:
                self.on_finished(self.exception)
        except Exception as ex:
            self.exception = ex


@staticmethod
def fromFunction(description, function, *args, on_finished=None, flags=QgsTask.AllFlags, **kwargs):
    """
    Creates a new QgsTask task from a python function.

    Example:

    def calculate(task):
        # pretend this is some complex maths and stuff we want
        # to run in the background
        return 5*6

    def calculation_finished(exception, value=None):
        if not exception:
            iface.messageBar().pushMessage(
                'the magic number is {}'.format(value))
        else:
            iface.messageBar().pushMessage(
                str(exception))

    task = QgsTask.fromFunction('my task', calculate,
            on_finished=calculation_finished)
    QgsApplication.taskManager().addTask(task)

    """

    assert function
    return QgsTaskWrapper(description, flags, function, on_finished, *args, **kwargs)


QgsTask.fromFunction = fromFunction


# add some __repr__ methods to processing classes
def processing_source_repr(self):
    return "<QgsProcessingFeatureSourceDefinition {{'source':{}, 'selectedFeaturesOnly': {}}}>".format(
        self.source.staticValue(), self.selectedFeaturesOnly)


QgsProcessingFeatureSourceDefinition.__repr__ = processing_source_repr


def processing_output_layer_repr(self):
    return "<QgsProcessingOutputLayerDefinition {{'sink':{}, 'createOptions': {}}}>".format(self.sink.staticValue(),
                                                                                            self.createOptions)


QgsProcessingOutputLayerDefinition.__repr__ = processing_output_layer_repr
