# The following has been generated automatically from src/gui/attributetable/qgsfeatureselectionmodel.h
try:
    QgsFeatureSelectionModel.__attribute_docs__ = {'requestRepaint': 'Request a repaint of the visible items of connected views. Views using\nthis model should connect to and properly process this signal.\n'}
    QgsFeatureSelectionModel.__virtual_methods__ = ['isSelected', 'selectFeatures', 'setFeatureSelectionManager']
    QgsFeatureSelectionModel.__overridden_methods__ = ['select']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsFeatureSelectionModel_setFeatureSelectionManager = QgsFeatureSelectionModel.setFeatureSelectionManager
    def __QgsFeatureSelectionModel_setFeatureSelectionManager_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFeatureSelectionModel_setFeatureSelectionManager(self, arg)
    QgsFeatureSelectionModel.setFeatureSelectionManager = _functools.update_wrapper(__QgsFeatureSelectionModel_setFeatureSelectionManager_wrapper, QgsFeatureSelectionModel.setFeatureSelectionManager)

    QgsFeatureSelectionModel.__group__ = ['attributetable']
except (NameError, AttributeError):
    pass
