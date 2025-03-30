# The following has been generated automatically from src/core/qgspropertycollection.h
try:
    QgsAbstractPropertyCollection.__virtual_methods__ = ['writeXml', 'readXml']
    QgsAbstractPropertyCollection.__abstract_methods__ = ['propertyKeys', 'clear', 'hasProperty', 'property', 'value', 'prepare', 'referencedFields', 'isActive', 'hasActiveProperties', 'hasDynamicProperties', 'toVariant', 'loadVariant']
except (NameError, AttributeError):
    pass
try:
    QgsPropertyCollection.__virtual_methods__ = ['property']
    QgsPropertyCollection.__overridden_methods__ = ['propertyKeys', 'clear', 'hasProperty', 'value', 'prepare', 'referencedFields', 'isActive', 'hasActiveProperties', 'hasDynamicProperties', 'toVariant', 'loadVariant']
except (NameError, AttributeError):
    pass
try:
    QgsPropertyCollectionStack.__virtual_methods__ = ['clear']
    QgsPropertyCollectionStack.__overridden_methods__ = ['hasActiveProperties', 'hasDynamicProperties', 'isActive', 'property', 'value', 'referencedFields', 'prepare', 'propertyKeys', 'hasProperty', 'toVariant', 'loadVariant']
except (NameError, AttributeError):
    pass
