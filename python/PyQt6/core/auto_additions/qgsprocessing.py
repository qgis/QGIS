# The following has been generated automatically from src/core/processing/qgsprocessing.h
QgsProcessing.TypeMapLayer = QgsProcessing.SourceType.TypeMapLayer
QgsProcessing.TypeVectorAnyGeometry = QgsProcessing.SourceType.TypeVectorAnyGeometry
QgsProcessing.TypeVectorPoint = QgsProcessing.SourceType.TypeVectorPoint
QgsProcessing.TypeVectorLine = QgsProcessing.SourceType.TypeVectorLine
QgsProcessing.TypeVectorPolygon = QgsProcessing.SourceType.TypeVectorPolygon
QgsProcessing.TypeRaster = QgsProcessing.SourceType.TypeRaster
QgsProcessing.TypeFile = QgsProcessing.SourceType.TypeFile
QgsProcessing.TypeVector = QgsProcessing.SourceType.TypeVector
QgsProcessing.TypeMesh = QgsProcessing.SourceType.TypeMesh
QgsProcessing.TypePlugin = QgsProcessing.SourceType.TypePlugin
QgsProcessing.TypePointCloud = QgsProcessing.SourceType.TypePointCloud
QgsProcessing.TypeAnnotation = QgsProcessing.SourceType.TypeAnnotation
QgsProcessing.TypeVectorTile = QgsProcessing.SourceType.TypeVectorTile
QgsProcessing.PythonQgsProcessingAlgorithmSubclass = QgsProcessing.PythonOutputType.PythonQgsProcessingAlgorithmSubclass
# monkey patching scoped based enum
QgsProcessing.LayerOptionsFlag.SkipIndexGeneration.__doc__ = "Do not generate index when creating a layer. Makes sense only for point cloud layers"
QgsProcessing.LayerOptionsFlag.__doc__ = "Layer options flags\n\n.. versionadded:: 3.32\n\n" + '* ``SkipIndexGeneration``: ' + QgsProcessing.LayerOptionsFlag.SkipIndexGeneration.__doc__
# --
QgsProcessing.LayerOptionsFlags = lambda flags=0: QgsProcessing.LayerOptionsFlag(flags)
