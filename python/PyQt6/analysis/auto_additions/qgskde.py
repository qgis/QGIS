# The following has been generated automatically from src/analysis/raster/qgskde.h
QgsKernelDensityEstimation.KernelQuartic = QgsKernelDensityEstimation.KernelShape.KernelQuartic
QgsKernelDensityEstimation.KernelTriangular = QgsKernelDensityEstimation.KernelShape.KernelTriangular
QgsKernelDensityEstimation.KernelUniform = QgsKernelDensityEstimation.KernelShape.KernelUniform
QgsKernelDensityEstimation.KernelTriweight = QgsKernelDensityEstimation.KernelShape.KernelTriweight
QgsKernelDensityEstimation.KernelEpanechnikov = QgsKernelDensityEstimation.KernelShape.KernelEpanechnikov
QgsKernelDensityEstimation.OutputRaw = QgsKernelDensityEstimation.OutputValues.OutputRaw
QgsKernelDensityEstimation.OutputScaled = QgsKernelDensityEstimation.OutputValues.OutputScaled
QgsKernelDensityEstimation.Success = QgsKernelDensityEstimation.Result.Success
QgsKernelDensityEstimation.DriverError = QgsKernelDensityEstimation.Result.DriverError
QgsKernelDensityEstimation.InvalidParameters = QgsKernelDensityEstimation.Result.InvalidParameters
QgsKernelDensityEstimation.FileCreationError = QgsKernelDensityEstimation.Result.FileCreationError
QgsKernelDensityEstimation.RasterIoError = QgsKernelDensityEstimation.Result.RasterIoError
try:
    QgsKernelDensityEstimation.Parameters.__attribute_docs__ = {'source': 'Point feature source', 'radius': 'Fixed radius, in map units', 'radiusField': 'Field for radius, or empty if using a fixed radius', 'weightField': 'Field name for weighting field, or empty if not using weights', 'pixelSize': 'Size of pixel in output file', 'shape': 'Kernel shape', 'decayRatio': 'Decay ratio (Triangular kernels only)', 'outputValues': 'Type of output value'}
    QgsKernelDensityEstimation.Parameters.__doc__ = """KDE parameters"""
    QgsKernelDensityEstimation.Parameters.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsKernelDensityEstimation.__group__ = ['raster']
except (NameError, AttributeError):
    pass
