# The following has been generated automatically from src/core/browser/qgsdatacollectionitem.h
try:
    QgsDataCollectionItem.iconDir = staticmethod(QgsDataCollectionItem.iconDir)
    QgsDataCollectionItem.iconDataCollection = staticmethod(QgsDataCollectionItem.iconDataCollection)
    QgsDataCollectionItem.openDirIcon = staticmethod(QgsDataCollectionItem.openDirIcon)
    QgsDataCollectionItem.homeDirIcon = staticmethod(QgsDataCollectionItem.homeDirIcon)
    QgsDataCollectionItem.__overridden_methods__ = ['databaseConnection']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsDataCollectionItem_iconDir = QgsDataCollectionItem.iconDir
    def __QgsDataCollectionItem_iconDir_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsDataCollectionItem_iconDir(self, arg)
    QgsDataCollectionItem.iconDir = _functools.update_wrapper(__QgsDataCollectionItem_iconDir_wrapper, QgsDataCollectionItem.iconDir)

    QgsDataCollectionItem.__group__ = ['browser']
except (NameError, AttributeError):
    pass
