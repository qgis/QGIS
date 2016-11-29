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
QgsGeometry.__nonzero__ = _geometryNonZero
QgsGeometry.__bool__ = _geometryNonZero


def register_function(function, arg_count, group, usesgeometry=False, referenced_columns=[QgsFeatureRequest.AllAttributes], **kwargs):
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
    class QgsExpressionFunction(QgsExpression.Function):

        def __init__(self, func, name, args, group, helptext='', usesGeometry=True, referencedColumns=QgsFeatureRequest.AllAttributes, expandargs=False):
            QgsExpression.Function.__init__(self, name, args, group, helptext)
            self.function = func
            self.expandargs = expandargs
            self.uses_geometry = usesGeometry
            self.referenced_columns = referencedColumns

        def func(self, values, context, parent):
            feature = None
            if context:
                feature = context.feature()
            try:
                if self.expandargs:
                    values.append(feature)
                    values.append(parent)
                    return self.function(*values)
                else:
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
    helptext = function.__doc__ or ''
    helptext = helptext.strip()
    expandargs = False

    if arg_count == "auto":
        # Work out the number of args we need.
        # Number of function args - 2.  The last two args are always feature, parent.
        args = inspect.getargspec(function).args
        number = len(args)
        arg_count = number - 2
        expandargs = True

    register = kwargs.get('register', True)
    if register and QgsExpression.isFunctionName(name):
        if not QgsExpression.unregisterFunction(name):
            msgtitle = QCoreApplication.translate("UserExpressions", "User expressions")
            msg = QCoreApplication.translate("UserExpressions", "The user expression {0} already exists and could not be unregistered.").format(name)
            QgsMessageLog.logMessage(msg + "\n", msgtitle, QgsMessageLog.WARNING)
            return None

    function.__name__ = name
    helptext = helptemplate.safe_substitute(name=name, doc=helptext)
    f = QgsExpressionFunction(function, name, arg_count, group, helptext, usesgeometry, referenced_columns, expandargs)

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


def copy_func(f):
    """Based on http://stackoverflow.com/a/6528148/190597 (Glenn Maynard)"""
    g = types.FunctionType(f.__code__, f.__globals__, name=f.__name__,
                           argdefs=f.__defaults__,
                           closure=f.__closure__)
    g = functools.update_wrapper(g, f)
    g.__kwdefaults__ = f.__kwdefaults__
    return g


class QgsTaskWrapper(QgsTask):

    def __init__(self, description, function, on_finished, *args, **kwargs):
        QgsTask.__init__(self, description)
        self.args = args
        self.kwargs = kwargs
        self.function = copy_func(function)
        self.on_finished = copy_func(on_finished)
        self.returned_values = None
        self.exception = None

    def _run(self):
        try:
            self.returned_values = self.function(self, *self.args, **self.kwargs)
        except Exception as ex:
            # report error
            self.exception = ex
            return QgsTask.ResultFail

        return QgsTask.ResultSuccess

    def finished(self, result):
        if not self.on_finished:
            return

        try:
            if self.returned_values:
                # we want to support singular returned values which are not iterable
                if hasattr(self.returned_values, '__iter__'):
                    self.on_finished(result, *self.returned_values)
                else:
                    self.on_finished(result, self.returned_values)
            else:
                self.on_finished(result)
        except Exception as ex:
            self.exception = ex


def fromFunction(cls, description, function, *args, on_finished=None, **kwargs):
    return QgsTaskWrapper(description, function, on_finished, *args, **kwargs)

QgsTask.fromFunction = classmethod(fromFunction)
