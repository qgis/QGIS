/***************************************************************************
  qgscustomlayerorderwidget.cpp
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscustomlayerorderwidget.h"

#include <QCheckBox>
#include <QListView>
#include <QMimeData>
#include <QVBoxLayout>

#include "qgslayertree.h"
#include "qgslayertreemapcanvasbridge.h"

#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"


class CustomLayerOrderModel : public QAbstractListModel
{
  public:
    CustomLayerOrderModel( QgsLayerTreeMapCanvasBridge* bridge, QObject* parent = 0 )
        : QAbstractListModel( parent ), mBridge( bridge )
    {
    }

    int rowCount( const QModelIndex & ) const override
    {
      return mOrder.count();
  }

    QVariant data( const QModelIndex &index, int role ) const override
    {
      QString id = mOrder.at( index.row() );

      if ( role == Qt::DisplayRole )
  {
    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( id );
      if ( layer )
        return layer->name();
    }

    if ( role == Qt::UserRole + 1 )
  {
    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( id );
      if ( layer )
        return layer->id();
    }

    if ( role == Qt::CheckStateRole )
  {
    QgsLayerTreeLayer* nodeLayer = mBridge->rootGroup()->findLayer( id );
      if ( nodeLayer )
        return nodeLayer->isVisible();
    }

    return QVariant();
  }

    bool setData( const QModelIndex &index, const QVariant &value, int role ) override
    {
      if ( role == Qt::CheckStateRole )
      {
        QString id = mOrder.at( index.row() );
        QgsLayerTreeLayer* nodeLayer = mBridge->rootGroup()->findLayer( id );
        if ( nodeLayer )
        {
          nodeLayer->setVisible(( Qt::CheckState )value.toInt() );
          return true;
        }
      }
      return false;
    }

    Qt::ItemFlags flags( const QModelIndex &index ) const override
    {
      if ( !index.isValid() )
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable;
    }

    Qt::DropActions supportedDropActions() const override
      {
        return Qt::MoveAction;
      }

      QStringList mimeTypes() const override
      {
        QStringList types;
        types << "application/qgis.layerorderdata";
        return types;
      }

      QMimeData* mimeData( const QModelIndexList& indexes ) const override
      {
        QStringList lst;
        foreach ( QModelIndex index, indexes )
        lst << data( index, Qt::UserRole + 1 ).toString();

        QMimeData* mimeData = new QMimeData();
        mimeData->setData( "application/qgis.layerorderdata", lst.join( "\n" ).toUtf8() );
        return mimeData;
      }

      bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override
      {
        Q_UNUSED( parent );
        Q_UNUSED( column );

        if ( action == Qt::IgnoreAction )
          return true;

        if ( !data->hasFormat( "application/qgis.layerorderdata" ) )
          return false;

        QByteArray encodedData = data->data( "application/qgis.layerorderdata" );
        QStringList lst = QString::fromUtf8( encodedData ).split( "\n" );

        if ( row < 0 )
          row = mOrder.count();

        beginInsertRows( QModelIndex(), row, row + lst.count() - 1 );
        for ( int i = 0; i < lst.count(); ++i )
          mOrder.insert( row + i, lst[i] );
        endInsertRows();

        return true;
      }

    bool removeRows( int row, int count, const QModelIndex& parent ) override
    {
      Q_UNUSED( parent );
      if ( count <= 0 )
        return false;

      beginRemoveRows( QModelIndex(), row, row + count - 1 );
      while ( --count >= 0 )
        mOrder.removeAt( row );
      endRemoveRows();
      return true;
    }

    void refreshModel( const QStringList& order )
    {
      beginResetModel();
      mOrder = order;
      endResetModel();
    }

    QStringList order() const { return mOrder; }

    void updateLayerVisibility( const QString& layerId )
    {
      int row = mOrder.indexOf( layerId );
      if ( row != -1 )
        emit dataChanged( index( row ), index( row ) );
    }

  protected:
    QgsLayerTreeMapCanvasBridge* mBridge;
    QStringList mOrder;
};


QgsCustomLayerOrderWidget::QgsCustomLayerOrderWidget( QgsLayerTreeMapCanvasBridge* bridge, QWidget* parent )
    : QWidget( parent )
    , mBridge( bridge )
{
  mModel = new CustomLayerOrderModel( bridge, this );

  mView = new QListView( this );
  mView->setDragEnabled( true );
  mView->setAcceptDrops( true );
  mView->setDropIndicatorShown( true );
  mView->setDragDropMode( QAbstractItemView::InternalMove );
  mView->setSelectionMode( QAbstractItemView::ExtendedSelection );

  mView->setModel( mModel );

  mChkOverride = new QCheckBox( tr( "Control rendering order" ) );
  bridgeHasCustomLayerOrderChanged( bridge->hasCustomLayerOrder() );
  connect( mChkOverride, SIGNAL( toggled( bool ) ), bridge, SLOT( setHasCustomLayerOrder( bool ) ) );

  connect( bridge, SIGNAL( hasCustomLayerOrderChanged( bool ) ), this, SLOT( bridgeHasCustomLayerOrderChanged( bool ) ) );
  connect( bridge, SIGNAL( customLayerOrderChanged( QStringList ) ), this, SLOT( bridgeCustomLayerOrderChanged( QStringList ) ) );

  connect( mModel, SIGNAL( rowsInserted( QModelIndex, int, int ) ), this, SLOT( modelUpdated() ) );
  connect( mModel, SIGNAL( rowsRemoved( QModelIndex, int, int ) ), this, SLOT( modelUpdated() ) );

  connect( bridge->rootGroup(), SIGNAL( visibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ), this, SLOT( nodeVisibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ) );

  QVBoxLayout* l = new QVBoxLayout;
  l->setMargin( 0 );
  l->addWidget( mView );
  l->addWidget( mChkOverride );
  setLayout( l );
}

void QgsCustomLayerOrderWidget::bridgeHasCustomLayerOrderChanged( bool state )
{
  mChkOverride->setChecked( state );
  mModel->refreshModel( mBridge->hasCustomLayerOrder() ? mBridge->customLayerOrder() : mBridge->defaultLayerOrder() );
  mView->setEnabled( state );
}

void QgsCustomLayerOrderWidget::bridgeCustomLayerOrderChanged( const QStringList& order )
{
  Q_UNUSED( order );
  mModel->refreshModel( mBridge->hasCustomLayerOrder() ? mBridge->customLayerOrder() : mBridge->defaultLayerOrder() );
}

void QgsCustomLayerOrderWidget::nodeVisibilityChanged( QgsLayerTreeNode* node, Qt::CheckState state )
{
  Q_UNUSED( state );
  if ( QgsLayerTree::isLayer( node ) )
  {
    mModel->updateLayerVisibility( QgsLayerTree::toLayer( node )->layerId() );
  }
}

void QgsCustomLayerOrderWidget::modelUpdated()
{
  mBridge->setCustomLayerOrder( mModel->order() );
}
