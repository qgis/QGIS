# The following has been generated automatically from src/core/qgis.h
QgsMapLayer.LayerType = QgsMapLayerType
# monkey patching scoped based enum
QgsMapLayer.VectorLayer = QgsMapLayerType.VectorLayer
QgsMapLayer.VectorLayer.__doc__ = ""
QgsMapLayer.RasterLayer = QgsMapLayerType.RasterLayer
QgsMapLayer.RasterLayer.__doc__ = ""
QgsMapLayer.PluginLayer = QgsMapLayerType.PluginLayer
QgsMapLayer.PluginLayer.__doc__ = ""
QgsMapLayer.MeshLayer = QgsMapLayerType.MeshLayer
QgsMapLayer.MeshLayer.__doc__ = "Added in 3.2"
QgsMapLayer.VectorTileLayer = QgsMapLayerType.VectorTileLayer
QgsMapLayer.VectorTileLayer.__doc__ = "Added in 3.14"
QgsMapLayer.AnnotationLayer = QgsMapLayerType.AnnotationLayer
QgsMapLayer.AnnotationLayer.__doc__ = "Contains freeform, georeferenced annotations. Added in QGIS 3.16"
QgsMapLayer.PointCloudLayer = QgsMapLayerType.PointCloudLayer
QgsMapLayer.PointCloudLayer.__doc__ = "Added in 3.18"
QgsMapLayerType.__doc__ = 'Types of layers that can be added to a map\n\n.. versionadded:: 3.8\n\n' + '* ``VectorLayer``: ' + QgsMapLayerType.VectorLayer.__doc__ + '\n' + '* ``RasterLayer``: ' + QgsMapLayerType.RasterLayer.__doc__ + '\n' + '* ``PluginLayer``: ' + QgsMapLayerType.PluginLayer.__doc__ + '\n' + '* ``MeshLayer``: ' + QgsMapLayerType.MeshLayer.__doc__ + '\n' + '* ``VectorTileLayer``: ' + QgsMapLayerType.VectorTileLayer.__doc__ + '\n' + '* ``AnnotationLayer``: ' + QgsMapLayerType.AnnotationLayer.__doc__ + '\n' + '* ``PointCloudLayer``: ' + QgsMapLayerType.PointCloudLayer.__doc__
# --
Qgis.DataType.baseClass = Qgis
Qgis.PythonMacroMode.baseClass = Qgis
QgsVectorDataProvider.FeatureCountState = Qgis.FeatureCountState
# monkey patching scoped based enum
QgsVectorDataProvider.Uncounted = Qgis.FeatureCountState.Uncounted
QgsVectorDataProvider.Uncounted.__doc__ = "Feature count not yet computed"
QgsVectorDataProvider.UnknownCount = Qgis.FeatureCountState.UnknownCount
QgsVectorDataProvider.UnknownCount.__doc__ = "Provider returned an unknown feature count"
Qgis.FeatureCountState.__doc__ = 'Enumeration of feature count states\n\n.. versionadded:: 3.20\n\n' + '* ``Uncounted``: ' + Qgis.FeatureCountState.Uncounted.__doc__ + '\n' + '* ``UnknownCount``: ' + Qgis.FeatureCountState.UnknownCount.__doc__
# --
QgsSymbol.SymbolType = Qgis.SymbolType
# monkey patching scoped based enum
QgsSymbol.Marker = Qgis.SymbolType.Marker
QgsSymbol.Marker.__doc__ = "Marker symbol"
QgsSymbol.Line = Qgis.SymbolType.Line
QgsSymbol.Line.__doc__ = "Line symbol"
QgsSymbol.Fill = Qgis.SymbolType.Fill
QgsSymbol.Fill.__doc__ = "Fill symbol"
QgsSymbol.Hybrid = Qgis.SymbolType.Hybrid
QgsSymbol.Hybrid.__doc__ = "Hybrid symbol"
Qgis.SymbolType.__doc__ = 'Symbol types\n\n.. versionadded:: 3.20\n\n' + '* ``Marker``: ' + Qgis.SymbolType.Marker.__doc__ + '\n' + '* ``Line``: ' + Qgis.SymbolType.Line.__doc__ + '\n' + '* ``Fill``: ' + Qgis.SymbolType.Fill.__doc__ + '\n' + '* ``Hybrid``: ' + Qgis.SymbolType.Hybrid.__doc__
# --
QgsSymbol.ScaleMethod = Qgis.ScaleMethod
# monkey patching scoped based enum
QgsSymbol.ScaleArea = Qgis.ScaleMethod.ScaleArea
QgsSymbol.ScaleArea.__doc__ = "Calculate scale by the area"
QgsSymbol.ScaleDiameter = Qgis.ScaleMethod.ScaleDiameter
QgsSymbol.ScaleDiameter.__doc__ = "Calculate scale by the diameter"
Qgis.ScaleMethod.__doc__ = 'Scale methods\n\n.. versionadded:: 3.20\n\n' + '* ``ScaleArea``: ' + Qgis.ScaleMethod.ScaleArea.__doc__ + '\n' + '* ``ScaleDiameter``: ' + Qgis.ScaleMethod.ScaleDiameter.__doc__
# --
QgsSymbol.RenderHint = Qgis.SymbolRenderHint
# monkey patching scoped based enum
QgsSymbol.DynamicRotation = Qgis.SymbolRenderHint.DynamicRotation
QgsSymbol.DynamicRotation.__doc__ = "Rotation of symbol may be changed during rendering and symbol should not be cached"
Qgis.SymbolRenderHint.__doc__ = 'Flags controlling behavior of symbols during rendering\n\n.. versionadded:: 3.20\n\n' + '* ``DynamicRotation``: ' + Qgis.SymbolRenderHint.DynamicRotation.__doc__
# --
QgsSymbol.PreviewFlag = Qgis.SymbolPreviewFlag
# monkey patching scoped based enum
QgsSymbol.FlagIncludeCrosshairsForMarkerSymbols = Qgis.SymbolPreviewFlag.FlagIncludeCrosshairsForMarkerSymbols
QgsSymbol.FlagIncludeCrosshairsForMarkerSymbols.__doc__ = "Include a crosshairs reference image in the background of marker symbol previews"
Qgis.SymbolPreviewFlag.__doc__ = 'Flags for controlling how symbol preview images are generated.\n\n.. versionadded:: 3.20\n\n' + '* ``FlagIncludeCrosshairsForMarkerSymbols``: ' + Qgis.SymbolPreviewFlag.FlagIncludeCrosshairsForMarkerSymbols.__doc__
# --
QgsVectorLayerExporter.ExportError = Qgis.VectorExportResult
# monkey patching scoped based enum
QgsVectorLayerExporter.NoError = Qgis.VectorExportResult.Success
QgsVectorLayerExporter.NoError.__doc__ = "No errors were encountered"
QgsVectorLayerExporter.ErrCreateDataSource = Qgis.VectorExportResult.ErrorCreatingDataSource
QgsVectorLayerExporter.ErrCreateDataSource.__doc__ = "Could not create the destination data source"
QgsVectorLayerExporter.ErrCreateLayer = Qgis.VectorExportResult.ErrorCreatingLayer
QgsVectorLayerExporter.ErrCreateLayer.__doc__ = "Could not create destination layer"
QgsVectorLayerExporter.ErrAttributeTypeUnsupported = Qgis.VectorExportResult.ErrorAttributeTypeUnsupported
QgsVectorLayerExporter.ErrAttributeTypeUnsupported.__doc__ = "Source layer has an attribute type which could not be handled by destination"
QgsVectorLayerExporter.ErrAttributeCreationFailed = Qgis.VectorExportResult.ErrorAttributeCreationFailed
QgsVectorLayerExporter.ErrAttributeCreationFailed.__doc__ = "Destination provider was unable to create an attribute"
QgsVectorLayerExporter.ErrProjection = Qgis.VectorExportResult.ErrorProjectingFeatures
QgsVectorLayerExporter.ErrProjection.__doc__ = "An error occurred while reprojecting features to destination CRS"
QgsVectorLayerExporter.ErrFeatureWriteFailed = Qgis.VectorExportResult.ErrorFeatureWriteFailed
QgsVectorLayerExporter.ErrFeatureWriteFailed.__doc__ = "An error occurred while writing a feature to the destination"
QgsVectorLayerExporter.ErrInvalidLayer = Qgis.VectorExportResult.ErrorInvalidLayer
QgsVectorLayerExporter.ErrInvalidLayer.__doc__ = "Could not access newly created destination layer"
QgsVectorLayerExporter.ErrInvalidProvider = Qgis.VectorExportResult.ErrorInvalidProvider
QgsVectorLayerExporter.ErrInvalidProvider.__doc__ = "Could not find a matching provider key"
QgsVectorLayerExporter.ErrProviderUnsupportedFeature = Qgis.VectorExportResult.ErrorProviderUnsupportedFeature
QgsVectorLayerExporter.ErrProviderUnsupportedFeature.__doc__ = "Provider does not support creation of empty layers"
QgsVectorLayerExporter.ErrConnectionFailed = Qgis.VectorExportResult.ErrorConnectionFailed
QgsVectorLayerExporter.ErrConnectionFailed.__doc__ = "Could not connect to destination"
QgsVectorLayerExporter.ErrUserCanceled = Qgis.VectorExportResult.UserCanceled
QgsVectorLayerExporter.ErrUserCanceled.__doc__ = "User canceled the export"
Qgis.VectorExportResult.__doc__ = 'Vector layer export result codes.\n\n.. versionadded:: 3.20\n\n' + '* ``NoError``: ' + Qgis.VectorExportResult.Success.__doc__ + '\n' + '* ``ErrCreateDataSource``: ' + Qgis.VectorExportResult.ErrorCreatingDataSource.__doc__ + '\n' + '* ``ErrCreateLayer``: ' + Qgis.VectorExportResult.ErrorCreatingLayer.__doc__ + '\n' + '* ``ErrAttributeTypeUnsupported``: ' + Qgis.VectorExportResult.ErrorAttributeTypeUnsupported.__doc__ + '\n' + '* ``ErrAttributeCreationFailed``: ' + Qgis.VectorExportResult.ErrorAttributeCreationFailed.__doc__ + '\n' + '* ``ErrProjection``: ' + Qgis.VectorExportResult.ErrorProjectingFeatures.__doc__ + '\n' + '* ``ErrFeatureWriteFailed``: ' + Qgis.VectorExportResult.ErrorFeatureWriteFailed.__doc__ + '\n' + '* ``ErrInvalidLayer``: ' + Qgis.VectorExportResult.ErrorInvalidLayer.__doc__ + '\n' + '* ``ErrInvalidProvider``: ' + Qgis.VectorExportResult.ErrorInvalidProvider.__doc__ + '\n' + '* ``ErrProviderUnsupportedFeature``: ' + Qgis.VectorExportResult.ErrorProviderUnsupportedFeature.__doc__ + '\n' + '* ``ErrConnectionFailed``: ' + Qgis.VectorExportResult.ErrorConnectionFailed.__doc__ + '\n' + '* ``ErrUserCanceled``: ' + Qgis.VectorExportResult.UserCanceled.__doc__
# --
Qgis.VectorExportResult.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DriveType.Unknown.__doc__ = "Unknown type"
Qgis.DriveType.Invalid.__doc__ = "Invalid path"
Qgis.DriveType.Removable.__doc__ = "Removable drive"
Qgis.DriveType.Fixed.__doc__ = "Fixed drive"
Qgis.DriveType.Remote.__doc__ = "Remote drive"
Qgis.DriveType.CdRom.__doc__ = "CD-ROM"
Qgis.DriveType.RamDisk.__doc__ = "RAM disk"
Qgis.DriveType.__doc__ = 'Drive types\n\n.. versionadded:: 3.20\n\n' + '* ``Unknown``: ' + Qgis.DriveType.Unknown.__doc__ + '\n' + '* ``Invalid``: ' + Qgis.DriveType.Invalid.__doc__ + '\n' + '* ``Removable``: ' + Qgis.DriveType.Removable.__doc__ + '\n' + '* ``Fixed``: ' + Qgis.DriveType.Fixed.__doc__ + '\n' + '* ``Remote``: ' + Qgis.DriveType.Remote.__doc__ + '\n' + '* ``CdRom``: ' + Qgis.DriveType.CdRom.__doc__ + '\n' + '* ``RamDisk``: ' + Qgis.DriveType.RamDisk.__doc__
# --
Qgis.DriveType.baseClass = Qgis
