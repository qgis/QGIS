import inspect
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

        def __init__(self, func, name, args, group, helptext='', usesgeometry=False, expandargs=False):
            QgsExpression.Function.__init__(self, name, args, group, helptext, usesgeometry)
            self.function = func
            self.expandargs = expandargs

        def func(self, values, feature, parent):
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
            raise TypeError("Unable to unregister function")

    function.__name__ = name
    helptext = helptemplate.safe_substitute(name=name, doc=helptext)
    f = QgsExpressionFunction(function, name, arg_count, group, helptext, usesgeometry, expandargs)

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

try:
    # Add a __nonzero__ method onto QPyNullVariant so we can check for null values easier.
    #   >>> value = QPyNullVariant("int")
    #   >>> if value:
    #   >>>	  print "Not a null value"
    from types import MethodType
    from PyQt4.QtCore import QPyNullVariant

    def __nonzero__(self):
        return False

    def __repr__(self):
        return 'NULL'

    def __eq__(self, other):
        return isinstance(other, QPyNullVariant) or other is None

    def __ne__(self, other):
        return not isinstance(other, QPyNullVariant) and other is not None

    def __hash__(self):
        return 2178309

    QPyNullVariant.__nonzero__ = MethodType(__nonzero__, None, QPyNullVariant)
    QPyNullVariant.__repr__ = MethodType(__repr__, None, QPyNullVariant)
    QPyNullVariant.__eq__ = MethodType(__eq__, None, QPyNullVariant)
    QPyNullVariant.__ne__ = MethodType(__ne__, None, QPyNullVariant)
    QPyNullVariant.__hash__ = MethodType(__hash__, None, QPyNullVariant)

    NULL = QPyNullVariant(int)

except ImportError:
    pass


class QgsEditError(Exception):

    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)

# Define a `with edit(layer)` statement


class edit:

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
