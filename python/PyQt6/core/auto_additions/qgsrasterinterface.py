# The following has been generated automatically from src/core/raster/qgsrasterinterface.h
try:
    QgsRasterBlockFeedback.__virtual_methods__ = ['onNewData']
    QgsRasterBlockFeedback.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterInterface.__virtual_methods__ = ['capabilities', 'sourceDataType', 'extent', 'xBlockSize', 'yBlockSize', 'xSize', 'ySize', 'generateBandName', 'colorInterpretationName', 'setInput', 'input', 'on', 'setOn', 'sourceInput', 'histogram', 'hasHistogram', 'cumulativeCut', 'writeXml', 'readXml']
    QgsRasterInterface.__abstract_methods__ = ['clone', 'dataType', 'bandCount', 'block']
    QgsRasterInterface.__group__ = ['raster']
except (NameError, AttributeError):
    pass
