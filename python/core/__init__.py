import string
from qgis._core import *


def register_function(function, arg_count, group, usesgeometry=False, **kwargs):
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
        def __init__(self, func, name, args, group, helptext='', usesgeometry=False):
            QgsExpression.Function.__init__(self, name, args, group, helptext, usesgeometry)
            self.function = func

        def func(self, values, feature, parent):
            try:
                return self.function(values, feature, parent)
            except Exception as ex:
                parent.setEvalErrorString(ex.message)

    helptemplate = string.Template("""<h3>$name function</h3><br>$doc""")
    name = kwargs.get('name', function.__name__)
    helptext = function.__doc__ or ''
    helptext = helptext.strip()
    if arg_count == 0 and not name[0] == '$':
        name = '${0}'.format(name)

    register = kwargs.get('register', True)
    if register and QgsExpression.isFunctionName(name):
        if not QgsExpression.unregisterFunction(name):
            raise TypeError("Unable to unregister function")

    function.__name__ = name
    helptext = helptemplate.safe_substitute(name=name, doc=helptext)
    f = QgsExpressionFunction(function, name, arg_count, group, helptext, usesgeometry)

    # This doesn't really make any sense here but does when used from a decorator context
    # so it can stay.
    if register:
        QgsExpression.registerFunction(f)
    return f


def qgsfunction(args, group, **kwargs):
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
        usesgeometry = kwargs.get('usesgeometry', False)
        return register_function(func, args, group, usesgeometry, **kwargs)
    return wrapper
