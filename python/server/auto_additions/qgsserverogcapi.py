# The following has been generated automatically from src/server/qgsserverogcapi.h
QgsServerOgcApi.Rel.baseClass = QgsServerOgcApi
QgsServerOgcApi.ContentType.baseClass = QgsServerOgcApi
try:
    QgsServerOgcApi.sanitizeUrl = staticmethod(QgsServerOgcApi.sanitizeUrl)
    QgsServerOgcApi.relToString = staticmethod(QgsServerOgcApi.relToString)
    QgsServerOgcApi.contentTypeToString = staticmethod(QgsServerOgcApi.contentTypeToString)
    QgsServerOgcApi.contentTypeToStdString = staticmethod(QgsServerOgcApi.contentTypeToStdString)
    QgsServerOgcApi.contentTypeToExtension = staticmethod(QgsServerOgcApi.contentTypeToExtension)
    QgsServerOgcApi.contenTypeFromExtension = staticmethod(QgsServerOgcApi.contenTypeFromExtension)
    QgsServerOgcApi.contentTypeFromExtension = staticmethod(QgsServerOgcApi.contentTypeFromExtension)
    QgsServerOgcApi.mimeType = staticmethod(QgsServerOgcApi.mimeType)
    QgsServerOgcApi.__overridden_methods__ = ['name', 'description', 'version', 'rootPath', 'executeRequest']
    import functools as _functools
    __wrapped_QgsServerOgcApi_registerHandler = QgsServerOgcApi.registerHandler
    def __QgsServerOgcApi_registerHandler_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsServerOgcApi_registerHandler(self, arg)
    QgsServerOgcApi.registerHandler = _functools.update_wrapper(__QgsServerOgcApi_registerHandler_wrapper, QgsServerOgcApi.registerHandler)

except (NameError, AttributeError):
    pass
