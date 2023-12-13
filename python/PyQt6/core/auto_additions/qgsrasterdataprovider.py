# The following has been generated automatically from src/core/raster/qgsrasterdataprovider.h
QgsRasterDataProvider.NoProviderCapabilities = QgsRasterDataProvider.ProviderCapability.NoProviderCapabilities
QgsRasterDataProvider.ReadLayerMetadata = QgsRasterDataProvider.ProviderCapability.ReadLayerMetadata
QgsRasterDataProvider.WriteLayerMetadata = QgsRasterDataProvider.ProviderCapability.WriteLayerMetadata
QgsRasterDataProvider.ProviderHintBenefitsFromResampling = QgsRasterDataProvider.ProviderCapability.ProviderHintBenefitsFromResampling
QgsRasterDataProvider.ProviderHintCanPerformProviderResampling = QgsRasterDataProvider.ProviderCapability.ProviderHintCanPerformProviderResampling
QgsRasterDataProvider.ReloadData = QgsRasterDataProvider.ProviderCapability.ReloadData
QgsRasterDataProvider.DpiDependentData = QgsRasterDataProvider.ProviderCapability.DpiDependentData
QgsRasterDataProvider.NativeRasterAttributeTable = QgsRasterDataProvider.ProviderCapability.NativeRasterAttributeTable
QgsRasterDataProvider.ProviderCapabilities = lambda flags=0: QgsRasterDataProvider.ProviderCapability(flags)
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
QgsRasterDataProvider.ResamplingMethod.__doc__ = "Resampling method for provider-level resampling.\n\n.. versionadded:: 3.16\n\n" + '* ``Nearest``: ' + QgsRasterDataProvider.ResamplingMethod.Nearest.__doc__ + '\n' + '* ``Bilinear``: ' + QgsRasterDataProvider.ResamplingMethod.Bilinear.__doc__ + '\n' + '* ``Cubic``: ' + QgsRasterDataProvider.ResamplingMethod.Cubic.__doc__ + '\n' + '* ``CubicSpline``: ' + QgsRasterDataProvider.ResamplingMethod.CubicSpline.__doc__ + '\n' + '* ``Lanczos``: ' + QgsRasterDataProvider.ResamplingMethod.Lanczos.__doc__ + '\n' + '* ``Average``: ' + QgsRasterDataProvider.ResamplingMethod.Average.__doc__ + '\n' + '* ``Mode``: ' + QgsRasterDataProvider.ResamplingMethod.Mode.__doc__ + '\n' + '* ``Gauss``: ' + QgsRasterDataProvider.ResamplingMethod.Gauss.__doc__
# --
