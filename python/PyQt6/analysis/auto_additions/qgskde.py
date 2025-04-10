# The following has been generated automatically from src/analysis/raster/qgskde.h
# monkey patching scoped based enum
QgsKernelDensityEstimation.KernelShape.KernelQuartic.__doc__ = "Quartic kernel"
QgsKernelDensityEstimation.KernelShape.KernelTriangular.__doc__ = "Triangular kernel"
QgsKernelDensityEstimation.KernelShape.KernelUniform.__doc__ = "Uniform (flat) kernel"
QgsKernelDensityEstimation.KernelShape.KernelTriweight.__doc__ = "Triweight kernel"
QgsKernelDensityEstimation.KernelShape.KernelEpanechnikov.__doc__ = "Epanechnikov kernel"
QgsKernelDensityEstimation.KernelShape.__doc__ = """Kernel shape type

* ``KernelQuartic``: Quartic kernel
* ``KernelTriangular``: Triangular kernel
* ``KernelUniform``: Uniform (flat) kernel
* ``KernelTriweight``: Triweight kernel
* ``KernelEpanechnikov``: Epanechnikov kernel

"""
# --
# monkey patching scoped based enum
QgsKernelDensityEstimation.OutputValues.OutputRaw.__doc__ = "Output the raw KDE values"
QgsKernelDensityEstimation.OutputValues.OutputScaled.__doc__ = "Output mathematically correct scaled values"
QgsKernelDensityEstimation.OutputValues.__doc__ = """Output values type

* ``OutputRaw``: Output the raw KDE values
* ``OutputScaled``: Output mathematically correct scaled values

"""
# --
# monkey patching scoped based enum
QgsKernelDensityEstimation.Result.Success.__doc__ = "Operation completed successfully"
QgsKernelDensityEstimation.Result.DriverError.__doc__ = "Could not open the driver for the specified format"
QgsKernelDensityEstimation.Result.InvalidParameters.__doc__ = "Input parameters were not valid"
QgsKernelDensityEstimation.Result.FileCreationError.__doc__ = "Error creating output file"
QgsKernelDensityEstimation.Result.RasterIoError.__doc__ = "Error writing to raster"
QgsKernelDensityEstimation.Result.__doc__ = """Result of operation

* ``Success``: Operation completed successfully
* ``DriverError``: Could not open the driver for the specified format
* ``InvalidParameters``: Input parameters were not valid
* ``FileCreationError``: Error creating output file
* ``RasterIoError``: Error writing to raster

"""
# --
try:
    QgsKernelDensityEstimation.Parameters.__attribute_docs__ = {'source': 'Point feature source', 'radius': 'Fixed radius, in map units', 'radiusField': 'Field for radius, or empty if using a fixed radius', 'weightField': 'Field name for weighting field, or empty if not using weights', 'pixelSize': 'Size of pixel in output file', 'shape': 'Kernel shape', 'decayRatio': 'Decay ratio (Triangular kernels only)', 'outputValues': 'Type of output value'}
    QgsKernelDensityEstimation.Parameters.__annotations__ = {'source': 'QgsFeatureSource', 'radius': float, 'radiusField': str, 'weightField': str, 'pixelSize': float, 'shape': 'QgsKernelDensityEstimation.KernelShape', 'decayRatio': float, 'outputValues': 'QgsKernelDensityEstimation.OutputValues'}
    QgsKernelDensityEstimation.Parameters.__doc__ = """KDE parameters"""
    QgsKernelDensityEstimation.Parameters.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsKernelDensityEstimation.__group__ = ['raster']
except (NameError, AttributeError):
    pass
