# The following has been generated automatically from src/core/annotations/qgsannotation.h
try:
    QgsAnnotation.__attribute_docs__ = {'appearanceChanged': "Emitted whenever the annotation's appearance changes\n", 'moved': "Emitted when the annotation's position has changed and items need to be\nmoved to reflect this.\n", 'mapLayerChanged': 'Emitted when the map layer associated with the annotation changes.\n'}
    QgsAnnotation.__virtual_methods__ = ['setAssociatedFeature', 'accept', 'minimumFrameSize']
    QgsAnnotation.__abstract_methods__ = ['clone', 'writeXml', 'readXml', 'renderAnnotation']
    QgsAnnotation.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
