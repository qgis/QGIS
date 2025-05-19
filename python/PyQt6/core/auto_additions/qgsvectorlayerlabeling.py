# The following has been generated automatically from src/core/labeling/qgsvectorlayerlabeling.h
try:
    QgsAbstractVectorLayerLabeling.create = staticmethod(QgsAbstractVectorLayerLabeling.create)
    QgsAbstractVectorLayerLabeling.defaultSettingsForLayer = staticmethod(QgsAbstractVectorLayerLabeling.defaultSettingsForLayer)
    QgsAbstractVectorLayerLabeling.__virtual_methods__ = ['subProviders', 'multiplyOpacity', 'toSld', 'accept', 'writeTextSymbolizer']
    QgsAbstractVectorLayerLabeling.__abstract_methods__ = ['type', 'clone', 'save', 'settings', 'setSettings', 'requiresAdvancedEffects', 'hasNonDefaultCompositionMode']
    QgsAbstractVectorLayerLabeling.__group__ = ['labeling']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerSimpleLabeling.create = staticmethod(QgsVectorLayerSimpleLabeling.create)
    QgsVectorLayerSimpleLabeling.__overridden_methods__ = ['type', 'clone', 'save', 'settings', 'accept', 'setSettings', 'requiresAdvancedEffects', 'hasNonDefaultCompositionMode', 'toSld', 'multiplyOpacity']
    QgsVectorLayerSimpleLabeling.__group__ = ['labeling']
except (NameError, AttributeError):
    pass
