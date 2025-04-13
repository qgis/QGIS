# The following has been generated automatically from src/analysis/raster/qgskde.h
# monkey patching scoped based enum
QgsKernelDensityEstimation.KernelQuartic = QgsKernelDensityEstimation.KernelShape.Quartic
QgsKernelDensityEstimation.KernelShape.KernelQuartic = QgsKernelDensityEstimation.KernelShape.Quartic
QgsKernelDensityEstimation.KernelQuartic.is_monkey_patched = True
QgsKernelDensityEstimation.KernelQuartic.__doc__ = "Quartic kernel"
QgsKernelDensityEstimation.KernelTriangular = QgsKernelDensityEstimation.KernelShape.Triangular
QgsKernelDensityEstimation.KernelShape.KernelTriangular = QgsKernelDensityEstimation.KernelShape.Triangular
QgsKernelDensityEstimation.KernelTriangular.is_monkey_patched = True
QgsKernelDensityEstimation.KernelTriangular.__doc__ = "Triangular kernel"
QgsKernelDensityEstimation.KernelUniform = QgsKernelDensityEstimation.KernelShape.Uniform
QgsKernelDensityEstimation.KernelShape.KernelUniform = QgsKernelDensityEstimation.KernelShape.Uniform
QgsKernelDensityEstimation.KernelUniform.is_monkey_patched = True
QgsKernelDensityEstimation.KernelUniform.__doc__ = "Uniform (flat) kernel"
QgsKernelDensityEstimation.KernelTriweight = QgsKernelDensityEstimation.KernelShape.Triweight
QgsKernelDensityEstimation.KernelShape.KernelTriweight = QgsKernelDensityEstimation.KernelShape.Triweight
QgsKernelDensityEstimation.KernelTriweight.is_monkey_patched = True
QgsKernelDensityEstimation.KernelTriweight.__doc__ = "Triweight kernel"
QgsKernelDensityEstimation.KernelEpanechnikov = QgsKernelDensityEstimation.KernelShape.Epanechnikov
QgsKernelDensityEstimation.KernelShape.KernelEpanechnikov = QgsKernelDensityEstimation.KernelShape.Epanechnikov
QgsKernelDensityEstimation.KernelEpanechnikov.is_monkey_patched = True
QgsKernelDensityEstimation.KernelEpanechnikov.__doc__ = "Epanechnikov kernel"
QgsKernelDensityEstimation.KernelShape.__doc__ = """Kernel shape type

* ``Quartic``: Quartic kernel

  Available as ``QgsKernelDensityEstimation.KernelQuartic`` in older QGIS releases.

* ``Triangular``: Triangular kernel

  Available as ``QgsKernelDensityEstimation.KernelTriangular`` in older QGIS releases.

* ``Uniform``: Uniform (flat) kernel

  Available as ``QgsKernelDensityEstimation.KernelUniform`` in older QGIS releases.

* ``Triweight``: Triweight kernel

  Available as ``QgsKernelDensityEstimation.KernelTriweight`` in older QGIS releases.

* ``Epanechnikov``: Epanechnikov kernel

  Available as ``QgsKernelDensityEstimation.KernelEpanechnikov`` in older QGIS releases.


"""
# --
# monkey patching scoped based enum
QgsKernelDensityEstimation.OutputRaw = QgsKernelDensityEstimation.OutputValues.Raw
QgsKernelDensityEstimation.OutputValues.OutputRaw = QgsKernelDensityEstimation.OutputValues.Raw
QgsKernelDensityEstimation.OutputRaw.is_monkey_patched = True
QgsKernelDensityEstimation.OutputRaw.__doc__ = "Output the raw KDE values"
QgsKernelDensityEstimation.OutputScaled = QgsKernelDensityEstimation.OutputValues.Scaled
QgsKernelDensityEstimation.OutputValues.OutputScaled = QgsKernelDensityEstimation.OutputValues.Scaled
QgsKernelDensityEstimation.OutputScaled.is_monkey_patched = True
QgsKernelDensityEstimation.OutputScaled.__doc__ = "Output mathematically correct scaled values"
QgsKernelDensityEstimation.OutputValues.__doc__ = """Output values type

* ``Raw``: Output the raw KDE values

  Available as ``QgsKernelDensityEstimation.OutputRaw`` in older QGIS releases.

* ``Scaled``: Output mathematically correct scaled values

  Available as ``QgsKernelDensityEstimation.OutputScaled`` in older QGIS releases.


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
