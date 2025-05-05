# The following has been generated automatically from src/core/pointcloud/qgspointcloudrendererregistry.h
try:
    QgsPointCloudRendererRegistry.defaultRenderer = staticmethod(QgsPointCloudRendererRegistry.defaultRenderer)
    QgsPointCloudRendererRegistry.classificationAttributeCategories = staticmethod(QgsPointCloudRendererRegistry.classificationAttributeCategories)
    import functools as _functools
    __wrapped_QgsPointCloudRendererRegistry_addRenderer = QgsPointCloudRendererRegistry.addRenderer
    def __QgsPointCloudRendererRegistry_addRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointCloudRendererRegistry_addRenderer(self, arg)
    QgsPointCloudRendererRegistry.addRenderer = _functools.update_wrapper(__QgsPointCloudRendererRegistry_addRenderer_wrapper, QgsPointCloudRendererRegistry.addRenderer)

    QgsPointCloudRendererRegistry.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
try:
    QgsPointCloudRendererAbstractMetadata.__abstract_methods__ = ['createRenderer']
    QgsPointCloudRendererAbstractMetadata.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
try:
    QgsPointCloudRendererMetadata.__overridden_methods__ = ['createRenderer']
    QgsPointCloudRendererMetadata.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
