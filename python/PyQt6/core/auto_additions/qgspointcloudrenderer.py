# The following has been generated automatically from src/core/pointcloud/qgspointcloudrenderer.h
# monkey patching scoped based enum
QgsPointCloudRenderer.Property.Color.__doc__ = "Point color"
QgsPointCloudRenderer.Property.__doc__ = """Data-defined properties that can be set on the renderer.

.. versionadded:: 4.2

* ``Color``: Point color

"""
# --
try:
    QgsPointCloudRenderer.load = staticmethod(QgsPointCloudRenderer.load)
    QgsPointCloudRenderer.pointXY = staticmethod(QgsPointCloudRenderer.pointXY)
    QgsPointCloudRenderer.pointZ = staticmethod(QgsPointCloudRenderer.pointZ)
    QgsPointCloudRenderer.__virtual_methods__ = ['willRenderPoint', 'usedAttributes', 'startRender', 'stopRender', 'legendItemChecked', 'checkLegendItem', 'createLegendNodes', 'legendRuleKeys']
    QgsPointCloudRenderer.__abstract_methods__ = ['type', 'clone', 'renderBlock', 'save']
    QgsPointCloudRenderer.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
try:
    QgsPointCloudRenderContext.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
