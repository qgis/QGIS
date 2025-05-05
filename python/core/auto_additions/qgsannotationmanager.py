# The following has been generated automatically from src/core/annotations/qgsannotationmanager.h
try:
    QgsAnnotationManager.__attribute_docs__ = {'annotationAdded': 'Emitted when a annotation has been added to the manager\n', 'annotationRemoved': 'Emitted when an annotation was removed from the manager\n', 'annotationAboutToBeRemoved': 'Emitted when an annotation is about to be removed from the manager\n'}
    QgsAnnotationManager.__signal_arguments__ = {'annotationAdded': ['annotation: QgsAnnotation'], 'annotationAboutToBeRemoved': ['annotation: QgsAnnotation']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsAnnotationManager_addAnnotation = QgsAnnotationManager.addAnnotation
    def __QgsAnnotationManager_addAnnotation_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationManager_addAnnotation(self, arg)
    QgsAnnotationManager.addAnnotation = _functools.update_wrapper(__QgsAnnotationManager_addAnnotation_wrapper, QgsAnnotationManager.addAnnotation)

    QgsAnnotationManager.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
