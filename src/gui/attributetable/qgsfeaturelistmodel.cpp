#include "qgsexception.h"
#include "qgsvectordataprovider.h"
#include "qgsfeaturelistmodel.h"
#include "qgsattributetablemodel.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsattributetablefiltermodel.h"

#include <QItemSelection>

QgsFeatureListModel::QgsFeatureListModel( QgsAttributeTableFilterModel *sourceModel, QObject *parent )
    : QAbstractProxyModel( parent )
{
  setSourceModel( sourceModel );
  mExpression = new QgsExpression( "" );
}

QgsFeatureListModel::~QgsFeatureListModel()
{
  delete mExpression;
}

void QgsFeatureListModel::setSourceModel( QgsAttributeTableFilterModel *sourceModel )
{
  QAbstractProxyModel::setSourceModel( sourceModel );
  mFilterModel = sourceModel;
  if ( mFilterModel )
  {
    // rewire (filter-)change events in the source model so this proxy reflects the changes
    connect( mFilterModel, SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ), SLOT( onBeginRemoveRows( const QModelIndex&, int, int ) ) );
    connect( mFilterModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ), SLOT( onEndRemoveRows( const QModelIndex&, int, int ) ) );
    connect( mFilterModel, SIGNAL( rowsAboutToBeInserted( const QModelIndex&, int, int ) ), SLOT( onBeginInsertRows( const QModelIndex&, int, int ) ) );
    connect( mFilterModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ), SLOT( onEndInsertRows( const QModelIndex&, int, int ) ) );
    // propagate sort order changes from source model to views connected to this model
    connect( mFilterModel, SIGNAL( layoutAboutToBeChanged() ), this, SIGNAL( layoutAboutToBeChanged() ) );
    connect( mFilterModel, SIGNAL( layoutChanged() ), this, SIGNAL( layoutChanged() ) );
  }
}

QgsVectorLayerCache *QgsFeatureListModel::layerCache()
{
  return mFilterModel->layerCache();
}

QgsFeatureId QgsFeatureListModel::idxToFid( const QModelIndex &index ) const
{
  return mFilterModel->masterModel()->rowToId( mapToMaster( index ).row() );
}

QModelIndex QgsFeatureListModel::fidToIdx( const QgsFeatureId fid ) const
{
  return mFilterModel->mapFromMaster( mFilterModel->masterModel()->idToIndex( fid ) );
}

QVariant QgsFeatureListModel::data( const QModelIndex &index, int role ) const
{
  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    QgsFeature feat;

    mFilterModel->layerCache()->featureAtId( idxToFid( index ), feat );

    const QgsFields fields = mFilterModel->layer()->pendingFields();

    return mExpression->evaluate( &feat, fields );
  }

  if ( role == Qt::UserRole )
  {
    FeatureInfo featInfo;

    QgsFeature feat;

    mFilterModel->layerCache()->featureAtId( idxToFid( index ), feat );

    QgsVectorLayerEditBuffer* editBuffer = mFilterModel->layer()->editBuffer();

    if ( editBuffer )
    {
      const QList<QgsFeatureId> addedFeatures = editBuffer->addedFeatures().keys();
      const QList<QgsFeatureId> changedFeatures = editBuffer->changedAttributeValues().keys();

      if ( addedFeatures.contains( feat.id() ) )
      {
        featInfo.isNew = true;
      }
      if ( changedFeatures.contains( feat.id() ) )
      {
        featInfo.isEdited = true;
      }
    }

    return QVariant::fromValue( featInfo );
  }

  return sourceModel()->data( mapToSource( index ), role );
}

Qt::ItemFlags QgsFeatureListModel::flags( const QModelIndex &index ) const
{
  return sourceModel()->flags( mapToSource( index ) ) & ~Qt::ItemIsEditable;
}

QgsAttributeTableModel* QgsFeatureListModel::masterModel()
{
  return mFilterModel->masterModel();
}

bool QgsFeatureListModel::setDisplayExpression( const QString expression )
{
  const QgsFields fields = mFilterModel->layer()->dataProvider()->fields();

  QgsExpression* exp = new QgsExpression( expression );

  exp->prepare( fields );

  if ( exp->hasParserError() )
  {
    mParserErrorString = exp->parserErrorString();
    delete exp;
    return false;
  }

  delete mExpression;
  mExpression = exp;

  emit( dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) ) );
  return true;
}

QString QgsFeatureListModel::parserErrorString()
{
  return mParserErrorString;
}

const QString& QgsFeatureListModel::displayExpression() const
{
  return mExpression->expression();
}

bool QgsFeatureListModel::featureByIndex( const QModelIndex &index, QgsFeature &feat )
{
  return mFilterModel->layerCache()->featureAtId( idxToFid( index ), feat );
}

void QgsFeatureListModel::onBeginRemoveRows( const QModelIndex& parent, int first, int last )
{
  beginRemoveRows( parent, first, last );
}

void QgsFeatureListModel::onEndRemoveRows( const QModelIndex& parent, int first, int last )
{
  Q_UNUSED( parent )
  Q_UNUSED( first )
  Q_UNUSED( last )
  endRemoveRows();
}

void QgsFeatureListModel::onBeginInsertRows( const QModelIndex& parent, int first, int last )
{
  beginInsertRows( parent, first, last );
}

void QgsFeatureListModel::onEndInsertRows( const QModelIndex& parent, int first, int last )
{
  Q_UNUSED( parent )
  Q_UNUSED( first )
  Q_UNUSED( last )
  endInsertRows();
}

QModelIndex QgsFeatureListModel::mapToMaster( const QModelIndex &proxyIndex ) const
{
  if ( !proxyIndex.isValid() )
    return QModelIndex();

  return mFilterModel->mapToMaster( mFilterModel->index( proxyIndex.row(), proxyIndex.column() ) );
}

QModelIndex QgsFeatureListModel::mapFromMaster( const QModelIndex &sourceIndex ) const
{
  if ( !sourceIndex.isValid() )
    return QModelIndex();

  return createIndex( mFilterModel->mapFromMaster( sourceIndex ).row(), 0 );
}

QItemSelection QgsFeatureListModel::mapSelectionFromMaster( const QItemSelection& selection ) const
{
  return mapSelectionFromSource( mFilterModel->mapSelectionFromSource( selection ) ) ;
}

QItemSelection QgsFeatureListModel::mapSelectionToMaster( const QItemSelection& selection ) const
{
  return mFilterModel->mapSelectionToSource( mapSelectionToSource( selection ) ) ;
}

// Override some methods from QAbstractProxyModel, not that interesting

QModelIndex QgsFeatureListModel::mapToSource( const QModelIndex &proxyIndex ) const
{
  if ( !proxyIndex.isValid() )
    return QModelIndex();

  return sourceModel()->index( proxyIndex.row(), proxyIndex.column() );
}

QModelIndex QgsFeatureListModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
  if ( !sourceIndex.isValid() )
    return QModelIndex();

  return createIndex( sourceIndex.row(), 0 );
}

QModelIndex QgsFeatureListModel::index( int row, int column, const QModelIndex& parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column );
}

QModelIndex QgsFeatureListModel::parent( const QModelIndex& child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsFeatureListModel::columnCount( const QModelIndex&parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

int QgsFeatureListModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent )
  return sourceModel()->rowCount();
}

QModelIndex QgsFeatureListModel::fidToIndex( QgsFeatureId fid )
{
  return mapFromMaster( masterModel()->idToIndex( fid ) );
}

QModelIndexList QgsFeatureListModel::fidToIndexList( QgsFeatureId fid )
{
  return QModelIndexList() << fidToIndex( fid );
}
