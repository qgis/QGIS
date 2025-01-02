# The following has been generated automatically from src/core/mesh/qgsmeshdataprovider.h
QgsMesh.Vertex = QgsMesh.ElementType.Vertex
QgsMesh.Edge = QgsMesh.ElementType.Edge
QgsMesh.Face = QgsMesh.ElementType.Face
try:
    QgsMeshDataProvider.__attribute_docs__ = {'datasetGroupsAdded': 'Emitted when some new dataset groups have been added\n'}
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
    QgsMeshDataSourceInterface.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDatasetSourceInterface.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
