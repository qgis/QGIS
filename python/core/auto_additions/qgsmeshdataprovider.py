# The following has been generated automatically from src/core/mesh/qgsmeshdataprovider.h
try:
    QgsMeshDataProvider.__attribute_docs__ = {'datasetGroupsAdded': 'Emitted when some new dataset groups have been added\n'}
    QgsMeshDataProvider.__virtual_methods__ = ['driverMetadata']
    QgsMeshDataProvider.__abstract_methods__ = ['close', 'removeDatasetGroup']
    QgsMeshDataProvider.__overridden_methods__ = ['temporalCapabilities']
    QgsMeshDataProvider.__signal_arguments__ = {'datasetGroupsAdded': ['count: int']}
    QgsMeshDataProvider.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMesh.compareFaces = staticmethod(QgsMesh.compareFaces)
    QgsMesh.__doc__ = """
Mesh - vertices, edges and faces

.. versionadded:: 3.6"""
    QgsMesh.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDataSourceInterface.__virtual_methods__ = ['maximumVerticesCountPerFace']
    QgsMeshDataSourceInterface.__abstract_methods__ = ['vertexCount', 'faceCount', 'edgeCount', 'populateMesh', 'saveMeshFrame']
    QgsMeshDataSourceInterface.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDatasetSourceInterface.__virtual_methods__ = ['persistDatasetGroup']
    QgsMeshDatasetSourceInterface.__abstract_methods__ = ['addDataset', 'extraDatasets', 'datasetGroupCount', 'datasetCount', 'datasetGroupMetadata', 'datasetMetadata', 'datasetValue', 'datasetValues', 'dataset3dValues', 'isFaceActive', 'areFacesActive', 'persistDatasetGroup']
    QgsMeshDatasetSourceInterface.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
