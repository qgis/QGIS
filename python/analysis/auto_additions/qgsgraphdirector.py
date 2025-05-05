# The following has been generated automatically from src/analysis/network/qgsgraphdirector.h
try:
    QgsGraphDirector.__virtual_methods__ = ['makeGraph']
    QgsGraphDirector.__abstract_methods__ = ['name']
    import functools as _functools
    __wrapped_QgsGraphDirector_addStrategy = QgsGraphDirector.addStrategy
    def __QgsGraphDirector_addStrategy_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGraphDirector_addStrategy(self, arg)
    QgsGraphDirector.addStrategy = _functools.update_wrapper(__QgsGraphDirector_addStrategy_wrapper, QgsGraphDirector.addStrategy)

    QgsGraphDirector.__group__ = ['network']
except (NameError, AttributeError):
    pass
