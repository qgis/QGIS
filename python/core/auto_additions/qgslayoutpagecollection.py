# The following has been generated automatically from src/core/layout/qgslayoutpagecollection.h
try:
    QgsLayoutPageCollection.__attribute_docs__ = {'changed': 'Emitted when pages are added or removed from the collection.\n', 'pageAboutToBeRemoved': 'Emitted just before a page is removed from the collection.\n\nPage numbers in collections begin at 0 - so a page number of 0 indicates\nthe first page.\n'}
    QgsLayoutPageCollection.__overridden_methods__ = ['stringType', 'layout', 'writeXml', 'readXml']
    QgsLayoutPageCollection.__signal_arguments__ = {'pageAboutToBeRemoved': ['pageNumber: int']}
    import functools as _functools
    __wrapped_QgsLayoutPageCollection_addPage = QgsLayoutPageCollection.addPage
    def __QgsLayoutPageCollection_addPage_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutPageCollection_addPage(self, arg)
    QgsLayoutPageCollection.addPage = _functools.update_wrapper(__QgsLayoutPageCollection_addPage_wrapper, QgsLayoutPageCollection.addPage)

    QgsLayoutPageCollection.__group__ = ['layout']
except (NameError, AttributeError):
    pass
