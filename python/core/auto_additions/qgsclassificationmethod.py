# The following has been generated automatically from src/core/classification/qgsclassificationmethod.h
try:
    QgsClassificationMethod.rangesToBreaks = staticmethod(QgsClassificationMethod.rangesToBreaks)
    QgsClassificationMethod.create = staticmethod(QgsClassificationMethod.create)
    QgsClassificationMethod.makeBreaksSymmetric = staticmethod(QgsClassificationMethod.makeBreaksSymmetric)
    QgsClassificationMethod.__virtual_methods__ = ['icon', 'writeXml', 'readXml', 'valuesRequired']
    QgsClassificationMethod.__abstract_methods__ = ['clone', 'name', 'id']
    import functools as _functools
    __wrapped_QgsClassificationMethod_addParameter = QgsClassificationMethod.addParameter
    def __QgsClassificationMethod_addParameter_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsClassificationMethod_addParameter(self, arg)
    QgsClassificationMethod.addParameter = _functools.update_wrapper(__QgsClassificationMethod_addParameter_wrapper, QgsClassificationMethod.addParameter)

    QgsClassificationMethod.__group__ = ['classification']
except (NameError, AttributeError):
    pass
try:
    QgsClassificationRange.__group__ = ['classification']
except (NameError, AttributeError):
    pass
