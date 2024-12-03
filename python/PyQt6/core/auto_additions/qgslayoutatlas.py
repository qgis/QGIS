# The following has been generated automatically from src/core/layout/qgslayoutatlas.h
try:
    QgsLayoutAtlas.__attribute_docs__ = {'changed': 'Emitted when one of the atlas parameters changes.\n', 'toggled': 'Emitted when atlas is enabled or disabled.\n', 'coverageLayerChanged': 'Emitted when the coverage layer for the atlas changes.\n', 'messagePushed': 'Emitted when the atlas has an updated status bar ``message``.\n', 'numberFeaturesChanged': 'Emitted when the number of features for the atlas changes.\n', 'featureChanged': 'Emitted when the current atlas ``feature`` changes.\n', 'renderBegun': 'Emitted when atlas rendering has begun.\n', 'renderEnded': 'Emitted when atlas rendering has ended.\n'}
    QgsLayoutAtlas.__signal_arguments__ = {'toggled': ['enabled: bool'], 'coverageLayerChanged': ['layer: QgsVectorLayer'], 'messagePushed': ['message: str'], 'numberFeaturesChanged': ['numFeatures: int'], 'featureChanged': ['feature: QgsFeature']}
    QgsLayoutAtlas.__group__ = ['layout']
except (NameError, AttributeError):
    pass
