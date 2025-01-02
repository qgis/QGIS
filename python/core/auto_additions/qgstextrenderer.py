# The following has been generated automatically from src/core/textrenderer/qgstextrenderer.h
try:
    QgsTextRenderer.__attribute_docs__ = {'FONT_WORKAROUND_SCALE': 'Scale factor for upscaling font sizes and downscaling destination painter devices.\n\nUsing this scale factor and manually adjusting any font metric based calculations results in more stable\nfont metrics and sizes for small font sizes.\n\n.. warning::\n\n   Deprecated, use :py:func:`~QgsTextRenderer.calculateScaleFactorForFormat` instead.\n\n.. versionadded:: 3.16', 'SUPERSCRIPT_SUBSCRIPT_FONT_SIZE_SCALING_FACTOR': "Scale factor to use for super or subscript text which doesn't have an explicit font size set.\n\n.. versionadded:: 3.32"}
    QgsTextRenderer.convertQtHAlignment = staticmethod(QgsTextRenderer.convertQtHAlignment)
    QgsTextRenderer.convertQtVAlignment = staticmethod(QgsTextRenderer.convertQtVAlignment)
    QgsTextRenderer.sizeToPixel = staticmethod(QgsTextRenderer.sizeToPixel)
    QgsTextRenderer.drawText = staticmethod(QgsTextRenderer.drawText)
    QgsTextRenderer.drawDocument = staticmethod(QgsTextRenderer.drawDocument)
    QgsTextRenderer.drawTextOnLine = staticmethod(QgsTextRenderer.drawTextOnLine)
    QgsTextRenderer.drawDocumentOnLine = staticmethod(QgsTextRenderer.drawDocumentOnLine)
    QgsTextRenderer.drawPart = staticmethod(QgsTextRenderer.drawPart)
    QgsTextRenderer.fontMetrics = staticmethod(QgsTextRenderer.fontMetrics)
    QgsTextRenderer.textWidth = staticmethod(QgsTextRenderer.textWidth)
    QgsTextRenderer.textHeight = staticmethod(QgsTextRenderer.textHeight)
    QgsTextRenderer.textRequiresWrapping = staticmethod(QgsTextRenderer.textRequiresWrapping)
    QgsTextRenderer.wrappedText = staticmethod(QgsTextRenderer.wrappedText)
    QgsTextRenderer.calculateScaleFactorForFormat = staticmethod(QgsTextRenderer.calculateScaleFactorForFormat)
    QgsTextRenderer.__group__ = ['textrenderer']
except (NameError, AttributeError):
    pass
