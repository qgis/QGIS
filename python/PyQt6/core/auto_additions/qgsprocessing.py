# The following has been generated automatically from src/core/processing/qgsprocessing.h
# monkey patching scoped based enum
QgsProcessing.PythonQgsProcessingAlgorithmSubclass = QgsProcessing.PythonOutputType.PythonQgsProcessingAlgorithmSubclass
QgsProcessing.PythonQgsProcessingAlgorithmSubclass.is_monkey_patched = True
QgsProcessing.PythonOutputType.PythonQgsProcessingAlgorithmSubclass.__doc__ = "Full Python QgsProcessingAlgorithm subclass"
QgsProcessing.PythonOutputType.__doc__ = "Available Python output types\n\n" + '* ``PythonQgsProcessingAlgorithmSubclass``: ' + QgsProcessing.PythonOutputType.PythonQgsProcessingAlgorithmSubclass.__doc__
# --
QgsProcessing.PythonOutputType.baseClass = QgsProcessing
# monkey patching scoped based enum
QgsProcessing.LayerOptionsFlag.SkipIndexGeneration.__doc__ = "Do not generate index when creating a layer. Makes sense only for point cloud layers"
QgsProcessing.LayerOptionsFlag.__doc__ = "Layer options flags\n\n.. versionadded:: 3.32\n\n" + '* ``SkipIndexGeneration``: ' + QgsProcessing.LayerOptionsFlag.SkipIndexGeneration.__doc__
# --
QgsProcessing.LayerOptionsFlag.baseClass = QgsProcessing
QgsProcessing.LayerOptionsFlags = lambda flags=0: QgsProcessing.LayerOptionsFlag(flags)
QgsProcessing.LayerOptionsFlags.baseClass = QgsProcessing
LayerOptionsFlags = QgsProcessing  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsProcessing.__attribute_docs__ = {'TEMPORARY_OUTPUT': 'Constant used to indicate that a Processing algorithm output should be a temporary layer/file.\n\n.. versionadded:: 3.6'}
except NameError:
    pass
