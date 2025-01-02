# The following has been generated automatically from src/core/processing/qgsprocessingparameters.h
try:
    QgsProcessingFeatureSourceDefinition.__attribute_docs__ = {'source': "Source definition. Usually a static property set to a source layer's ID or file name.", 'selectedFeaturesOnly': '``True`` if only selected features in the source should be used by algorithms.', 'featureLimit': 'If set to a value > 0, places a limit on the maximum number of features which will be\nread from the source.\n\n.. versionadded:: 3.14', 'filterExpression': 'Optional expression filter to use for filtering features which will be read from the source.\n\n.. versionadded:: 3.32', 'flags': 'Flags which dictate source behavior.\n\n.. versionadded:: 3.14', 'geometryCheck': 'Geometry check method to apply to this source. This setting is only\nutilized if the :py:class:`Qgis`.ProcessingFeatureSourceDefinitionFlag.OverrideDefaultGeometryCheck is\nset in QgsProcessingFeatureSourceDefinition.flags.\n\n.. versionadded:: 3.14'}
    QgsProcessingFeatureSourceDefinition.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingOutputLayerDefinition.__attribute_docs__ = {'sink': "Sink/layer definition. Usually a static property set to the destination file name for the sink's layer.", 'destinationProject': 'Destination project. Can be set to a :py:class:`QgsProject` instance in which\nto automatically load the resulting sink/layer after completing processing.\nThe default behavior is not to load the result into any project (``None``).', 'destinationName': "Name to use for sink if it's to be loaded into a destination project.", 'createOptions': "Map of optional sink/layer creation options, which\nare passed to the underlying provider when creating new layers. Known options also\ninclude 'fileEncoding', which is used to specify a file encoding to use for created\nfiles."}
    QgsProcessingOutputLayerDefinition.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameters.isDynamic = staticmethod(QgsProcessingParameters.isDynamic)
    QgsProcessingParameters.parameterAsString = staticmethod(QgsProcessingParameters.parameterAsString)
    QgsProcessingParameters.parameterAsExpression = staticmethod(QgsProcessingParameters.parameterAsExpression)
    QgsProcessingParameters.parameterAsDouble = staticmethod(QgsProcessingParameters.parameterAsDouble)
    QgsProcessingParameters.parameterAsInt = staticmethod(QgsProcessingParameters.parameterAsInt)
    QgsProcessingParameters.parameterAsInts = staticmethod(QgsProcessingParameters.parameterAsInts)
    QgsProcessingParameters.parameterAsDateTime = staticmethod(QgsProcessingParameters.parameterAsDateTime)
    QgsProcessingParameters.parameterAsDate = staticmethod(QgsProcessingParameters.parameterAsDate)
    QgsProcessingParameters.parameterAsTime = staticmethod(QgsProcessingParameters.parameterAsTime)
    QgsProcessingParameters.parameterAsEnum = staticmethod(QgsProcessingParameters.parameterAsEnum)
    QgsProcessingParameters.parameterAsEnums = staticmethod(QgsProcessingParameters.parameterAsEnums)
    QgsProcessingParameters.parameterAsEnumString = staticmethod(QgsProcessingParameters.parameterAsEnumString)
    QgsProcessingParameters.parameterAsEnumStrings = staticmethod(QgsProcessingParameters.parameterAsEnumStrings)
    QgsProcessingParameters.parameterAsBool = staticmethod(QgsProcessingParameters.parameterAsBool)
    QgsProcessingParameters.parameterAsBoolean = staticmethod(QgsProcessingParameters.parameterAsBoolean)
    QgsProcessingParameters.parameterAsSink = staticmethod(QgsProcessingParameters.parameterAsSink)
    QgsProcessingParameters.parameterAsSource = staticmethod(QgsProcessingParameters.parameterAsSource)
    QgsProcessingParameters.parameterAsCompatibleSourceLayerPath = staticmethod(QgsProcessingParameters.parameterAsCompatibleSourceLayerPath)
    QgsProcessingParameters.parameterAsCompatibleSourceLayerPathAndLayerName = staticmethod(QgsProcessingParameters.parameterAsCompatibleSourceLayerPathAndLayerName)
    QgsProcessingParameters.parameterAsLayer = staticmethod(QgsProcessingParameters.parameterAsLayer)
    QgsProcessingParameters.parameterAsRasterLayer = staticmethod(QgsProcessingParameters.parameterAsRasterLayer)
    QgsProcessingParameters.parameterAsOutputLayer = staticmethod(QgsProcessingParameters.parameterAsOutputLayer)
    QgsProcessingParameters.parameterAsFileOutput = staticmethod(QgsProcessingParameters.parameterAsFileOutput)
    QgsProcessingParameters.parameterAsVectorLayer = staticmethod(QgsProcessingParameters.parameterAsVectorLayer)
    QgsProcessingParameters.parameterAsMeshLayer = staticmethod(QgsProcessingParameters.parameterAsMeshLayer)
    QgsProcessingParameters.parameterAsCrs = staticmethod(QgsProcessingParameters.parameterAsCrs)
    QgsProcessingParameters.parameterAsExtent = staticmethod(QgsProcessingParameters.parameterAsExtent)
    QgsProcessingParameters.parameterAsExtentGeometry = staticmethod(QgsProcessingParameters.parameterAsExtentGeometry)
    QgsProcessingParameters.parameterAsExtentCrs = staticmethod(QgsProcessingParameters.parameterAsExtentCrs)
    QgsProcessingParameters.parameterAsPoint = staticmethod(QgsProcessingParameters.parameterAsPoint)
    QgsProcessingParameters.parameterAsPointCrs = staticmethod(QgsProcessingParameters.parameterAsPointCrs)
    QgsProcessingParameters.parameterAsGeometry = staticmethod(QgsProcessingParameters.parameterAsGeometry)
    QgsProcessingParameters.parameterAsGeometryCrs = staticmethod(QgsProcessingParameters.parameterAsGeometryCrs)
    QgsProcessingParameters.parameterAsFile = staticmethod(QgsProcessingParameters.parameterAsFile)
    QgsProcessingParameters.parameterAsMatrix = staticmethod(QgsProcessingParameters.parameterAsMatrix)
    QgsProcessingParameters.parameterAsLayerList = staticmethod(QgsProcessingParameters.parameterAsLayerList)
    QgsProcessingParameters.parameterAsFileList = staticmethod(QgsProcessingParameters.parameterAsFileList)
    QgsProcessingParameters.parameterAsRange = staticmethod(QgsProcessingParameters.parameterAsRange)
    QgsProcessingParameters.parameterAsFields = staticmethod(QgsProcessingParameters.parameterAsFields)
    QgsProcessingParameters.parameterAsStrings = staticmethod(QgsProcessingParameters.parameterAsStrings)
    QgsProcessingParameters.parameterAsLayout = staticmethod(QgsProcessingParameters.parameterAsLayout)
    QgsProcessingParameters.parameterAsLayoutItem = staticmethod(QgsProcessingParameters.parameterAsLayoutItem)
    QgsProcessingParameters.parameterAsColor = staticmethod(QgsProcessingParameters.parameterAsColor)
    QgsProcessingParameters.parameterAsConnectionName = staticmethod(QgsProcessingParameters.parameterAsConnectionName)
    QgsProcessingParameters.parameterAsSchema = staticmethod(QgsProcessingParameters.parameterAsSchema)
    QgsProcessingParameters.parameterAsDatabaseTableName = staticmethod(QgsProcessingParameters.parameterAsDatabaseTableName)
    QgsProcessingParameters.parameterAsPointCloudLayer = staticmethod(QgsProcessingParameters.parameterAsPointCloudLayer)
    QgsProcessingParameters.parameterAsAnnotationLayer = staticmethod(QgsProcessingParameters.parameterAsAnnotationLayer)
    QgsProcessingParameters.parameterFromVariantMap = staticmethod(QgsProcessingParameters.parameterFromVariantMap)
    QgsProcessingParameters.descriptionFromName = staticmethod(QgsProcessingParameters.descriptionFromName)
    QgsProcessingParameters.parameterFromScriptCode = staticmethod(QgsProcessingParameters.parameterFromScriptCode)
    QgsProcessingParameters.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterBoolean.typeName = staticmethod(QgsProcessingParameterBoolean.typeName)
    QgsProcessingParameterBoolean.fromScriptCode = staticmethod(QgsProcessingParameterBoolean.fromScriptCode)
    QgsProcessingParameterBoolean.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterCrs.typeName = staticmethod(QgsProcessingParameterCrs.typeName)
    QgsProcessingParameterCrs.fromScriptCode = staticmethod(QgsProcessingParameterCrs.fromScriptCode)
    QgsProcessingParameterCrs.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterExtent.typeName = staticmethod(QgsProcessingParameterExtent.typeName)
    QgsProcessingParameterExtent.fromScriptCode = staticmethod(QgsProcessingParameterExtent.fromScriptCode)
    QgsProcessingParameterExtent.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterPoint.typeName = staticmethod(QgsProcessingParameterPoint.typeName)
    QgsProcessingParameterPoint.fromScriptCode = staticmethod(QgsProcessingParameterPoint.fromScriptCode)
    QgsProcessingParameterPoint.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterGeometry.typeName = staticmethod(QgsProcessingParameterGeometry.typeName)
    QgsProcessingParameterGeometry.fromScriptCode = staticmethod(QgsProcessingParameterGeometry.fromScriptCode)
    QgsProcessingParameterGeometry.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterFile.typeName = staticmethod(QgsProcessingParameterFile.typeName)
    QgsProcessingParameterFile.fromScriptCode = staticmethod(QgsProcessingParameterFile.fromScriptCode)
    QgsProcessingParameterFile.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterMatrix.typeName = staticmethod(QgsProcessingParameterMatrix.typeName)
    QgsProcessingParameterMatrix.fromScriptCode = staticmethod(QgsProcessingParameterMatrix.fromScriptCode)
    QgsProcessingParameterMatrix.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterMultipleLayers.typeName = staticmethod(QgsProcessingParameterMultipleLayers.typeName)
    QgsProcessingParameterMultipleLayers.fromScriptCode = staticmethod(QgsProcessingParameterMultipleLayers.fromScriptCode)
    QgsProcessingParameterMultipleLayers.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterNumber.typeName = staticmethod(QgsProcessingParameterNumber.typeName)
    QgsProcessingParameterNumber.fromScriptCode = staticmethod(QgsProcessingParameterNumber.fromScriptCode)
    QgsProcessingParameterNumber.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterDistance.typeName = staticmethod(QgsProcessingParameterDistance.typeName)
    QgsProcessingParameterDistance.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterArea.typeName = staticmethod(QgsProcessingParameterArea.typeName)
    QgsProcessingParameterArea.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterVolume.typeName = staticmethod(QgsProcessingParameterVolume.typeName)
    QgsProcessingParameterVolume.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterDuration.typeName = staticmethod(QgsProcessingParameterDuration.typeName)
    QgsProcessingParameterDuration.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterScale.typeName = staticmethod(QgsProcessingParameterScale.typeName)
    QgsProcessingParameterScale.fromScriptCode = staticmethod(QgsProcessingParameterScale.fromScriptCode)
    QgsProcessingParameterScale.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterRange.typeName = staticmethod(QgsProcessingParameterRange.typeName)
    QgsProcessingParameterRange.fromScriptCode = staticmethod(QgsProcessingParameterRange.fromScriptCode)
    QgsProcessingParameterRange.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterRasterLayer.typeName = staticmethod(QgsProcessingParameterRasterLayer.typeName)
    QgsProcessingParameterRasterLayer.fromScriptCode = staticmethod(QgsProcessingParameterRasterLayer.fromScriptCode)
    QgsProcessingParameterRasterLayer.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterEnum.typeName = staticmethod(QgsProcessingParameterEnum.typeName)
    QgsProcessingParameterEnum.fromScriptCode = staticmethod(QgsProcessingParameterEnum.fromScriptCode)
    QgsProcessingParameterEnum.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterString.typeName = staticmethod(QgsProcessingParameterString.typeName)
    QgsProcessingParameterString.fromScriptCode = staticmethod(QgsProcessingParameterString.fromScriptCode)
    QgsProcessingParameterString.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterAuthConfig.typeName = staticmethod(QgsProcessingParameterAuthConfig.typeName)
    QgsProcessingParameterAuthConfig.fromScriptCode = staticmethod(QgsProcessingParameterAuthConfig.fromScriptCode)
    QgsProcessingParameterAuthConfig.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterExpression.typeName = staticmethod(QgsProcessingParameterExpression.typeName)
    QgsProcessingParameterExpression.fromScriptCode = staticmethod(QgsProcessingParameterExpression.fromScriptCode)
    QgsProcessingParameterExpression.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterVectorLayer.typeName = staticmethod(QgsProcessingParameterVectorLayer.typeName)
    QgsProcessingParameterVectorLayer.fromScriptCode = staticmethod(QgsProcessingParameterVectorLayer.fromScriptCode)
    QgsProcessingParameterVectorLayer.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterMeshLayer.typeName = staticmethod(QgsProcessingParameterMeshLayer.typeName)
    QgsProcessingParameterMeshLayer.fromScriptCode = staticmethod(QgsProcessingParameterMeshLayer.fromScriptCode)
    QgsProcessingParameterMeshLayer.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterMapLayer.typeName = staticmethod(QgsProcessingParameterMapLayer.typeName)
    QgsProcessingParameterMapLayer.fromScriptCode = staticmethod(QgsProcessingParameterMapLayer.fromScriptCode)
    QgsProcessingParameterMapLayer.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterField.typeName = staticmethod(QgsProcessingParameterField.typeName)
    QgsProcessingParameterField.fromScriptCode = staticmethod(QgsProcessingParameterField.fromScriptCode)
    QgsProcessingParameterField.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterFeatureSource.typeName = staticmethod(QgsProcessingParameterFeatureSource.typeName)
    QgsProcessingParameterFeatureSource.fromScriptCode = staticmethod(QgsProcessingParameterFeatureSource.fromScriptCode)
    QgsProcessingParameterFeatureSource.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterFeatureSink.typeName = staticmethod(QgsProcessingParameterFeatureSink.typeName)
    QgsProcessingParameterFeatureSink.fromScriptCode = staticmethod(QgsProcessingParameterFeatureSink.fromScriptCode)
    QgsProcessingParameterFeatureSink.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterVectorDestination.typeName = staticmethod(QgsProcessingParameterVectorDestination.typeName)
    QgsProcessingParameterVectorDestination.fromScriptCode = staticmethod(QgsProcessingParameterVectorDestination.fromScriptCode)
    QgsProcessingParameterVectorDestination.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterRasterDestination.typeName = staticmethod(QgsProcessingParameterRasterDestination.typeName)
    QgsProcessingParameterRasterDestination.fromScriptCode = staticmethod(QgsProcessingParameterRasterDestination.fromScriptCode)
    QgsProcessingParameterRasterDestination.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterFileDestination.typeName = staticmethod(QgsProcessingParameterFileDestination.typeName)
    QgsProcessingParameterFileDestination.fromScriptCode = staticmethod(QgsProcessingParameterFileDestination.fromScriptCode)
    QgsProcessingParameterFileDestination.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterFolderDestination.typeName = staticmethod(QgsProcessingParameterFolderDestination.typeName)
    QgsProcessingParameterFolderDestination.fromScriptCode = staticmethod(QgsProcessingParameterFolderDestination.fromScriptCode)
    QgsProcessingParameterFolderDestination.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterBand.typeName = staticmethod(QgsProcessingParameterBand.typeName)
    QgsProcessingParameterBand.fromScriptCode = staticmethod(QgsProcessingParameterBand.fromScriptCode)
    QgsProcessingParameterBand.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterLayout.typeName = staticmethod(QgsProcessingParameterLayout.typeName)
    QgsProcessingParameterLayout.fromScriptCode = staticmethod(QgsProcessingParameterLayout.fromScriptCode)
    QgsProcessingParameterLayout.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterLayoutItem.typeName = staticmethod(QgsProcessingParameterLayoutItem.typeName)
    QgsProcessingParameterLayoutItem.fromScriptCode = staticmethod(QgsProcessingParameterLayoutItem.fromScriptCode)
    QgsProcessingParameterLayoutItem.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterColor.typeName = staticmethod(QgsProcessingParameterColor.typeName)
    QgsProcessingParameterColor.fromScriptCode = staticmethod(QgsProcessingParameterColor.fromScriptCode)
    QgsProcessingParameterColor.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterCoordinateOperation.typeName = staticmethod(QgsProcessingParameterCoordinateOperation.typeName)
    QgsProcessingParameterCoordinateOperation.fromScriptCode = staticmethod(QgsProcessingParameterCoordinateOperation.fromScriptCode)
    QgsProcessingParameterCoordinateOperation.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterMapTheme.typeName = staticmethod(QgsProcessingParameterMapTheme.typeName)
    QgsProcessingParameterMapTheme.fromScriptCode = staticmethod(QgsProcessingParameterMapTheme.fromScriptCode)
    QgsProcessingParameterMapTheme.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterDateTime.typeName = staticmethod(QgsProcessingParameterDateTime.typeName)
    QgsProcessingParameterDateTime.fromScriptCode = staticmethod(QgsProcessingParameterDateTime.fromScriptCode)
    QgsProcessingParameterDateTime.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterProviderConnection.typeName = staticmethod(QgsProcessingParameterProviderConnection.typeName)
    QgsProcessingParameterProviderConnection.fromScriptCode = staticmethod(QgsProcessingParameterProviderConnection.fromScriptCode)
    QgsProcessingParameterProviderConnection.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterDatabaseSchema.typeName = staticmethod(QgsProcessingParameterDatabaseSchema.typeName)
    QgsProcessingParameterDatabaseSchema.fromScriptCode = staticmethod(QgsProcessingParameterDatabaseSchema.fromScriptCode)
    QgsProcessingParameterDatabaseSchema.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterDatabaseTable.typeName = staticmethod(QgsProcessingParameterDatabaseTable.typeName)
    QgsProcessingParameterDatabaseTable.fromScriptCode = staticmethod(QgsProcessingParameterDatabaseTable.fromScriptCode)
    QgsProcessingParameterDatabaseTable.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterPointCloudLayer.typeName = staticmethod(QgsProcessingParameterPointCloudLayer.typeName)
    QgsProcessingParameterPointCloudLayer.fromScriptCode = staticmethod(QgsProcessingParameterPointCloudLayer.fromScriptCode)
    QgsProcessingParameterPointCloudLayer.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterAnnotationLayer.typeName = staticmethod(QgsProcessingParameterAnnotationLayer.typeName)
    QgsProcessingParameterAnnotationLayer.fromScriptCode = staticmethod(QgsProcessingParameterAnnotationLayer.fromScriptCode)
    QgsProcessingParameterAnnotationLayer.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterPointCloudDestination.typeName = staticmethod(QgsProcessingParameterPointCloudDestination.typeName)
    QgsProcessingParameterPointCloudDestination.fromScriptCode = staticmethod(QgsProcessingParameterPointCloudDestination.fromScriptCode)
    QgsProcessingParameterPointCloudDestination.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterPointCloudAttribute.typeName = staticmethod(QgsProcessingParameterPointCloudAttribute.typeName)
    QgsProcessingParameterPointCloudAttribute.fromScriptCode = staticmethod(QgsProcessingParameterPointCloudAttribute.fromScriptCode)
    QgsProcessingParameterPointCloudAttribute.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterVectorTileDestination.typeName = staticmethod(QgsProcessingParameterVectorTileDestination.typeName)
    QgsProcessingParameterVectorTileDestination.fromScriptCode = staticmethod(QgsProcessingParameterVectorTileDestination.fromScriptCode)
    QgsProcessingParameterVectorTileDestination.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterDefinition.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterLimitedDataTypes.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingDestinationParameter.__group__ = ['processing']
except (NameError, AttributeError):
    pass
