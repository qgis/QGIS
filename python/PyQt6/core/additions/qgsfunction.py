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

from qgis.PyQt.QtCore import QCoreApplication
from qgis._core import (
    QgsExpressionFunction,
    QgsExpression,
    QgsMessageLog,
    QgsFeatureRequest,
    Qgis,
)


class QgsPyExpressionFunction(QgsExpressionFunction):
    """Python expression function"""

    def __init__(
        self,
        function,
        name,
        group,
        helptext="",
        usesgeometry=False,
        referenced_columns=QgsFeatureRequest.ALL_ATTRIBUTES,
        handlesnull=False,
        params_as_list=False,
    ):
        # Call the parent constructor
        # -1 means that function can take any number of arguments
        QgsExpressionFunction.__init__(self, name, -1, group, helptext)
        self.function = function
        self.params_as_list = params_as_list
        self.usesgeometry = usesgeometry
        self.referenced_columns = referenced_columns
        self.handlesnull = handlesnull

    def func(self, values, context, parent, node):
        feature = None
        if context:
            feature = context.feature()
        try:
            # Inspect the inner function signature to get the list of parameters
            parameters = inspect.signature(self.function).parameters
            kwvalues = {}

            # Handle special parameters
            # those will not be inserted in the arguments list
            # if they are present in the function signature
            if "context" in parameters:
                kwvalues["context"] = context
            if "feature" in parameters:
                kwvalues["feature"] = feature
            if "parent" in parameters:
                kwvalues["parent"] = parent

            # In this context, values is a list of the parameters passed to the expression.
            # If self.params_as_list is True, values is passed as is to the inner function.
            if self.params_as_list:
                return self.function(values, **kwvalues)
            # Otherwise (default), the parameters are expanded
            return self.function(*values, **kwvalues)

        except Exception as ex:
            tb = traceback.format_exception(None, ex, ex.__traceback__)
            formatted_traceback = "".join(tb)
            formatted_exception = f"{ex}:<pre>{formatted_traceback}</pre>"
            parent.setEvalErrorString(formatted_exception)
            return None

    def usesGeometry(self, node):
        return self.usesgeometry

    def referencedColumns(self, node):
        return self.referenced_columns

    def handlesNull(self):
        return self.handlesnull


def register_function(
    function,
    args="auto",
    group="custom",
    usesgeometry=False,
    referenced_columns=[QgsFeatureRequest.ALL_ATTRIBUTES],
    handlesnull=False,
    params_as_list=None,
    **kwargs,
):
    """
    Register a Python function to be used as a expression function.

    The function signature may contain special parameters (in any order at the end of the signature):

    - feature: the QgsFeature related to the current evaluation
    - parent: the QgsExpressionFunction parent
    - context: the QgsExpressionContext related to the current evaluation

    If those parameters are present in the function signature, they will be automatically passed to the function,
    without the need to specify them in the expression.

    Functions should return a value compatible with QVariant
    Eval errors can be raised using parent.setEvalErrorString("Error message")

    :param function: the Python function to be used as an expression function
    :param args: DEPRECATED since QGIS 3.32.  Use ``params_as_list`` if you want to pass parameters as a list.
    :param group: the expression group in which the function should be added
    :param usesgeometry: Defines if this expression requires the geometry. By default False.
    :param referenced_columns: An array of names of fields on which this expression works. By default ``[QgsFeatureRequest.ALL_ATTRIBUTES]``. Specifying a subset of fields or an empty list will result in a faster execution.
    :param handlesnull: Defines if this expression has custom handling for NULL values. If False, the result will always be NULL as soon as any parameter is NULL. False by default.
    :param params_as_list: If True, the function will receive the expression parameters as a list. If False, the function will receive the parameters as individual arguments. False by default.

    :Keyword Arguments:

    * *register* (``bool``): Set to False to create the QgsPyExpressionFunction without registering it. Useful for testing puposes. By default True.
    * *name* (``str``): If provided, replace the function name
    * *helpText* (``str``): If provided, used in the help tooltip instead of the function docstring

    :return:
    """

    # Format the help text
    helptemplate = string.Template("<h3>$name function</h3><br>$doc")
    name = kwargs.get("name", function.__name__)
    helptext = kwargs.get("helpText") or function.__doc__ or ""
    helptext = helptext.strip()

    register = kwargs.get("register", True)
    if register and QgsExpression.isFunctionName(name):
        if not QgsExpression.unregisterFunction(name):
            msgtitle = QCoreApplication.translate("UserExpressions", "User expressions")
            msg = QCoreApplication.translate(
                "UserExpressions",
                "The user expression {0} already exists and could not be unregistered.",
            ).format(name)
            QgsMessageLog.logMessage(msg + "\n", msgtitle, Qgis.MessageLevel.Warning)
            return None

    function.__name__ = name
    helptext = helptemplate.safe_substitute(name=name, doc=helptext)

    # Legacy: if args was not 'auto', parameters were passed as a list
    params_as_list = params_as_list or args != "auto"
    f = QgsPyExpressionFunction(
        function,
        name,
        group,
        helptext,
        usesgeometry,
        referenced_columns,
        handlesnull,
        params_as_list,
    )

    if register:
        QgsExpression.registerFunction(f)
    return f


def qgsfunction(args="auto", group="custom", **kwargs):
    r"""
    Decorator function used to define a user expression function.

    The decorated function signature may contain special parameters (in any order at the end of the signature):

    - feature: the QgsFeature related to the current evaluation
    - parent: the QgsExpressionFunction parent
    - context: the QgsExpressionContext related to the current evaluation

    If those parameters are present in the function signature, they will be automatically passed to the function,
    without the need to specify them in the expression.

    Functions should return a value compatible with QVariant
    Eval errors can be raised using parent.setEvalErrorString("Error message")

    :param args: DEPRECATED since QGIS 3.32. Use the "params_as_list" keyword argument if you want to pass parameters as a list.
    :param group: The expression group to which this expression should be added.
    :param \**kwargs:
        See below
    :Keyword Arguments:
        * *usesgeometry* (``bool``) --
          Defines if this expression requires the geometry. By default False.
        * *referenced_columns* (``list``) --
          An array of names of fields on which this expression works. By default ``[QgsFeatureRequest.ALL_ATTRIBUTES]``. Specifying a subset of fields or an empty list will result in a faster execution.
        * *handlesnull* (``bool``) --
          Defines if this expression has custom handling for NULL values. If False, the result will always be NULL as soon as any parameter is NULL. False by default.
        * *params_as_list* (``bool``) \since QGIS 3.32 --
          Defines if the parameters are passed to the function as a list. If set the False, they will be expanded. By default False.
        * *register* (``bool``) --
            Set to False to create the QgsPyExpressionFunction without registering it. Useful for testing puposes. By default True.
        * *name* (``str``) --
            If provided, replace the function name
        * *helpText* (``str``) --
            If provided, used in the help tooltip instead of the function docstring


    Example 1 (Basic function, with default parameters)
    ---------------------------------------------------

    .. code-block:: python

        @qgsfunction(group="python")
        def center(text, width, fillchar=" "):
            return text.center(width, fillchar)

    This registers a function called "center" in the "python" group.
    This expression requires two parameters: text and width.
    An optional third parameter can be provided: fillchar.

    .. code-block:: text

        >>> center('Hello', 10)
        '  Hello   '
        >>> center('Hello', 15, '*')
        '*****Hello*****'


    Example 2 (variable number of parameters)
    -----------------------------------------

    .. code-block:: python

        @qgsfunction(group="custom")
        def fibonnaci_numbers(*args):
            def fibonnaci(n):
                if n <= 1:
                    return n
                else:
                    return fibonnaci(n - 1) + fibonnaci(n - 2)
            return [fibonnaci(arg) for arg in args]

    This registers a function called "fibonnaci_numbers" in the "custom" group.
    This expression can be called with any number of parameters, and will return a list of the fibonnaci numbers
    corresponding to the parameters.

    .. code-block:: text

        >>> fibonnaci_numbers()
        []
        >>> fibonnaci_numbers(1)
        [1]
        >>> fibonnaci_numbers(3)
        [2]
        >>> fibonnaci_numbers(1, 2, 3, 4, 5, 6, 7)
        [ 1, 1, 2, 3, 5, 8, 13 ]


    Example 3 (feature and parent)
    ------------------------------

    .. code-block:: python

        @qgsfunction(group="custom")
        def display_field(fieldname, feature, parent):
            try:
                return f"<b>{fieldname}</b>: {feature[fieldname]}"
            except KeyError:
                parent.setEvalErrorString(f"Field {fieldname} not found")

    This registers a function called "display_field" in the "custom" group.
    This expression requires an unique parameter: fieldname. Feature is automatically passed to the function.
    parent is the QgsExpressionFunction parent, and can be used to raise an error.

    .. code-block:: text

        >>> display_field("altitude")
        <b>altitude</b>: 164
        >>> display_field("lat")
        <b>lat</b>: 45.4

    Example 4 (context)
    -------------------

    .. code-block:: python

        @qgsfunction(group="custom")
        def title_layer_name(context):
            return context.variable("layer_name").title()

    This registers a function called "title_layer_name" in the "custom" group. It takes no parameters,
    but extracts the layer name from the expression context and returns it as a title case string.

    """

    def wrapper(func):
        return register_function(func, args, group, **kwargs)

    return wrapper
