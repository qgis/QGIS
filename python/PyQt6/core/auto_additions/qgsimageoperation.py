# The following has been generated automatically from src/core/effects/qgsimageoperation.h
QgsImageOperation.GrayscaleLightness = QgsImageOperation.GrayscaleMode.GrayscaleLightness
QgsImageOperation.GrayscaleLuminosity = QgsImageOperation.GrayscaleMode.GrayscaleLuminosity
QgsImageOperation.GrayscaleAverage = QgsImageOperation.GrayscaleMode.GrayscaleAverage
QgsImageOperation.GrayscaleOff = QgsImageOperation.GrayscaleMode.GrayscaleOff
QgsImageOperation.FlipHorizontal = QgsImageOperation.FlipType.FlipHorizontal
QgsImageOperation.FlipVertical = QgsImageOperation.FlipType.FlipVertical
try:
    QgsImageOperation.__attribute_docs__ = {'shadeExterior': 'Set to ``True`` to perform the distance transform on transparent pixels\nin the source image, set to ``False`` to perform the distance transform\non opaque pixels', 'useMaxDistance': 'Set to ``True`` to automatically calculate the maximum distance in the\ntransform to use as the spread value', 'spread': 'Maximum distance (in pixels) for the distance transform shading to\nspread', 'ramp': 'Color ramp to use for shading the distance transform'}
except NameError:
    pass
