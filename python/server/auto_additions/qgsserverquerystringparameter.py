# The following has been generated automatically from src/server/qgsserverquerystringparameter.h
# monkey patching scoped based enum
QgsServerQueryStringParameter.Type.String.__doc__ = ""
QgsServerQueryStringParameter.Type.Integer.__doc__ = ""
QgsServerQueryStringParameter.Type.Double.__doc__ = ""
QgsServerQueryStringParameter.Type.Boolean.__doc__ = ""
QgsServerQueryStringParameter.Type.List.__doc__ = ""
QgsServerQueryStringParameter.Type.__doc__ = """The Type enum represents the parameter type

* ``String``: 
* ``Integer``: 
* ``Double``: 
* ``Boolean``: 
* ``List``: 

"""
# --
QgsServerQueryStringParameter.Type.baseClass = QgsServerQueryStringParameter
QgsServerQueryStringParameter.typeName = staticmethod(QgsServerQueryStringParameter.typeName)
