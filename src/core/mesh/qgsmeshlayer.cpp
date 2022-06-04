/***************************************************************************
                         qgsmeshlayer.cpp
                         ----------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstddef>
#include <limits>

#include <QUuid>
#include <QUrl>

#include "qgscolorrampimpl.h"
#include "qgslogger.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerfactory.h"
#include "qgsmaplayerutils.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshdatasetgroupstore.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerrenderer.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshtimesettings.h"
#include "qgspainting.h"
#include "qgsproviderregistry.h"
#include "qgsreadwritecontext.h"
#include "qgsstyle.h"
#include "qgstriangularmesh.h"
#include "qgsmesh3daveraging.h"
#include "qgslayermetadataformatter.h"
#include "qgsmesheditor.h"
#include "qgsmessagelog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmeshlayerprofilegenerator.h"
#include "qgsmeshlayerelevationproperties.h"

QgsMeshLayer::QgsMeshLayer( const QString &meshLayerPath,
                            const QString &baseName,
                            const QString &providerKey,
                            const QgsMeshLayer::LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::MeshLayer, baseName, meshLayerPath )
  , mDatasetGroupStore( new QgsMeshDatasetGroupStore( this ) )
  , mTemporalProperties( new QgsMeshLayerTemporalProperties( this ) )
  , mElevationProperties( new QgsMeshLayerElevationProperties( this ) )
{
  mShouldValidateCrs = !options.skipCrsValidation;

  const QgsDataProvider::ProviderOptions providerOptions { options.transformContext };
  QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags();
  if ( options.loadDefaultStyle )
  {
    flags |= QgsDataProvider::FlagLoadDefaultStyle;
  }
  if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
  {
    flags |= QgsDataProvider::FlagTrustDataSource;
  }
  setDataSourcePrivate( meshLayerPath, baseName, providerKey, providerOptions, flags );
  resetDatasetGroupTreeItem();
  setLegend( QgsMapLayerLegend::defaultMeshLegend( this ) );

  if ( isValid() && options.loadDefaultStyle )
    setDefaultRendererSettings( mDatasetGroupStore->datasetGroupIndexes() );

  connect( mDatasetGroupStore.get(), &QgsMeshDatasetGroupStore::datasetGroupsAdded, this, &QgsMeshLayer::onDatasetGroupsAdded );
}


void QgsMeshLayer::setDefaultRendererSettings( const QList<int> &groupIndexes )
{
  QgsMeshRendererMeshSettings meshSettings;
  if ( groupIndexes.count() > 0 )
  {
    // Show data from the first dataset group
    mRendererSettings.setActiveScalarDatasetGroup( 0 );
    // If the first dataset group has nan min/max, display the mesh to avoid nothing displayed
    const QgsMeshDatasetGroupMetadata &meta = datasetGroupMetadata( 0 );
    if ( meta.maximum() == std::numeric_limits<double>::quiet_NaN() &&
         meta.minimum() == std::numeric_limits<double>::quiet_NaN() )
      meshSettings.setEnabled( true );
  }
  else
  {
    // show at least the mesh by default
    meshSettings.setEnabled( true );
    return;
  }
  mRendererSettings.setNativeMeshSettings( meshSettings );

  // Sets default resample method for scalar dataset
  for ( const int i : groupIndexes )
  {
    const QgsMeshDatasetGroupMetadata meta = datasetGroupMetadata( i );
    QgsMeshRendererScalarSettings scalarSettings = mRendererSettings.scalarSettings( i );
    switch ( meta.dataType() )
    {
      case QgsMeshDatasetGroupMetadata::DataOnFaces:
      case QgsMeshDatasetGroupMetadata::DataOnVolumes: // data on volumes are averaged to 2D data on faces
        scalarSettings.setDataResamplingMethod( QgsMeshRendererScalarSettings::NeighbourAverage );
        break;
      case QgsMeshDatasetGroupMetadata::DataOnVertices:
        scalarSettings.setDataResamplingMethod( QgsMeshRendererScalarSettings::None );
        break;
      case QgsMeshDatasetGroupMetadata::DataOnEdges:
        break;
    }

    //override color ramp if the values in the dataset group are classified
    applyClassificationOnScalarSettings( meta, scalarSettings );

    mRendererSettings.setScalarSettings( i, scalarSettings );
  }

}

void QgsMeshLayer::createSimplifiedMeshes()
{
  if ( mSimplificationSettings.isEnabled() && !hasSimplifiedMeshes() )
  {
    const double reductionFactor = mSimplificationSettings.reductionFactor();

    QVector<QgsTriangularMesh *> simplifyMeshes =
      mTriangularMeshes[0]->simplifyMesh( reductionFactor );

    for ( int i = 0; i < simplifyMeshes.count() ; ++i )
    {
      mTriangularMeshes.emplace_back( simplifyMeshes[i] );
    }
  }
}

bool QgsMeshLayer::hasSimplifiedMeshes() const
{
  //First mesh is the base mesh, so if size>1, there is no simplified meshes
  return ( mTriangularMeshes.size() > 1 );
}

QgsMeshLayer::~QgsMeshLayer()
{
  delete mDataProvider;
}

QgsMeshDataProvider *QgsMeshLayer::dataProvider()
{
  return mDataProvider;
}

const QgsMeshDataProvider *QgsMeshLayer::dataProvider() const
{
  return mDataProvider;
}

QgsMeshLayer *QgsMeshLayer::clone() const
{
  QgsMeshLayer::LayerOptions options;
  if ( mDataProvider )
  {
    options.transformContext = mDataProvider->transformContext();
  }
  QgsMeshLayer *layer = new QgsMeshLayer( source(), name(), mProviderKey,  options );
  QgsMapLayer::clone( layer );

  layer->mElevationProperties = mElevationProperties->clone();
  layer->mElevationProperties->setParent( layer );

  return layer;
}

QgsRectangle QgsMeshLayer::extent() const
{
  if ( mMeshEditor )
    return mMeshEditor->extent();

  if ( mDataProvider )
    return mDataProvider->extent();
  else
  {
    QgsRectangle rec;
    rec.setMinimal();
    return rec;
  }
}

QString QgsMeshLayer::providerType() const
{
  return mProviderKey;
}

bool QgsMeshLayer::supportsEditing() const
{
  if ( !mDataProvider )
    return false;

  if ( mMeshEditor )
    return true;

  const QgsMeshDriverMetadata driverMetadata = mDataProvider->driverMetadata();

  return driverMetadata.capabilities() & QgsMeshDriverMetadata::CanWriteMeshData;
}

QString QgsMeshLayer::loadDefaultStyle( bool &resultFlag )
{
  const QList<int> groupsList = datasetGroupsIndexes();

  for ( const int index : groupsList )
    assignDefaultStyleToDatasetGroup( index );

  if ( !groupsList.isEmpty() )
  {
    emit rendererChanged();
    emitStyleChanged();
  }

  return QgsMapLayer::loadDefaultStyle( resultFlag );
}

bool QgsMeshLayer::addDatasets( const QString &path, const QDateTime &defaultReferenceTime )
{
  const bool isTemporalBefore = dataProvider()->temporalCapabilities()->hasTemporalCapabilities();
  if ( mDatasetGroupStore->addPersistentDatasets( path ) )
  {
    mExtraDatasetUri.append( path );
    QgsMeshLayerTemporalProperties *temporalProperties = qobject_cast< QgsMeshLayerTemporalProperties * >( mTemporalProperties );
    if ( !isTemporalBefore && dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
    {
      mTemporalProperties->setDefaultsFromDataProviderTemporalCapabilities(
        dataProvider()->temporalCapabilities() );

      if ( ! temporalProperties->referenceTime().isValid() )
      {
        QDateTime referenceTime = defaultReferenceTime;
        if ( !defaultReferenceTime.isValid() ) // If project reference time is invalid, use current date
          referenceTime = QDateTime( QDate::currentDate(), QTime( 0, 0, 0 ), Qt::UTC );
        temporalProperties->setReferenceTime( referenceTime, dataProvider()->temporalCapabilities() );
      }

      mTemporalProperties->setIsActive( true );
    }
    emit dataSourceChanged();
    return true;
  }

  return false;
}

bool QgsMeshLayer::addDatasets( QgsMeshDatasetGroup *datasetGroup )
{
  if ( mDatasetGroupStore->addDatasetGroup( datasetGroup ) )
  {
    emit dataChanged();
    return true;
  }
  return false;
}

bool QgsMeshLayer::saveDataset( const QString &path, int datasetGroupIndex, QString driver )
{
  return mDatasetGroupStore->saveDatasetGroup( path, datasetGroupIndex, driver );
}

QgsMesh *QgsMeshLayer::nativeMesh()
{
  return mNativeMesh.get();
}

const QgsMesh *QgsMeshLayer::nativeMesh() const
{
  return mNativeMesh.get();
}

QgsTriangularMesh *QgsMeshLayer::triangularMesh( double minimumTriangleSize ) const
{
  for ( const std::unique_ptr<QgsTriangularMesh> &lod : mTriangularMeshes )
  {
    if ( lod && lod->averageTriangleSize() > minimumTriangleSize )
      return lod.get();
  }

  if ( !mTriangularMeshes.empty() )
    return mTriangularMeshes.back().get();
  else
    return nullptr;
}

int QgsMeshLayer::triangularMeshLevelOfDetailCount() const SIP_SKIP
{
  return mTriangularMeshes.size();
}

QgsTriangularMesh *QgsMeshLayer::triangularMeshByLodIndex( int lodIndex ) const SIP_SKIP
{
  if ( mTriangularMeshes.empty() )
    return nullptr;
  if ( lodIndex < 0 )
    return mTriangularMeshes.front().get();

  if ( lodIndex >= int( mTriangularMeshes.size() ) )
    return mTriangularMeshes.back().get();

  return mTriangularMeshes.at( lodIndex ).get();
}

void  QgsMeshLayer::updateTriangularMesh( const QgsCoordinateTransform &transform )
{
  // Native mesh
  if ( !mNativeMesh )
  {
    // lazy loading of mesh data
    fillNativeMesh();
  }

  // Triangular mesh
  if ( mTriangularMeshes.empty() )
  {
    QgsTriangularMesh *baseMesh = new QgsTriangularMesh;
    mTriangularMeshes.emplace_back( baseMesh );
  }

  if ( mTriangularMeshes[0].get()->update( mNativeMesh.get(), transform ) )
    mTriangularMeshes.resize( 1 ); //if the base triangular mesh is effectivly updated, remove simplified meshes

  createSimplifiedMeshes();
}

QgsMeshLayerRendererCache *QgsMeshLayer::rendererCache()
{
  return mRendererCache.get();
}

QgsMeshRendererSettings QgsMeshLayer::rendererSettings() const
{
  return mRendererSettings;
}

void QgsMeshLayer::setRendererSettings( const QgsMeshRendererSettings &settings )
{
  const int oldActiveScalar = mRendererSettings.activeScalarDatasetGroup();
  const int oldActiveVector = mRendererSettings.activeVectorDatasetGroup();
  mRendererSettings = settings;

  if ( oldActiveScalar != mRendererSettings.activeScalarDatasetGroup() )
    emit activeScalarDatasetGroupChanged( mRendererSettings.activeScalarDatasetGroup() );

  if ( oldActiveVector != mRendererSettings.activeVectorDatasetGroup() )
    emit activeVectorDatasetGroupChanged( mRendererSettings.activeVectorDatasetGroup() );

  emit rendererChanged();
  triggerRepaint();
}

QgsMeshTimeSettings QgsMeshLayer::timeSettings() const
{
  return mTimeSettings;
}

void QgsMeshLayer::setTimeSettings( const QgsMeshTimeSettings &settings )
{
  mTimeSettings = settings;
  emit timeSettingsChanged();
}

QString QgsMeshLayer::formatTime( double hours )
{
  if ( dataProvider() && dataProvider()->temporalCapabilities()->hasReferenceTime() )
    return QgsMeshLayerUtils::formatTime( hours, mTemporalProperties->referenceTime(), mTimeSettings );
  else
    return QgsMeshLayerUtils::formatTime( hours, QDateTime(), mTimeSettings );
}

int QgsMeshLayer::datasetGroupCount() const
{
  return mDatasetGroupStore->datasetGroupCount();
}

int QgsMeshLayer::extraDatasetGroupCount() const
{
  return mDatasetGroupStore->extraDatasetGroupCount();
}

QList<int> QgsMeshLayer::datasetGroupsIndexes() const
{
  return mDatasetGroupStore->datasetGroupIndexes();
}

QList<int> QgsMeshLayer::enabledDatasetGroupsIndexes() const
{
  return mDatasetGroupStore->enabledDatasetGroupIndexes();
}

QgsMeshDatasetGroupMetadata QgsMeshLayer::datasetGroupMetadata( const QgsMeshDatasetIndex &index ) const
{
  return mDatasetGroupStore->datasetGroupMetadata( index );
}

int QgsMeshLayer::datasetCount( const QgsMeshDatasetIndex &index ) const
{
  return mDatasetGroupStore->datasetCount( index.group() );
}

QgsMeshDatasetMetadata QgsMeshLayer::datasetMetadata( const QgsMeshDatasetIndex &index ) const
{
  return mDatasetGroupStore->datasetMetadata( index );
}

QgsMeshDatasetValue QgsMeshLayer::datasetValue( const QgsMeshDatasetIndex &index, int valueIndex ) const
{
  return mDatasetGroupStore->datasetValue( index, valueIndex );
}

QgsMeshDataBlock QgsMeshLayer::datasetValues( const QgsMeshDatasetIndex &index, int valueIndex, int count ) const
{
  return mDatasetGroupStore->datasetValues( index, valueIndex, count );
}

QgsMesh3dDataBlock QgsMeshLayer::dataset3dValues( const QgsMeshDatasetIndex &index, int faceIndex, int count ) const
{
  return mDatasetGroupStore->dataset3dValues( index, faceIndex, count );
}

QgsMeshDataBlock QgsMeshLayer::areFacesActive( const QgsMeshDatasetIndex &index, int faceIndex, int count ) const
{
  return mDatasetGroupStore->areFacesActive( index, faceIndex, count );
}

bool QgsMeshLayer::isFaceActive( const QgsMeshDatasetIndex &index, int faceIndex ) const
{
  return mDatasetGroupStore->isFaceActive( index, faceIndex );
}

QgsMeshDatasetValue QgsMeshLayer::datasetValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point, double searchRadius ) const
{
  QgsMeshDatasetValue value;
  const QgsTriangularMesh *mesh = triangularMesh();

  if ( mesh && index.isValid() )
  {
    if ( contains( QgsMesh::ElementType::Edge ) )
    {
      const QgsRectangle searchRectangle( point.x() - searchRadius, point.y() - searchRadius, point.x() + searchRadius, point.y() + searchRadius );
      return dataset1dValue( index, point, searchRadius );
    }
    const int faceIndex = mesh->faceIndexForPoint_v2( point ) ;
    if ( faceIndex >= 0 )
    {
      const int nativeFaceIndex = mesh->trianglesToNativeFaces().at( faceIndex );
      const QgsMeshDatasetGroupMetadata::DataType dataType = datasetGroupMetadata( index ).dataType();
      if ( isFaceActive( index, nativeFaceIndex ) )
      {
        switch ( dataType )
        {
          case QgsMeshDatasetGroupMetadata::DataOnFaces:
          {
            value = datasetValue( index, nativeFaceIndex );
          }
          break;

          case QgsMeshDatasetGroupMetadata::DataOnVertices:
          {
            const QgsMeshFace &face = mesh->triangles()[faceIndex];
            const int v1 = face[0], v2 = face[1], v3 = face[2];
            const QgsPoint p1 = mesh->vertices()[v1], p2 = mesh->vertices()[v2], p3 = mesh->vertices()[v3];
            const QgsMeshDatasetValue val1 = datasetValue( index, v1 );
            const QgsMeshDatasetValue val2 = datasetValue( index, v2 );
            const QgsMeshDatasetValue val3 = datasetValue( index, v3 );
            const double x = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.x(), val2.x(), val3.x(), point );
            double y = std::numeric_limits<double>::quiet_NaN();
            const bool isVector = datasetGroupMetadata( index ).isVector();
            if ( isVector )
              y = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.y(), val2.y(), val3.y(), point );

            value = QgsMeshDatasetValue( x, y );
          }
          break;

          case QgsMeshDatasetGroupMetadata::DataOnVolumes:
          {
            const QgsMesh3dAveragingMethod *avgMethod = mRendererSettings.averagingMethod();
            if ( avgMethod )
            {
              const QgsMesh3dDataBlock block3d = dataset3dValues( index, nativeFaceIndex, 1 );
              const QgsMeshDataBlock block2d = avgMethod->calculate( block3d );
              if ( block2d.isValid() )
              {
                value = block2d.value( 0 );
              }
            }
          }
          break;

          default:
            break;
        }
      }
    }
  }

  return value;
}

QgsMesh3dDataBlock QgsMeshLayer::dataset3dValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point ) const
{
  QgsMesh3dDataBlock block3d;

  const QgsTriangularMesh *baseTriangularMesh = triangularMesh();

  if ( baseTriangularMesh && dataProvider() && dataProvider()->isValid() && index.isValid() )
  {
    const QgsMeshDatasetGroupMetadata::DataType dataType = datasetGroupMetadata( index ).dataType();
    if ( dataType == QgsMeshDatasetGroupMetadata::DataOnVolumes )
    {
      const int faceIndex = baseTriangularMesh->faceIndexForPoint_v2( point );
      if ( faceIndex >= 0 )
      {
        const int nativeFaceIndex = baseTriangularMesh->trianglesToNativeFaces().at( faceIndex );
        block3d = dataset3dValues( index, nativeFaceIndex, 1 );
      }
    }
  }
  return block3d;
}

QgsMeshDatasetValue QgsMeshLayer::dataset1dValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point, double searchRadius ) const
{
  QgsMeshDatasetValue value;
  QgsPointXY projectedPoint;
  const int selectedIndex = closestEdge( point, searchRadius, projectedPoint );
  const QgsTriangularMesh *mesh = triangularMesh();
  if ( selectedIndex >= 0 )
  {
    const QgsMeshDatasetGroupMetadata::DataType dataType = datasetGroupMetadata( index ).dataType();
    switch ( dataType )
    {
      case QgsMeshDatasetGroupMetadata::DataOnEdges:
      {
        value = datasetValue( index, selectedIndex );
      }
      break;

      case QgsMeshDatasetGroupMetadata::DataOnVertices:
      {
        const QgsMeshEdge &edge = mesh->edges()[selectedIndex];
        const int v1 = edge.first, v2 = edge.second;
        const QgsPoint p1 = mesh->vertices()[v1], p2 = mesh->vertices()[v2];
        const QgsMeshDatasetValue val1 = datasetValue( index, v1 );
        const QgsMeshDatasetValue val2 = datasetValue( index, v2 );
        const double edgeLength = p1.distance( p2 );
        const double dist1 = p1.distance( projectedPoint.x(), projectedPoint.y() );
        value = QgsMeshLayerUtils::interpolateFromVerticesData( dist1 / edgeLength, val1, val2 );
      }
      break;
      default:
        break;
    }
  }

  return value;
}

void QgsMeshLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );
  invalidateWgs84Extent();
}

QgsMeshDatasetIndex QgsMeshLayer::datasetIndexAtTime( const QgsDateTimeRange &timeRange, int datasetGroupIndex ) const
{
  if ( ! mTemporalProperties->isActive() )
    return QgsMeshDatasetIndex( datasetGroupIndex, -1 );

  const QDateTime layerReferenceTime = mTemporalProperties->referenceTime();
  QDateTime utcTime = timeRange.begin();
  if ( utcTime.timeSpec() != Qt::UTC )
    utcTime.setTimeSpec( Qt::UTC );
  const qint64 startTime = layerReferenceTime.msecsTo( utcTime );

  return  mDatasetGroupStore->datasetIndexAtTime( startTime, datasetGroupIndex, mTemporalProperties->matchingMethod() );
}

QgsMeshDatasetIndex QgsMeshLayer::datasetIndexAtRelativeTime( const QgsInterval &relativeTime, int datasetGroupIndex ) const
{
  return  mDatasetGroupStore->datasetIndexAtTime( relativeTime.seconds() * 1000, datasetGroupIndex, mTemporalProperties->matchingMethod() );
}

QList<QgsMeshDatasetIndex> QgsMeshLayer::datasetIndexInRelativeTimeInterval( const QgsInterval &startRelativeTime, const QgsInterval &endRelativeTime, int datasetGroupIndex ) const
{
  qint64 usedRelativeTime1 = startRelativeTime.seconds() * 1000;
  qint64 usedRelativeTime2 = endRelativeTime.seconds() * 1000;

  //adjust relative time if layer reference time is different from provider reference time
  if ( mTemporalProperties->referenceTime().isValid() &&
       mDataProvider &&
       mDataProvider->isValid() &&
       mTemporalProperties->referenceTime() != mDataProvider->temporalCapabilities()->referenceTime() )
  {
    usedRelativeTime1 = usedRelativeTime1 + mTemporalProperties->referenceTime().msecsTo( mDataProvider->temporalCapabilities()->referenceTime() );
    usedRelativeTime2 = usedRelativeTime2 + mTemporalProperties->referenceTime().msecsTo( mDataProvider->temporalCapabilities()->referenceTime() );
  }

  return  mDatasetGroupStore->datasetIndexInTimeInterval( usedRelativeTime1, usedRelativeTime2, datasetGroupIndex );
}

void QgsMeshLayer::applyClassificationOnScalarSettings( const QgsMeshDatasetGroupMetadata &meta, QgsMeshRendererScalarSettings &scalarSettings ) const
{
  if ( meta.extraOptions().contains( QStringLiteral( "classification" ) ) )
  {
    QgsColorRampShader colorRampShader = scalarSettings.colorRampShader();
    QgsColorRamp *colorRamp = colorRampShader.sourceColorRamp();
    const QStringList classes = meta.extraOptions()[QStringLiteral( "classification" )].split( QStringLiteral( ";;" ) );

    QString units;
    if ( meta.extraOptions().contains( QStringLiteral( "units" ) ) )
      units = meta.extraOptions()[ QStringLiteral( "units" )];

    QVector<QVector<double>> bounds;
    for ( const QString &classe : classes )
    {
      const QStringList boundsStr = classe.split( ',' );
      QVector<double> bound;
      for ( const QString &boundStr : boundsStr )
        bound.append( boundStr.toDouble() );
      bounds.append( bound );
    }

    if ( ( bounds.count() == 1  && bounds.first().count() > 2 ) || // at least a class with two value
         ( bounds.count() > 1 ) ) // or at least two classes
    {
      const QVector<double> firstClass = bounds.first();
      const QVector<double> lastClass = bounds.last();
      const double minValue = firstClass.count() > 1 ? ( firstClass.first() + firstClass.last() ) / 2 : firstClass.first();
      const double maxValue = lastClass.count() > 1 ? ( lastClass.first() + lastClass.last() ) / 2 : lastClass.first();
      const double diff = maxValue - minValue;
      QList<QgsColorRampShader::ColorRampItem> colorRampItemlist;
      for ( int i = 0; i < bounds.count(); ++i )
      {
        const QVector<double> &boundClass = bounds.at( i );
        QgsColorRampShader::ColorRampItem item;
        item.value = i + 1;
        if ( !boundClass.isEmpty() )
        {
          const double scalarValue = ( boundClass.first() + boundClass.last() ) / 2;
          item.color = colorRamp->color( ( scalarValue - minValue ) / diff );
          if ( i != 0 && i < bounds.count() - 1 ) //The first and last labels are treated after
          {
            item.label = QString( ( "%1 - %2 %3" ) ).
                         arg( QString::number( boundClass.first() ) ).
                         arg( QString::number( boundClass.last() ) ).
                         arg( units );
          }
        }
        colorRampItemlist.append( item );
      }
      //treat first and last labels
      if ( firstClass.count() == 1 )
        colorRampItemlist.first().label = QObject::tr( "below %1 %2" ).
                                          arg( QString::number( firstClass.first() ) ).
                                          arg( units );
      else
      {
        colorRampItemlist.first().label = QString( ( "%1 - %2 %3" ) ).
                                          arg( QString::number( firstClass.first() ) ).
                                          arg( QString::number( firstClass.last() ) ).
                                          arg( units );
      }

      if ( lastClass.count() == 1 )
        colorRampItemlist.last().label = QObject::tr( "above %1 %2" ).
                                         arg( QString::number( lastClass.first() ) ).
                                         arg( units );
      else
      {
        colorRampItemlist.last().label = QString( ( "%1 - %2 %3" ) ).
                                         arg( QString::number( lastClass.first() ) ).
                                         arg( QString::number( lastClass.last() ) ).
                                         arg( units );
      }

      colorRampShader.setMinimumValue( 0 );
      colorRampShader.setMaximumValue( colorRampItemlist.count() - 1 );
      scalarSettings.setClassificationMinimumMaximum( 0, colorRampItemlist.count() - 1 );
      colorRampShader.setColorRampItemList( colorRampItemlist );
      colorRampShader.setColorRampType( QgsColorRampShader::Exact );
      colorRampShader.setClassificationMode( QgsColorRampShader::EqualInterval );
    }

    scalarSettings.setColorRampShader( colorRampShader );
    scalarSettings.setDataResamplingMethod( QgsMeshRendererScalarSettings::None );
  }
}

QgsMeshDatasetIndex QgsMeshLayer::activeScalarDatasetAtTime( const QgsDateTimeRange &timeRange ) const
{
  if ( mTemporalProperties->isActive() )
    return datasetIndexAtTime( timeRange, mRendererSettings.activeScalarDatasetGroup() );
  else
    return QgsMeshDatasetIndex( mRendererSettings.activeScalarDatasetGroup(), mStaticScalarDatasetIndex );
}

QgsMeshDatasetIndex QgsMeshLayer::activeVectorDatasetAtTime( const QgsDateTimeRange &timeRange ) const
{
  if ( mTemporalProperties->isActive() )
    return datasetIndexAtTime( timeRange, mRendererSettings.activeVectorDatasetGroup() );
  else
    return QgsMeshDatasetIndex( mRendererSettings.activeVectorDatasetGroup(), mStaticVectorDatasetIndex );
}

void QgsMeshLayer::fillNativeMesh()
{
  Q_ASSERT( !mNativeMesh );

  mNativeMesh.reset( new QgsMesh() );

  if ( !( dataProvider() && dataProvider()->isValid() ) )
    return;

  dataProvider()->populateMesh( mNativeMesh.get() );
}

void QgsMeshLayer::onDatasetGroupsAdded( const QList<int> &datasetGroupIndexes )
{
  // assign default style to new dataset groups
  for ( int datasetGroupIndex : datasetGroupIndexes )
  {
    if ( !mRendererSettings.hasSettings( datasetGroupIndex ) )
      assignDefaultStyleToDatasetGroup( datasetGroupIndex );
  }

  temporalProperties()->setIsActive( mDatasetGroupStore->hasTemporalCapabilities() );
  emit rendererChanged();
}

void QgsMeshLayer::onMeshEdited()
{
  mRendererCache.reset( new QgsMeshLayerRendererCache() );
  emit layerModified();
  triggerRepaint();
  trigger3DUpdate();
}

QgsMeshDatasetGroupTreeItem *QgsMeshLayer::datasetGroupTreeRootItem() const
{
  return mDatasetGroupStore->datasetGroupTreeItem();
}

void QgsMeshLayer::setDatasetGroupTreeRootItem( QgsMeshDatasetGroupTreeItem *rootItem )
{
  mDatasetGroupStore->setDatasetGroupTreeItem( rootItem );
  updateActiveDatasetGroups();
}

int QgsMeshLayer::closestEdge( const QgsPointXY &point, double searchRadius, QgsPointXY &projectedPoint ) const
{
  const QgsRectangle searchRectangle( point.x() - searchRadius, point.y() - searchRadius, point.x() + searchRadius, point.y() + searchRadius );
  const QgsTriangularMesh *mesh = triangularMesh();
  // search for the closest edge in search area from point
  const QList<int> edgeIndexes = mesh->edgeIndexesForRectangle( searchRectangle );
  int selectedIndex = -1;
  if ( mesh->contains( QgsMesh::Edge ) &&
       mDataProvider->isValid() )
  {
    double sqrMaxDistFromPoint = pow( searchRadius, 2 );
    for ( const int edgeIndex : edgeIndexes )
    {
      const QgsMeshEdge &edge = mesh->edges().at( edgeIndex );
      const QgsMeshVertex &vertex1 = mesh->vertices()[edge.first];
      const QgsMeshVertex &vertex2 = mesh->vertices()[edge.second];
      QgsPointXY projPoint;
      const double sqrDist = point.sqrDistToSegment( vertex1.x(), vertex1.y(), vertex2.x(), vertex2.y(), projPoint );
      if ( sqrDist < sqrMaxDistFromPoint )
      {
        selectedIndex = edgeIndex;
        projectedPoint = projPoint;
        sqrMaxDistFromPoint = sqrDist;
      }
    }
  }

  return selectedIndex;
}

QgsMeshDatasetIndex QgsMeshLayer::staticVectorDatasetIndex() const
{
  return QgsMeshDatasetIndex( mRendererSettings.activeVectorDatasetGroup(), mStaticVectorDatasetIndex );
}

void QgsMeshLayer::setReferenceTime( const QDateTime &referenceTime )
{
  if ( auto *lDataProvider = dataProvider() )
    mTemporalProperties->setReferenceTime( referenceTime, lDataProvider->temporalCapabilities() );
  else
    mTemporalProperties->setReferenceTime( referenceTime, nullptr );
}

void QgsMeshLayer::setTemporalMatchingMethod( const QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod &matchingMethod )
{
  mTemporalProperties->setMatchingMethod( matchingMethod );
}

QgsPointXY QgsMeshLayer::snapOnVertex( const QgsPointXY &point, double searchRadius )
{
  const QgsTriangularMesh *mesh = triangularMesh();
  QgsPointXY exactPosition;
  if ( !mesh )
    return exactPosition;
  const QgsRectangle rectangle( point.x() - searchRadius, point.y() - searchRadius, point.x() + searchRadius, point.y() + searchRadius );
  double maxDistance = searchRadius;
  //attempt to snap on edges's vertices
  const QList<int> edgeIndexes = mesh->edgeIndexesForRectangle( rectangle );
  for ( const int edgeIndex : edgeIndexes )
  {
    const QgsMeshEdge &edge = mesh->edges().at( edgeIndex );
    const QgsMeshVertex &vertex1 = mesh->vertices()[edge.first];
    const QgsMeshVertex &vertex2 = mesh->vertices()[edge.second];
    const double dist1 = point.distance( vertex1 );
    const double dist2 = point.distance( vertex2 );
    if ( dist1 < maxDistance )
    {
      maxDistance = dist1;
      exactPosition = vertex1;
    }
    if ( dist2 < maxDistance )
    {
      maxDistance = dist2;
      exactPosition = vertex2;
    }
  }

  //attempt to snap on face's vertices
  const QList<int> faceIndexes = mesh->faceIndexesForRectangle( rectangle );
  for ( const int faceIndex : faceIndexes )
  {
    const QgsMeshFace &face = mesh->triangles().at( faceIndex );
    for ( int i = 0; i < 3; ++i )
    {
      const QgsMeshVertex &vertex = mesh->vertices()[face.at( i )];
      const double dist = point.distance( vertex );
      if ( dist < maxDistance )
      {
        maxDistance = dist;
        exactPosition = vertex;
      }
    }
  }

  return exactPosition;
}

QgsPointXY QgsMeshLayer::snapOnEdge( const QgsPointXY &point, double searchRadius )
{
  QgsPointXY projectedPoint;
  closestEdge( point, searchRadius, projectedPoint );

  return projectedPoint;
}

QgsPointXY QgsMeshLayer::snapOnFace( const QgsPointXY &point, double searchRadius )
{
  const QgsTriangularMesh *mesh = triangularMesh();
  QgsPointXY centroidPosition;
  if ( !mesh )
    return centroidPosition;
  const QgsRectangle rectangle( point.x() - searchRadius, point.y() - searchRadius, point.x() + searchRadius, point.y() + searchRadius );
  double maxDistance = std::numeric_limits<double>::max();

  const QList<int> faceIndexes = mesh->faceIndexesForRectangle( rectangle );
  for ( const int faceIndex : faceIndexes )
  {
    const int nativefaceIndex = mesh->trianglesToNativeFaces().at( faceIndex );
    if ( nativefaceIndex < 0 && nativefaceIndex >= mesh->faceCentroids().count() )
      continue;
    const QgsPointXY centroid = mesh->faceCentroids()[nativefaceIndex];
    const double dist = point.distance( centroid );
    if ( dist < maxDistance )
    {
      maxDistance = dist;
      centroidPosition = centroid;
    }
  }

  return centroidPosition;
}

void QgsMeshLayer::resetDatasetGroupTreeItem()
{
  mDatasetGroupStore->resetDatasetGroupTreeItem();
  updateActiveDatasetGroups();
}

QgsInterval QgsMeshLayer::firstValidTimeStep() const
{
  if ( !mDataProvider )
    return QgsInterval();
  const int groupCount = mDataProvider->datasetGroupCount();
  for ( int i = 0; i < groupCount; ++i )
  {
    const qint64 timeStep = mDataProvider->temporalCapabilities()->firstTimeStepDuration( i );
    if ( timeStep > 0 )
      return QgsInterval( timeStep, QgsUnitTypes::TemporalMilliseconds );
  }

  return QgsInterval();
}

QgsInterval QgsMeshLayer::datasetRelativeTime( const QgsMeshDatasetIndex &index )
{
  const qint64 time = mDatasetGroupStore->datasetRelativeTime( index );

  if ( time == INVALID_MESHLAYER_TIME )
    return QgsInterval();
  else
    return QgsInterval( time, QgsUnitTypes::TemporalMilliseconds );
}

qint64 QgsMeshLayer::datasetRelativeTimeInMilliseconds( const QgsMeshDatasetIndex &index )
{
  return mDatasetGroupStore->datasetRelativeTime( index );
}

static QString detailsErrorMessage( const QgsMeshEditingError &error )
{
  QString message;

  switch ( error.errorType )
  {
    case Qgis::MeshEditingErrorType::NoError:
      break;
    case Qgis::MeshEditingErrorType::InvalidFace:
      message = QObject::tr( "Face %1 invalid" ).arg( error.elementIndex );
      break;
    case Qgis::MeshEditingErrorType::TooManyVerticesInFace:
      message =  QObject::tr( "Too many vertices for face %1" ).arg( error.elementIndex );
      break;
    case Qgis::MeshEditingErrorType::FlatFace:
      message =  QObject::tr( "Face %1 is flat" ).arg( error.elementIndex );
      break;
    case Qgis::MeshEditingErrorType::UniqueSharedVertex:
      message =  QObject::tr( "Vertex %1 is a unique shared vertex" ).arg( error.elementIndex );
      break;
    case Qgis::MeshEditingErrorType::InvalidVertex:
      message =  QObject::tr( "Vertex %1 is invalid" ).arg( error.elementIndex );
      break;
    case Qgis::MeshEditingErrorType::ManifoldFace:
      message =  QObject::tr( "Face %1 is manifold" ).arg( error.elementIndex );
      break;
  }

  return message;
}

bool QgsMeshLayer::startFrameEditing( const QgsCoordinateTransform &transform )
{
  if ( !supportsEditing() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Mesh layer \"%1\" not support mesh editing" ).arg( name() ) );
    return false;
  }

  if ( mMeshEditor )
  {
    QgsMessageLog::logMessage( QObject::tr( "Mesh layer \"%1\" already in editing mode" ).arg( name() ) );
    return false;
  }

  mSimplificationSettings.setEnabled( false );

  updateTriangularMesh( transform );

  mMeshEditor = new QgsMeshEditor( this );

  const QgsMeshEditingError error = mMeshEditor->initialize();

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
  {
    mMeshEditor->deleteLater();
    mMeshEditor = nullptr;

    QgsMessageLog::logMessage( QObject::tr( "Unable to start editing of mesh layer \"%1\": %2" ).
                               arg( name(), detailsErrorMessage( error ) ), QString(), Qgis::MessageLevel::Critical );
    return false;
  }

  // During editing, we don't need anymore the provider data. Mesh frame data is stored in the mesh editor.
  mDataProvider->close();

  // All dataset group are removed and replace by a unique virtual dataset group that provide vertices elevation value.
  mExtraDatasetUri.clear();
  mDatasetGroupStore.reset( new QgsMeshDatasetGroupStore( this ) );
  mDatasetGroupStore->addDatasetGroup( mMeshEditor->createZValueDatasetGroup() );
  resetDatasetGroupTreeItem();

  connect( mMeshEditor, &QgsMeshEditor::meshEdited, this, &QgsMeshLayer::onMeshEdited );

  emit dataChanged();
  emit editingStarted();

  return true;
}

bool QgsMeshLayer::commitFrameEditing( const QgsCoordinateTransform &transform, bool continueEditing )
{
  QgsMeshEditingError error;
  QString detailsError;
  if ( !mMeshEditor->checkConsistency( error ) )
  {
    if ( error.errorType == Qgis::MeshEditingErrorType::NoError )
      detailsError = tr( "Unknown inconsistent mesh error" );
  }
  else
  {
    error = QgsTopologicalMesh::checkTopology( *mNativeMesh, mMeshEditor->maximumVerticesPerFace() );
    detailsError = detailsErrorMessage( error );
  }

  if ( !detailsError.isEmpty() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Edited mesh layer \"%1\" can't be save due to an error: %2" ).
                               arg( name(), detailsError ), QString(), Qgis::MessageLevel::Critical );
    return false;
  }

  stopFrameEditing( transform );

  if ( !mDataProvider )
    return false;

  const bool res = mDataProvider->saveMeshFrame( *mNativeMesh.get() );

  if ( continueEditing )
  {
    mMeshEditor->initialize();
    emit layerModified();
    return res;
  }

  mMeshEditor->deleteLater();
  mMeshEditor = nullptr;
  emit editingStopped();

  mDataProvider->reloadData();
  mDataProvider->populateMesh( mNativeMesh.get() );
  mDatasetGroupStore.reset( new QgsMeshDatasetGroupStore( this ) );
  mDatasetGroupStore->setPersistentProvider( mDataProvider, QStringList() );
  resetDatasetGroupTreeItem();
  return true;
}

bool QgsMeshLayer::rollBackFrameEditing( const QgsCoordinateTransform &transform, bool continueEditing )
{
  stopFrameEditing( transform );

  if ( !mDataProvider )
    return false;

  mTriangularMeshes.clear();
  mDataProvider->reloadData();
  mDataProvider->populateMesh( mNativeMesh.get() );
  updateTriangularMesh( transform );
  mRendererCache.reset( new QgsMeshLayerRendererCache() );
  trigger3DUpdate();

  if ( continueEditing )
  {
    mMeshEditor->resetTriangularMesh( triangularMesh() );
    return mMeshEditor->initialize() == QgsMeshEditingError();
  }
  else
  {
    mMeshEditor->deleteLater();
    mMeshEditor = nullptr;
    emit editingStopped();

    mDatasetGroupStore.reset( new QgsMeshDatasetGroupStore( this ) );
    mDatasetGroupStore->setPersistentProvider( mDataProvider, QStringList() );
    resetDatasetGroupTreeItem();
    emit dataChanged();
    return true;
  }
}

void QgsMeshLayer::stopFrameEditing( const QgsCoordinateTransform &transform )
{
  if ( !mMeshEditor )
    return;

  mMeshEditor->stopEditing();
  mTriangularMeshes.at( 0 )->update( mNativeMesh.get(), transform );
  mRendererCache.reset( new QgsMeshLayerRendererCache() );
}

bool QgsMeshLayer::reindex( const QgsCoordinateTransform &transform, bool renumber )
{
  if ( !mMeshEditor )
    return false;

  if ( !mMeshEditor->reindex( renumber ) )
    return false;

  mTriangularMeshes.clear();
  mTriangularMeshes.emplace_back( new QgsTriangularMesh );
  mTriangularMeshes.at( 0 )->update( mNativeMesh.get(), transform );
  mRendererCache.reset( new QgsMeshLayerRendererCache() );
  mMeshEditor->resetTriangularMesh( mTriangularMeshes.at( 0 ).get() );

  return true;
}

QgsMeshEditor *QgsMeshLayer::meshEditor()
{
  return mMeshEditor;
}

bool QgsMeshLayer::isModified() const
{
  if ( mMeshEditor )
    return mMeshEditor->isModified();

  return false;
}

bool QgsMeshLayer::contains( const QgsMesh::ElementType &type ) const
{
  switch ( type )
  {
    case QgsMesh::ElementType::Vertex:
      return meshVertexCount() != 0;
    case QgsMesh::ElementType::Edge:
      return meshEdgeCount() != 0;
    case QgsMesh::ElementType::Face:
      return meshFaceCount() != 0;
  }
  return false;
}

int QgsMeshLayer::meshVertexCount() const
{
  if ( mMeshEditor )
    return mMeshEditor->validVerticesCount();
  else if ( mDataProvider )
    return mDataProvider->vertexCount();
  else return 0;
}

int QgsMeshLayer::meshFaceCount() const
{
  if ( mMeshEditor )
    return mMeshEditor->validFacesCount();
  else if ( mDataProvider )
    return mDataProvider->faceCount();
  else return 0;
}

int QgsMeshLayer::meshEdgeCount() const
{
  if ( mMeshEditor )
    return mNativeMesh->edgeCount();
  else if ( mDataProvider )
    return mDataProvider->edgeCount();
  else return 0;
}

void QgsMeshLayer::updateActiveDatasetGroups()
{
  QgsMeshDatasetGroupTreeItem *treeItem = mDatasetGroupStore->datasetGroupTreeItem();

  if ( !mDatasetGroupStore->datasetGroupTreeItem() )
    return;

  QgsMeshRendererSettings settings = rendererSettings();
  const int oldActiveScalar = settings.activeScalarDatasetGroup();
  const int oldActiveVector = settings.activeVectorDatasetGroup();

  QgsMeshDatasetGroupTreeItem *activeScalarItem =
    treeItem->childFromDatasetGroupIndex( oldActiveScalar );

  if ( !activeScalarItem && treeItem->childCount() > 0 )
    activeScalarItem = treeItem->child( 0 );

  if ( activeScalarItem && !activeScalarItem->isEnabled() )
  {
    for ( int i = 0; i < treeItem->childCount(); ++i )
    {
      activeScalarItem = treeItem->child( i );
      if ( activeScalarItem->isEnabled() )
        break;
      else
        activeScalarItem = nullptr;
    }
  }

  if ( activeScalarItem )
    settings.setActiveScalarDatasetGroup( activeScalarItem->datasetGroupIndex() );
  else
    settings.setActiveScalarDatasetGroup( -1 );

  QgsMeshDatasetGroupTreeItem *activeVectorItem =
    treeItem->childFromDatasetGroupIndex( oldActiveVector );

  if ( !( activeVectorItem && activeVectorItem->isEnabled() ) )
    settings.setActiveVectorDatasetGroup( -1 );

  setRendererSettings( settings );

  if ( oldActiveScalar != settings.activeScalarDatasetGroup() )
    emit activeScalarDatasetGroupChanged( settings.activeScalarDatasetGroup() );
  if ( oldActiveVector != settings.activeVectorDatasetGroup() )
    emit activeVectorDatasetGroupChanged( settings.activeVectorDatasetGroup() );
}

void QgsMeshLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  mDataSource = dataSource;
  mLayerName = baseName;
  setProviderType( provider );

  if ( !mDataSource.isEmpty() && !provider.isEmpty() )
    setDataProvider( provider, options, flags );
}

QgsPointXY QgsMeshLayer::snapOnElement( QgsMesh::ElementType elementType, const QgsPointXY &point, double searchRadius )
{
  switch ( elementType )
  {
    case QgsMesh::Vertex:
      return snapOnVertex( point, searchRadius );
    case QgsMesh::Edge:
      return snapOnEdge( point, searchRadius );
    case QgsMesh::Face:
      return snapOnFace( point, searchRadius );
  }
  return QgsPointXY(); // avoid warnings
}

QList<int> QgsMeshLayer::selectVerticesByExpression( QgsExpression expression )
{
  if ( !mNativeMesh )
  {
    // lazy loading of mesh data
    fillNativeMesh();
  }

  QList<int> ret;

  if ( !mNativeMesh )
    return ret;

  QgsExpressionContext context;
  std::unique_ptr<QgsExpressionContextScope> expScope( QgsExpressionContextUtils::meshExpressionScope( QgsMesh::Vertex ) );
  context.appendScope( expScope.release() );
  context.lastScope()->setVariable( QStringLiteral( "_mesh_layer" ), QVariant::fromValue( this ) );

  expression.prepare( &context );

  for ( int i = 0; i < mNativeMesh->vertexCount(); ++i )
  {
    context.lastScope()->setVariable( QStringLiteral( "_mesh_vertex_index" ), i, false );

    if ( expression.evaluate( &context ).toBool() )
      ret.append( i );
  }

  return ret;
}

QList<int> QgsMeshLayer::selectFacesByExpression( QgsExpression expression )
{
  if ( !mNativeMesh )
  {
    // lazy loading of mesh data
    fillNativeMesh();
  }

  QList<int> ret;

  if ( !mNativeMesh )
    return ret;

  QgsExpressionContext context;
  std::unique_ptr<QgsExpressionContextScope> expScope( QgsExpressionContextUtils::meshExpressionScope( QgsMesh::Face ) );
  context.appendScope( expScope.release() );
  context.lastScope()->setVariable( QStringLiteral( "_mesh_layer" ), QVariant::fromValue( this ) );

  expression.prepare( &context );

  for ( int i = 0; i < mNativeMesh->faceCount(); ++i )
  {
    context.lastScope()->setVariable( QStringLiteral( "_mesh_face_index" ), i, false );

    if ( expression.evaluate( &context ).toBool() )
      ret.append( i );
  }

  return ret;
}

QgsMeshDatasetIndex QgsMeshLayer::staticScalarDatasetIndex() const
{
  return QgsMeshDatasetIndex( mRendererSettings.activeScalarDatasetGroup(), mStaticScalarDatasetIndex );
}

void QgsMeshLayer::setStaticVectorDatasetIndex( const QgsMeshDatasetIndex &staticVectorDatasetIndex )
{
  const int oldActiveVector = mRendererSettings.activeVectorDatasetGroup();

  mStaticVectorDatasetIndex = staticVectorDatasetIndex.dataset();
  mRendererSettings.setActiveVectorDatasetGroup( staticVectorDatasetIndex.group() );

  if ( oldActiveVector != mRendererSettings.activeVectorDatasetGroup() )
    emit activeVectorDatasetGroupChanged( mRendererSettings.activeVectorDatasetGroup() );
}

void QgsMeshLayer::setStaticScalarDatasetIndex( const QgsMeshDatasetIndex &staticScalarDatasetIndex )
{
  const int oldActiveScalar = mRendererSettings.activeScalarDatasetGroup();

  mStaticScalarDatasetIndex = staticScalarDatasetIndex.dataset();
  mRendererSettings.setActiveScalarDatasetGroup( staticScalarDatasetIndex.group() );

  if ( oldActiveScalar != mRendererSettings.activeScalarDatasetGroup() )
    emit activeScalarDatasetGroupChanged( mRendererSettings.activeScalarDatasetGroup() );
}

QgsMeshSimplificationSettings QgsMeshLayer::meshSimplificationSettings() const
{
  return mSimplificationSettings;
}

void QgsMeshLayer::setMeshSimplificationSettings( const QgsMeshSimplificationSettings &simplifySettings )
{
  mSimplificationSettings = simplifySettings;
}

static QgsColorRamp *_createDefaultColorRamp()
{
  QgsColorRamp *ramp = QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Plasma" ) );
  if ( ramp )
    return ramp;

  // definition of "Plasma" color ramp (in case it is not available in the style for some reason)
  QVariantMap props;
  props["color1"] = "13,8,135,255";
  props["color2"] = "240,249,33,255";
  props["stops"] =
    "0.0196078;27,6,141,255:0.0392157;38,5,145,255:0.0588235;47,5,150,255:0.0784314;56,4,154,255:0.0980392;65,4,157,255:"
    "0.117647;73,3,160,255:0.137255;81,2,163,255:0.156863;89,1,165,255:0.176471;97,0,167,255:0.196078;105,0,168,255:"
    "0.215686;113,0,168,255:0.235294;120,1,168,255:0.254902;128,4,168,255:0.27451;135,7,166,255:0.294118;142,12,164,255:"
    "0.313725;149,17,161,255:0.333333;156,23,158,255:0.352941;162,29,154,255:0.372549;168,34,150,255:0.392157;174,40,146,255:"
    "0.411765;180,46,141,255:0.431373;186,51,136,255:0.45098;191,57,132,255:0.470588;196,62,127,255:0.490196;201,68,122,255:"
    "0.509804;205,74,118,255:0.529412;210,79,113,255:0.54902;214,85,109,255:0.568627;218,91,105,255:0.588235;222,97,100,255:"
    "0.607843;226,102,96,255:0.627451;230,108,92,255:0.647059;233,114,87,255:0.666667;237,121,83,255:0.686275;240,127,79,255:"
    "0.705882;243,133,75,255:0.72549;245,140,70,255:0.745098;247,147,66,255:0.764706;249,154,62,255:0.784314;251,161,57,255:"
    "0.803922;252,168,53,255:0.823529;253,175,49,255:0.843137;254,183,45,255:0.862745;254,190,42,255:0.882353;253,198,39,255:"
    "0.901961;252,206,37,255:0.921569;251,215,36,255:0.941176;248,223,37,255:0.960784;246,232,38,255:0.980392;243,240,39,255";
  return QgsGradientColorRamp::create( props );
}

void QgsMeshLayer::assignDefaultStyleToDatasetGroup( int groupIndex )
{
  const QgsMeshDatasetGroupMetadata metadata = datasetGroupMetadata( groupIndex );
  const double groupMin = metadata.minimum();
  const double groupMax = metadata.maximum();

  QgsColorRampShader fcn( groupMin, groupMax, _createDefaultColorRamp() );
  fcn.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );

  QgsMeshRendererScalarSettings scalarSettings;
  scalarSettings.setClassificationMinimumMaximum( groupMin, groupMax );
  scalarSettings.setColorRampShader( fcn );
  QgsInterpolatedLineWidth edgeStrokeWidth;
  edgeStrokeWidth.setMinimumValue( groupMin );
  edgeStrokeWidth.setMaximumValue( groupMax );
  const QgsInterpolatedLineColor edgeStrokeColor( fcn );
  const QgsInterpolatedLineRenderer edgeStrokePen;
  scalarSettings.setEdgeStrokeWidth( edgeStrokeWidth );
  mRendererSettings.setScalarSettings( groupIndex, scalarSettings );

  if ( metadata.isVector() )
  {
    QgsMeshRendererVectorSettings vectorSettings;
    vectorSettings.setColorRampShader( fcn );
    mRendererSettings.setVectorSettings( groupIndex, vectorSettings );
  }
}

QgsMapLayerRenderer *QgsMeshLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  // Triangular mesh
  updateTriangularMesh( rendererContext.coordinateTransform() );

  // Build overview triangular meshes if needed
  createSimplifiedMeshes();

  // Cache
  if ( !mRendererCache )
    mRendererCache.reset( new QgsMeshLayerRendererCache() );

  return new QgsMeshLayerRenderer( this, rendererContext );
}

QgsAbstractProfileGenerator *QgsMeshLayer::createProfileGenerator( const QgsProfileRequest &request )
{
  return new QgsMeshLayerProfileGenerator( this, request );
}

bool QgsMeshLayer::readSymbology( const QDomNode &node, QString &errorMessage,
                                  QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  Q_UNUSED( errorMessage )
  // TODO: implement categories for raster layer

  const QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  const QDomElement elemRendererSettings = elem.firstChildElement( "mesh-renderer-settings" );
  if ( !elemRendererSettings.isNull() )
    mRendererSettings.readXml( elemRendererSettings, context );

  const QDomElement elemSimplifySettings = elem.firstChildElement( "mesh-simplify-settings" );
  if ( !elemSimplifySettings.isNull() )
    mSimplificationSettings.readXml( elemSimplifySettings, context );

  // get and set the blend mode if it exists
  const QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
  if ( !blendModeNode.isNull() )
  {
    const QDomElement e = blendModeNode.toElement();
    setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
  }

  // get and set the layer transparency
  if ( categories.testFlag( Rendering ) )
  {
    const QDomNode layerOpacityNode = node.namedItem( QStringLiteral( "layerOpacity" ) );
    if ( !layerOpacityNode.isNull() )
    {
      const QDomElement e = layerOpacityNode.toElement();
      setOpacity( e.text().toDouble() );
    }
  }

  return true;
}

bool QgsMeshLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                                   const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( errorMessage )
  // TODO: implement categories for raster layer

  QDomElement elem = node.toElement();

  writeCommonStyle( elem, doc, context, categories );

  const QDomElement elemRendererSettings = mRendererSettings.writeXml( doc, context );
  elem.appendChild( elemRendererSettings );

  const QDomElement elemSimplifySettings = mSimplificationSettings.writeXml( doc, context );
  elem.appendChild( elemSimplifySettings );

  // add blend mode node
  QDomElement blendModeElement  = doc.createElement( QStringLiteral( "blendMode" ) );
  const QDomText blendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
  blendModeElement.appendChild( blendModeText );
  node.appendChild( blendModeElement );

  // add the layer opacity
  if ( categories.testFlag( Rendering ) )
  {
    QDomElement layerOpacityElem  = doc.createElement( QStringLiteral( "layerOpacity" ) );
    const QDomText layerOpacityText = doc.createTextNode( QString::number( opacity() ) );
    layerOpacityElem.appendChild( layerOpacityText );
    node.appendChild( layerOpacityElem );
  }

  return true;
}

bool QgsMeshLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  return writeSymbology( node, doc, errorMessage, context, categories );
}

bool QgsMeshLayer::readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  return readSymbology( node, errorMessage, context, categories );
}

QString QgsMeshLayer::decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const
{
  QString src( source );
  if ( provider == QLatin1String( "mdal" ) )
  {
    src = context.pathResolver().readPath( src );
  }
  return src;
}

QString QgsMeshLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QString src( source );
  if ( providerType() == QLatin1String( "mdal" ) )
  {
    src = context.pathResolver().writePath( src );
  }
  return src;
}

bool QgsMeshLayer::readXml( const QDomNode &layer_node, QgsReadWriteContext &context )
{
  QgsDebugMsgLevel( QStringLiteral( "Datasource in QgsMeshLayer::readXml: %1" ).arg( mDataSource.toLocal8Bit().data() ), 3 );

  //process provider key
  const QDomNode pkeyNode = layer_node.namedItem( QStringLiteral( "provider" ) );

  if ( pkeyNode.isNull() )
  {
    mProviderKey.clear();
  }
  else
  {
    const QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
  }

  if ( mReadFlags & QgsMapLayer::FlagDontResolveLayers )
  {
    return false;
  }

  const QgsDataProvider::ProviderOptions providerOptions;
  QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags();
  if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
  {
    flags |= QgsDataProvider::FlagTrustDataSource;
  }

  const QDomElement elemExtraDatasets = layer_node.firstChildElement( QStringLiteral( "extra-datasets" ) );
  if ( !elemExtraDatasets.isNull() )
  {
    QDomElement elemUri = elemExtraDatasets.firstChildElement( QStringLiteral( "uri" ) );
    while ( !elemUri.isNull() )
    {
      const QString uri = context.pathResolver().readPath( elemUri.text() );
      mExtraDatasetUri.append( uri );
      elemUri = elemUri.nextSiblingElement( QStringLiteral( "uri" ) );
    }
  }

  if ( pkeyNode.toElement().hasAttribute( QStringLiteral( "time-unit" ) ) )
    mTemporalUnit = static_cast<QgsUnitTypes::TemporalUnit>( pkeyNode.toElement().attribute( QStringLiteral( "time-unit" ) ).toInt() );

  // read dataset group store
  const QDomElement elemDatasetGroupsStore = layer_node.firstChildElement( QStringLiteral( "mesh-dataset-groups-store" ) );
  if ( elemDatasetGroupsStore.isNull() )
    resetDatasetGroupTreeItem();
  else
    mDatasetGroupStore->readXml( elemDatasetGroupsStore, context );

  setDataProvider( mProviderKey, providerOptions, flags );

  QString errorMsg;
  readSymbology( layer_node, errorMsg, context );

  if ( !mTemporalProperties->timeExtent().begin().isValid() )
    temporalProperties()->setDefaultsFromDataProviderTemporalCapabilities( dataProvider()->temporalCapabilities() );

  // read static dataset
  const QDomElement elemStaticDataset = layer_node.firstChildElement( QStringLiteral( "static-active-dataset" ) );
  if ( elemStaticDataset.hasAttribute( QStringLiteral( "scalar" ) ) )
  {
    mStaticScalarDatasetIndex = elemStaticDataset.attribute( QStringLiteral( "scalar" ) ).toInt();
  }
  if ( elemStaticDataset.hasAttribute( QStringLiteral( "vector" ) ) )
  {
    mStaticVectorDatasetIndex = elemStaticDataset.attribute( QStringLiteral( "vector" ) ).toInt();
  }

  return isValid(); // should be true if read successfully
}

bool QgsMeshLayer::writeXml( QDomNode &layer_node, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  // first get the layer element so that we can append the type attribute
  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || ( QLatin1String( "maplayer" ) != mapLayerNode.nodeName() ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "can't find <maplayer>" ), 2 );
    return false;
  }

  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( QgsMapLayerType::MeshLayer ) );

  // add provider node
  if ( mDataProvider )
  {
    QDomElement provider  = document.createElement( QStringLiteral( "provider" ) );
    const QDomText providerText = document.createTextNode( providerType() );
    provider.appendChild( providerText );
    layer_node.appendChild( provider );
    provider.setAttribute( QStringLiteral( "time-unit" ), mDataProvider->temporalCapabilities()->temporalUnit() );

    const QStringList extraDatasetUris = mDataProvider->extraDatasets();
    QDomElement elemExtraDatasets = document.createElement( QStringLiteral( "extra-datasets" ) );
    for ( const QString &uri : extraDatasetUris )
    {
      const QString path = context.pathResolver().writePath( uri );
      QDomElement elemUri = document.createElement( QStringLiteral( "uri" ) );
      elemUri.appendChild( document.createTextNode( path ) );
      elemExtraDatasets.appendChild( elemUri );
    }
    layer_node.appendChild( elemExtraDatasets );
  }

  QDomElement elemStaticDataset = document.createElement( QStringLiteral( "static-active-dataset" ) );
  elemStaticDataset.setAttribute( QStringLiteral( "scalar" ), mStaticScalarDatasetIndex );
  elemStaticDataset.setAttribute( QStringLiteral( "vector" ), mStaticVectorDatasetIndex );
  layer_node.appendChild( elemStaticDataset );

  // write dataset group store if not in edting mode
  if ( !isEditable() )
    layer_node.appendChild( mDatasetGroupStore->writeXml( document, context ) );

  // renderer specific settings
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg, context );
}

void QgsMeshLayer::reload()
{
  if ( !mMeshEditor && mDataProvider && mDataProvider->isValid() )
  {
    mDataProvider->reloadData();

    //reload the mesh structure
    if ( !mNativeMesh )
      mNativeMesh.reset( new QgsMesh );

    dataProvider()->populateMesh( mNativeMesh.get() );

    //clear the TriangularMeshes
    mTriangularMeshes.clear();

    //clear the rendererCache
    mRendererCache.reset( new QgsMeshLayerRendererCache() );
  }
}

QStringList QgsMeshLayer::subLayers() const
{
  if ( mDataProvider )
    return mDataProvider->subLayers();
  else
    return QStringList();
}

QString QgsMeshLayer::htmlMetadata() const
{
  const QgsLayerMetadataFormatter htmlFormatter( metadata() );
  QString myMetadata = QStringLiteral( "<html>\n<body>\n" );

  myMetadata += generalHtmlMetadata();

  // Begin Provider section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += QLatin1String( "<table class=\"list-view\">\n" );

  // Extent
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Extent" ) + QStringLiteral( "</td><td>" ) + extent().toString() + QStringLiteral( "</td></tr>\n" );

  // feature count
  QLocale locale = QLocale();
  locale.setNumberOptions( locale.numberOptions() &= ~QLocale::NumberOption::OmitGroupSeparator );

  if ( dataProvider() )
  {
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Vertex count" ) + QStringLiteral( "</td><td>" )
                  + ( locale.toString( static_cast<qlonglong>( meshVertexCount() ) ) )
                  + QStringLiteral( "</td></tr>\n" );
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Face count" ) + QStringLiteral( "</td><td>" )
                  + ( locale.toString( static_cast<qlonglong>( meshFaceCount() ) ) )
                  + QStringLiteral( "</td></tr>\n" );
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Edge count" ) + QStringLiteral( "</td><td>" )
                  + ( locale.toString( static_cast<qlonglong>( meshEdgeCount() ) ) )
                  + QStringLiteral( "</td></tr>\n" );
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Dataset groups count" ) + QStringLiteral( "</td><td>" )
                  + ( locale.toString( static_cast<qlonglong>( datasetGroupCount() ) ) )
                  + QStringLiteral( "</td></tr>\n" );
  }

  // End Provider section
  myMetadata += QLatin1String( "</table>\n<br><br>" );

  // CRS
  myMetadata += crsHtmlMetadata();

  // identification section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Identification" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.identificationSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // extent section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Extent" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.extentSectionHtml( isSpatial() );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the Access section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Access" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.accessSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the contacts section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Contacts" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.contactsSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the links section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Links" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.linksSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the history section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "History" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.historySectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  myMetadata += QLatin1String( "\n</body>\n</html>\n" );
  return myMetadata;
}

bool QgsMeshLayer::isEditable() const
{
  return mMeshEditor != nullptr;
}

bool QgsMeshLayer::setDataProvider( QString const &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  mDatasetGroupStore->setPersistentProvider( nullptr, QStringList() );

  delete mDataProvider;
  mProviderKey = provider;
  const QString dataSource = mDataSource;

  mDataProvider = qobject_cast<QgsMeshDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource, options, flags ) );

  if ( !mDataProvider )
  {
    QgsDebugMsgLevel( QStringLiteral( "Unable to get mesh data provider" ), 2 );
    return false;
  }

  mDataProvider->setParent( this );
  QgsDebugMsgLevel( QStringLiteral( "Instantiated the mesh data provider plugin" ), 2 );

  setValid( mDataProvider->isValid() );
  if ( !isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Invalid mesh provider plugin %1" ).arg( QString( mDataSource.toUtf8() ) ), 2 );
    return false;
  }

  if ( !mTemporalProperties->isValid() )
  {
    mTemporalProperties->setDefaultsFromDataProviderTemporalCapabilities( dataProvider()->temporalCapabilities() );
  }

  mDataProvider->setTemporalUnit( mTemporalUnit );

  mDatasetGroupStore->setPersistentProvider( mDataProvider, mExtraDatasetUri );

  setCrs( mDataProvider->crs() );

  if ( provider == QLatin1String( "mesh_memory" ) )
  {
    // required so that source differs between memory layers
    mDataSource = mDataSource + QStringLiteral( "&uid=%1" ).arg( QUuid::createUuid().toString() );
  }

  // set default style if required by flags or if the dataset group does not has a style yet
  for ( int i = 0; i < mDataProvider->datasetGroupCount(); ++i )
  {
    int globalIndex = mDatasetGroupStore->globalDatasetGroupIndexInSource( mDataProvider, i );
    if ( globalIndex != -1 &&
         ( !mRendererSettings.hasSettings( globalIndex ) || ( flags & QgsDataProvider::FlagLoadDefaultStyle ) ) )
      assignDefaultStyleToDatasetGroup( globalIndex );
  }

  emit rendererChanged();
  emitStyleChanged();

  connect( mDataProvider, &QgsMeshDataProvider::dataChanged, this, &QgsMeshLayer::dataChanged );

  return true;
}

QgsMapLayerTemporalProperties *QgsMeshLayer::temporalProperties()
{
  return mTemporalProperties;
}

QgsMapLayerElevationProperties *QgsMeshLayer::elevationProperties()
{
  return mElevationProperties;
}
