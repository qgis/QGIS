# The following has been generated automatically from src/server/qgsserverquerystringparameter.h
# monkey patching scoped based enum
QgsServerQueryStringParameter.Type.String.__doc__ = "Parameter is a string"
QgsServerQueryStringParameter.Type.Integer.__doc__ = "Parameter is an integer"
QgsServerQueryStringParameter.Type.Double.__doc__ = "Parameter is a double"
QgsServerQueryStringParameter.Type.Boolean.__doc__ = "Parameter is a boolean"
QgsServerQueryStringParameter.Type.List.__doc__ = "Parameter is a (comma separated) list of strings, the handler will perform any further required conversion of the list values"
QgsServerQueryStringParameter.Type.__doc__ = """The Type enum represents the parameter type

* ``String``: Parameter is a string
* ``Integer``: Parameter is an integer
* ``Double``: Parameter is a double
* ``Boolean``: Parameter is a boolean
* ``List``: Parameter is a (comma separated) list of strings, the handler will perform any further required conversion of the list values

"""
# --
QgsServerQueryStringParameter.Type.baseClass = QgsServerQueryStringParameter
try:
    QgsServerQueryStringParameter.typeName = staticmethod(QgsServerQueryStringParameter.typeName)
except (NameError, AttributeError):
    pass
