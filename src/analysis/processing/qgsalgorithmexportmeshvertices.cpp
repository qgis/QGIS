/***************************************************************************
                         qgsalgorithmtinmeshcreation.h
                         ---------------------------
    begin                : October 2020
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

#include "qgsalgorithmexportmeshvertices.h"
#include "qgsprocessingparametermeshdataset.h"
#include "qgsmeshdataset.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshlayertemporalproperties.h"

QString QgsExportMeshVerticesAlgorithm::group() const
{
  return QObject::tr( "Mesh" );
}

QString QgsExportMeshVerticesAlgorithm::groupId() const
{
  return QStringLiteral( "mesh" );
}

QString QgsExportMeshVerticesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Export mesh's vertices to a point vector layer with dataset values on vertices as attribute values" );
}

QString QgsExportMeshVerticesAlgorithm::name() const
{
  return QStringLiteral( "exportmeshvertices" );
}

QString QgsExportMeshVerticesAlgorithm::displayName() const
{
  return QObject::tr( "Export Mesh Vertices" );
}

QgsProcessingAlgorithm *QgsExportMeshVerticesAlgorithm::createInstance() const
{
  return new QgsExportMeshVerticesAlgorithm();
}

void QgsExportMeshVerticesAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT_LAYER" ), QObject::tr( "Input Mesh Layer" ) ) );


  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset Groups" ),
                  QStringLiteral( "INPUT_LAYER" ),
                  QgsMeshDatasetGroupMetadata::DataOnVertices ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "DATASET_TIME" ),
                  QObject::tr( "Dataset Time" ),
                  QStringLiteral( "INPUT_LAYER" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output Coordinate System" ), QVariant(), true ) );

  QStringList exportVectorOptions;
  exportVectorOptions << QObject::tr( "Cartesian (x,y)" )
                      << QObject::tr( "Polar (magnitude,degree)" )
                      << QObject::tr( "Cartesian and Polar" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "VECTOR_OPTION" ), QObject::tr( "Export Vector Option" ), exportVectorOptions, false, 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Output Vector Layer" ), QgsProcessing::TypeVectorPoint ) );
}

static QVector<double> vectorValue( const QgsMeshDatasetValue &value, int exportOption )
{
  QVector<double> ret;
  if ( exportOption == 0 || exportOption == 2 )
  {
    ret.append( value.x() );
    ret.append( value.y() );
  }
  if ( exportOption == 1 || exportOption == 2 )
  {
    double x = value.x();
    double y = value.y();
    double magnitude = sqrt( x * x + y * y );
    double direction = ( asin( x / magnitude ) ) / M_PI * 180;
    if ( y < 0 )
      direction = 180 - direction;
    ret.append( magnitude );
    ret.append( direction );
  }
  return ret;
}

bool QgsExportMeshVerticesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMeshLayer *meshLayer = parameterAsMeshLayer( parameters, QStringLiteral( "INPUT_LAYER" ), context );

  if ( !meshLayer || !meshLayer->isValid() )
    return false;

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  mTransform = QgsCoordinateTransform( meshLayer->crs(), outputCrs, context.transformContext() );
  if ( !meshLayer->nativeMesh() )
    meshLayer->updateTriangularMesh( mTransform ); //necessary to load the native mesh

  mNativeMesh = *meshLayer->nativeMesh();

  QVariant datasetGroupIndexVariant = parameters[QStringLiteral( "DATASET_GROUPS" )];

  if ( datasetGroupIndexVariant.type() != QVariant::List )
    return false;

  QVariantList datasetGroupsList = datasetGroupIndexVariant.toList();

  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return false;
    feedback->setProgress( 0 );
    feedback->setProgressText( QObject::tr( "Preparing data" ) );
  }

  QDateTime layerReferenceTime = static_cast<QgsMeshLayerTemporalProperties *>( meshLayer->temporalProperties() )->referenceTime();

  //! Extract the date time used to export dataset values under a relative time
  QgsInterval relativeTime;
  QVariant parameterTimeVariant = parameters.value( QStringLiteral( "DATASET_TIME" ) );
  if ( !parameterTimeVariant.isValid() || parameterTimeVariant.type() != QVariant::Map )
    return false;

  QVariantMap parameterTimeMap = parameterTimeVariant.toMap();
  if ( !parameterTimeMap.contains( QStringLiteral( "type" ) ) )
    return false;

  if ( parameterTimeMap.value( QStringLiteral( "type" ) ) == QStringLiteral( "static" ) )
  {
    relativeTime = 0;
  }
  else if ( parameterTimeMap.value( QStringLiteral( "type" ) ) == QStringLiteral( "dataset-time-step" ) )
  {
    QVariant datasetIndexVariant = parameterTimeMap.value( QStringLiteral( "value" ) );
    if ( datasetGroupIndexVariant.type() != QVariant::List )
      return false;
    QVariantList datasetIndexVariantSplit = datasetIndexVariant.toList();
    if ( datasetIndexVariantSplit.count() != 2 )
      return false;
    QgsMeshDatasetIndex datasetIndex( datasetIndexVariantSplit.at( 0 ).toInt(), datasetIndexVariantSplit.at( 1 ).toInt() );
    relativeTime = meshLayer->datasetRelativeTime( datasetIndex );
  }
  else
  {
    QVariant dateTimeVariant = parameterTimeMap.value( QStringLiteral( "value" ) );
    if ( dateTimeVariant.type() != QVariant::DateTime )
      return false;
    QDateTime dateTime = dateTimeVariant.toDateTime();
    relativeTime = QgsInterval( layerReferenceTime.secsTo( dateTime ) );
  }

  for ( int i = 0; i < datasetGroupsList.count(); ++i )
  {
    int  groupIndex = datasetGroupsList.at( i ).toInt();
    QgsMeshDatasetIndex datasetIndex = meshLayer->datasetIndexAtRelativeTime( relativeTime, groupIndex );

    DataGroup dataGroup;
    dataGroup.metadata = meshLayer->datasetGroupMetadata( datasetIndex );
    int dataCount = dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ? mNativeMesh.vertexCount() : mNativeMesh.faceCount();
    dataGroup.datasetValues = meshLayer->datasetValues( datasetIndex, 0, dataCount );
    dataGroup.activeFaceFlagValues = meshLayer->areFacesActive( datasetIndex, 0, mNativeMesh.faceCount() );

    mDataPerGroup.append( dataGroup );
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        feedback->setProgress( 100 * i / datasetGroupsList.count() );
    }
  }

  mExportVectorOption = parameterAsInt( parameters, QStringLiteral( "VECTOR_OPTION" ), context );

  return true;
}

QVariantMap QgsExportMeshVerticesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    feedback->setProgress( 0 );
    feedback->setProgressText( QObject::tr( "Creating output vector layer" ) );
  }

  QgsFields fields;
  for ( int i = 0; i < mDataPerGroup.count(); ++i )
  {
    const DataGroup &dataGroup = mDataPerGroup.at( i );
    if ( dataGroup.metadata.isVector() )
    {
      if ( mExportVectorOption == 0 or mExportVectorOption == 2 )
      {
        fields.append( QString( "%1_x" ).arg( dataGroup.metadata.name() ) );
        fields.append( QString( "%1_y" ).arg( dataGroup.metadata.name() ) );
      }

      if ( mExportVectorOption == 1 or mExportVectorOption == 2 )
      {
        fields.append( QString( "%1_mag" ).arg( dataGroup.metadata.name() ) );
        fields.append( QString( "%1_dir" ).arg( dataGroup.metadata.name() ) );
      }
    }
    else
      fields.append( dataGroup.metadata.name() );
  }

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  QString identifier;
  QgsFeatureSink *sink = parameterAsSink( parameters,
                                          QStringLiteral( "OUTPUT_LAYER" ),
                                          context,
                                          identifier,
                                          fields,
                                          QgsWkbTypes::PointZ,
                                          outputCrs );
  if ( !sink )
    return QVariantMap();

  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    feedback->setProgress( 0 );
    feedback->setProgressText( QObject::tr( "Creating points for each vertices" ) );
  }

  int verticesCount = mNativeMesh.vertexCount();
  for ( int i = 0; i < verticesCount; ++i )
  {
    QgsAttributes attributes;
    for ( const DataGroup &dataGroup : mDataPerGroup )
    {
      const QgsMeshDatasetGroupMetadata &meta = dataGroup.metadata;
      const QgsMeshDatasetValue &value = dataGroup.datasetValues.value( i );
      if ( meta.isVector() )
      {
        QVector<double> vector = vectorValue( dataGroup.datasetValues.value( i ), mExportVectorOption );
        for ( double value : vector )
        {
          attributes.append( value );
        }
      }
      else
        attributes.append( value.scalar() );
    }

    QgsFeature feat;
    QgsGeometry geom( new QgsPoint( mNativeMesh.vertex( i ) ) );
    try
    {
      geom.transform( mTransform );
    }
    catch ( QgsCsException &e )
    {
      geom = QgsGeometry( new QgsPoint( mNativeMesh.vertex( i ) ) );
    }
    feat.setGeometry( geom );
    feat.setAttributes( attributes );

    sink->addFeature( feat );

    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();
      feedback->setProgress( 100 * i / verticesCount );
    }
  }

  const QString fileName = parameterAsFile( parameters, QStringLiteral( "OUTPUT_LAYER" ), context );

//  context.addLayerToLoadOnCompletion( fileName, QgsProcessingContext::LayerDetails( QObject::tr( "Mesh Vertices" ),
//                                      context.project(),
//                                      QStringLiteral( "OUTPUT_LAYER" ),
//                                      QgsProcessingUtils::LayerHint::Vector ) );

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = identifier;

  return ret;
}
