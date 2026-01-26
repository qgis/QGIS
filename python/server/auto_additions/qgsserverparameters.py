# The following has been generated automatically from src/server/qgsserverparameters.h
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
