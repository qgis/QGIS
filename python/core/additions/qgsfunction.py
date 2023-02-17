# -*- coding: utf-8 -*-

"""
***************************************************************************
    qgsfunction.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Denis Rouzaud
    Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


import inspect
import string
import traceback
from builtins import str
from qgis.PyQt.QtCore import QCoreApplication
from qgis._core import QgsExpressionFunction, QgsExpression, QgsMessageLog, QgsFeatureRequest, Qgis


def register_function(function, arg_count, group, usesgeometry=False,
                      referenced_columns=[QgsFeatureRequest.ALL_ATTRIBUTES], handlesnull=False, **kwargs):
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
    :param handlesnull: Needs to be set to True if this function does not always return NULL if any parameter is NULL. Default False.
    :return:
    """

    class QgsPyExpressionFunction(QgsExpressionFunction):

        def __init__(self, func, name, args, group, helptext='', usesGeometry=True,
                     referencedColumns=QgsFeatureRequest.ALL_ATTRIBUTES, expandargs=False, handlesNull=False):
            QgsExpressionFunction.__init__(self, name, args, group, helptext)
            self.function = func
            self.expandargs = expandargs
            self.uses_geometry = usesGeometry
            self.referenced_columns = referencedColumns
            self.handles_null = handlesNull

        def func(self, values, context, parent, node):
            feature = None
            if context:
                feature = context.feature()

            try:
                if self.expandargs:
                    values.append(feature)
                    values.append(parent)
                    if inspect.getfullargspec(self.function).args[-1] == 'context':
                        values.append(context)
                    return self.function(*values)
                else:
                    if inspect.getfullargspec(self.function).args[-1] == 'context':
                        self.function(values, feature, parent, context)
                    return self.function(values, feature, parent)
            except Exception as ex:
                tb = traceback.format_exception(None, ex, ex.__traceback__)
                formatted_traceback = ''.join(tb)
                formatted_exception = f"{ex}:<pre>{formatted_traceback}</pre>"
                parent.setEvalErrorString(formatted_exception)
                return None

        def usesGeometry(self, node):
            return self.uses_geometry

        def referencedColumns(self, node):
            return self.referenced_columns

        def handlesNull(self):
            return self.handles_null

    helptemplate = string.Template("""<h3>$name function</h3><br>$doc""")
    name = kwargs.get('name', function.__name__)
    helptext = kwargs.get('helpText') or function.__doc__ or ''
    helptext = helptext.strip()
    expandargs = False

    if arg_count == "auto":
        # Work out the number of args we need.
        # Number of function args - 2.  The last two args are always feature, parent.
        args = inspect.getfullargspec(function).args
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
            QgsMessageLog.logMessage(msg + "\n", msgtitle, Qgis.Warning)
            return None

    function.__name__ = name
    helptext = helptemplate.safe_substitute(name=name, doc=helptext)
    f = QgsPyExpressionFunction(function, name, arg_count, group, helptext, usesgeometry, referenced_columns,
                                expandargs, handlesnull)

    # This doesn't really make any sense here but does when used from a decorator context
    # so it can stay.
    if register:
        QgsExpression.registerFunction(f)
    return f


def qgsfunction(args='auto', group='custom', **kwargs):
    r"""
    Decorator function used to define a user expression function.

    :param args: Number of parameters, set to 'auto' to accept a variable length of parameters.
    :param group: The expression group to which this expression should be added.
    :param \**kwargs:
        See below

    :Keyword Arguments:
        * *referenced_columns* (``list``) --
          An array of field names on which this expression works. Can be set to ``[QgsFeatureRequest.ALL_ATTRIBUTES]``. By default empty.
        * *usesgeometry* (``bool``) --
          Defines if this expression requires the geometry. By default False.
        * *handlesnull* (``bool``) --
          Defines if this expression has custom handling for NULL values. If False, the result will always be NULL as soon as any parameter is NULL. False by default.

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
