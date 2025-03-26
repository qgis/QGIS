# The following has been generated automatically from src/core/project/qgsprojectproperty.h
try:
    QgsProjectProperty.__virtual_methods__ = ['writeXml']
    QgsProjectProperty.__abstract_methods__ = ['dump', 'isKey', 'isValue', 'isLeaf', 'readXml', 'writeXml', 'value']
    QgsProjectProperty.__group__ = ['project']
except (NameError, AttributeError):
    pass
try:
    QgsProjectPropertyKey.__virtual_methods__ = ['clear', 'clearKeys']
    QgsProjectPropertyKey.__overridden_methods__ = ['value', 'dump', 'readXml', 'writeXml', 'isKey', 'isValue', 'isLeaf']
    QgsProjectPropertyKey.__group__ = ['project']
except (NameError, AttributeError):
    pass
try:
    QgsProjectPropertyValue.__overridden_methods__ = ['isKey', 'isValue', 'value', 'isLeaf', 'dump', 'readXml', 'writeXml']
    QgsProjectPropertyValue.__group__ = ['project']
except (NameError, AttributeError):
    pass
