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


def register_function(
    function,
    group,
    usesgeometry=False,
    referenced_columns=[QgsFeatureRequest.ALL_ATTRIBUTES],
    handlesnull=False,
    **kwargs,
):
    """
    Register a Python function to be used as a expression function.

    The function signature may contains special parameters:
    - context: the QgsExpressionContext-related to the current evaluation
    - feature: the QgsFeature-related to the current evaluation
    - parent: the QgsExpressionFunction parent

    If those parameters are present in the function signature, they will be automatically passed to the function,
    without the need to specify them in the expression.

    If the only other parameter in the signature is called "values", parameters will be passed as a list.
    Otherwise, parameters will be expanded in the parameter list.

    Functions should return a value compatible with QVariant
    Eval errors can be raised using parent.setEvalErrorString("Error message")
    :param function:
    :param group:
    :param usesgeometry:
    :param handlesnull: Needs to be set to True if this function does not always return NULL if any parameter is NULL. Default False.
    :return:
    """

    class QgsPyExpressionFunction(QgsExpressionFunction):
        def __init__(
            self,
            func,
            name,
            group,
            helptext="",
            usesGeometry=True,
            referencedColumns=QgsFeatureRequest.ALL_ATTRIBUTES,
            handlesNull=False,
            paramsAsList=False,
        ):
            QgsExpressionFunction.__init__(self, name, -1, group, helptext)
            self.function = func
            self.params_as_list = paramsAsList
            self.uses_geometry = usesGeometry
            self.referenced_columns = referencedColumns
            self.handles_null = handlesNull

        def func(self, values, context, parent, node):
            feature = None
            if context:
                feature = context.feature()
            try:
                parameters = inspect.signature(self.function).parameters
                kwvalues = {}

                # Handle special parameters
                # those will not be inserted in the parameter list
                # if they are present in the function signature
                if "context" in parameters:
                    kwvalues["context"] = context
                if "feature" in parameters:
                    kwvalues["feature"] = feature
                if "parent" in parameters:
                    kwvalues["parent"] = parent

                if self.params_as_list:
                    return self.function(values, **kwvalues)
                return self.function(*values, **kwvalues)
            except Exception as ex:
                tb = traceback.format_exception(None, ex, ex.__traceback__)
                formatted_traceback = "".join(tb)
                formatted_exception = f"{ex}:<pre>{formatted_traceback}</pre>"
                parent.setEvalErrorString(formatted_exception)
                return None

        def usesGeometry(self, node):
            return self.uses_geometry

        def referencedColumns(self, node):
            return self.referenced_columns

        def handlesNull(self):
            return self.handles_null

    helptemplate = string.Template("<h3>$name function</h3><br>$doc")
    name = kwargs.get("name", function.__name__)
    helptext = kwargs.get("helpText") or function.__doc__ or ""
    helptext = helptext.strip()

    register = kwargs.get("register", True)
    if register and QgsExpression.isFunctionName(name):
        if not QgsExpression.unregisterFunction(name):
            msgtitle = QCoreApplication.translate("UserExpressions", "User expressions")
            msg = QCoreApplication.translate(
                "UserExpressions", "The user expression {0} already exists and could not be unregistered."
            ).format(name)
            QgsMessageLog.logMessage(msg + "\n", msgtitle, Qgis.Warning)
            return None

    function.__name__ = name
    helptext = helptemplate.safe_substitute(name=name, doc=helptext)

    # Legacy: if args was not 'auto', parameters were passed as a list
    params_as_list = kwargs.get("params_as_list", kwargs.get("args", "auto") != "auto")
    f = QgsPyExpressionFunction(
        function, name, group, helptext, usesgeometry, referenced_columns, handlesnull, params_as_list
    )

    if register:
        QgsExpression.registerFunction(f)
    return f


def qgsfunction(args="auto", group="custom", **kwargs):
    r"""
    Decorator function used to define a user expression function.
    :param args: DEPRECATED since QGIS 3.32. Use the "params_as_list" keyword argument instead if you want to pass parameters as a list.
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
        * *params_as_list* (``bool``) \since QGIS 3.32 --
          Defines if the parameters are passed to the function as a list, or if they are expanded. Default to False.

    Examples:

    @qgsfunction(group="custom")
    def myfunc(values, feature, parent):
        return values + [feature.id()]

    This register a function called "myfunc" in the "custom" group. It can then be called with any number of parameters
    which will be passed as a list to the function. From the function, it is possible to access the feature and the parent

    >>> myfunc("a", "b", "c")
    ["a", "b", "c", 1]


    @qgsfunction(group="custom")
    def myfunc2(val1, val2, context):
        return val1 + val2

    This register a function called "myfunc2" in the "custom" group. It expects exactly two parameters, val1 and val2
    From the function, it is possible to access the feature and the parent

    >>> myfunc2(40, 2)
    42
    """

    kwargs["args"] = args

    def wrapper(func):
        return register_function(func, group, **kwargs)

    return wrapper
