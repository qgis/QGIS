#include "qgsgeometryvalidationmodel.h"

#include "qgsvectorlayer.h"
#include "qgssinglegeometrycheck.h"

#include <QIcon>

QgsGeometryValidationModel::QgsGeometryValidationModel( QgsGeometryValidationService *geometryValidationService, QObject *parent )
  : QAbstractItemModel( parent )
  , mGeometryValidationService( geometryValidationService )
{
  connect( mGeometryValidationService, &QgsGeometryValidationService::geometryCheckCompleted, this, &QgsGeometryValidationModel::onGeometryCheckCompleted );
  connect( mGeometryValidationService, &QgsGeometryValidationService::geometryCheckStarted, this, &QgsGeometryValidationModel::onGeometryCheckStarted );
  connect( mGeometryValidationService, &QgsGeometryValidationService::topologyChecksUpdated, this, &QgsGeometryValidationModel::onTopologyChecksUpdated );
  connect( mGeometryValidationService, &QgsGeometryValidationService::topologyChecksCleared, this, &QgsGeometryValidationModel::onTopologyChecksCleared );
}

QModelIndex QgsGeometryValidationModel::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column );
}

QModelIndex QgsGeometryValidationModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsGeometryValidationModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mErrorStorage.value( mCurrentLayer ).size() + mTopologyErrorStorage.value( mCurrentLayer ).size();
}

int QgsGeometryValidationModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsGeometryValidationModel::data( const QModelIndex &index, int role ) const
{
  const auto &layerErrors = mErrorStorage.value( mCurrentLayer );

  if ( index.row() >= layerErrors.size() )
  {
    // Topology error
    const auto &topologyErrors = mTopologyErrorStorage.value( mCurrentLayer );
    auto topologyError = topologyErrors.at( index.row() - layerErrors.size() );

    switch ( role )
    {
      case Qt::DisplayRole:
      {
        const QgsFeatureId fid = topologyError->featureId();
        const QgsFeature feature = mCurrentLayer->getFeature( fid ); // TODO: this should be cached!
        mExpressionContext.setFeature( feature );
        const QVariant featureTitle = mDisplayExpression.evaluate( &mExpressionContext );

        if ( featureTitle.isNull() )
        {
          return topologyError->description();
        }
        else
        {
          return tr( "%1: %2" ).arg( featureTitle.toString(), topologyError->description() );
        }

      }

      case FeatureExtentRole:
      {
        const QgsFeatureId fid = topologyError->featureId();
        const QgsFeature feature = mCurrentLayer->getFeature( fid ); // TODO: this should be cached!
        return feature.geometry().boundingBox();
      }

      case ProblemExtentRole:
      {
        return topologyError->affectedAreaBBox();
      }

      case ErrorGeometryRole:
      {
        // TODO: save as QgsGeometry already in the error
        return QgsGeometry( topologyError->geometry()->clone() );
      }

      case FeatureGeometryRole:
      {
        const QgsFeatureId fid = topologyError->featureId();
        const QgsFeature feature = mCurrentLayer->getFeature( fid ); // TODO: this should be cached!
        return feature.geometry();
      }

      case ErrorLocationGeometryRole:
      {
        return topologyError->location();
      }

      case GeometryCheckErrorRole:
      {
        return QVariant::fromValue<QgsGeometryCheckError *>( topologyError.get() );
      }
    }
  }
  else
  {
    // Geometry error
    const auto &featureItem = layerErrors.at( index.row() );

    switch ( role )
    {
      case Qt::DisplayRole:
      {
        QgsFeature feature = mCurrentLayer->getFeature( featureItem.fid ); // TODO: this should be cached!
        mExpressionContext.setFeature( feature );
        QString featureTitle = mDisplayExpression.evaluate( &mExpressionContext ).toString();
        if ( featureTitle.isEmpty() )
          featureTitle = featureItem.fid;

        if ( featureItem.errors.count() > 1 )
          return tr( "%1: %n Errors", "", featureItem.errors.count() ).arg( featureTitle );
        else if ( featureItem.errors.count() == 1 )
          return tr( "%1: %2" ).arg( featureTitle, featureItem.errors.at( 0 )->description() );
        else
          return QVariant();
      }

      case Qt::DecorationRole:
      {
        if ( mGeometryValidationService->validationActive( mCurrentLayer, featureItem.fid ) )
          return QgsApplication::getThemeIcon( "/mActionTracing.svg" );
        else
          return QVariant();
      }

      default:
        return QVariant();
    }
  }


  return QVariant();
}

QgsVectorLayer *QgsGeometryValidationModel::currentLayer() const
{
  return mCurrentLayer;
}

void QgsGeometryValidationModel::setCurrentLayer( QgsVectorLayer *currentLayer )
{
  if ( mCurrentLayer == currentLayer )
    return;

  beginResetModel();
  mCurrentLayer = currentLayer;
  if ( mCurrentLayer )
  {
    mDisplayExpression = mCurrentLayer ? mCurrentLayer->displayExpression() : QString();
    mExpressionContext = QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( mCurrentLayer ) );
    mDisplayExpression.prepare( &mExpressionContext );
  }
  else
  {
    mDisplayExpression = QString();
    mExpressionContext = QgsExpressionContext();
  }
  endResetModel();
}

void QgsGeometryValidationModel::onGeometryCheckCompleted( QgsVectorLayer *layer, QgsFeatureId fid, const QList<std::shared_ptr<QgsSingleGeometryCheckError>> &errors )
{
  auto &layerErrors = mErrorStorage[layer];

  int featureIdx = errorsForFeature( layer, fid );

  // The last check for this feature finished: remove
  if ( featureIdx > -1 && errors.empty() && !mGeometryValidationService->validationActive( layer, fid ) )
  {
    if ( mCurrentLayer == layer )
      beginRemoveRows( QModelIndex(), featureIdx, featureIdx );

    layerErrors.removeAt( featureIdx );

    if ( mCurrentLayer == layer )
      endRemoveRows();
  }
  else if ( !errors.empty() )
  {
    // a new or updated feature
    if ( featureIdx == -1 )
    {
      featureIdx = layerErrors.count();
      if ( mCurrentLayer == layer )
        beginInsertRows( QModelIndex(), featureIdx, featureIdx );

      layerErrors << FeatureErrors( fid );

      if ( mCurrentLayer == layer )
        endInsertRows();
    }

    auto &featureItem = layerErrors[featureIdx];
    featureItem.errors.append( errors );
    if ( mCurrentLayer == layer )
    {
      QModelIndex modelIndex = index( featureIdx, 0, QModelIndex() );
      emit dataChanged( modelIndex, modelIndex );
    }
  }
}

void QgsGeometryValidationModel::onGeometryCheckStarted( QgsVectorLayer *layer, QgsFeatureId fid )
{
  auto &layerErrors = mErrorStorage[layer];
  int featureIdx = errorsForFeature( layer, fid );
  if ( featureIdx != -1 )
  {
    auto &featureItem = layerErrors[featureIdx];

    featureItem.errors.clear();

    if ( mCurrentLayer == layer )
    {
      QModelIndex modelIndex = index( featureIdx, 0, QModelIndex() );
      emit dataChanged( modelIndex, modelIndex );
    }
  }
}

void QgsGeometryValidationModel::onTopologyChecksUpdated( QgsVectorLayer *layer, const QList<std::shared_ptr<QgsGeometryCheckError> > &errors )
{
  auto &topologyLayerErrors = mTopologyErrorStorage[layer];

  if ( layer == currentLayer() )
  {
    const int oldRowCount = rowCount();
    beginInsertRows( QModelIndex(), oldRowCount, oldRowCount + errors.size() );
  }

  topologyLayerErrors.append( errors );

  if ( layer == currentLayer() )
  {
    endInsertRows();
  }
}

void QgsGeometryValidationModel::onTopologyChecksCleared( QgsVectorLayer *layer )
{
  auto &topologyLayerErrors = mTopologyErrorStorage[layer];
  if ( topologyLayerErrors.empty() )
    return;

  if ( layer == currentLayer() )
  {
    beginRemoveRows( QModelIndex(), mErrorStorage.size(), rowCount() - 1 );
  }
  topologyLayerErrors.clear();
  if ( layer == currentLayer() )
  {
    endRemoveRows();
  }
}

int QgsGeometryValidationModel::errorsForFeature( QgsVectorLayer *layer, QgsFeatureId fid )
{
  const auto &layerErrors = mErrorStorage[layer];
  int idx = 0;

  for ( const auto &feature : layerErrors )
  {
    if ( feature.fid == fid )
      return idx;
    idx++;
  }
  return -1;
}
