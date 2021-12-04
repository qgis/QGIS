/***************************************************************************
                         qgsalgorithmtinmeshcreation.cpp
                         ---------------------------
    begin                : August 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmtinmeshcreation.h"
#include "qgsfileutils.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsprocessingparametertininputlayers.h"
#include "qgsmeshtriangulation.h"
#include "qgsmeshlayer.h"
#include "qgis.h"

///@cond PRIVATE

QString QgsTinMeshCreationAlgorithm::group() const
{
  return QObject::tr( "Mesh" );
}

QString QgsTinMeshCreationAlgorithm::groupId() const
{
  return QStringLiteral( "mesh" );
}

QString QgsTinMeshCreationAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a TIN mesh layer from vector layers" );
}

QString QgsTinMeshCreationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a TIN mesh layer from vector layers." );
}

QString QgsTinMeshCreationAlgorithm::name() const
{
  return QStringLiteral( "tinmeshcreation" );
}

QString QgsTinMeshCreationAlgorithm::displayName() const
{
  return QObject::tr( "TIN Mesh Creation" );
}

QgsProcessingAlgorithm *QgsTinMeshCreationAlgorithm::createInstance() const
{
  return new QgsTinMeshCreationAlgorithm();
}

void QgsTinMeshCreationAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );
  addParameter( new QgsProcessingParameterTinInputLayers( QStringLiteral( "SOURCE_DATA" ), QObject::tr( "Input layers" ) ) );

  QgsProviderMetadata *meta = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "mdal" ) );

  QList<QgsMeshDriverMetadata> driverList;
  if ( meta )
    driverList = meta->meshDriversMetadata();

  for ( const QgsMeshDriverMetadata &driverMeta : std::as_const( driverList ) )
    if ( driverMeta.capabilities() & QgsMeshDriverMetadata::CanWriteMeshData )
    {
      const QString name = driverMeta.name();
      mDriverSuffix[name] = driverMeta.writeMeshFrameOnFileSuffix();
      mAvailableFormat.append( name );
    }

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "MESH_FORMAT" ), QObject::tr( "Output format" ), mAvailableFormat, false, 0 ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output coordinate system" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_MESH" ), QObject::tr( "Output file" ) ) );
}

bool QgsTinMeshCreationAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QVariant layersVariant = parameters.value( parameterDefinition( QStringLiteral( "SOURCE_DATA" ) )->name() );
  if ( layersVariant.type() != QVariant::List )
    return false;

  const QVariantList layersList = layersVariant.toList();

  QgsCoordinateReferenceSystem destinationCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  if ( !destinationCrs.isValid() && context.project() )
    destinationCrs = context.project()->crs();

  for ( const QVariant &layer : layersList )
  {
    if ( feedback && feedback->isCanceled() )
      return false;

    if ( layer.type() != QVariant::Map )
      continue;
    const QVariantMap layerMap = layer.toMap();
    const QString layerSource = layerMap.value( QStringLiteral( "source" ) ).toString();
    const QgsProcessingParameterTinInputLayers::Type type =
      static_cast<QgsProcessingParameterTinInputLayers::Type>( layerMap.value( QStringLiteral( "type" ) ).toInt() );
    const int attributeIndex = layerMap.value( QStringLiteral( "attributeIndex" ) ).toInt();

    std::unique_ptr<QgsProcessingFeatureSource> featureSource( QgsProcessingUtils::variantToSource( layerSource, context ) );

    if ( !featureSource )
      continue;

    const QgsCoordinateTransform transform( featureSource->sourceCrs(), destinationCrs, context.transformContext() );
    const long long featureCount = featureSource->featureCount();
    switch ( type )
    {
      case QgsProcessingParameterTinInputLayers::Vertices:
        mVerticesLayer.append( {featureSource->getFeatures(), transform, attributeIndex, featureCount} );
        break;
      case QgsProcessingParameterTinInputLayers::BreakLines:
        mBreakLinesLayer.append( {featureSource->getFeatures(), transform, attributeIndex, featureCount} );
        break;
      default:
        break;
    }
  }

  if ( mVerticesLayer.isEmpty() && mBreakLinesLayer.isEmpty() )
    return false;

  return true;
}

QVariantMap QgsTinMeshCreationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMeshTriangulation triangulation;
  QgsCoordinateReferenceSystem destinationCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  if ( !destinationCrs.isValid() && context.project() )
    destinationCrs = context.project()->crs();
  triangulation.setCrs( destinationCrs );

  if ( !mVerticesLayer.isEmpty() && feedback )
    feedback->setProgressText( QObject::tr( "Adding vertices layer(s) to the triangulation" ) );
  for ( Layer &l : mVerticesLayer )
  {
    if ( feedback && feedback->isCanceled() )
      break;
    triangulation.addVertices( l.fit, l.attributeIndex, l.transform, feedback, l.featureCount );
  }

  if ( !mBreakLinesLayer.isEmpty() && feedback )
    feedback->setProgressText( QObject::tr( "Adding break lines layer(s) to the triangulation" ) );
  for ( Layer &l : mBreakLinesLayer )
  {
    if ( feedback && feedback->isCanceled() )
      break;
    triangulation.addBreakLines( l.fit, l.attributeIndex, l.transform, feedback, l.featureCount );
  }

  if ( feedback && feedback->isCanceled() )
    return QVariantMap();

  QString fileName = parameterAsFile( parameters, QStringLiteral( "OUTPUT_MESH" ), context );
  const int driverIndex = parameterAsEnum( parameters, QStringLiteral( "MESH_FORMAT" ), context );
  const QString driver = mAvailableFormat.at( driverIndex );
  if ( feedback )
    feedback->setProgressText( QObject::tr( "Creating mesh from triangulation" ) );
  const QgsMesh mesh = triangulation.triangulatedMesh( feedback );

  if ( feedback && feedback->isCanceled() )
    return QVariantMap();

  const QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "mdal" ) );

  fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, QStringList() << mDriverSuffix.value( driver ) );

  if ( feedback )
    feedback->setProgressText( QObject::tr( "Saving mesh to file" ) );
  if ( providerMetadata )
    providerMetadata->createMeshData( mesh, fileName, driver, destinationCrs );

  context.addLayerToLoadOnCompletion( fileName, QgsProcessingContext::LayerDetails( "TIN Mesh",
                                      context.project(),
                                      "TIN",
                                      QgsProcessingUtils::LayerHint::Mesh ) );

  //SELAFIN format doesn't support saving Z value on mesh vertices, so create a specific dataset group
  if ( driver == "SELAFIN" )
  {
    addZValueDataset( fileName, mesh, driver );
  }

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT_MESH" )] = fileName;

  return ret;
}

void QgsTinMeshCreationAlgorithm::addZValueDataset( const QString &fileName, const QgsMesh &mesh, const QString &driver )
{
  std::unique_ptr<QgsMeshLayer> tempLayer = std::make_unique<QgsMeshLayer>( fileName, "temp", "mdal" );
  QgsMeshZValueDatasetGroup *zValueDatasetGroup = new QgsMeshZValueDatasetGroup( QObject::tr( "Terrain Elevation" ), mesh );
  tempLayer->addDatasets( zValueDatasetGroup );
  const int datasetGroupIndex = tempLayer->datasetGroupCount() - 1;
  tempLayer->saveDataset( fileName, datasetGroupIndex, driver );
}

bool QgsTinMeshCreationAlgorithm::canExecute( QString *errorMessage ) const
{
  if ( mAvailableFormat.count() == 0 )
  {
    *errorMessage = QObject::tr( "MDAL not available" );
    return false;
  }

  return true;
}

///@endcond PRIVATE
