#include "qgsgeometryvalidationmodel.h"

#include "qgsvectorlayer.h"

#include <QIcon>

QgsGeometryValidationModel::QgsGeometryValidationModel( QgsGeometryValidationService *geometryValidationService, QObject *parent )
  : QAbstractItemModel( parent )
  , mGeometryValidationService( geometryValidationService )
{
  connect( mGeometryValidationService, &QgsGeometryValidationService::geometryCheckCompleted, this, &QgsGeometryValidationModel::onGeometryCheckCompleted );
  connect( mGeometryValidationService, &QgsGeometryValidationService::geometryCheckStarted, this, &QgsGeometryValidationModel::onGeometryCheckStarted );
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
  return mErrorStorage.value( mCurrentLayer ).size();
}

int QgsGeometryValidationModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsGeometryValidationModel::data( const QModelIndex &index, int role ) const
{
  const auto &layerErrors = mErrorStorage.value( mCurrentLayer );

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
        return tr( "%1: %2" ).arg( featureTitle, featureItem.errors.at( 0 ).what() );
#if 0
      else
        return tr( "%1: No Errors" ).arg( featureTitle );
#endif
    }


    case Qt::DecorationRole:
    {
      if ( mGeometryValidationService->validationActive( mCurrentLayer, featureItem.fid ) )
        return QgsApplication::getThemeIcon( "/mActionTracing.svg" );
      else
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

void QgsGeometryValidationModel::onGeometryCheckCompleted( QgsVectorLayer *layer, QgsFeatureId fid, const QList<QgsGeometry::Error> &errors )
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
