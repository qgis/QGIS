/***************************************************************************
  qgslayertreeembeddedconfigwidget.cpp
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeembeddedconfigwidget.h"

#include "qgsmaplayer.h"
#include "qgslayertreeembeddedwidgetregistry.h"

#include <QStringListModel>
#include <QStandardItemModel>

QgsLayerTreeEmbeddedConfigWidget::QgsLayerTreeEmbeddedConfigWidget( QWidget* parent )
    : QWidget( parent )
    , mLayer( nullptr )
{
  setupUi( this );
}

void QgsLayerTreeEmbeddedConfigWidget::setLayer( QgsMapLayer* layer )
{
  mLayer = layer;

  connect( mBtnAdd, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
  connect( mBtnRemove, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );

  QStandardItemModel* modelAvailable = new QStandardItemModel( this );
  QStandardItemModel* modelUsed = new QStandardItemModel( this );

  // populate available
  Q_FOREACH ( const QString& providerId, QgsLayerTreeEmbeddedWidgetRegistry::instance()->providers() )
  {
    QgsLayerTreeEmbeddedWidgetProvider* provider = QgsLayerTreeEmbeddedWidgetRegistry::instance()->provider( providerId );
    QStandardItem* item = new QStandardItem( provider->name() );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    item->setData( provider->id(), Qt::UserRole + 1 );
    modelAvailable->appendRow( item );
  }
  mListAvailable->setModel( modelAvailable );

  // populate used
  int widgetsCount = layer->customProperty( "embeddedWidgets/count", 0 ).toInt();
  for ( int i = 0; i < widgetsCount; ++i )
  {
    QString providerId = layer->customProperty( QString( "embeddedWidgets/%1/id" ).arg( i ) ).toString();
    if ( QgsLayerTreeEmbeddedWidgetProvider* provider = QgsLayerTreeEmbeddedWidgetRegistry::instance()->provider( providerId ) )
    {
      QStandardItem* item = new QStandardItem( provider->name() );
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      item->setData( provider->id(), Qt::UserRole + 1 );
      modelUsed->appendRow( item );
    }
  }
  mListUsed->setModel( modelUsed );
}

void QgsLayerTreeEmbeddedConfigWidget::onAddClicked()
{
  if ( !mListAvailable->currentIndex().isValid() )
    return;

  QString providerId = mListAvailable->model()->data( mListAvailable->currentIndex(), Qt::UserRole + 1 ).toString();
  QgsLayerTreeEmbeddedWidgetProvider* provider = QgsLayerTreeEmbeddedWidgetRegistry::instance()->provider( providerId );
  if ( !provider )
    return;

  if ( QStandardItemModel* model = qobject_cast<QStandardItemModel*>( mListUsed->model() ) )
  {
    QStandardItem* item = new QStandardItem( provider->name() );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    item->setData( provider->id(), Qt::UserRole + 1 );
    model->appendRow( item );
  }
}

void QgsLayerTreeEmbeddedConfigWidget::onRemoveClicked()
{
  if ( !mListUsed->currentIndex().isValid() )
    return;

  int row = mListUsed->currentIndex().row();
  mListUsed->model()->removeRow( row );
}

void QgsLayerTreeEmbeddedConfigWidget::applyToLayer()
{
  if ( !mLayer )
    return;

  // clear old properties
  int widgetsCount = mLayer->customProperty( "embeddedWidgets/count", 0 ).toInt();
  for ( int i = 0; i < widgetsCount; ++i )
  {
    mLayer->removeCustomProperty( QString( "embeddedWidgets/%1/id" ).arg( i ) );
  }

  // setup new properties
  int newCount = mListUsed->model()->rowCount();
  mLayer->setCustomProperty( "embeddedWidgets/count", newCount );
  for ( int i = 0; i < newCount; ++i )
  {
    QString providerId = mListUsed->model()->data( mListUsed->model()->index( i, 0 ), Qt::UserRole + 1 ).toString();
    mLayer->setCustomProperty( QString( "embeddedWidgets/%1/id" ).arg( i ), providerId );
  }
}
