# The following has been generated automatically from src/core/project/qgsprojectstoredobjectmanagermodel.h
# monkey patching scoped based enum
QgsProjectStoredObjectManagerModelBase.CustomRole.Object.__doc__ = "Object"
QgsProjectStoredObjectManagerModelBase.CustomRole.__doc__ = """Custom model roles.

* ``Object``: Object

"""
# --
QgsProjectStoredObjectManagerModelBase.CustomRole.baseClass = QgsProjectStoredObjectManagerModelBase
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
