# The following has been generated automatically from src/core/qgsmaplayerlegend.h
try:
    QgsMapLayerLegend.__attribute_docs__ = {'itemsChanged': 'Emitted when existing items/nodes got invalid and should be replaced by\nnew ones\n'}
    QgsMapLayerLegend.defaultVectorLegend = staticmethod(QgsMapLayerLegend.defaultVectorLegend)
    QgsMapLayerLegend.defaultRasterLegend = staticmethod(QgsMapLayerLegend.defaultRasterLegend)
    QgsMapLayerLegend.defaultMeshLegend = staticmethod(QgsMapLayerLegend.defaultMeshLegend)
    QgsMapLayerLegend.defaultPointCloudLegend = staticmethod(QgsMapLayerLegend.defaultPointCloudLegend)
    QgsMapLayerLegend.__virtual_methods__ = ['readXml', 'writeXml']
    QgsMapLayerLegend.__abstract_methods__ = ['createLayerTreeModelLegendNodes']
except (NameError, AttributeError):
    pass
try:
    QgsMapLayerLegendUtils.setLegendNodeOrder = staticmethod(QgsMapLayerLegendUtils.setLegendNodeOrder)
    QgsMapLayerLegendUtils.legendNodeOrder = staticmethod(QgsMapLayerLegendUtils.legendNodeOrder)
    QgsMapLayerLegendUtils.hasLegendNodeOrder = staticmethod(QgsMapLayerLegendUtils.hasLegendNodeOrder)
    QgsMapLayerLegendUtils.setLegendNodeUserLabel = staticmethod(QgsMapLayerLegendUtils.setLegendNodeUserLabel)
    QgsMapLayerLegendUtils.legendNodeUserLabel = staticmethod(QgsMapLayerLegendUtils.legendNodeUserLabel)
    QgsMapLayerLegendUtils.hasLegendNodeUserLabel = staticmethod(QgsMapLayerLegendUtils.hasLegendNodeUserLabel)
    QgsMapLayerLegendUtils.setLegendNodePatchShape = staticmethod(QgsMapLayerLegendUtils.setLegendNodePatchShape)
    QgsMapLayerLegendUtils.legendNodePatchShape = staticmethod(QgsMapLayerLegendUtils.legendNodePatchShape)
    QgsMapLayerLegendUtils.setLegendNodeSymbolSize = staticmethod(QgsMapLayerLegendUtils.setLegendNodeSymbolSize)
    QgsMapLayerLegendUtils.legendNodeSymbolSize = staticmethod(QgsMapLayerLegendUtils.legendNodeSymbolSize)
    QgsMapLayerLegendUtils.setLegendNodeCustomSymbol = staticmethod(QgsMapLayerLegendUtils.setLegendNodeCustomSymbol)
    QgsMapLayerLegendUtils.legendNodeCustomSymbol = staticmethod(QgsMapLayerLegendUtils.legendNodeCustomSymbol)
    QgsMapLayerLegendUtils.setLegendNodeColorRampSettings = staticmethod(QgsMapLayerLegendUtils.setLegendNodeColorRampSettings)
    QgsMapLayerLegendUtils.legendNodeColorRampSettings = staticmethod(QgsMapLayerLegendUtils.legendNodeColorRampSettings)
    QgsMapLayerLegendUtils.setLegendNodeColumnBreak = staticmethod(QgsMapLayerLegendUtils.setLegendNodeColumnBreak)
    QgsMapLayerLegendUtils.legendNodeColumnBreak = staticmethod(QgsMapLayerLegendUtils.legendNodeColumnBreak)
    QgsMapLayerLegendUtils.applyLayerNodeProperties = staticmethod(QgsMapLayerLegendUtils.applyLayerNodeProperties)
except (NameError, AttributeError):
    pass
try:
    QgsDefaultVectorLayerLegend.__overridden_methods__ = ['createLayerTreeModelLegendNodes', 'readXml', 'writeXml']
except (NameError, AttributeError):
    pass
try:
    QgsDefaultRasterLayerLegend.__overridden_methods__ = ['createLayerTreeModelLegendNodes']
except (NameError, AttributeError):
    pass
try:
    QgsDefaultMeshLayerLegend.__overridden_methods__ = ['createLayerTreeModelLegendNodes']
except (NameError, AttributeError):
    pass
try:
    QgsDefaultPointCloudLayerLegend.__overridden_methods__ = ['createLayerTreeModelLegendNodes']
except (NameError, AttributeError):
    pass
