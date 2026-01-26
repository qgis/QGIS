# The following has been generated automatically from src/server/qgsserverparameters.h
QgsServerParameter.UNKNOWN = QgsServerParameter.Name.UNKNOWN
QgsServerParameter.SERVICE = QgsServerParameter.Name.SERVICE
QgsServerParameter.VERSION_SERVICE = QgsServerParameter.Name.VERSION_SERVICE
QgsServerParameter.REQUEST = QgsServerParameter.Name.REQUEST
QgsServerParameter.MAP = QgsServerParameter.Name.MAP
QgsServerParameter.FILE_NAME = QgsServerParameter.Name.FILE_NAME
QgsServerParameter.Name.baseClass = QgsServerParameter
try:
    QgsServerParameterDefinition.raiseError = staticmethod(QgsServerParameterDefinition.raiseError)
    QgsServerParameterDefinition.__virtual_methods__ = ['isValid']
except (NameError, AttributeError):
    pass
try:
    QgsServerParameter.name = staticmethod(QgsServerParameter.name)
except (NameError, AttributeError):
    pass
try:
    QgsServerParameters.__virtual_methods__ = ['request', 'version', 'loadParameter']
except (NameError, AttributeError):
    pass
