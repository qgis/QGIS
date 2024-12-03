# The following has been generated automatically from src/core/vector/qgsvectorlayerutils.h
QgsVectorLayerUtils.IgnoreAuxiliaryLayers = QgsVectorLayerUtils.CascadedFeatureFlag.IgnoreAuxiliaryLayers
QgsVectorLayerUtils.CascadedFeatureFlags = lambda flags=0: QgsVectorLayerUtils.CascadedFeatureFlag(flags)
try:
    QgsVectorLayerUtils.getValuesIterator = staticmethod(QgsVectorLayerUtils.getValuesIterator)
    QgsVectorLayerUtils.getValues = staticmethod(QgsVectorLayerUtils.getValues)
    QgsVectorLayerUtils.getDoubleValues = staticmethod(QgsVectorLayerUtils.getDoubleValues)
    QgsVectorLayerUtils.valueExists = staticmethod(QgsVectorLayerUtils.valueExists)
    QgsVectorLayerUtils.createUniqueValue = staticmethod(QgsVectorLayerUtils.createUniqueValue)
    QgsVectorLayerUtils.createUniqueValueFromCache = staticmethod(QgsVectorLayerUtils.createUniqueValueFromCache)
    QgsVectorLayerUtils.attributeHasConstraints = staticmethod(QgsVectorLayerUtils.attributeHasConstraints)
    QgsVectorLayerUtils.validateAttribute = staticmethod(QgsVectorLayerUtils.validateAttribute)
    QgsVectorLayerUtils.createFeature = staticmethod(QgsVectorLayerUtils.createFeature)
    QgsVectorLayerUtils.createFeatures = staticmethod(QgsVectorLayerUtils.createFeatures)
    QgsVectorLayerUtils.duplicateFeature = staticmethod(QgsVectorLayerUtils.duplicateFeature)
    QgsVectorLayerUtils.matchAttributesToFields = staticmethod(QgsVectorLayerUtils.matchAttributesToFields)
    QgsVectorLayerUtils.makeFeatureCompatible = staticmethod(QgsVectorLayerUtils.makeFeatureCompatible)
    QgsVectorLayerUtils.makeFeaturesCompatible = staticmethod(QgsVectorLayerUtils.makeFeaturesCompatible)
    QgsVectorLayerUtils.fieldIsEditable = staticmethod(QgsVectorLayerUtils.fieldIsEditable)
    QgsVectorLayerUtils.fieldIsReadOnly = staticmethod(QgsVectorLayerUtils.fieldIsReadOnly)
    QgsVectorLayerUtils.fieldEditabilityDependsOnFeature = staticmethod(QgsVectorLayerUtils.fieldEditabilityDependsOnFeature)
    QgsVectorLayerUtils.getFeatureDisplayString = staticmethod(QgsVectorLayerUtils.getFeatureDisplayString)
    QgsVectorLayerUtils.impactsCascadeFeatures = staticmethod(QgsVectorLayerUtils.impactsCascadeFeatures)
    QgsVectorLayerUtils.guessFriendlyIdentifierField = staticmethod(QgsVectorLayerUtils.guessFriendlyIdentifierField)
    QgsVectorLayerUtils.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUtils.QgsDuplicateFeatureContext.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerUtils.QgsFeatureData.__group__ = ['vector']
except (NameError, AttributeError):
    pass
