# The following has been generated automatically from src/core/project/qgsprojectstorage.h
try:
    QgsProjectStorage.Metadata.__attribute_docs__ = {'name': "Name of the project - equivalent to a file's base name (i.e. without path and extension).", 'lastModified': 'Date and local time when the file was last modified.'}
    QgsProjectStorage.Metadata.__annotations__ = {'name': str, 'lastModified': 'QDateTime'}
    QgsProjectStorage.Metadata.__group__ = ['project']
except (NameError, AttributeError):
    pass
try:
    QgsProjectStorage.__virtual_methods__ = ['isSupportedUri', 'renameProject', 'readProjectStorageMetadata', 'filePath', 'visibleName', 'showLoadGui', 'showSaveGui']
    QgsProjectStorage.__abstract_methods__ = ['type', 'listProjects', 'readProject', 'writeProject', 'removeProject']
    QgsProjectStorage.__group__ = ['project']
except (NameError, AttributeError):
    pass
