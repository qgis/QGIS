# The following has been generated automatically from src/core/browser/qgsfielddomainsitem.h
try:
    QgsFieldDomainsItem.__overridden_methods__ = ['createChildren', 'icon']
    QgsFieldDomainsItem.__group__ = ['browser']
except (NameError, AttributeError):
    pass
try:
    QgsFieldDomainItem.__overridden_methods__ = ['icon']
    import functools as _functools
    __wrapped_QgsFieldDomainItem_QgsFieldDomainItem = QgsFieldDomainItem.QgsFieldDomainItem
    def __QgsFieldDomainItem_QgsFieldDomainItem_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFieldDomainItem_QgsFieldDomainItem(self, arg)
    QgsFieldDomainItem.QgsFieldDomainItem = _functools.update_wrapper(__QgsFieldDomainItem_QgsFieldDomainItem_wrapper, QgsFieldDomainItem.QgsFieldDomainItem)

    QgsFieldDomainItem.__group__ = ['browser']
except (NameError, AttributeError):
    pass
