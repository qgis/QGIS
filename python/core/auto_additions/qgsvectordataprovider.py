# The following has been generated automatically from src/core/vector/qgsvectordataprovider.h
try:
    QgsVectorDataProvider.__attribute_docs__ = {'EditingCapabilities': "Bitmask of all provider's editing capabilities", 'raiseError': 'Signals an error in this provider\n'}
    QgsVectorDataProvider.__annotations__ = {'EditingCapabilities': int}
    QgsVectorDataProvider.availableEncodings = staticmethod(QgsVectorDataProvider.availableEncodings)
    QgsVectorDataProvider.convertValue = staticmethod(QgsVectorDataProvider.convertValue)
    QgsVectorDataProvider.__virtual_methods__ = ['storageType', 'empty', 'isSqlQuery', 'vectorLayerTypeFlags', 'uniqueStringsMatching', 'aggregate', 'enumValues', 'deleteFeatures', 'truncate', 'cancelReload', 'addAttributes', 'deleteAttributes', 'renameAttributes', 'changeAttributeValues', 'changeFeatures', 'defaultValue', 'defaultValueClause', 'skipConstraintCheck', 'changeGeometryValues', 'createSpatialIndex', 'createAttributeIndex', 'capabilities', 'attributeEditCapabilities', 'setEncoding', 'attributeIndexes', 'pkAttributeIndexes', 'geometryColumnName', 'doesStrictFeatureTypeCheck', 'createRenderer', 'createLabeling', 'transaction', 'forceReload', 'dependencies', 'discoverRelations', 'metadata', 'translateMetadataKey', 'translateMetadataValue', 'hasMetadata', 'handlePostCloneOperations']
    QgsVectorDataProvider.__abstract_methods__ = ['featureSource', 'getFeatures', 'wkbType', 'featureCount', 'fields']
    QgsVectorDataProvider.__overridden_methods__ = ['getFeatures', 'wkbType', 'featureCount', 'hasFeatures', 'fields', 'sourceCrs', 'sourceExtent', 'sourceExtent3D', 'sourceName', 'dataComment', 'minimumValue', 'maximumValue', 'addFeatures', 'lastError', 'temporalCapabilities', 'elevationProperties']
    QgsVectorDataProvider.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorDataProvider.NativeType.__group__ = ['vector']
except (NameError, AttributeError):
    pass
