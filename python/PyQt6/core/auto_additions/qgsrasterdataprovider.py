# The following has been generated automatically from src/core/raster/qgsrasterdataprovider.h
QgsRasterDataProvider.TransformImageToLayer = QgsRasterDataProvider.TransformType.TransformImageToLayer
QgsRasterDataProvider.TransformLayerToImage = QgsRasterDataProvider.TransformType.TransformLayerToImage
# monkey patching scoped based enum
QgsRasterDataProvider.ResamplingMethod.Nearest.__doc__ = "Nearest-neighbour resampling"
QgsRasterDataProvider.ResamplingMethod.Bilinear.__doc__ = "Bilinear (2x2 kernel) resampling"
QgsRasterDataProvider.ResamplingMethod.Cubic.__doc__ = "Cubic Convolution Approximation (4x4 kernel) resampling"
QgsRasterDataProvider.ResamplingMethod.CubicSpline.__doc__ = "Cubic B-Spline Approximation (4x4 kernel)"
QgsRasterDataProvider.ResamplingMethod.Lanczos.__doc__ = "Lanczos windowed sinc interpolation (6x6 kernel)"
QgsRasterDataProvider.ResamplingMethod.Average.__doc__ = "Average resampling"
QgsRasterDataProvider.ResamplingMethod.Mode.__doc__ = "Mode (selects the value which appears most often of all the sampled points)"
QgsRasterDataProvider.ResamplingMethod.Gauss.__doc__ = "Gauss blurring"
QgsRasterDataProvider.ResamplingMethod.__doc__ = """Resampling method for provider-level resampling.

.. versionadded:: 3.16

* ``Nearest``: Nearest-neighbour resampling
* ``Bilinear``: Bilinear (2x2 kernel) resampling
* ``Cubic``: Cubic Convolution Approximation (4x4 kernel) resampling
* ``CubicSpline``: Cubic B-Spline Approximation (4x4 kernel)
* ``Lanczos``: Lanczos windowed sinc interpolation (6x6 kernel)
* ``Average``: Average resampling
* ``Mode``: Mode (selects the value which appears most often of all the sampled points)
* ``Gauss``: Gauss blurring

"""
# --
try:
    QgsImageFetcher.__attribute_docs__ = {'finish': 'Emitted when the download completes\n\n:param legend: The downloaded legend image\n', 'progress': 'Emitted to report progress\n', 'error': 'Emitted when an error occurs\n'}
    QgsImageFetcher.__signal_arguments__ = {'finish': ['legend: QImage'], 'progress': ['received: int', 'total: int'], 'error': ['msg: str']}
    QgsImageFetcher.__group__ = ['raster']
except NameError:
    pass
try:
    QgsRasterDataProvider.__attribute_docs__ = {'statusChanged': 'Emit a message to be displayed on status bar, usually used by network providers (WMS,WCS)\n'}
    QgsRasterDataProvider.create = staticmethod(QgsRasterDataProvider.create)
    QgsRasterDataProvider.pyramidResamplingMethods = staticmethod(QgsRasterDataProvider.pyramidResamplingMethods)
    QgsRasterDataProvider.decodeVirtualRasterProviderUri = staticmethod(QgsRasterDataProvider.decodeVirtualRasterProviderUri)
    QgsRasterDataProvider.encodeVirtualRasterProviderUri = staticmethod(QgsRasterDataProvider.encodeVirtualRasterProviderUri)
    QgsRasterDataProvider.identifyFormatName = staticmethod(QgsRasterDataProvider.identifyFormatName)
    QgsRasterDataProvider.identifyFormatFromName = staticmethod(QgsRasterDataProvider.identifyFormatFromName)
    QgsRasterDataProvider.identifyFormatLabel = staticmethod(QgsRasterDataProvider.identifyFormatLabel)
    QgsRasterDataProvider.identifyFormatToCapability = staticmethod(QgsRasterDataProvider.identifyFormatToCapability)
    QgsRasterDataProvider.__group__ = ['raster']
except NameError:
    pass
try:
    QgsRasterDataProvider.VirtualRasterInputLayers.__doc__ = """Struct that stores information of the raster used in :py:class:`QgsVirtualRasterProvider` for the calculations,
this struct is  stored in the DecodedUriParameters

.. note::

   used by :py:class:`QgsVirtualRasterProvider` only"""
    QgsRasterDataProvider.VirtualRasterInputLayers.__group__ = ['raster']
except NameError:
    pass
try:
    QgsRasterDataProvider.VirtualRasterParameters.__doc__ = """Struct that stores the information about the parameters that should be given to the
:py:class:`QgsVirtualRasterProvider` through the :py:class:`QgsRasterDataProvider`.DecodedUriParameters

.. note::

   used by :py:class:`QgsVirtualRasterProvider` only"""
    QgsRasterDataProvider.VirtualRasterParameters.__group__ = ['raster']
except NameError:
    pass
