# The following has been generated automatically from src/core/maprenderer/qgsmaprendererjob.h
try:
    QgsMapRendererJob.__attribute_docs__ = {'renderingLayersFinished': 'Emitted when the layers are rendered. Rendering labels is not yet done.\nIf the fully rendered layer including labels is required use\n:py:func:`~QgsMapRendererJob.finished` instead.\n', 'layerRenderingStarted': 'Emitted just before rendering starts for a particular layer.\n\n.. note::\n\n   the :py:class:`QgsMapRendererParallelJob` subclass does not emit this signal.\n\n.. versionadded:: 3.24\n', 'layerRendered': 'Emitted when a layer has completed rendering.\n\n.. note::\n\n   the :py:class:`QgsMapRendererParallelJob` subclass does not emit this signal.\n\n.. versionadded:: 3.24\n', 'finished': 'emitted when asynchronous rendering is finished (or canceled).\n'}
    QgsMapRendererJob.__abstract_methods__ = ['cancel', 'cancelWithoutBlocking', 'waitForFinished', 'isActive', 'usedCachedLabels', 'takeLabelingResults']
    QgsMapRendererJob.__signal_arguments__ = {'layerRenderingStarted': ['layerId: str'], 'layerRendered': ['layerId: str']}
    QgsMapRendererJob.__group__ = ['maprenderer']
except (NameError, AttributeError):
    pass
try:
    QgsMapRendererQImageJob.__abstract_methods__ = ['renderedImage']
    QgsMapRendererQImageJob.__group__ = ['maprenderer']
except (NameError, AttributeError):
    pass
try:
    QgsMapRendererJob.Error.__group__ = ['maprenderer']
except (NameError, AttributeError):
    pass
