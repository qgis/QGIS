# The following has been generated automatically from src/core/vector/qgsvectorlayerjoininfo.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsVectorLayerJoinInfo_setJoinFieldNamesSubset = QgsVectorLayerJoinInfo.setJoinFieldNamesSubset
    def __QgsVectorLayerJoinInfo_setJoinFieldNamesSubset_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorLayerJoinInfo_setJoinFieldNamesSubset(self, arg)
    QgsVectorLayerJoinInfo.setJoinFieldNamesSubset = _functools.update_wrapper(__QgsVectorLayerJoinInfo_setJoinFieldNamesSubset_wrapper, QgsVectorLayerJoinInfo.setJoinFieldNamesSubset)

    QgsVectorLayerJoinInfo.__group__ = ['vector']
except (NameError, AttributeError):
    pass
