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

///@cond PRIVATE

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
  return QObject::tr( "Exports a mesh layer's vertices to a point vector layer, with the dataset values on vertices as attribute values" );
}

QString QgsExportMeshVerticesAlgorithm::name() const
{
  return QStringLiteral( "exportmeshvertices" );
}

QString QgsExportMeshVerticesAlgorithm::displayName() const
{
  return QObject::tr( "Export mesh vertices" );
}

QgsProcessingAlgorithm *QgsExportMeshVerticesAlgorithm::createInstance() const
{
  return new QgsExportMeshVerticesAlgorithm();
}

void QgsExportMeshVerticesAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input Mesh Layer" ) ) );


  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset groups" ),
                  QStringLiteral( "INPUT" ),
                  QgsMeshDatasetGroupMetadata::DataOnVertices ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "DATASET_TIME" ),
                  QObject::tr( "Dataset time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output coordinate system" ), QVariant(), true ) );

  QStringList exportVectorOptions;
  exportVectorOptions << QObject::tr( "Cartesian (x,y)" )
                      << QObject::tr( "Polar (magnitude,degree)" )
                      << QObject::tr( "Cartesian and Polar" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "VECTOR_OPTION" ), QObject::tr( "Export vector option" ), exportVectorOptions, false, 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output vector layer" ), QgsProcessing::TypeVectorPoint ) );
}

static QVector<double> vectorValue( const QgsMeshDatasetValue &value, int exportOption )
{
  QVector<double> ret( exportOption == 2 ? 4 : 2 );

  if ( exportOption == 0 || exportOption == 2 )
  {
    ret[0] = value.x();
    ret[1] = value.y();
  }
  if ( exportOption == 1 || exportOption == 2 )
  {
    double x = value.x();
    double y = value.y();
    double magnitude = sqrt( x * x + y * y );
    double direction = ( asin( x / magnitude ) ) / M_PI * 180;
    if ( y < 0 )
      direction = 180 - direction;

    if ( exportOption == 1 )
    {
      ret[0] = magnitude;
      ret[1] = direction;
    }
    if ( exportOption == 2 )
    {
      ret[2] = magnitude;
      ret[3] = direction;
    }
  }
  return ret;
}

bool QgsExportMeshVerticesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMeshLayer *meshLayer = parameterAsMeshLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !meshLayer || !meshLayer->isValid() )
    return false;

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  mTransform = QgsCoordinateTransform( meshLayer->crs(), outputCrs, context.transformContext() );
  if ( !meshLayer->nativeMesh() )
    meshLayer->updateTriangularMesh( mTransform ); //necessary to load the native mesh

  mNativeMesh = *meshLayer->nativeMesh();

  QList<int> datasetGroups =
    QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( parameters.value( QStringLiteral( "DATASET_GROUPS" ) ) );

  if ( feedback )
  {
    feedback->setProgressText( QObject::tr( "Preparing data" ) );
  }

  QDateTime layerReferenceTime = static_cast<QgsMeshLayerTemporalProperties *>( meshLayer->temporalProperties() )->referenceTime();

  //! Extract the date time used to export dataset values under a relative time
  QgsInterval relativeTime( 0 );
  QVariant parameterTimeVariant = parameters.value( QStringLiteral( "DATASET_TIME" ) );

  QString timeType = QgsProcessingParameterMeshDatasetTime::valueAsTimeType( parameterTimeVariant );

  if ( timeType == QStringLiteral( "dataset-time-step" ) )
  {
    QgsMeshDatasetIndex datasetIndex = QgsProcessingParameterMeshDatasetTime::timeValueAsDatasetIndex( parameterTimeVariant );
    relativeTime = meshLayer->datasetRelativeTime( datasetIndex );
  }
  else if ( timeType == QStringLiteral( "defined-date-time" ) )
  {
    QDateTime dateTime = QgsProcessingParameterMeshDatasetTime::timeValueAsDefinedDateTime( parameterTimeVariant );
    if ( dateTime.isValid() )
      relativeTime = QgsInterval( layerReferenceTime.secsTo( dateTime ) );
  }
  else if ( timeType == QStringLiteral( "current-context-time" ) )
  {
    QDateTime dateTime = context.currentTimeRange().begin();
    if ( dateTime.isValid() )
      relativeTime = QgsInterval( layerReferenceTime.secsTo( dateTime ) );
  }

  for ( int i = 0; i < datasetGroups.count(); ++i )
  {
    int  groupIndex = datasetGroups.at( i );
    QgsMeshDatasetIndex datasetIndex = meshLayer->datasetIndexAtRelativeTime( relativeTime, groupIndex );

    DataGroup dataGroup;
    dataGroup.metadata = meshLayer->datasetGroupMetadata( datasetIndex );
    int dataCount = dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ? mNativeMesh.vertexCount() : mNativeMesh.faceCount();
    dataGroup.datasetValues = meshLayer->datasetValues( datasetIndex, 0, dataCount );
    dataGroup.activeFaceFlagValues = meshLayer->areFacesActive( datasetIndex, 0, mNativeMesh.faceCount() );

    mDataPerGroup.append( dataGroup );
    if ( feedback )
      feedback->setProgress( 100 * i / datasetGroups.count() );
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
        fields.append( QStringLiteral( "%1_x" ).arg( dataGroup.metadata.name() ) );
        fields.append( QStringLiteral( "%1_y" ).arg( dataGroup.metadata.name() ) );
      }

      if ( mExportVectorOption == 1 or mExportVectorOption == 2 )
      {
        fields.append( QStringLiteral( "%1_mag" ).arg( dataGroup.metadata.name() ) );
        fields.append( QStringLiteral( "%1_dir" ).arg( dataGroup.metadata.name() ) );
      }
    }
    else
      fields.append( dataGroup.metadata.name() );
  }

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  QString identifier;
  QgsFeatureSink *sink = parameterAsSink( parameters,
                                          QStringLiteral( "OUTPUT" ),
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
      feedback->reportError( QObject::tr( "Could not transform point to destination CRS" ) );
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

  const QString fileName = parameterAsFile( parameters, QStringLiteral( "OUTPUT" ), context );

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = identifier;

  return ret;
}

///@endcond PRIVATE
