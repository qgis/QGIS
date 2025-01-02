# The following has been generated automatically from src/core/vector/qgsvectorlayerfeatureiterator.h
try:
    QgsVectorLayerFeatureIterator.FetchJoinInfo.__attribute_docs__ = {'joinInfo': 'Canonical source of information about the join', 'attributes': 'Attributes to fetch', 'indexOffset': 'At what position the joined fields start', 'targetField': 'Index of field (of this layer) that drives the join', 'joinField': '< Index of field (of the joined layer) must have equal value'}
    QgsVectorLayerFeatureIterator.FetchJoinInfo.__doc__ = """Join information prepared for fast attribute id mapping in :py:func:`QgsVectorLayerJoinBuffer.updateFeatureAttributes()`.
Created in the :py:func:`~QgsVectorLayerFeatureIterator.select` method of :py:class:`QgsVectorLayerJoinBuffer` for the joins that contain fetched attributes"""
    QgsVectorLayerFeatureIterator.FetchJoinInfo.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerFeatureSource.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerFeatureIterator.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerSelectedFeatureSource.__group__ = ['vector']
except (NameError, AttributeError):
    pass
