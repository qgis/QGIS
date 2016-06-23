/***************************************************************************
    qgsfeaturelistmodel.cpp
    ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsexception.h"
#include "qgsvectordataprovider.h"
#include "qgsfeaturelistmodel.h"
#include "qgsattributetablemodel.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsattributetablefiltermodel.h"

#include <QItemSelection>
#include <QSettings>

QgsFeatureListModel::QgsFeatureListModel( QgsAttributeTableFilterModel *sourceModel, QObject *parent )
    : QAbstractProxyModel( parent )
    , mInjectNull( false )
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
  if ( mInjectNull && index.row() == 0 )
  {
    if ( role == Qt::DisplayRole )
    {
      return QSettings().value( "qgis/nullValue", "NULL" ).toString();
    }
    else if ( role == QgsAttributeTableModel::FeatureIdRole )
    {
      return QVariant( QVariant::Int );
    }
    else
    {
      return QVariant( QVariant::Invalid );
    }
  }

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    QgsFeature feat;

    mFilterModel->layerCache()->featureAtId( idxToFid( index ), feat );

    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope()
    << QgsExpressionContextUtils::layerScope( mFilterModel->layer() );
    context.setFeature( feat );
    return mExpression->evaluate( &context );
  }

  if ( role == FeatureInfoRole )
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
  else if ( role == FeatureRole )
  {
    QgsFeature feat;

    mFilterModel->layerCache()->featureAtId( idxToFid( index ), feat );

    return QVariant::fromValue( feat );
  }
  else if ( role == Qt::TextAlignmentRole )
  {
    return Qt::AlignLeft;
  }

  return sourceModel()->data( mapToSource( index ), role );
}

Qt::ItemFlags QgsFeatureListModel::flags( const QModelIndex &index ) const
{
  if ( mInjectNull && index.row() == 0 )
  {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }
  else
  {
    return sourceModel()->flags( mapToSource( index ) ) & ~Qt::ItemIsEditable;
  }
}

void QgsFeatureListModel::setInjectNull( bool injectNull )
{
  if ( mInjectNull != injectNull )
  {
    emit beginResetModel();
    mInjectNull = injectNull;
    emit endResetModel();
  }
}

bool QgsFeatureListModel::injectNull()
{
  return mInjectNull;
}

QgsAttributeTableModel* QgsFeatureListModel::masterModel()
{
  return mFilterModel->masterModel();
}

bool QgsFeatureListModel::setDisplayExpression( const QString& expression )
{
  QgsExpression* exp = new QgsExpression( expression );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mFilterModel->layer() );

  exp->prepare( &context );

  if ( exp->hasParserError() )
  {
    mParserErrorString = exp->parserErrorString();
    delete exp;
    return false;
  }

  delete mExpression;
  mExpression = exp;

  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) );
  return true;
}

QString QgsFeatureListModel::parserErrorString()
{
  return mParserErrorString;
}

QString QgsFeatureListModel::displayExpression() const
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

  int offset = mInjectNull ? 1 : 0;

  return mFilterModel->mapToMaster( mFilterModel->index( proxyIndex.row() - offset, proxyIndex.column() ) );
}

QModelIndex QgsFeatureListModel::mapFromMaster( const QModelIndex &sourceIndex ) const
{
  if ( !sourceIndex.isValid() )
    return QModelIndex();

  int offset = mInjectNull ? 1 : 0;

  return createIndex( mFilterModel->mapFromMaster( sourceIndex ).row() + offset, 0 );
}

QItemSelection QgsFeatureListModel::mapSelectionFromMaster( const QItemSelection& selection ) const
{
  return mapSelectionFromSource( mFilterModel->mapSelectionFromSource( selection ) );
}

QItemSelection QgsFeatureListModel::mapSelectionToMaster( const QItemSelection& selection ) const
{
  return mFilterModel->mapSelectionToSource( mapSelectionToSource( selection ) );
}

// Override some methods from QAbstractProxyModel, not that interesting

QModelIndex QgsFeatureListModel::mapToSource( const QModelIndex &proxyIndex ) const
{
  if ( !proxyIndex.isValid() )
    return QModelIndex();

  int offset = mInjectNull ? 1 : 0;

  return sourceModel()->index( proxyIndex.row() - offset, proxyIndex.column() );
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

  int offset = mInjectNull ? 1 : 0;

  return sourceModel()->rowCount() + offset;
}

QModelIndex QgsFeatureListModel::fidToIndex( QgsFeatureId fid )
{
  return mapFromMaster( masterModel()->idToIndex( fid ) );
}

QModelIndexList QgsFeatureListModel::fidToIndexList( QgsFeatureId fid )
{
  return QModelIndexList() << fidToIndex( fid );
}
