# The following has been generated automatically from src/core/project/qgsprojectstoredobjectmanagermodel.h
# monkey patching scoped based enum
QgsProjectStoredObjectManagerModelBase.CustomRole.Object.__doc__ = "Object"
QgsProjectStoredObjectManagerModelBase.CustomRole.__doc__ = """Custom model roles.

* ``Object``: Object

"""
# --
QgsProjectStoredObjectManagerModelBase.CustomRole.baseClass = QgsProjectStoredObjectManagerModelBase
try:
    QgsProjectStoredObjectManagerProxyModelBase.__virtual_methods__ = ['filterAcceptsRowInternal']
    QgsProjectStoredObjectManagerProxyModelBase.__overridden_methods__ = ['lessThan', 'filterAcceptsRow']
    QgsProjectStoredObjectManagerProxyModelBase.__group__ = ['project']
except (NameError, AttributeError):
    pass
try:
    QgsProjectStoredObjectManagerModelBase.__overridden_methods__ = ['rowCount', 'data', 'setData', 'flags']
    QgsProjectStoredObjectManagerModelBase.__group__ = ['project']
except (NameError, AttributeError):
    pass
try:
    QgsProjectStoredObjectManagerModel.__overridden_methods__ = ['rowCountInternal', 'dataInternal', 'setDataInternal', 'flagsInternal', 'objectAboutToBeAddedInternal', 'objectAboutToBeRemovedInternal', 'objectAddedInternal', 'objectRemovedInternal']
    QgsProjectStoredObjectManagerModel.__group__ = ['project']
except (NameError, AttributeError):
    pass
try:
    QgsProjectStoredObjectManagerProxyModel.__overridden_methods__ = ['filterAcceptsRowInternal']
    QgsProjectStoredObjectManagerProxyModel.__group__ = ['project']
except (NameError, AttributeError):
    pass
