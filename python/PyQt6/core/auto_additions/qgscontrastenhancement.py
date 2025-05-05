# The following has been generated automatically from src/core/raster/qgscontrastenhancement.h
QgsContrastEnhancement.NoEnhancement = QgsContrastEnhancement.ContrastEnhancementAlgorithm.NoEnhancement
QgsContrastEnhancement.StretchToMinimumMaximum = QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum
QgsContrastEnhancement.StretchAndClipToMinimumMaximum = QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchAndClipToMinimumMaximum
QgsContrastEnhancement.ClipToMinimumMaximum = QgsContrastEnhancement.ContrastEnhancementAlgorithm.ClipToMinimumMaximum
QgsContrastEnhancement.UserDefinedEnhancement = QgsContrastEnhancement.ContrastEnhancementAlgorithm.UserDefinedEnhancement
try:
    QgsContrastEnhancement.maximumValuePossible = staticmethod(QgsContrastEnhancement.maximumValuePossible)
    QgsContrastEnhancement.minimumValuePossible = staticmethod(QgsContrastEnhancement.minimumValuePossible)
    QgsContrastEnhancement.contrastEnhancementAlgorithmString = staticmethod(QgsContrastEnhancement.contrastEnhancementAlgorithmString)
    QgsContrastEnhancement.contrastEnhancementAlgorithmFromString = staticmethod(QgsContrastEnhancement.contrastEnhancementAlgorithmFromString)
    import functools as _functools
    __wrapped_QgsContrastEnhancement_setContrastEnhancementFunction = QgsContrastEnhancement.setContrastEnhancementFunction
    def __QgsContrastEnhancement_setContrastEnhancementFunction_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsContrastEnhancement_setContrastEnhancementFunction(self, arg)
    QgsContrastEnhancement.setContrastEnhancementFunction = _functools.update_wrapper(__QgsContrastEnhancement_setContrastEnhancementFunction_wrapper, QgsContrastEnhancement.setContrastEnhancementFunction)

    QgsContrastEnhancement.__group__ = ['raster']
except (NameError, AttributeError):
    pass
