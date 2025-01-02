# The following has been generated automatically from src/core/effects/qgsimageoperation.h
try:
    QgsImageOperation.DistanceTransformProperties.__attribute_docs__ = {'shadeExterior': 'Set to ``True`` to perform the distance transform on transparent pixels\nin the source image, set to ``False`` to perform the distance transform\non opaque pixels', 'useMaxDistance': 'Set to ``True`` to automatically calculate the maximum distance in the\ntransform to use as the spread value', 'spread': 'Maximum distance (in pixels) for the distance transform shading to\nspread', 'ramp': 'Color ramp to use for shading the distance transform'}
    QgsImageOperation.DistanceTransformProperties.__doc__ = """Struct for storing properties of a distance transform operation"""
    QgsImageOperation.DistanceTransformProperties.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsImageOperation.convertToGrayscale = staticmethod(QgsImageOperation.convertToGrayscale)
    QgsImageOperation.adjustBrightnessContrast = staticmethod(QgsImageOperation.adjustBrightnessContrast)
    QgsImageOperation.adjustHueSaturation = staticmethod(QgsImageOperation.adjustHueSaturation)
    QgsImageOperation.multiplyOpacity = staticmethod(QgsImageOperation.multiplyOpacity)
    QgsImageOperation.overlayColor = staticmethod(QgsImageOperation.overlayColor)
    QgsImageOperation.distanceTransform = staticmethod(QgsImageOperation.distanceTransform)
    QgsImageOperation.stackBlur = staticmethod(QgsImageOperation.stackBlur)
    QgsImageOperation.gaussianBlur = staticmethod(QgsImageOperation.gaussianBlur)
    QgsImageOperation.flipImage = staticmethod(QgsImageOperation.flipImage)
    QgsImageOperation.nonTransparentImageRect = staticmethod(QgsImageOperation.nonTransparentImageRect)
    QgsImageOperation.cropTransparent = staticmethod(QgsImageOperation.cropTransparent)
    QgsImageOperation.__group__ = ['effects']
except (NameError, AttributeError):
    pass
