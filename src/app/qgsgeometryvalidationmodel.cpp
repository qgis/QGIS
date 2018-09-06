#include "qgsgeometryvalidationmodel.h"

#include "qgsvectorlayer.h"

QgsGeometryValidationModel::QgsGeometryValidationModel( QgsGeometryValidationService *geometryValidationService, QObject *parent )
  : QAbstractItemModel( parent )
  , mGeometryValidationService( geometryValidationService )
{

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
  return mGeometryValidationService->featureErrors( mCurrentLayer ).size();
}

int QgsGeometryValidationModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsGeometryValidationModel::data( const QModelIndex &index, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
      QgsGeometryValidationService::FeatureError error = mGeometryValidationService->featureError( mCurrentLayer, index.row() );
      QgsFeature feature = mCurrentLayer->getFeature( error.featureId );
      mExpressionContext.setFeature( feature );
      QString featureTitle = mDisplayExpression.evaluate( &mExpressionContext ).toString();
      return QStringLiteral( "<b>%1</b>: %2" ).arg( featureTitle, error.error.what() );
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
  mDisplayExpression = mCurrentLayer->displayExpression();
  mExpressionContext = QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( mCurrentLayer ) );
  mDisplayExpression.prepare( &mExpressionContext );
  endResetModel();
}
