# The following has been generated automatically from src/core/mesh/qgsmeshdataset.h
try:
    QgsMeshDatasetGroup.__virtual_methods__ = ['datasetGroupNamesDependentOn', 'description']
    QgsMeshDatasetGroup.__abstract_methods__ = ['initialize', 'datasetMetadata', 'datasetCount', 'dataset', 'type', 'writeXml']
    QgsMeshDatasetGroup.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDataset.__abstract_methods__ = ['datasetValue', 'datasetValues', 'areFacesActive', 'isActive', 'metadata', 'valuesCount']
    QgsMeshDataset.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMeshDatasetGroupTreeItem_appendChild = QgsMeshDatasetGroupTreeItem.appendChild
    def __QgsMeshDatasetGroupTreeItem_appendChild_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMeshDatasetGroupTreeItem_appendChild(self, arg)
    QgsMeshDatasetGroupTreeItem.appendChild = _functools.update_wrapper(__QgsMeshDatasetGroupTreeItem_appendChild_wrapper, QgsMeshDatasetGroupTreeItem.appendChild)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMeshDatasetGroupTreeItem_removeChild = QgsMeshDatasetGroupTreeItem.removeChild
    def __QgsMeshDatasetGroupTreeItem_removeChild_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMeshDatasetGroupTreeItem_removeChild(self, arg)
    QgsMeshDatasetGroupTreeItem.removeChild = _functools.update_wrapper(__QgsMeshDatasetGroupTreeItem_removeChild_wrapper, QgsMeshDatasetGroupTreeItem.removeChild)

    QgsMeshDatasetGroupTreeItem.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDatasetIndex.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDatasetValue.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDataBlock.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMesh3DDataBlock.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDatasetGroupMetadata.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
try:
    QgsMeshDatasetMetadata.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
