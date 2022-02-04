/***************************************************************************
                         qgsalgorithmexportmesh.cpp
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
#include "qgsmeshcontours.h"
#include "qgsmeshdataset.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsmeshlayerinterpolator.h"
#include "qgspolygon.h"
#include "qgsrasterfilewriter.h"
#include "qgslinestring.h"

#include <QTextStream>

///@cond PRIVATE


static QgsFields createFields( const QList<QgsMeshDatasetGroupMetadata> &groupMetadataList, int vectorOption )
{
  QgsFields fields;
  for ( const QgsMeshDatasetGroupMetadata &meta : groupMetadataList )
  {
    if ( meta.isVector() )
    {
      if ( vectorOption == 0 || vectorOption == 2 )
      {
        fields.append( QStringLiteral( "%1_x" ).arg( meta.name() ) );
        fields.append( QStringLiteral( "%1_y" ).arg( meta.name() ) );
      }

      if ( vectorOption == 1 || vectorOption == 2 )
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

static QgsMeshDatasetValue extractDatasetValue(
  const QgsPointXY &point,
  int nativeFaceIndex,
  int triangularFaceIndex,
  const QgsTriangularMesh &triangularMesh,
  const QgsMeshDataBlock &activeFaces,
  const QgsMeshDataBlock &datasetValues,
  const QgsMeshDatasetGroupMetadata &metadata )
{
  bool faceActive = activeFaces.active( nativeFaceIndex );
  QgsMeshDatasetValue value;
  if ( faceActive )
  {
    switch ( metadata.dataType() )
    {
      case QgsMeshDatasetGroupMetadata::DataOnEdges:
        //not supported
        break;
      case QgsMeshDatasetGroupMetadata::DataOnVolumes:
      case QgsMeshDatasetGroupMetadata::DataOnFaces:
      {
        value = datasetValues.value( nativeFaceIndex );
      }
      break;

      case QgsMeshDatasetGroupMetadata::DataOnVertices:
      {
        const QgsMeshFace &face = triangularMesh.triangles()[triangularFaceIndex];
        const int v1 = face[0], v2 = face[1], v3 = face[2];
        const QgsPoint p1 = triangularMesh.vertices()[v1], p2 = triangularMesh.vertices()[v2], p3 = triangularMesh.vertices()[v3];
        const QgsMeshDatasetValue val1 = datasetValues.value( v1 );
        const QgsMeshDatasetValue val2 = datasetValues.value( v2 );
        const QgsMeshDatasetValue val3 = datasetValues.value( v3 );
        const double x = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.x(), val2.x(), val3.x(), point );
        double y = std::numeric_limits<double>::quiet_NaN();
        bool isVector = metadata.isVector();
        if ( isVector )
          y = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.y(), val2.y(), val3.y(), point );

        value = QgsMeshDatasetValue( x, y );
      }
      break;
    }
  }

  return value;
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
  return QObject::tr( "This algorithm exports a mesh layer's vertices to a point vector layer, with the dataset values on vertices as attribute values." );
}

QString QgsExportMeshVerticesAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports mesh vertices to a point vector layer" );
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

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input mesh layer" ) ) );


  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset groups" ),
                  QStringLiteral( "INPUT" ),
                  supportedDataType() ) );

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
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output vector layer" ), sinkType() ) );
}

static QgsInterval datasetRelativetime( const QVariant parameterTimeVariant, QgsMeshLayer *meshLayer, const QgsProcessingContext &context )
{
  QgsInterval relativeTime( 0 );
  QDateTime layerReferenceTime = static_cast<QgsMeshLayerTemporalProperties *>( meshLayer->temporalProperties() )->referenceTime();
  QString timeType = QgsProcessingParameterMeshDatasetTime::valueAsTimeType( parameterTimeVariant );

  if ( timeType == QLatin1String( "dataset-time-step" ) )
  {
    QgsMeshDatasetIndex datasetIndex = QgsProcessingParameterMeshDatasetTime::timeValueAsDatasetIndex( parameterTimeVariant );
    relativeTime = meshLayer->datasetRelativeTime( datasetIndex );
  }
  else if ( timeType == QLatin1String( "defined-date-time" ) )
  {
    QDateTime dateTime = QgsProcessingParameterMeshDatasetTime::timeValueAsDefinedDateTime( parameterTimeVariant );
    if ( dateTime.isValid() )
      relativeTime = QgsInterval( layerReferenceTime.secsTo( dateTime ) );
  }
  else if ( timeType == QLatin1String( "current-context-time" ) )
  {
    QDateTime dateTime = context.currentTimeRange().begin();
    if ( dateTime.isValid() )
      relativeTime = QgsInterval( layerReferenceTime.secsTo( dateTime ) );
  }

  return relativeTime;
}


bool QgsExportMeshOnElement::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMeshLayer *meshLayer = parameterAsMeshLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !meshLayer || !meshLayer->isValid() )
    return false;

  if ( meshLayer->isEditable() )
    throw QgsProcessingException( QObject::tr( "Input mesh layer in edit mode is not supported" ) );

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

  // Extract the date time used to export dataset values under a relative time
  QVariant parameterTimeVariant = parameters.value( QStringLiteral( "DATASET_TIME" ) );
  QgsInterval relativeTime = datasetRelativetime( parameterTimeVariant, meshLayer, context );

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
    for ( const DataGroup &dataGroup : std::as_const( mDataPerGroup ) )
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
    catch ( QgsCsException & )
    {
      geom = meshElement( i );
      if ( feedback )
        feedback->reportError( QObject::tr( "Could not transform point to destination CRS" ) );
    }
    feat.setGeometry( geom );
    feat.setAttributes( attributes );

    if ( !sink->addFeature( feat ) )
      throw QgsProcessingException( writeFeatureError( sink, parameters, QStringLiteral( "OUTPUT" ) ) );

    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();
      feedback->setProgress( 100 * i / mElementCount );
    }
  }

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = identifier;

  return ret;
}

QString QgsExportMeshFacesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a mesh layer's faces to a polygon vector layer, with the dataset values on faces as attribute values." );
}

QString QgsExportMeshFacesAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports mesh faces to a polygon vector layer" );
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
  std::unique_ptr<QgsPolygon> polygon = std::make_unique<QgsPolygon>();
  polygon->setExteriorRing( new QgsLineString( vertices ) );
  return QgsGeometry( polygon.release() );
}

QString QgsExportMeshEdgesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a mesh layer's edges to a line vector layer, with the dataset values on edges as attribute values." );
}

QString QgsExportMeshEdgesAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports mesh edges to a line vector layer" );
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


QString QgsExportMeshOnGridAlgorithm::name() const {return QStringLiteral( "exportmeshongrid" );}

QString QgsExportMeshOnGridAlgorithm::displayName() const {return QObject::tr( "Export mesh on grid" );}

QString QgsExportMeshOnGridAlgorithm::group() const {return QObject::tr( "Mesh" );}

QString QgsExportMeshOnGridAlgorithm::groupId() const {return QStringLiteral( "mesh" );}

QString QgsExportMeshOnGridAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a mesh layer's dataset values to a gridded point vector layer, with the dataset values on each point as attribute values.\n"
                      "For data on volume (3D stacked dataset values), the exported dataset values are averaged on faces using the method defined in the mesh layer properties (default is Multi level averaging method).\n"
                      "1D meshes are not supported." );
}

QString QgsExportMeshOnGridAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports mesh dataset values to a gridded point vector layer" );
}

QgsProcessingAlgorithm *QgsExportMeshOnGridAlgorithm::createInstance() const
{
  return new QgsExportMeshOnGridAlgorithm();
}

void QgsExportMeshOnGridAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input mesh layer" ) ) );

  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset groups" ),
                  QStringLiteral( "INPUT" ),
                  supportedDataType() ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "DATASET_TIME" ),
                  QObject::tr( "Dataset time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ), QVariant(), true ) );

  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "GRID_SPACING" ), QObject::tr( "Grid spacing" ), 10, QStringLiteral( "INPUT" ), false ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output coordinate system" ), QVariant(), true ) );

  QStringList exportVectorOptions;
  exportVectorOptions << QObject::tr( "Cartesian (x,y)" )
                      << QObject::tr( "Polar (magnitude,degree)" )
                      << QObject::tr( "Cartesian and Polar" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "VECTOR_OPTION" ), QObject::tr( "Export vector option" ), exportVectorOptions, false, 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output vector layer" ), QgsProcessing::TypeVectorPoint ) );
}

static void extractDatasetValues( const QList<int> &datasetGroups,
                                  QgsMeshLayer *meshLayer,
                                  const QgsMesh &nativeMesh,
                                  const QgsInterval &relativeTime,
                                  const QSet<int> supportedDataType,
                                  QList<DataGroup> &datasetPerGroup,
                                  QgsProcessingFeedback *feedback )
{
  for ( int i = 0; i < datasetGroups.count(); ++i )
  {
    int  groupIndex = datasetGroups.at( i );
    QgsMeshDatasetIndex datasetIndex = meshLayer->datasetIndexAtRelativeTime( relativeTime, groupIndex );

    DataGroup dataGroup;
    dataGroup.metadata = meshLayer->datasetGroupMetadata( datasetIndex );
    if ( supportedDataType.contains( dataGroup.metadata.dataType() ) )
    {
      int valueCount = dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ?
                       nativeMesh.vertices.count() : nativeMesh.faceCount();
      dataGroup.datasetValues = meshLayer->datasetValues( datasetIndex, 0, valueCount );
      dataGroup.activeFaces = meshLayer->areFacesActive( datasetIndex, 0, nativeMesh.faceCount() );
      if ( dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVolumes )
      {
        dataGroup.dataset3dStakedValue = meshLayer->dataset3dValues( datasetIndex, 0, valueCount );
      }
      datasetPerGroup.append( dataGroup );
    }
    if ( feedback )
      feedback->setProgress( 100 * i / datasetGroups.count() );
  }
}

bool QgsExportMeshOnGridAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  const QgsMesh &nativeMesh = *meshLayer->nativeMesh();

  QList<int> datasetGroups =
    QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( parameters.value( QStringLiteral( "DATASET_GROUPS" ) ) );

  if ( feedback )
  {
    feedback->setProgressText( QObject::tr( "Preparing data" ) );
  }

  // Extract the date time used to export dataset values under a relative time
  QVariant parameterTimeVariant = parameters.value( QStringLiteral( "DATASET_TIME" ) );
  QgsInterval relativeTime = datasetRelativetime( parameterTimeVariant, meshLayer, context );

  extractDatasetValues( datasetGroups, meshLayer, nativeMesh, relativeTime, supportedDataType(), mDataPerGroup, feedback );
  mTriangularMesh.update( meshLayer->nativeMesh(), mTransform );

  mExportVectorOption = parameterAsInt( parameters, QStringLiteral( "VECTOR_OPTION" ), context );

  return true;
}

QVariantMap QgsExportMeshOnGridAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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
  for ( const DataGroup &dataGroup : std::as_const( mDataPerGroup ) )
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
          QgsMeshDatasetValue value = extractDatasetValue(
                                        point,
                                        nativeFaceIndex,
                                        triangularFaceIndex,
                                        mTriangularMesh,
                                        dataGroup.activeFaces,
                                        dataGroup.datasetValues,
                                        dataGroup.metadata );

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
        catch ( QgsCsException & )
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

QSet<int> QgsExportMeshOnGridAlgorithm::supportedDataType()
{
  return QSet<int>(
  {
    QgsMeshDatasetGroupMetadata::DataOnVertices,
    QgsMeshDatasetGroupMetadata::DataOnFaces,
    QgsMeshDatasetGroupMetadata::DataOnVolumes} );
}

QString QgsMeshRasterizeAlgorithm::name() const
{
  return QStringLiteral( "meshrasterize" );
}

QString QgsMeshRasterizeAlgorithm::displayName() const
{
  return QObject::tr( "Rasterize mesh dataset" );
}

QString QgsMeshRasterizeAlgorithm::group() const
{
  return QObject::tr( "Mesh" );
}

QString QgsMeshRasterizeAlgorithm::groupId() const
{
  return QStringLiteral( "mesh" );
}

QString QgsMeshRasterizeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a raster layer from a mesh dataset.\n"
                      "For data on volume (3D stacked dataset values), the exported dataset values are averaged on faces using the method defined in the mesh layer properties (default is Multi level averaging method).\n"
                      "1D meshes are not supported." );
}

QString QgsMeshRasterizeAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a raster layer from a mesh dataset" );
}

QgsProcessingAlgorithm *QgsMeshRasterizeAlgorithm::createInstance() const
{
  return new QgsMeshRasterizeAlgorithm();
}

void QgsMeshRasterizeAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input mesh layer" ) ) );

  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset groups" ),
                  QStringLiteral( "INPUT" ),
                  supportedDataType(),
                  true ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "DATASET_TIME" ),
                  QObject::tr( "Dataset time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ), QVariant(), true ) );

  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ), 1, QStringLiteral( "INPUT" ), false ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output coordinate system" ), QVariant(), true ) );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output raster layer" ) ) );
}

bool QgsMeshRasterizeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  mTriangularMesh.update( meshLayer->nativeMesh(), mTransform );

  QList<int> datasetGroups =
    QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( parameters.value( QStringLiteral( "DATASET_GROUPS" ) ) );

  if ( feedback )
  {
    feedback->setProgressText( QObject::tr( "Preparing data" ) );
  }

  // Extract the date time used to export dataset values under a relative time
  QVariant parameterTimeVariant = parameters.value( QStringLiteral( "DATASET_TIME" ) );
  QgsInterval relativeTime = datasetRelativetime( parameterTimeVariant, meshLayer, context );

  extractDatasetValues( datasetGroups, meshLayer, *meshLayer->nativeMesh(), relativeTime, supportedDataType(), mDataPerGroup, feedback );

  mLayerRendererSettings = meshLayer->rendererSettings();

  return true;
}

QVariantMap QgsMeshRasterizeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    feedback->setProgress( 0 );
    feedback->setProgressText( QObject::tr( "Creating raster layer" ) );
  }

  //First, if present, average 3D staked dataset value to 2D face value
  const QgsMesh3dAveragingMethod *avgMethod = mLayerRendererSettings.averagingMethod();
  for ( DataGroup &dataGroup : mDataPerGroup )
  {
    if ( dataGroup.dataset3dStakedValue.isValid() )
      dataGroup.datasetValues = avgMethod->calculate( dataGroup.dataset3dStakedValue );
  }

  // create raster
  double pixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );
  QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context );
  if ( extent.isEmpty() )
    extent = mTriangularMesh.extent();

  int width = extent.width() / pixelSize;
  int height = extent.height() / pixelSize;

  QString fileName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fileInfo( fileName );
  QString outputFormat = QgsRasterFileWriter::driverForExtension( fileInfo.suffix() );
  QgsRasterFileWriter rasterFileWriter( fileName );
  rasterFileWriter.setOutputProviderKey( QStringLiteral( "gdal" ) );
  rasterFileWriter.setOutputFormat( outputFormat );

  std::unique_ptr<QgsRasterDataProvider> rasterDataProvider(
    rasterFileWriter.createMultiBandRaster( Qgis::DataType::Float64, width, height, extent, mTransform.destinationCrs(), mDataPerGroup.count() ) );
  rasterDataProvider->setEditable( true );

  for ( int i = 0; i < mDataPerGroup.count(); ++i )
  {
    const DataGroup &dataGroup = mDataPerGroup.at( i );
    QgsRasterBlockFeedback rasterBlockFeedBack;
    if ( feedback )
      QObject::connect( &rasterBlockFeedBack, &QgsFeedback::canceled, feedback, &QgsFeedback::cancel );

    if ( dataGroup.datasetValues.isValid() )
    {
      std::unique_ptr<QgsRasterBlock> block( QgsMeshUtils::exportRasterBlock(
          mTriangularMesh,
          dataGroup.datasetValues,
          dataGroup.activeFaces,
          dataGroup.metadata.dataType(),
          mTransform,
          pixelSize,
          extent,
          &rasterBlockFeedBack ) );

      rasterDataProvider->writeBlock( block.get(), i + 1 );
      rasterDataProvider->setNoDataValue( i + 1, block->noDataValue() );
    }
    else
      rasterDataProvider->setNoDataValue( i + 1, std::numeric_limits<double>::quiet_NaN() );

    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();
      feedback->setProgress( 100 * i / mDataPerGroup.count() );
    }
  }

  rasterDataProvider->setEditable( false );

  if ( feedback )
    feedback->setProgress( 100 );

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = fileName;

  return ret;
}

QSet<int> QgsMeshRasterizeAlgorithm::supportedDataType()
{
  return QSet<int>(
  {
    QgsMeshDatasetGroupMetadata::DataOnVertices,
    QgsMeshDatasetGroupMetadata::DataOnFaces,
    QgsMeshDatasetGroupMetadata::DataOnVolumes} );
}

QString QgsMeshContoursAlgorithm::name() const
{
  return QStringLiteral( "meshcontours" );
}

QString QgsMeshContoursAlgorithm::displayName() const
{
  return QObject::tr( "Export contours" );
}

QString QgsMeshContoursAlgorithm::group() const
{
  return QObject::tr( "Mesh" );
}

QString QgsMeshContoursAlgorithm::groupId() const
{
  return QStringLiteral( "mesh" );
}

QString QgsMeshContoursAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates contours as a vector layer from a mesh scalar dataset." );
}

QString QgsMeshContoursAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates contours as vector layer from mesh scalar dataset" );
}

QgsProcessingAlgorithm *QgsMeshContoursAlgorithm::createInstance() const
{
  return new QgsMeshContoursAlgorithm();
}

void QgsMeshContoursAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input mesh layer" ) ) );

  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset groups" ),
                  QStringLiteral( "INPUT" ),
                  supportedDataType() ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "DATASET_TIME" ),
                  QObject::tr( "Dataset time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterNumber(
                  QStringLiteral( "INCREMENT" ), QObject::tr( "Increment between contour levels" ), QgsProcessingParameterNumber::Double, QVariant(), true ) );

  addParameter( new QgsProcessingParameterNumber(
                  QStringLiteral( "MINIMUM" ), QObject::tr( "Minimum contour level" ), QgsProcessingParameterNumber::Double, QVariant(), true ) );
  addParameter( new QgsProcessingParameterNumber(
                  QStringLiteral( "MAXIMUM" ), QObject::tr( "Maximum contour level" ), QgsProcessingParameterNumber::Double, QVariant(), true ) );

  std::unique_ptr< QgsProcessingParameterString > contourLevelList = std::make_unique < QgsProcessingParameterString >(
        QStringLiteral( "CONTOUR_LEVEL_LIST" ), QObject::tr( "List of contours level" ), QVariant(), false, true );
  contourLevelList->setHelp( QObject::tr( "Comma separated list of values to export. If filled, the increment, minimum and maximum settings are ignored." ) );
  addParameter( contourLevelList.release() );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output coordinate system" ), QVariant(), true ) );


  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_LINES" ), QObject::tr( "Exported contour lines" ), QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_POLYGONS" ), QObject::tr( "Exported contour polygons" ), QgsProcessing::TypeVectorPolygon ) );
}

bool QgsMeshContoursAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  mTriangularMesh.update( meshLayer->nativeMesh(), mTransform );
  mNativeMesh = *meshLayer->nativeMesh();

  // Prepare levels
  mLevels.clear();
  // First, try with the levels list
  QString levelsString = parameterAsString( parameters, QStringLiteral( "CONTOUR_LEVEL_LIST" ), context );
  if ( ! levelsString.isEmpty() )
  {
    QStringList levelStringList = levelsString.split( ',' );
    if ( !levelStringList.isEmpty() )
    {
      for ( const QString &stringVal : levelStringList )
      {
        bool ok;
        double val = stringVal.toDouble( &ok );
        if ( ok )
          mLevels.append( val );
        else
          throw QgsProcessingException( QObject::tr( "Invalid format for level values, must be numbers separated with comma" ) );

        if ( mLevels.count() >= 2 )
          if ( mLevels.last() <= mLevels.at( mLevels.count() - 2 ) )
            throw QgsProcessingException( QObject::tr( "Invalid format for level values, must be different numbers and in increasing order" ) );
      }
    }
  }

  if ( mLevels.isEmpty() )
  {
    double minimum = parameterAsDouble( parameters, QStringLiteral( "MINIMUM" ), context );
    double maximum = parameterAsDouble( parameters, QStringLiteral( "MAXIMUM" ), context );
    double interval = parameterAsDouble( parameters, QStringLiteral( "INCREMENT" ), context );

    if ( interval <= 0 )
      throw QgsProcessingException( QObject::tr( "Invalid interval value, must be greater than zero" ) );

    if ( minimum >= maximum )
      throw QgsProcessingException( QObject::tr( "Invalid minimum and maximum values, minimum must be lesser than maximum" ) );

    if ( interval > ( maximum - minimum ) )
      throw QgsProcessingException( QObject::tr( "Invalid minimum, maximum and interval values, difference between minimum and maximum must be greater or equal than interval" ) );

    int intervalCount = ( maximum - minimum ) / interval;

    mLevels.reserve( intervalCount );
    for ( int i = 0; i < intervalCount; ++i )
    {
      mLevels.append( minimum + i * interval );
    }
  }

  // Prepare data
  QList<int> datasetGroups =
    QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( parameters.value( QStringLiteral( "DATASET_GROUPS" ) ) );

  if ( feedback )
  {
    feedback->setProgressText( QObject::tr( "Preparing data" ) );
  }

  // Extract the date time used to export dataset values under a relative time
  QVariant parameterTimeVariant = parameters.value( QStringLiteral( "DATASET_TIME" ) );
  QgsInterval relativeTime = datasetRelativetime( parameterTimeVariant, meshLayer, context );

  mDateTimeString = meshLayer->formatTime( relativeTime.hours() );

  extractDatasetValues( datasetGroups, meshLayer, mNativeMesh, relativeTime, supportedDataType(), mDataPerGroup, feedback );

  mLayerRendererSettings = meshLayer->rendererSettings();

  return true;
}

QVariantMap QgsMeshContoursAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  //First, if present, average 3D staked dataset value to 2D face value
  const QgsMesh3dAveragingMethod *avgMethod = mLayerRendererSettings.averagingMethod();
  for ( DataGroup &dataGroup : mDataPerGroup )
  {
    if ( dataGroup.dataset3dStakedValue.isValid() )
      dataGroup.datasetValues = avgMethod->calculate( dataGroup.dataset3dStakedValue );
  }

  // Create vector layers
  QgsFields polygonFields;
  QgsFields lineFields;
  polygonFields.append( QObject::tr( "group" ) );
  polygonFields.append( QObject::tr( "time" ) );
  polygonFields.append( QObject::tr( "min_value" ) );
  polygonFields.append( QObject::tr( "max_value" ) );
  lineFields.append( QObject::tr( "group" ) );
  lineFields.append( QObject::tr( "time" ) );
  lineFields.append( QObject::tr( "value" ) );

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );

  QString lineIdentifier;
  QString polygonIdentifier;
  QgsFeatureSink *sinkPolygons = parameterAsSink( parameters,
                                 QStringLiteral( "OUTPUT_POLYGONS" ),
                                 context,
                                 polygonIdentifier,
                                 polygonFields,
                                 QgsWkbTypes::PolygonZ,
                                 outputCrs );
  QgsFeatureSink *sinkLines = parameterAsSink( parameters,
                              QStringLiteral( "OUTPUT_LINES" ),
                              context,
                              lineIdentifier,
                              lineFields,
                              QgsWkbTypes::LineStringZ,
                              outputCrs );

  if ( !sinkLines || !sinkPolygons )
    return QVariantMap();


  for ( int i = 0; i < mDataPerGroup.count(); ++i )
  {
    DataGroup dataGroup = mDataPerGroup.at( i );
    bool scalarDataOnVertices = dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
    int count =  scalarDataOnVertices ? mNativeMesh.vertices.count() : mNativeMesh.faces.count();

    QVector<double> values;
    if ( dataGroup.datasetValues.isValid() )
    {
      // vals could be scalar or vectors, for contour rendering we want always magnitude
      values = QgsMeshLayerUtils::calculateMagnitudes( dataGroup.datasetValues );
    }
    else
    {
      values = QVector<double>( count, std::numeric_limits<double>::quiet_NaN() );
    }

    if ( ( !scalarDataOnVertices ) )
    {
      values = QgsMeshLayerUtils::interpolateFromFacesData(
                 values,
                 mNativeMesh,
                 &dataGroup.activeFaces,
                 QgsMeshRendererScalarSettings::NeighbourAverage
               );
    }

    QgsMeshContours contoursExported( mTriangularMesh, mNativeMesh, values, dataGroup.activeFaces );

    QgsAttributes firstAttributes;
    firstAttributes.append( dataGroup.metadata.name() );
    firstAttributes.append( mDateTimeString );

    for ( double level : std::as_const( mLevels ) )
    {
      QgsGeometry line = contoursExported.exportLines( level, feedback );
      if ( feedback->isCanceled() )
        return QVariantMap();
      if ( line.isEmpty() )
        continue;
      QgsAttributes lineAttributes = firstAttributes;
      lineAttributes.append( level );

      QgsFeature lineFeat;
      lineFeat.setGeometry( line );
      lineFeat.setAttributes( lineAttributes );

      sinkLines->addFeature( lineFeat );

    }

    for ( int l = 0; l < mLevels.count() - 1; ++l )
    {
      QgsGeometry polygon = contoursExported.exportPolygons( mLevels.at( l ), mLevels.at( l + 1 ), feedback );
      if ( feedback->isCanceled() )
        return QVariantMap();

      if ( polygon.isEmpty() )
        continue;
      QgsAttributes polygonAttributes = firstAttributes;
      polygonAttributes.append( mLevels.at( l ) );
      polygonAttributes.append( mLevels.at( l + 1 ) );

      QgsFeature polygonFeature;
      polygonFeature.setGeometry( polygon );
      polygonFeature.setAttributes( polygonAttributes );
      sinkPolygons->addFeature( polygonFeature );
    }

    if ( feedback )
    {
      feedback->setProgress( 100 * i / mDataPerGroup.count() );
    }
  }

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT_LINES" )] = lineIdentifier;
  ret[QStringLiteral( "OUTPUT_POLYGONS" )] = polygonIdentifier;

  return ret;
}

QString QgsMeshExportCrossSection::name() const
{
  return QStringLiteral( "meshexportcrosssection" );
}

QString QgsMeshExportCrossSection::displayName() const
{
  return QObject::tr( "Export cross section dataset values on lines from mesh" );
}

QString QgsMeshExportCrossSection::group() const
{
  return QObject::tr( "Mesh" );
}

QString QgsMeshExportCrossSection::groupId() const
{
  return QStringLiteral( "mesh" );
}

QString QgsMeshExportCrossSection::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts mesh's dataset values from line contained in a vector layer.\n"
                      "Each line is discretized with a resolution distance parameter for extraction of values on its vertices." );
}

QString QgsMeshExportCrossSection::shortDescription() const
{
  return QObject::tr( "Extracts a mesh dataset's values from lines contained in a vector layer" );
}

QgsProcessingAlgorithm *QgsMeshExportCrossSection::createInstance() const
{
  return new QgsMeshExportCrossSection();
}

void QgsMeshExportCrossSection::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input mesh layer" ) ) );

  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset groups" ),
                  QStringLiteral( "INPUT" ),
                  supportedDataType() ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "DATASET_TIME" ),
                  QObject::tr( "Dataset time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  QList<int> datatype;
  datatype << QgsProcessing::TypeVectorLine;
  addParameter( new QgsProcessingParameterFeatureSource(
                  QStringLiteral( "INPUT_LINES" ), QObject::tr( "Lines for data export" ), datatype, QVariant(), false ) );

  addParameter( new QgsProcessingParameterDistance(
                  QStringLiteral( "RESOLUTION" ), QObject::tr( "Line segmentation resolution" ), 10.0, QStringLiteral( "INPUT_LINES" ), false, 0 ) );

  addParameter( new QgsProcessingParameterNumber(
                  QStringLiteral( "COORDINATES_DIGITS" ), QObject::tr( "Digits count for coordinates" ), QgsProcessingParameterNumber::Integer, 2 ) );

  addParameter( new QgsProcessingParameterNumber(
                  QStringLiteral( "DATASET_DIGITS" ), QObject::tr( "Digits count for dataset value" ), QgsProcessingParameterNumber::Integer, 2 ) );

  addParameter( new QgsProcessingParameterFileDestination(
                  QStringLiteral( "OUTPUT" ), QObject::tr( "Exported data CSV file" ), QObject::tr( "CSV file (*.csv)" ) ) );
}

bool QgsMeshExportCrossSection::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMeshLayer *meshLayer = parameterAsMeshLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !meshLayer || !meshLayer->isValid() )
    return false;

  mMeshLayerCrs = meshLayer->crs();
  mTriangularMesh.update( meshLayer->nativeMesh() );
  QList<int> datasetGroups =
    QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( parameters.value( QStringLiteral( "DATASET_GROUPS" ) ) );

  if ( feedback )
  {
    feedback->setProgressText( QObject::tr( "Preparing data" ) );
  }

  // Extract the date time used to export dataset values under a relative time
  QVariant parameterTimeVariant = parameters.value( QStringLiteral( "DATASET_TIME" ) );
  QgsInterval relativeTime = datasetRelativetime( parameterTimeVariant, meshLayer, context );

  extractDatasetValues( datasetGroups, meshLayer, *meshLayer->nativeMesh(), relativeTime, supportedDataType(), mDataPerGroup, feedback );

  mLayerRendererSettings = meshLayer->rendererSettings();

  return true;
}

QVariantMap QgsMeshExportCrossSection::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( feedback )
    feedback->setProgress( 0 );
  //First, if present, average 3D staked dataset value to 2D face value
  const QgsMesh3dAveragingMethod *avgMethod = mLayerRendererSettings.averagingMethod();
  for ( DataGroup &dataGroup : mDataPerGroup )
  {
    if ( dataGroup.dataset3dStakedValue.isValid() )
      dataGroup.datasetValues = avgMethod->calculate( dataGroup.dataset3dStakedValue );
  }
  double resolution = parameterAsDouble( parameters, QStringLiteral( "RESOLUTION" ), context );
  int datasetDigits = parameterAsInt( parameters, QStringLiteral( "DATASET_DIGITS" ), context );
  int coordDigits = parameterAsInt( parameters, QStringLiteral( "COORDINATES_DIGITS" ), context );

  QgsProcessingFeatureSource *featureSource = parameterAsSource( parameters, QStringLiteral( "INPUT_LINES" ), context );
  if ( !featureSource )
    throw QgsProcessingException( QObject::tr( "Input lines vector layer required" ) );

  QgsCoordinateTransform transform( featureSource->sourceCrs(), mMeshLayerCrs, context.transformContext() );

  QString outputFileName = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );
  QFile file( outputFileName );
  if ( ! file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    throw QgsProcessingException( QObject::tr( "Unable to create the output file" ) );

  QTextStream textStream( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  textStream.setCodec( "UTF-8" );
#endif
  QStringList header;
  header << QStringLiteral( "fid" ) << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QObject::tr( "offset" );
  for ( const DataGroup &datagroup : std::as_const( mDataPerGroup ) )
    header << datagroup.metadata.name();
  textStream << header.join( ',' ) << QStringLiteral( "\n" );

  long long featCount = featureSource->featureCount();
  long long featCounter = 0;
  QgsFeatureIterator featIt = featureSource->getFeatures();
  QgsFeature feat;
  while ( featIt.nextFeature( feat ) )
  {
    QgsFeatureId fid = feat.id();
    QgsGeometry line = feat.geometry();
    try
    {
      line.transform( transform );
    }
    catch ( QgsCsException & )
    {
      line = feat.geometry();
      feedback->reportError( QObject::tr( "Could not transform line to mesh CRS" ) );
    }

    if ( line.isEmpty() )
      continue;
    double offset = 0;
    while ( offset <= line.length() )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();

      QStringList textLine;
      QgsPointXY point = line.interpolate( offset ).asPoint();
      int triangularFaceIndex = mTriangularMesh.faceIndexForPoint_v2( point );
      textLine << QString::number( fid ) << QString::number( point.x(), 'f', coordDigits ) << QString::number( point.y(), 'f', coordDigits ) << QString::number( offset, 'f', coordDigits );
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
          QgsMeshDatasetValue value = extractDatasetValue(
                                        point,
                                        nativeFaceIndex,
                                        triangularFaceIndex,
                                        mTriangularMesh,
                                        dataGroup.activeFaces,
                                        dataGroup.datasetValues,
                                        dataGroup.metadata );

          if ( abs( value.x() ) == std::numeric_limits<double>::quiet_NaN() )
            textLine << QString( ' ' );
          else
            textLine << QString::number( value.scalar(), 'f', datasetDigits );
        }
      }
      else
        for ( int i = 0; i < mDataPerGroup.count(); ++i )
          textLine << QString( ' ' );

      textStream << textLine.join( ',' ) << QStringLiteral( "\n" );

      offset += resolution;
    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 * featCounter / featCount );
      if ( feedback->isCanceled() )
        return QVariantMap();
    }
  }

  file.close();

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = outputFileName;
  return ret;
}

QString QgsMeshExportTimeSeries::name() const
{
  return QStringLiteral( "meshexporttimeseries" );
}

QString QgsMeshExportTimeSeries::displayName() const
{
  return QObject::tr( "Export time series values from points of a mesh dataset" );
}

QString QgsMeshExportTimeSeries::group() const
{
  return QObject::tr( "Mesh" );
}

QString QgsMeshExportTimeSeries::groupId() const
{
  return QStringLiteral( "mesh" );
}

QString QgsMeshExportTimeSeries::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts mesh's dataset time series values from points contained in a vector layer.\n"
                      "If the time step is kept to its default value (0 hours), the time step used is the one of the two first datasets of the first selected dataset group." );
}

QString QgsMeshExportTimeSeries::shortDescription() const
{
  return QObject::tr( "Extracts a mesh dataset's time series values from points contained in a vector layer" );
}

QgsProcessingAlgorithm *QgsMeshExportTimeSeries::createInstance() const
{
  return new QgsMeshExportTimeSeries();
}

void QgsMeshExportTimeSeries::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input mesh layer" ) ) );

  addParameter( new QgsProcessingParameterMeshDatasetGroups(
                  QStringLiteral( "DATASET_GROUPS" ),
                  QObject::tr( "Dataset groups" ),
                  QStringLiteral( "INPUT" ),
                  supportedDataType() ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "STARTING_TIME" ),
                  QObject::tr( "Starting time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterMeshDatasetTime(
                  QStringLiteral( "FINISHING_TIME" ),
                  QObject::tr( "Finishing time" ),
                  QStringLiteral( "INPUT" ),
                  QStringLiteral( "DATASET_GROUPS" ) ) );

  addParameter( new QgsProcessingParameterNumber(
                  QStringLiteral( "TIME_STEP" ), QObject::tr( "Time step (hours)" ), QgsProcessingParameterNumber::Double, 0, true, 0 ) );

  QList<int> datatype;
  datatype << QgsProcessing::TypeVectorPoint;
  addParameter( new QgsProcessingParameterFeatureSource(
                  QStringLiteral( "INPUT_POINTS" ), QObject::tr( "Points for data export" ), datatype, QVariant(), false ) );

  addParameter( new QgsProcessingParameterNumber(
                  QStringLiteral( "COORDINATES_DIGITS" ), QObject::tr( "Digits count for coordinates" ), QgsProcessingParameterNumber::Integer, 2 ) );

  addParameter( new QgsProcessingParameterNumber(
                  QStringLiteral( "DATASET_DIGITS" ), QObject::tr( "Digits count for dataset value" ), QgsProcessingParameterNumber::Integer, 2 ) );

  addParameter( new QgsProcessingParameterFileDestination(
                  QStringLiteral( "OUTPUT" ), QObject::tr( "Exported data CSV file" ), QObject::tr( "CSV file (*.csv)" ) ) );
}

bool QgsMeshExportTimeSeries::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMeshLayer *meshLayer = parameterAsMeshLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !meshLayer || !meshLayer->isValid() )
    return false;

  mMeshLayerCrs = meshLayer->crs();
  mTriangularMesh.update( meshLayer->nativeMesh() );

  QList<int> datasetGroups =
    QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( parameters.value( QStringLiteral( "DATASET_GROUPS" ) ) );

  if ( feedback )
  {
    feedback->setProgressText( QObject::tr( "Preparing data" ) );
  }

  // Extract the date times used to export dataset values
  QVariant parameterStartTimeVariant = parameters.value( QStringLiteral( "STARTING_TIME" ) );
  QgsInterval relativeStartTime = datasetRelativetime( parameterStartTimeVariant, meshLayer, context );

  QVariant parameterEndTimeVariant = parameters.value( QStringLiteral( "FINISHING_TIME" ) );
  QgsInterval relativeEndTime = datasetRelativetime( parameterEndTimeVariant, meshLayer, context );

  // calculate time steps
  qint64 timeStepInterval = parameterAsDouble( parameters, QStringLiteral( "TIME_STEP" ), context ) * 1000 * 3600;
  if ( timeStepInterval == 0 )
  {
    //take the first time step of the first temporal dataset group
    for ( int groupIndex : datasetGroups )
    {
      QgsMeshDatasetGroupMetadata meta = meshLayer->datasetGroupMetadata( QgsMeshDatasetIndex( groupIndex, 0 ) );
      if ( !meta.isTemporal() && meshLayer->datasetCount( QgsMeshDatasetIndex( groupIndex, 0 ) ) < 2 )
        continue;
      else
      {
        timeStepInterval = meshLayer->datasetRelativeTimeInMilliseconds( QgsMeshDatasetIndex( groupIndex, 1 ) )
                           - meshLayer->datasetRelativeTimeInMilliseconds( QgsMeshDatasetIndex( groupIndex, 0 ) );
        break;
      }
    }
  }

  mRelativeTimeSteps.clear();
  mTimeStepString.clear();
  if ( timeStepInterval != 0 )
  {
    mRelativeTimeSteps.append( relativeStartTime.seconds() * 1000 );
    while ( mRelativeTimeSteps.last() < relativeEndTime.seconds() * 1000 )
      mRelativeTimeSteps.append( mRelativeTimeSteps.last() + timeStepInterval );

    for ( qint64  relativeTimeStep : std::as_const( mRelativeTimeSteps ) )
    {
      mTimeStepString.append( meshLayer->formatTime( relativeTimeStep / 3600.0 / 1000.0 ) );
    }
  }

  //Extract needed dataset values
  for ( int i = 0; i < datasetGroups.count(); ++i )
  {
    int  groupIndex = datasetGroups.at( i );
    QgsMeshDatasetGroupMetadata meta = meshLayer->datasetGroupMetadata( QgsMeshDatasetIndex( groupIndex, 0 ) );
    if ( supportedDataType().contains( meta.dataType() ) )
    {
      mGroupIndexes.append( groupIndex );
      mGroupsMetadata[groupIndex] = meta;
      int valueCount = meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ?
                       mTriangularMesh.vertices().count() : meshLayer->nativeMesh()->faceCount();

      if ( !mRelativeTimeSteps.isEmpty() )
      {
        //QMap<qint64, DataGroup> temporalGroup;
        QgsMeshDatasetIndex lastDatasetIndex;
        for ( qint64  relativeTimeStep : mRelativeTimeSteps )
        {
          QMap<int, int> &groupIndexToData = mRelativeTimeToData[relativeTimeStep];
          QgsInterval timeStepInterval( relativeTimeStep / 1000.0 );
          QgsMeshDatasetIndex datasetIndex = meshLayer->datasetIndexAtRelativeTime( timeStepInterval, groupIndex );
          if ( !datasetIndex.isValid() )
            continue;
          if ( datasetIndex != lastDatasetIndex )
          {
            DataGroup dataGroup;
            dataGroup.metadata = meta;
            dataGroup.datasetValues = meshLayer->datasetValues( datasetIndex, 0, valueCount );
            dataGroup.activeFaces = meshLayer->areFacesActive( datasetIndex, 0, meshLayer->nativeMesh()->faceCount() );
            if ( dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVolumes )
            {
              dataGroup.dataset3dStakedValue = meshLayer->dataset3dValues( datasetIndex, 0, valueCount );
            }
            mDatasets.append( dataGroup );
            lastDatasetIndex = datasetIndex;
          }
          groupIndexToData[groupIndex] = mDatasets.count() - 1;
        }
      }
      else
      {
        // we have only static dataset group
        QMap<int, int> &groupIndexToData = mRelativeTimeToData[0];
        QgsMeshDatasetIndex datasetIndex( groupIndex, 0 );
        DataGroup dataGroup;
        dataGroup.metadata = meta;
        dataGroup.datasetValues = meshLayer->datasetValues( datasetIndex, 0, valueCount );
        dataGroup.activeFaces = meshLayer->areFacesActive( datasetIndex, 0, meshLayer->nativeMesh()->faceCount() );
        if ( dataGroup.metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVolumes )
        {
          dataGroup.dataset3dStakedValue = meshLayer->dataset3dValues( datasetIndex, 0, valueCount );
        }
        mDatasets.append( dataGroup );
        groupIndexToData[groupIndex] = mDatasets.count() - 1;
      }
    }

    if ( feedback )
      feedback->setProgress( 100 * i / datasetGroups.count() );
  }

  mLayerRendererSettings = meshLayer->rendererSettings();

  return true;
}


QVariantMap QgsMeshExportTimeSeries::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( feedback )
    feedback->setProgress( 0 );
  //First, if present, average 3D staked dataset value to 2D face value
  const QgsMesh3dAveragingMethod *avgMethod = mLayerRendererSettings.averagingMethod();

  for ( DataGroup &dataGroup : mDatasets )
  {
    if ( dataGroup.dataset3dStakedValue.isValid() )
      dataGroup.datasetValues = avgMethod->calculate( dataGroup.dataset3dStakedValue );
  }

  int datasetDigits = parameterAsInt( parameters, QStringLiteral( "DATASET_DIGITS" ), context );
  int coordDigits = parameterAsInt( parameters, QStringLiteral( "COORDINATES_DIGITS" ), context );

  QgsProcessingFeatureSource *featureSource = parameterAsSource( parameters, QStringLiteral( "INPUT_POINTS" ), context );
  if ( !featureSource )
    throw QgsProcessingException( QObject::tr( "Input points vector layer required" ) );

  QgsCoordinateTransform transform( featureSource->sourceCrs(), mMeshLayerCrs, context.transformContext() );

  QString outputFileName = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );
  QFile file( outputFileName );
  if ( ! file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    throw QgsProcessingException( QObject::tr( "Unable to create the output file" ) );

  QTextStream textStream( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  textStream.setCodec( "UTF-8" );
#endif
  QStringList header;
  header << QStringLiteral( "fid" ) << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QObject::tr( "time" );

  for ( int gi : std::as_const( mGroupIndexes ) )
    header << mGroupsMetadata.value( gi ).name();

  textStream << header.join( ',' ) << QStringLiteral( "\n" );

  long long featCount = featureSource->featureCount();
  long long featCounter = 0;
  QgsFeatureIterator featIt = featureSource->getFeatures();
  QgsFeature feat;
  while ( featIt.nextFeature( feat ) )
  {
    QgsFeatureId fid = feat.id();
    QgsGeometry geom = feat.geometry();
    try
    {
      geom.transform( transform );
    }
    catch ( QgsCsException & )
    {
      geom = feat.geometry();
      feedback->reportError( QObject::tr( "Could not transform line to mesh CRS" ) );
    }

    if ( geom.isEmpty() )
      continue;

    QgsPointXY point = geom.asPoint();
    int triangularFaceIndex = mTriangularMesh.faceIndexForPoint_v2( point );

    if ( triangularFaceIndex >= 0 )
    {
      int nativeFaceIndex = mTriangularMesh.trianglesToNativeFaces().at( triangularFaceIndex );
      if ( !mRelativeTimeSteps.isEmpty() )
      {
        for ( int timeIndex = 0; timeIndex < mRelativeTimeSteps.count(); ++timeIndex )
        {
          qint64 timeStep = mRelativeTimeSteps.at( timeIndex );
          QStringList textLine;
          textLine << QString::number( fid )
                   << QString::number( point.x(), 'f', coordDigits )
                   << QString::number( point.y(), 'f', coordDigits )
                   << mTimeStepString.at( timeIndex );

          if ( mRelativeTimeToData.contains( timeStep ) )
          {
            const QMap<int, int> &groupToData = mRelativeTimeToData.value( timeStep );
            for ( int groupIndex : std::as_const( mGroupIndexes ) )
            {
              if ( !groupToData.contains( groupIndex ) )
                continue;
              int dataIndex = groupToData.value( groupIndex );
              if ( dataIndex < 0 || dataIndex > mDatasets.count() - 1 )
                continue;

              const DataGroup &dataGroup = mDatasets.at( dataIndex );
              QgsMeshDatasetValue value = extractDatasetValue( point,
                                          nativeFaceIndex,
                                          triangularFaceIndex,
                                          mTriangularMesh,
                                          dataGroup.activeFaces,
                                          dataGroup.datasetValues,
                                          dataGroup.metadata );
              if ( abs( value.x() ) == std::numeric_limits<double>::quiet_NaN() )
                textLine << QString( ' ' );
              else
                textLine << QString::number( value.scalar(), 'f', datasetDigits ) ;
            }
          }
          textStream << textLine.join( ',' ) << QStringLiteral( "\n" );
        }
      }
      else
      {
        QStringList textLine;
        textLine << QString::number( fid )
                 << QString::number( point.x(), 'f', coordDigits )
                 << QString::number( point.y(), 'f', coordDigits )
                 << QObject::tr( "static dataset" );
        const QMap<int, int> &groupToData = mRelativeTimeToData.value( 0 );
        for ( int groupIndex : std::as_const( mGroupIndexes ) )
        {
          if ( !groupToData.contains( groupIndex ) )
            continue;
          int dataIndex = groupToData.value( groupIndex );
          if ( dataIndex < 0 || dataIndex > mDatasets.count() - 1 )
            continue;
          const DataGroup &dataGroup = mDatasets.at( dataIndex );
          QgsMeshDatasetValue value = extractDatasetValue( point,
                                      nativeFaceIndex,
                                      triangularFaceIndex,
                                      mTriangularMesh,
                                      dataGroup.activeFaces,
                                      dataGroup.datasetValues,
                                      dataGroup.metadata );
          if ( abs( value.x() ) == std::numeric_limits<double>::quiet_NaN() )
            textLine << QString( ' ' );
          else
            textLine << QString::number( value.scalar(), 'f', datasetDigits );
        }
        textStream << textLine.join( ',' ) << QStringLiteral( "\n" );
      }
    }
    featCounter++;
    if ( feedback )
    {
      feedback->setProgress( 100.0 * featCounter / featCount );
      if ( feedback->isCanceled() )
        return QVariantMap();
    }
  }

  file.close();

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = outputFileName;
  return ret;
}

///@endcond PRIVATE
