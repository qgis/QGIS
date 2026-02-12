# The following has been generated automatically from src/core/mesh/qgsmeshlayerlabeling.h
try:
    QgsAbstractMeshLayerLabeling.create = staticmethod(QgsAbstractMeshLayerLabeling.create)
    QgsAbstractMeshLayerLabeling.defaultSettingsForLayer = staticmethod(QgsAbstractMeshLayerLabeling.defaultSettingsForLayer)
    QgsAbstractMeshLayerLabeling.__virtual_methods__ = ['subProviders', 'multiplyOpacity', 'toSld', 'accept']
    QgsAbstractMeshLayerLabeling.__abstract_methods__ = ['type', 'clone', 'save', 'settings', 'setSettings', 'requiresAdvancedEffects', 'hasNonDefaultCompositionMode']
    QgsAbstractMeshLayerLabeling.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshLayerSimpleLabeling.create = staticmethod(QgsMeshLayerSimpleLabeling.create)
    QgsMeshLayerSimpleLabeling.__overridden_methods__ = ['type', 'clone', 'save', 'settings', 'accept', 'setSettings', 'requiresAdvancedEffects', 'hasNonDefaultCompositionMode', 'multiplyOpacity']
    QgsMeshLayerSimpleLabeling.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
