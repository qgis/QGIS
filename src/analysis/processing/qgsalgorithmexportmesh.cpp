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

#include "qgsalgorithmexportmesh.h"
#include "qgsprocessingparametermeshdataset.h"
#include "qgsmeshdataset.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgspolygon.h"
#include "qgslinestring.h"

///@cond PRIVATE


static QgsFields createFields( const QList<QgsMeshDatasetGroupMetadata> &groupMetadataList, int vectorOption )
{
  QgsFields fields;
  for ( const QgsMeshDatasetGroupMetadata &meta : groupMetadataList )
  {
    if ( meta.isVector() )
    {
      if ( vectorOption == 0 or vectorOption == 2 )
      {
        fields.append( QStringLiteral( "%1_x" ).arg( meta.name() ) );
        fields.append( QStringLiteral( "%1_y" ).arg( meta.name() ) );
      }

      if ( vectorOption == 1 or vectorOption == 2 )
      {
        fields.append( QStringLiteral( "%1_mag" ).arg( meta.name() ) );
        fields.append( QStringLiteral( "%1_dir" ).arg( meta.name() ) );
      }
    }
    else
      fields.append( meta.name() );
  }
  return fields;
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

static void addAttributes( const QgsMeshDatasetValue &value, QgsAttributes &attributes, bool isVector, int vectorOption )
{
  if ( isVector )
  {
    QVector<double> vectorValues = vectorValue( value, vectorOption );
    for ( double v : vectorValues )
    {
      if ( v == std::numeric_limits<double>::quiet_NaN() )
        attributes.append( QVariant() );
      else
        attributes.append( v );
    }
  }
  else
  {
    if ( value.scalar() == std::numeric_limits<double>::quiet_NaN() )
      attributes.append( QVariant() );
    else
      attributes.append( value.scalar() );
  }
}

QString QgsExportMeshOnElement::group() const
{
  return QObject::tr( "Mesh" );
}

QString QgsExportMeshOnElement::groupId() const
{
  return QStringLiteral( "mesh" );
}

QString QgsExportMeshVerticesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Exports mesh layer's vertices to a point vector layer, with the dataset values on vertices as attribute values" );
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

QgsGeometry QgsExportMeshVerticesAlgorithm::meshElement( int index ) const
{
  return QgsGeometry( new QgsPoint( mNativeMesh.vertex( index ) ) );
}

void QgsExportMeshOnElement::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input Mesh Layer" ) ) );


  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset Groups" ),
                  QStringLiteral( "INPUT" ),
                  supportedDataType(), true ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "DATASET_TIME" ),
                  QObject::tr( "Dataset Time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output Coordinate System" ), QVariant(), true ) );

  QStringList exportVectorOptions;
  exportVectorOptions << QObject::tr( "Cartesian (x,y)" )
                      << QObject::tr( "Polar (magnitude,degree)" )
                      << QObject::tr( "Cartesian and Polar" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "VECTOR_OPTION" ), QObject::tr( "Export Vector Option" ), exportVectorOptions, false, 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output Vector Layer" ), sinkType() ) );
}

bool QgsExportMeshOnElement::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMeshLayer *meshLayer = parameterAsMeshLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !meshLayer || !meshLayer->isValid() )
    return false;

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  if ( !outputCrs.isValid() )
    outputCrs = meshLayer->crs();
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

  switch ( meshElementType() )
  {
    case QgsMesh::Face:
      mElementCount = mNativeMesh.faceCount();
      break;
    case QgsMesh::Vertex:
      mElementCount = mNativeMesh.vertexCount();
      break;
    case QgsMesh::Edge:
      mElementCount = mNativeMesh.edgeCount();
      break;
  }

  for ( int i = 0; i < datasetGroups.count(); ++i )
  {
    int  groupIndex = datasetGroups.at( i );
    QgsMeshDatasetIndex datasetIndex = meshLayer->datasetIndexAtRelativeTime( relativeTime, groupIndex );

    DataGroup dataGroup;
    dataGroup.metadata = meshLayer->datasetGroupMetadata( datasetIndex );
    if ( supportedDataType().contains( dataGroup.metadata.dataType() ) )
    {
      dataGroup.datasetValues = meshLayer->datasetValues( datasetIndex, 0, mElementCount );
      mDataPerGroup.append( dataGroup );
    }
    if ( feedback )
      feedback->setProgress( 100 * i / datasetGroups.count() );
  }

  mExportVectorOption = parameterAsInt( parameters, QStringLiteral( "VECTOR_OPTION" ), context );

  return true;
}

QVariantMap QgsExportMeshOnElement::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    feedback->setProgress( 0 );
    feedback->setProgressText( QObject::tr( "Creating output vector layer" ) );
  }

  QList<QgsMeshDatasetGroupMetadata> metaList;
  metaList.reserve( mDataPerGroup.size() );
  for ( const DataGroup &dataGroup : mDataPerGroup )
    metaList.append( dataGroup.metadata );
  QgsFields fields = createFields( metaList, mExportVectorOption );

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  QString identifier;
  QgsFeatureSink *sink = parameterAsSink( parameters,
                                          QStringLiteral( "OUTPUT" ),
                                          context,
                                          identifier,
                                          fields,
                                          sinkGeometryType(),
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

  for ( int i = 0; i < mElementCount; ++i )
  {
    QgsAttributes attributes;
    for ( const DataGroup &dataGroup : mDataPerGroup )
    {
      const QgsMeshDatasetValue &value = dataGroup.datasetValues.value( i );
      addAttributes( value, attributes, dataGroup.metadata.isVector(), mExportVectorOption );
    }

    QgsFeature feat;
    QgsGeometry geom = meshElement( i );
    try
    {
      geom.transform( mTransform );
    }
    catch ( QgsCsException &e )
    {
      geom = meshElement( i );
      feedback->reportError( QObject::tr( "Could not transform point to destination CRS" ) );
    }
    feat.setGeometry( geom );
    feat.setAttributes( attributes );

    sink->addFeature( feat );

    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();
      feedback->setProgress( 100 * i / mElementCount );
    }
  }

  const QString fileName = parameterAsFile( parameters, QStringLiteral( "OUTPUT" ), context );

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = identifier;

  return ret;
}

QString QgsExportMeshFacesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Exports mesh layer's faces to a polygon vector layer, with the dataset values on faces as attribute values" );
}

QString QgsExportMeshFacesAlgorithm::name() const
{
  return QStringLiteral( "exportmeshfaces" );
}

QString QgsExportMeshFacesAlgorithm::displayName() const
{
  return QObject::tr( "Export mesh faces" );
}

QgsProcessingAlgorithm *QgsExportMeshFacesAlgorithm::createInstance() const
{
  return new QgsExportMeshFacesAlgorithm();
}

QgsGeometry QgsExportMeshFacesAlgorithm::meshElement( int index ) const
{
  const QgsMeshFace &face = mNativeMesh.face( index );
  QVector<QgsPoint> vertices( face.size() );
  for ( int i = 0; i < face.size(); ++i )
    vertices[i] = mNativeMesh.vertex( face.at( i ) );
  std::unique_ptr<QgsPolygon> polygon = qgis::make_unique<QgsPolygon>();
  polygon->setExteriorRing( new QgsLineString( vertices ) );
  return QgsGeometry( polygon.release() );
}

QString QgsExportMeshEdgesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Exports mesh layer's edges to a line vector layer, with the dataset values on edges as attribute values" );
}

QString QgsExportMeshEdgesAlgorithm::name() const
{
  return QStringLiteral( "exportmeshedges" );
}

QString QgsExportMeshEdgesAlgorithm::displayName() const
{
  return QObject::tr( "Export mesh edges" );
}

QgsProcessingAlgorithm *QgsExportMeshEdgesAlgorithm::createInstance() const
{
  return new QgsExportMeshEdgesAlgorithm();
}

QgsGeometry QgsExportMeshEdgesAlgorithm::meshElement( int index ) const
{
  const QgsMeshEdge &edge = mNativeMesh.edge( index );
  QVector<QgsPoint> vertices( 2 );
  vertices[0] = mNativeMesh.vertex( edge.first );
  vertices[1] = mNativeMesh.vertex( edge.second );
  return QgsGeometry( new QgsLineString( vertices ) );
}


QString QgsExportMeshOnGrid::name() const {return QStringLiteral( "exportmeshongrid" );}

QString QgsExportMeshOnGrid::displayName() const {return QStringLiteral( "Export mesh on grid" );}

QString QgsExportMeshOnGrid::group() const {return QStringLiteral( "Mesh" );}

QString QgsExportMeshOnGrid::groupId() const {return QStringLiteral( "mesh" );}

QString QgsExportMeshOnGrid::shortHelpString() const
{
  return QObject::tr( "Exports mesh layer's dataset values to a gridded point vector layer, with the dataset values on this point as attribute values.\n"
                      "For data on volume (3D stacked dataset values), the exported dataset values are averaged on faces using the method defined in the mesh layer properties (default is Multi level averaging method).\n"
                      "1D meshes are not supported" );
}

QgsProcessingAlgorithm *QgsExportMeshOnGrid::createInstance() const
{
  return new QgsExportMeshOnGrid();
}

void QgsExportMeshOnGrid::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input Mesh Layer" ) ) );

  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset Groups" ),
                  QStringLiteral( "INPUT" ),
                  supportedDataType() ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "DATASET_TIME" ),
                  QObject::tr( "Dataset Time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ), QVariant(), true ) );

  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "GRID_SPACING" ), QObject::tr( "Grid spacing" ), 10, QStringLiteral( "INPUT" ), false ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output Coordinate System" ), QVariant(), true ) );

  QStringList exportVectorOptions;
  exportVectorOptions << QObject::tr( "Cartesian (x,y)" )
                      << QObject::tr( "Polar (magnitude,degree)" )
                      << QObject::tr( "Cartesian and Polar" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "VECTOR_OPTION" ), QObject::tr( "Export Vector Option" ), exportVectorOptions, false, 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output Vector Layer" ), QgsProcessing::TypeVectorPoint ) );
}

bool QgsExportMeshOnGrid::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMeshLayer *meshLayer = parameterAsMeshLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !meshLayer || !meshLayer->isValid() )
    return false;

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  if ( !outputCrs.isValid() )
    outputCrs = meshLayer->crs();
  mTransform = QgsCoordinateTransform( meshLayer->crs(), outputCrs, context.transformContext() );
  if ( !meshLayer->nativeMesh() )
    meshLayer->updateTriangularMesh( mTransform ); //necessary to load the native mesh

  mTriangularMesh = *meshLayer->triangularMesh();
  QgsMesh *nativeMesh = meshLayer->nativeMesh();

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
    if ( supportedDataType().contains( dataGroup.metadata.dataType() ) )
    {
      int valueCount = dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ?
                       mTriangularMesh.vertices().count() : nativeMesh->faceCount();
      dataGroup.datasetValues = meshLayer->datasetValues( datasetIndex, 0, valueCount );
      dataGroup.activeFaces = meshLayer->areFacesActive( datasetIndex, 0, nativeMesh->faceCount() );
      if ( dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVolumes )
      {
        dataGroup.dataset3dStakedValue = meshLayer->dataset3dValues( datasetIndex, 0, valueCount );
      }
      mDataPerGroup.append( dataGroup );
    }
    if ( feedback )
      feedback->setProgress( 100 * i / datasetGroups.count() );
  }

  mExportVectorOption = parameterAsInt( parameters, QStringLiteral( "VECTOR_OPTION" ), context );

  return true;
}

QVariantMap QgsExportMeshOnGrid::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    feedback->setProgress( 0 );
    feedback->setProgressText( QObject::tr( "Creating output vector layer" ) );
  }

  //First, if present, average 3D staked dataset value to 2D face value
  const QgsMesh3dAveragingMethod *avgMethod = mLayerRendererSettings.averagingMethod();
  for ( DataGroup &dataGroup : mDataPerGroup )
  {
    if ( dataGroup.dataset3dStakedValue.isValid() )
      dataGroup.datasetValues = avgMethod->calculate( dataGroup.dataset3dStakedValue );
  }

  QList<QgsMeshDatasetGroupMetadata> metaList;
  metaList.reserve( mDataPerGroup.size() );
  for ( const DataGroup &dataGroup : mDataPerGroup )
    metaList.append( dataGroup.metadata );
  QgsFields fields = createFields( metaList, mExportVectorOption );

  //create sink
  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  QString identifier;
  QgsFeatureSink *sink = parameterAsSink( parameters,
                                          QStringLiteral( "OUTPUT" ),
                                          context,
                                          identifier,
                                          fields,
                                          QgsWkbTypes::Point,
                                          outputCrs );
  if ( !sink )
    return QVariantMap();

  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    feedback->setProgress( 0 );
    feedback->setProgressText( QObject::tr( "Creating gridded points" ) );
  }

  // grid definition
  double gridSpacing = parameterAsDouble( parameters, QStringLiteral( "GRID_SPACING" ), context );
  QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context );
  if ( extent.isEmpty() )
    extent = mTriangularMesh.extent();
  int pointXCount = int( extent.width() / gridSpacing ) + 1;
  int pointYCount = int( extent.height() / gridSpacing ) + 1;

  for ( int ix = 0; ix < pointXCount; ++ix )
  {
    for ( int iy = 0; iy < pointYCount; ++iy )
    {
      QgsPoint point( extent.xMinimum() + ix * gridSpacing, extent.yMinimum() + iy * gridSpacing );
      int triangularFaceIndex = mTriangularMesh.faceIndexForPoint_v2( point );
      if ( triangularFaceIndex >= 0 )
      {
        //extract dataset values for the point
        QgsAttributes attributes;
        int nativeFaceIndex = mTriangularMesh.trianglesToNativeFaces().at( triangularFaceIndex );

        for ( int i = 0; i < mDataPerGroup.count(); ++i )
        {
          const DataGroup &dataGroup = mDataPerGroup.at( i );
          bool faceActive = dataGroup.activeFaces.active( nativeFaceIndex );
          if ( !faceActive )
            continue;
          QgsMeshDatasetValue value;
          switch ( dataGroup.metadata.dataType() )
          {
            case QgsMeshDatasetGroupMetadata::DataOnEdges:
              //not supported
              break;
            case QgsMeshDatasetGroupMetadata::DataOnVolumes:
            case QgsMeshDatasetGroupMetadata::DataOnFaces:
            {
              value = dataGroup.datasetValues.value( nativeFaceIndex );
            }
            break;

            case QgsMeshDatasetGroupMetadata::DataOnVertices:
            {
              const QgsMeshFace &face = mTriangularMesh.triangles()[triangularFaceIndex];
              const int v1 = face[0], v2 = face[1], v3 = face[2];
              const QgsPoint p1 = mTriangularMesh.vertices()[v1], p2 = mTriangularMesh.vertices()[v2], p3 = mTriangularMesh.vertices()[v3];
              const QgsMeshDatasetValue val1 = dataGroup.datasetValues.value( v1 );
              const QgsMeshDatasetValue val2 = dataGroup.datasetValues.value( v2 );
              const QgsMeshDatasetValue val3 = dataGroup.datasetValues.value( v3 );
              const double x = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.x(), val2.x(), val3.x(), point );
              double y = std::numeric_limits<double>::quiet_NaN();
              bool isVector = dataGroup.metadata.isVector();
              if ( isVector )
                y = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.y(), val2.y(), val3.y(), point );

              value = QgsMeshDatasetValue( x, y );
            }
            break;
          }
          if ( dataGroup.metadata.isVector() )
          {
            QVector<double> vector = vectorValue( dataGroup.datasetValues.value( i ), mExportVectorOption );
            for ( double v : vector )
            {
              attributes.append( v );
            }
          }
          else
            attributes.append( value.scalar() );
        }
        QgsFeature feat;
        QgsGeometry geom( point.clone() );
        try
        {
          geom.transform( mTransform );
        }
        catch ( QgsCsException &e )
        {
          geom = QgsGeometry( point.clone() );
          feedback->reportError( QObject::tr( "Could not transform point to destination CRS" ) );
        }
        feat.setGeometry( geom );
        feat.setAttributes( attributes );

        sink->addFeature( feat );
      }
    }
  }

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = identifier;

  return ret;
}

QSet<QgsMeshDatasetGroupMetadata::DataType> QgsExportMeshOnGrid::supportedDataType()
{
  return QSet<QgsMeshDatasetGroupMetadata::DataType>(
  {
    QgsMeshDatasetGroupMetadata::DataOnVertices,
    QgsMeshDatasetGroupMetadata::DataOnFaces,
    QgsMeshDatasetGroupMetadata::DataOnVolumes} );
}

///@endcond PRIVATE
