# The following has been generated automatically from src/core/providers/sensorthings/qgssensorthingsutils.h
try:
    QgsSensorThingsUtils.ServiceCapabilities.__attribute_docs__ = {'version': 'SensorThings API version', 'availableExtensions': 'Available SensorThings extensions', 'availableEntities': 'Available SensorThings entities'}
    QgsSensorThingsUtils.ServiceCapabilities.__annotations__ = {'version': 'Qgis.SensorThingsVersion', 'availableExtensions': 'Qgis.SensorThingsExtensions', 'availableEntities': 'Set[Qgis.SensorThingsEntity]'}
    QgsSensorThingsUtils.ServiceCapabilities.__doc__ = """SensorThings service capabilities"""
    QgsSensorThingsUtils.ServiceCapabilities.__group__ = ['providers', 'sensorthings']
except (NameError, AttributeError):
    pass
try:
    QgsSensorThingsUtils.stringToEntity = staticmethod(QgsSensorThingsUtils.stringToEntity)
    QgsSensorThingsUtils.displayString = staticmethod(QgsSensorThingsUtils.displayString)
    QgsSensorThingsUtils.entitySetStringToEntity = staticmethod(QgsSensorThingsUtils.entitySetStringToEntity)
    QgsSensorThingsUtils.entityToSetString = staticmethod(QgsSensorThingsUtils.entityToSetString)
    QgsSensorThingsUtils.propertiesForEntityType = staticmethod(QgsSensorThingsUtils.propertiesForEntityType)
    QgsSensorThingsUtils.fieldsForEntityType = staticmethod(QgsSensorThingsUtils.fieldsForEntityType)
    QgsSensorThingsUtils.fieldsForExpandedEntityType = staticmethod(QgsSensorThingsUtils.fieldsForExpandedEntityType)
    QgsSensorThingsUtils.geometryFieldForEntityType = staticmethod(QgsSensorThingsUtils.geometryFieldForEntityType)
    QgsSensorThingsUtils.entityTypeHasGeometry = staticmethod(QgsSensorThingsUtils.entityTypeHasGeometry)
    QgsSensorThingsUtils.geometryTypeForEntity = staticmethod(QgsSensorThingsUtils.geometryTypeForEntity)
    QgsSensorThingsUtils.filterForWkbType = staticmethod(QgsSensorThingsUtils.filterForWkbType)
    QgsSensorThingsUtils.filterForExtent = staticmethod(QgsSensorThingsUtils.filterForExtent)
    QgsSensorThingsUtils.combineFilters = staticmethod(QgsSensorThingsUtils.combineFilters)
    QgsSensorThingsUtils.determineServiceCapabilities = staticmethod(QgsSensorThingsUtils.determineServiceCapabilities)
    QgsSensorThingsUtils.availableGeometryTypes = staticmethod(QgsSensorThingsUtils.availableGeometryTypes)
    QgsSensorThingsUtils.expandableTargets = staticmethod(QgsSensorThingsUtils.expandableTargets)
    QgsSensorThingsUtils.relationshipCardinality = staticmethod(QgsSensorThingsUtils.relationshipCardinality)
    QgsSensorThingsUtils.asQueryString = staticmethod(QgsSensorThingsUtils.asQueryString)
    QgsSensorThingsUtils.__group__ = ['providers', 'sensorthings']
except (NameError, AttributeError):
    pass
try:
    QgsSensorThingsExpansionDefinition.defaultDefinitionForEntity = staticmethod(QgsSensorThingsExpansionDefinition.defaultDefinitionForEntity)
    QgsSensorThingsExpansionDefinition.fromString = staticmethod(QgsSensorThingsExpansionDefinition.fromString)
    QgsSensorThingsExpansionDefinition.__group__ = ['providers', 'sensorthings']
except (NameError, AttributeError):
    pass
