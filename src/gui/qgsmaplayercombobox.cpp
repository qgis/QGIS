/***************************************************************************
   qgsmaplayercombobox.cpp
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsmaplayercombobox.h"
#include "qgsmaplayermodel.h"
#include "qgsmimedatautils.h"
#include <QDragEnterEvent>
#include <QPainter>

QgsMapLayerComboBox::QgsMapLayerComboBox( QWidget *parent )
  : QComboBox( parent )
{
  mProxyModel = new QgsMapLayerProxyModel( this );
  setModel( mProxyModel );

  connect( this, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::activated ), this, &QgsMapLayerComboBox::indexChanged );
  connect( mProxyModel, &QAbstractItemModel::rowsInserted, this, &QgsMapLayerComboBox::rowsChanged );
  connect( mProxyModel, &QAbstractItemModel::rowsRemoved, this, &QgsMapLayerComboBox::rowsChanged );

  setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLengthWithIcon );

  setAcceptDrops( true );
}

void QgsMapLayerComboBox::setExcludedProviders( const QStringList &providers )
{
  mProxyModel->setExcludedProviders( providers );
}

void  QgsMapLayerComboBox::setProject( QgsProject *project )
{
  mProxyModel->setProject( project );
}


QStringList QgsMapLayerComboBox::excludedProviders() const
{
  return mProxyModel->excludedProviders();
}

void QgsMapLayerComboBox::setAllowEmptyLayer( bool allowEmpty, const QString &text, const QIcon &icon )
{
  mProxyModel->sourceLayerModel()->setAllowEmptyLayer( allowEmpty, text, icon );
}

bool QgsMapLayerComboBox::allowEmptyLayer() const
{
  return mProxyModel->sourceLayerModel()->allowEmptyLayer();
}

void QgsMapLayerComboBox::setShowCrs( bool showCrs )
{
  mProxyModel->sourceLayerModel()->setShowCrs( showCrs );
}

bool QgsMapLayerComboBox::showCrs() const
{
  return mProxyModel->sourceLayerModel()->showCrs();
}

void QgsMapLayerComboBox::setAdditionalItems( const QStringList &items )
{
  mProxyModel->sourceLayerModel()->setAdditionalItems( items );
}

QStringList QgsMapLayerComboBox::additionalItems() const
{
  return mProxyModel->sourceLayerModel()->additionalItems();
}

void QgsMapLayerComboBox::setAdditionalLayers( const QList<QgsMapLayer *> &layers )
{
  mProxyModel->sourceLayerModel()->setAdditionalLayers( layers );
}

QList<QgsMapLayer *> QgsMapLayerComboBox::additionalLayers() const
{
  return mProxyModel->sourceLayerModel()->additionalLayers();
}

void QgsMapLayerComboBox::setLayer( QgsMapLayer *layer )
{
  if ( layer == currentLayer() && ( layer || !isEditable() || currentText().isEmpty() ) )
    return;

  if ( !layer )
  {
    setCurrentIndex( -1 );
    emit layerChanged( nullptr );
    return;
  }

  const QModelIndex idx = mProxyModel->sourceLayerModel()->indexFromLayer( layer );
  if ( idx.isValid() )
  {
    const QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
    if ( proxyIdx.isValid() )
    {
      setCurrentIndex( proxyIdx.row() );
      emit layerChanged( currentLayer() );
      return;
    }
  }
  setCurrentIndex( -1 );
  emit layerChanged( currentLayer() );
}

QgsMapLayer *QgsMapLayerComboBox::currentLayer() const
{
  return layer( currentIndex() );
}

QgsMapLayer *QgsMapLayerComboBox::layer( int layerIndex ) const
{
  const QModelIndex proxyIndex = mProxyModel->index( layerIndex, 0 );
  if ( !proxyIndex.isValid() )
  {
    return nullptr;
  }

  const QModelIndex index = mProxyModel->mapToSource( proxyIndex );
  if ( !index.isValid() )
  {
    return nullptr;
  }

  QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
  if ( layer )
  {
    return layer;
  }
  return nullptr;
}

void QgsMapLayerComboBox::indexChanged( int i )
{
  Q_UNUSED( i )
  QgsMapLayer *layer = currentLayer();
  emit layerChanged( layer );
}

void QgsMapLayerComboBox::rowsChanged()
{
  if ( count() == 1 )
  {
    //currently selected layer item has changed
    emit layerChanged( currentLayer() );
  }
  else if ( count() == 0 )
  {
    emit layerChanged( nullptr );
  }
}

QgsMapLayer *QgsMapLayerComboBox::compatibleMapLayerFromMimeData( const QMimeData *data ) const
{
  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    // is this uri from the current project?
    if ( QgsMapLayer *layer = u.mapLayer() )
    {
      if ( mProxyModel->acceptsLayer( layer ) )
        return layer;
    }
  }
  return nullptr;
}

void QgsMapLayerComboBox::dragEnterEvent( QDragEnterEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
  {
    event->ignore();
    return;
  }

  if ( compatibleMapLayerFromMimeData( event->mimeData() ) )
  {
    // dragged an acceptable layer, phew
    event->setDropAction( Qt::CopyAction );
    event->accept();
    mDragActive = true;
    update();
  }
  else
  {
    event->ignore();
  }
}

void QgsMapLayerComboBox::dragLeaveEvent( QDragLeaveEvent *event )
{
  if ( mDragActive )
  {
    event->accept();
    mDragActive = false;
    update();
  }
  else
  {
    event->ignore();
  }
}

void QgsMapLayerComboBox::dropEvent( QDropEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
  {
    event->ignore();
    return;
  }

  if ( QgsMapLayer *layer = compatibleMapLayerFromMimeData( event->mimeData() ) )
  {
    // dropped an acceptable layer, phew
    setFocus( Qt::MouseFocusReason );
    event->setDropAction( Qt::CopyAction );
    event->accept();

    setLayer( layer );
  }
  else
  {
    event->ignore();
  }
  mDragActive = false;
  update();
}

void QgsMapLayerComboBox::paintEvent( QPaintEvent *e )
{
  QComboBox::paintEvent( e );
  if ( mDragActive || mHighlight )
  {
    QPainter p( this );
    const int width = 2;  // width of highlight rectangle inside frame
    p.setPen( QPen( palette().highlight(), width ) );
    const QRect r = rect().adjusted( width, width, -width, -width );
    p.drawRect( r );
  }
}
